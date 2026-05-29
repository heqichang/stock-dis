#pragma once

#include "core/common.h"
#include <vector>
#include <optional>

class DeviceRepository {
public:
    static DeviceRepository& Instance();

    bool Add(Device& device);
    bool Update(const Device& device);
    bool Delete(int64_t id);
    std::optional<Device> GetById(int64_t id);
    std::optional<Device> GetByCode(const std::wstring& code);
    PagedResult Query(const DeviceQuery& query);
    bool ExistsByCode(const std::wstring& code);
    bool UpdateStatus(int64_t id, DeviceStatus status);
    std::vector<Device> GetAll();
    bool Import(const std::vector<Device>& devices);

    int64_t GetNextCodeSequence();
    std::vector<ParamField> GetAllParamFields();
    std::optional<ParamField> GetParamFieldById(int64_t id);
    std::optional<ParamField> GetParamFieldByKey(const std::wstring& key);
    bool AddParamField(ParamField& field);
    bool UpdateParamField(const ParamField& field);
    bool DeleteParamField(int64_t id);
    bool MoveParamField(int64_t id, bool moveUp);

private:
    DeviceRepository();
    ~DeviceRepository();
    DeviceRepository(const DeviceRepository&) = delete;
    DeviceRepository& operator=(const DeviceRepository&) = delete;
};
