#include "DirectoryMonitor.hpp"
#include "core/Logger.hpp"
#include "core/Event.hpp"
#include "core/StringUtil.hpp"

namespace {
	bool isRegularFile(const std::wstring& path) {
		DWORD attrs = GetFileAttributesW(path.c_str());
		if (attrs == INVALID_FILE_ATTRIBUTES) {
			// 파일이 없거나 접근 불가
			return false;
		}
		// 폴더 비트가 꺼져 있으면 일반 파일
		return !(attrs & FILE_ATTRIBUTE_DIRECTORY);
	}
}

DirectoryMonitor::~DirectoryMonitor() {
	if (m_hDir != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hDir);
		m_hDir = INVALID_HANDLE_VALUE;
	}
	if (m_hChangeEvent != NULL) {
		CloseHandle(m_hChangeEvent);
		m_hChangeEvent = NULL;
	}
	if (m_hStopEvent != NULL) {
		CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}
}

bool DirectoryMonitor::start(const std::wstring& watchPath, EventBus& bus) {
	m_hDir = CreateFileW(
		watchPath.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		NULL
	);
	if (m_hDir == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		Logger::error("CreateFileW failed, error code: " + std::to_string(err));
		return false;
	}

	m_hChangeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


	OVERLAPPED ov;
	BYTE buffer[4096];

	while (m_running) {

		ov = {};
		//ov.hEvent = hEvent;
		ov.hEvent = m_hChangeEvent;

		BOOL ok = ReadDirectoryChangesW(
			m_hDir,
			buffer,
			sizeof(buffer),
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
			NULL,
			&ov,
			NULL
		);
		if (!ok) {
			Logger::error("ReadDirectoryChangesW failed: " + std::to_string(GetLastError()));
			break;
		}

		Logger::info("Monitoring... (drop a file into watch folder to trigger)");

		HANDLE handles[2] = { m_hChangeEvent, m_hStopEvent };
		DWORD wr = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

		if (wr == WAIT_OBJECT_0 + 0) {
			DWORD bytesReturned = 0;
			if (!GetOverlappedResult(m_hDir, &ov, &bytesReturned, FALSE)) {
				Logger::error("GetOverlappedResult failed: " + std::to_string(GetLastError()));
				break;
			}

			if (bytesReturned == 0) {
				Logger::warn("Empty notification received (buffer overflow?)");
				continue;
			}
			else {
				Logger::info("Change detected! " + std::to_string(bytesReturned) + " bytes");
			}

			BYTE* ptr = buffer;
			while (true) {
				auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(ptr);

				// 파일명 추출
				std::wstring wname(info->FileName, info->FileNameLength / sizeof(WCHAR));

				std::wstring wfullPath = watchPath + L"/" + wname;
				std::string fullPath = StringUtil::wstringToUtf8(wfullPath);

				switch (info->Action) {
				case FILE_ACTION_ADDED:
					Logger::info("\t[ADDED] " + fullPath);
					if (isRegularFile(wfullPath)) {
						bus.publish(makeFileSystemEvent(EventType::FileCreated, fullPath));
					}
					break;
				case FILE_ACTION_REMOVED:
					Logger::info("\t[REMOVED] " + fullPath);
					break;
				case FILE_ACTION_MODIFIED:
					Logger::info("\t[MODIFIED] " + fullPath);
					if (isRegularFile(wfullPath)) {
						bus.publish(makeFileSystemEvent(EventType::FileModified, fullPath));
					}
					break;
				case FILE_ACTION_RENAMED_OLD_NAME:
					Logger::info("\t[RENAMED_OLD] " + fullPath);
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					Logger::info("\t[RENAMED_NEW] " + fullPath);
					if (isRegularFile(wfullPath)) {
						bus.publish(makeFileSystemEvent(EventType::FileCreated, fullPath));
					}
					break;
				default:
					Logger::warn("Unknown action: " + std::to_string(info->Action));
					break;
				}

				if (info->NextEntryOffset == 0) break;
				ptr += info->NextEntryOffset;
			}
		}
		else if (wr == WAIT_OBJECT_0 + 1) {
			CancelIo(m_hDir);
			break;
		}
		else {
			break;
		}
	}
	return true;
}

void DirectoryMonitor::stop() {
	m_running = false;
	SetEvent(m_hStopEvent);
}