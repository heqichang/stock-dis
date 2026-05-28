#include "DeviceRepository.h"
#include "Database.h"
#include "sqlite3.h"

DeviceRepository::DeviceRepository() {}

DeviceRepository::~DeviceRepository() {}

DeviceRepository& DeviceRepository::Instance() {
    static DeviceRepository instance;
    return instance;
}

bool DeviceRepository::Add(Device& device) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return false;

    if (device.code.empty()) {
        device.code = GenerateUniqueCode();
        while (ExistsByCode(device.code)) {
            device.code = GenerateUniqueCode();
        }
    }

    device.created_at = GetCurrentDateTime();
    device.updated_at = device.created_at;

    const char* sql = "INSERT INTO devices (name, code, status, params, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;

    std::string name_utf8 = WStringToString(device.name);
    std::string code_utf8 = WStringToString(device.code);
    std::string status_utf8 = WStringToString(StatusToString(device.status));
    std::string params_utf8 = WStringToString(device.params);
    std::string created_utf8 = WStringToString(device.created_at);
    std::string updated_utf8 = WStringToString(device.updated_at);

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, name_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, code_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, status_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, params_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, created_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, updated_utf8.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    device.id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return true;
}

bool DeviceRepository::Update(const Device& device) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return false;

    std::wstring updated_at = GetCurrentDateTime();

    const char* sql = "UPDATE devices SET name = ?, params = ?, updated_at = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;

    std::string name_utf8 = WStringToString(device.name);
    std::string params_utf8 = WStringToString(device.params);
    std::string updated_utf8 = WStringToString(updated_at);

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, name_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, params_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, updated_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, device.id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool DeviceRepository::Delete(int64_t id) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return false;

    const char* sql = "DELETE FROM devices WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

std::optional<Device> DeviceRepository::GetById(int64_t id) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return std::nullopt;

    const char* sql = "SELECT id, name, code, status, params, created_at, updated_at FROM devices WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    sqlite3_bind_int64(stmt, 1, id);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    Device device;
    device.id = sqlite3_column_int64(stmt, 0);
    device.name = StringToWString((const char*)sqlite3_column_text(stmt, 1));
    device.code = StringToWString((const char*)sqlite3_column_text(stmt, 2));
    device.status = StringToStatus(StringToWString((const char*)sqlite3_column_text(stmt, 3)));
    device.params = StringToWString((const char*)sqlite3_column_text(stmt, 4));
    device.created_at = StringToWString((const char*)sqlite3_column_text(stmt, 5));
    device.updated_at = StringToWString((const char*)sqlite3_column_text(stmt, 6));

    sqlite3_finalize(stmt);
    return device;
}

std::optional<Device> DeviceRepository::GetByCode(const std::wstring& code) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return std::nullopt;

    const char* sql = "SELECT id, name, code, status, params, created_at, updated_at FROM devices WHERE code = ?";
    sqlite3_stmt* stmt = nullptr;

    std::string code_utf8 = WStringToString(code);

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    sqlite3_bind_text(stmt, 1, code_utf8.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    Device device;
    device.id = sqlite3_column_int64(stmt, 0);
    device.name = StringToWString((const char*)sqlite3_column_text(stmt, 1));
    device.code = StringToWString((const char*)sqlite3_column_text(stmt, 2));
    device.status = StringToStatus(StringToWString((const char*)sqlite3_column_text(stmt, 3)));
    device.params = StringToWString((const char*)sqlite3_column_text(stmt, 4));
    device.created_at = StringToWString((const char*)sqlite3_column_text(stmt, 5));
    device.updated_at = StringToWString((const char*)sqlite3_column_text(stmt, 6));

    sqlite3_finalize(stmt);
    return device;
}

