#pragma once

#include <sqlite3.h>
#include <string>
#include "../core/Event.hpp"

class Database {
public:
	void init(const std::string& dbPath);
	void shutdown();

	void insertEvent(const Event& e);
private:
	sqlite3* m_db = nullptr;
};
