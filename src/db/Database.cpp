#include "Database.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

Database::Database() {}

Database::~Database() {
    Close();
}

Database& Database::Instance() {
    static Database instance;
    return instance;
}

bool Database::Init(const std::wstring& db_path) {
    if (db_) return true;

    std::string db_path_utf8 = WStringToString(db_path);
    
    int rc = sqlite3_open(db_path_utf8.c_str(), &db_);
    if (rc != SQLITE_OK) {
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    return CreateTables();
}

void Database::Close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::CreateTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS devices (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            code TEXT NOT NULL UNIQUE,
            status TEXT NOT NULL CHECK(status IN ('使用中', '报废')),
            params TEXT DEFAULT '{}',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE INDEX IF NOT EXISTS idx_devices_code ON devices(code);
        CREATE INDEX IF NOT EXISTS idx_devices_status ON devices(status);
    )";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        if (err_msg) {
            OutputDebugStringA(err_msg);
            sqlite3_free(err_msg);
        }
        return false;
    }

    return true;
}
