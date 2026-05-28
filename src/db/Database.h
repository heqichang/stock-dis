#pragma once

#include "core/common.h"
#include "sqlite3.h"
#include <string>
#include <vector>
#include <functional>

class Database {
public:
    static Database& Instance();
    bool Init(const std::wstring& db_path);
    void Close();
    sqlite3* GetHandle() { return db_; }

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool CreateTables();
    sqlite3* db_ = nullptr;
};
