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

int64_t DeviceRepository::GetNextCodeSequence() {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return 0;

    char* err_msg = nullptr;
    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, &err_msg);

    const char* select_sql = "SELECT current_value FROM code_sequence WHERE id = 1";
    sqlite3_stmt* stmt = nullptr;
    int64_t current_value = 0;

    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, &err_msg);
        return 0;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        current_value = sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);

    current_value++;

    const char* update_sql = "UPDATE code_sequence SET current_value = ? WHERE id = 1";
    rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, &err_msg);
        return 0;
    }

    sqlite3_bind_int64(stmt, 1, current_value);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_exec(db, "COMMIT", nullptr, nullptr, &err_msg);

    return current_value;
}

std::vector<ParamField> DeviceRepository::GetAllParamFields() {
    std::vector<ParamField> fields;
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return fields;

    const char* sql = "SELECT id, field_key, field_label, field_type, field_options, sort_order, is_required, created_at FROM param_fields ORDER BY sort_order, id";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return fields;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ParamField field;
        field.id = sqlite3_column_int64(stmt, 0);
        field.field_key = StringToWString((const char*)sqlite3_column_text(stmt, 1));
        field.field_label = StringToWString((const char*)sqlite3_column_text(stmt, 2));
        field.field_type = StringToFieldType(StringToWString((const char*)sqlite3_column_text(stmt, 3)));
        field.field_options = StringToWString((const char*)sqlite3_column_text(stmt, 4));
        field.sort_order = sqlite3_column_int(stmt, 5);
        field.is_required = sqlite3_column_int(stmt, 6) != 0;
        field.created_at = StringToWString((const char*)sqlite3_column_text(stmt, 7));
        fields.push_back(field);
    }

    sqlite3_finalize(stmt);
    return fields;
}

std::optional<ParamField> DeviceRepository::GetParamFieldById(int64_t id) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return std::nullopt;

    const char* sql = "SELECT id, field_key, field_label, field_type, field_options, sort_order, is_required, created_at FROM param_fields WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    sqlite3_bind_int64(stmt, 1, id);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    ParamField field;
    field.id = sqlite3_column_int64(stmt, 0);
    field.field_key = StringToWString((const char*)sqlite3_column_text(stmt, 1));
    field.field_label = StringToWString((const char*)sqlite3_column_text(stmt, 2));
    field.field_type = StringToFieldType(StringToWString((const char*)sqlite3_column_text(stmt, 3)));
    field.field_options = StringToWString((const char*)sqlite3_column_text(stmt, 4));
    field.sort_order = sqlite3_column_int(stmt, 5);
    field.is_required = sqlite3_column_int(stmt, 6) != 0;
    field.created_at = StringToWString((const char*)sqlite3_column_text(stmt, 7));

    sqlite3_finalize(stmt);
    return field;
}

std::optional<ParamField> DeviceRepository::GetParamFieldByKey(const std::wstring& key) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return std::nullopt;

    const char* sql = "SELECT id, field_key, field_label, field_type, field_options, sort_order, is_required, created_at FROM param_fields WHERE field_key = ?";
    sqlite3_stmt* stmt = nullptr;

    std::string key_utf8 = WStringToString(key);
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    sqlite3_bind_text(stmt, 1, key_utf8.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    ParamField field;
    field.id = sqlite3_column_int64(stmt, 0);
    field.field_key = StringToWString((const char*)sqlite3_column_text(stmt, 1));
    field.field_label = StringToWString((const char*)sqlite3_column_text(stmt, 2));
    field.field_type = StringToFieldType(StringToWString((const char*)sqlite3_column_text(stmt, 3)));
    field.field_options = StringToWString((const char*)sqlite3_column_text(stmt, 4));
    field.sort_order = sqlite3_column_int(stmt, 5);
    field.is_required = sqlite3_column_int(stmt, 6) != 0;
    field.created_at = StringToWString((const char*)sqlite3_column_text(stmt, 7));

    sqlite3_finalize(stmt);
    return field;
}

bool DeviceRepository::AddParamField(ParamField& field) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return false;

    field.created_at = GetCurrentDateTime();

    const char* sql = "INSERT INTO param_fields (field_key, field_label, field_type, field_options, sort_order, is_required, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;

    std::string key_utf8 = WStringToString(field.field_key);
    std::string label_utf8 = WStringToString(field.field_label);
    std::string type_utf8 = WStringToString(FieldTypeToString(field.field_type));
    std::string options_utf8 = WStringToString(field.field_options);
    std::string created_utf8 = WStringToString(field.created_at);

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, key_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, label_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, type_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, options_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, field.sort_order);
    sqlite3_bind_int(stmt, 6, field.is_required ? 1 : 0);
    sqlite3_bind_text(stmt, 7, created_utf8.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    field.id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return true;
}

bool DeviceRepository::UpdateParamField(const ParamField& field) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return false;

    const char* sql = "UPDATE param_fields SET field_key = ?, field_label = ?, field_type = ?, field_options = ?, sort_order = ?, is_required = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;

    std::string key_utf8 = WStringToString(field.field_key);
    std::string label_utf8 = WStringToString(field.field_label);
    std::string type_utf8 = WStringToString(FieldTypeToString(field.field_type));
    std::string options_utf8 = WStringToString(field.field_options);

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, key_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, label_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, type_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, options_utf8.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, field.sort_order);
    sqlite3_bind_int(stmt, 6, field.is_required ? 1 : 0);
    sqlite3_bind_int64(stmt, 7, field.id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool DeviceRepository::DeleteParamField(int64_t id) {
    sqlite3* db = Database::Instance().GetHandle();
    if (!db) return false;

    const char* sql = "DELETE FROM param_fields WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_int64(stmt, 1, id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool DeviceRepository::MoveParamField(int64_t id, bool moveUp) {
    auto current = GetParamFieldById(id);
    if (!current) return false;

    auto fields = GetAllParamFields();
    if (fields.size() < 2) return false;

    int currentIndex = -1;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (fields[i].id == id) {
            currentIndex = (int)i;
            break;
        }
    }

    if (currentIndex == -1) return false;
    if (moveUp && currentIndex == 0) return false;
    if (!moveUp && currentIndex == (int)fields.size() - 1) return false;

    int swapIndex = moveUp ? currentIndex - 1 : currentIndex + 1;
    ParamField& currentField = fields[currentIndex];
    ParamField& swapField = fields[swapIndex];

    int tempOrder = currentField.sort_order;
    currentField.sort_order = swapField.sort_order;
    swapField.sort_order = tempOrder;

    return UpdateParamField(currentField) && UpdateParamField(swapField);
}

int64_t GetNextCodeSequenceFromRepo() {
    return DeviceRepository::Instance().GetNextCodeSequence();
}
