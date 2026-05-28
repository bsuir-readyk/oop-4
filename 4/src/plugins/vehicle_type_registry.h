#ifndef VEHICLE_TYPE_REGISTRY_H
#define VEHICLE_TYPE_REGISTRY_H

#include "vehicle_plugin_api.h"

#include <QString>

#include <string>
#include <vector>

class VehicleTypeRegistry {
public:
    static constexpr const char* kBuiltinOwner = "builtin";

    static VehicleTypeRegistry& Instance();

    bool RegisterType(const VehicleTypeRegistration& registration, QString* error = nullptr);
    void UnregisterOwner(const std::string& owner_plugin_id);
    void SetOwnerEnabled(const std::string& owner_plugin_id, bool enabled);

    std::vector<VehicleTypeRegistration> AvailableTypes() const;
    std::vector<VehicleTypeRegistration> AllTypes() const;
    const VehicleTypeRegistration* Find(const std::string& type_id) const;
    const VehicleTypeRegistration* FindEnabled(const std::string& type_id) const;
    std::string OwnerForType(const std::string& type_id) const;

private:
    VehicleTypeRegistry();
    void RegisterBuiltinTypes();

    std::vector<VehicleTypeRegistration> types_;
};

#endif // VEHICLE_TYPE_REGISTRY_H
