#include "Database.hpp"
#include "../core/Logger.hpp"

void Database::init(const std::string& dbPath) {
	int rc = sqlite3_open(dbPath.c_str(), &m_db);
	if (rc != SQLITE_OK) {
		Logger::error(sqlite3_errmsg(m_db));
		return;
	}

	const char* sql = R"(
		CREATE TABLE IF NOT EXISTS events(
			id INTEGER PRIMARY KEY,
			timestamp TEXT NOT NULL,
			type TEXT NOT NULL,
			source TEXT NOT NULL,
			severity TEXT NOT NULL,
			message TEXT NOT NULL,
			file_path TEXT,
			rule_name TEXT,
			quarantine_path TEXT
		)
	)";

	rc = sqlite3_exec(m_db, sql, nullptr, nullptr, nullptr);

	if (rc != SQLITE_OK) {
		Logger::error(sqlite3_errmsg(m_db));
		sqlite3_close(m_db);     // 열어둔 DB 닫기
		m_db = nullptr;
		return;
	}

	Logger::info("Database initialized");
}

void Database::shutdown() {
	if (m_db) {
		sqlite3_close(m_db);
		m_db = nullptr;
	}
}

void Database::insertEvent(const Event& e) {
	sqlite3_stmt* stmt = nullptr;

	const char* sql = R"(
		INSERT INTO events
			(timestamp, type, source, severity, message, file_path, rule_name, quarantine_path)
		VALUES
			(?, ?, ?, ?, ?, ?, ?, ?)
	)";

	int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		Logger::error(sqlite3_errmsg(m_db));
		return;
	}

	sqlite3_bind_text(stmt, 1, timePointToString(e.timestamp).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, eventTypeToString(e.type).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, e.source.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 4, severityToString(e.severity).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 5, e.message.c_str(), -1, SQLITE_TRANSIENT);
	if (e.file_path.has_value()) {
		sqlite3_bind_text(stmt, 6, e.file_path.value().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 6);
	}
	if (e.rule_name.has_value()) {
		sqlite3_bind_text(stmt, 7, e.rule_name.value().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 7);
	}
	if (e.quarantine_path.has_value()) {
		sqlite3_bind_text(stmt, 8, e.quarantine_path.value().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 8);
	}

	rc = sqlite3_step(stmt);

	if (rc != SQLITE_DONE) {
		Logger::error(sqlite3_errmsg(m_db));
		// return 하면 안됨 (stmt 초기화 안됨)
	}

	sqlite3_finalize(stmt);
}