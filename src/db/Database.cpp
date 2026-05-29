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

        CREATE TABLE IF NOT EXISTS code_sequence (
            id INTEGER PRIMARY KEY CHECK(id = 1),
            current_value INTEGER NOT NULL DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS param_fields (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            field_key TEXT NOT NULL UNIQUE,
            field_label TEXT NOT NULL,
            field_type TEXT NOT NULL CHECK(field_type IN ('text', 'number', 'date', 'select')),
            field_options TEXT DEFAULT '',
            sort_order INTEGER DEFAULT 0,
            is_required INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE INDEX IF NOT EXISTS idx_param_fields_key ON param_fields(field_key);
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

    const char* init_sql = R"(
        INSERT OR IGNORE INTO code_sequence (id, current_value) VALUES (1, 0);

        INSERT OR IGNORE INTO param_fields (field_key, field_label, field_type, field_options, sort_order, is_required) VALUES
        ('brand', '品牌', 'text', '', 1, 0),
        ('model', '型号', 'text', '', 2, 0),
        ('purchase_date', '购入日期', 'date', '', 3, 0),
        ('price', '价格', 'number', '', 4, 0),
        ('location', '存放位置', 'text', '', 5, 0);
    )";

    rc = sqlite3_exec(db_, init_sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        if (err_msg) {
            OutputDebugStringA(err_msg);
            sqlite3_free(err_msg);
        }
    }

    return true;
}
