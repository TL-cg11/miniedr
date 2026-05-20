#include "DirectoryMonitor.hpp"
#include "core/Logger.hpp"
#include "core/Event.hpp"
#include "core/StringUtil.hpp"
#include <Windows.h>

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

bool DirectoryMonitor::start(const std::wstring& watchPath, EventBus& bus) {
	HANDLE hDir = CreateFileW(
		watchPath.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		NULL
	);
	if (hDir == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		Logger::error("CreateFileW failed, error code: " + std::to_string(err));
		return false;
	}

	HANDLE hEvent = CreateEventW(
		NULL,
		FALSE,
		FALSE,
		NULL
	);
	if (hEvent == NULL) {
		DWORD err = GetLastError();
		Logger::error("CreateEventW failed, error code: " + std::to_string(err));
		CloseHandle(hDir);
		return 1;
	}

	OVERLAPPED ov;
	BYTE buffer[4096];

	while (true) {

		ov = {};
		ov.hEvent = hEvent;

		BOOL ok = ReadDirectoryChangesW(
			hDir,
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
			CloseHandle(hEvent);
			CloseHandle(hDir);
			break;
		}

		Logger::info("Monitoring... (drop a file into watch folder to trigger)");

		DWORD wr = WaitForSingleObject(
			hEvent,
			INFINITE
		);
		if (wr != WAIT_OBJECT_0) {
			Logger::error("WaitForSingleObject failed: " + std::to_string(wr));
			CloseHandle(hEvent);
			CloseHandle(hDir);
			break;
		}

		DWORD bytesReturned = 0;
		if (!GetOverlappedResult(hDir, &ov, &bytesReturned, FALSE)) {
			Logger::error("GetOverlappedResult failed: " + std::to_string(GetLastError()));
			CloseHandle(hEvent);
			CloseHandle(hDir);
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

	CloseHandle(hEvent);
	CloseHandle(hDir);
	return true;
}