PagedResult DeviceRepository::Query(const DeviceQuery& query) {
    PagedResult result;
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return result;

    std::string sql = "SELECT id, name, code, status, params, created_at, updated_at FROM devices WHERE 1=1";
    std::string count_sql = "SELECT COUNT(*) FROM devices WHERE 1=1";
    std::vector<std::string> params;

    if (!query.keyword.empty()) {
        std::string keyword = "%" + WStringToString(query.keyword) + "%";
        sql += " AND (name LIKE ? OR code LIKE ?)";
        count_sql += " AND (name LIKE ? OR code LIKE ?)";
        params.push_back(keyword);
        params.push_back(keyword);
    }

    if (query.status != (DeviceStatus)-1) {
        std::string status = WStringToString(StatusToString(query.status));
        sql += " AND status = ?";
        count_sql += " AND status = ?";
        params.push_back(status);
    }

    sql += " ORDER BY id DESC LIMIT ? OFFSET ?";
    int offset = (query.page - 1) * query.page_size;

    sqlite3_stmt* count_stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, count_sql.c_str(), -1, &count_stmt, nullptr);
    if (rc != SQLITE_OK) return result;

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(count_stmt, (int)(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    if (sqlite3_step(count_stmt) == SQLITE_ROW) {
        result.total = sqlite3_column_int(count_stmt, 0);
    }
    sqlite3_finalize(count_stmt);

    result.page_count = (result.total + query.page_size - 1) / query.page_size;
    result.page = query.page;
    result.page_size = query.page_size;

    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return result;

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, (int)(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_int(stmt, (int)params.size() + 1, query.page_size);
    sqlite3_bind_int(stmt, (int)params.size() + 2, offset);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Device device;
        device.id = sqlite3_column_int64(stmt, 0);
        device.name = StringToWString((const char*)sqlite3_column_text(stmt, 1));
        device.code = StringToWString((const char*)sqlite3_column_text(stmt, 2));
        device.status = StringToStatus(StringToWString((const char*)sqlite3_column_text(stmt, 3)));
        device.params = StringToWString((const char*)sqlite3_column_text(stmt, 4));
        device.created_at = StringToWString((const char*)sqlite3_column_text(stmt, 5));
        device.updated_at = StringToWString((const char*)sqlite3_column_text(stmt, 6));
        result.items.push_back(device);
    }

    sqlite3_finalize(stmt);
    return result;
}

bool DeviceRepository::ExistsByCode(const std::wstring& code) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return false;

    const char* sql = "SELECT 1 FROM devices WHERE code = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;

    std::string code_utf8 = WStringToString(code);

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, code_utf8.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_ROW;
}

bool DeviceRepository::UpdateStatus(int64_t id, DeviceStatus status) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return false;

    std::wstring updated_at = GetCurrentDateTime();

    const char* sql = "UPDATE devices SET status = ?, updated_at = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;

    std::string status_utf8 = WStringToString(StatusToString(status));
    std::string updated_utf8 = WStringToString(updated_at);

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, status_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, updated_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

std::vector<Device> DeviceRepository::GetAll() {
    std::vector<Device> devices;
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return devices;

    const char* sql = "SELECT id, name, code, status, params, created_at, updated_at FROM devices ORDER BY id";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return devices;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Device device;
        device.id = sqlite3_column_int64(stmt, 0);
        device.name = StringToWString((const char*)sqlite3_column_text(stmt, 1));
        device.code = StringToWString((const char*)sqlite3_column_text(stmt, 2));
        device.status = StringToStatus(StringToWString((const char*)sqlite3_column_text(stmt, 3)));
        device.params = StringToWString((const char*)sqlite3_column_text(stmt, 4));
        device.created_at = StringToWString((const char*)sqlite3_column_text(stmt, 5));
        device.updated_at = StringToWString((const char*)sqlite3_column_text(stmt, 6));
        devices.push_back(device);
    }

    sqlite3_finalize(stmt);
    return devices;
}

bool DeviceRepository::Import(const std::vector<Device>& devices) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return false;

    char* err_msg = nullptr;
    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, &err_msg);

    bool success = true;
    for (const auto& device : devices) {
        Device new_device = device;
        if (!Add(new_device)) {
            success = false;
            break;
        }
    }

    if (success) {
        sqlite3_exec(db, "COMMIT", nullptr, nullptr, &err_msg);
    } else {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, &err_msg);
    }

    return success;
}
