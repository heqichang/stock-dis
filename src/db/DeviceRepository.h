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

private:
    DeviceRepository();
    ~DeviceRepository();
    DeviceRepository(const DeviceRepository&) = delete;
    DeviceRepository& operator=(const DeviceRepository&) = delete;
};
