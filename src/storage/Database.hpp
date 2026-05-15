#pragma once

#include <sqlite3.h>
#include <string>
#include "../core/Event.hpp"

class Database {
public:
	Database(const std::string& dbPath);
	~Database();

	void insertEvent(const Event& e);
private:
	sqlite3* m_db = nullptr;
};
