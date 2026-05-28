#include "vehicle_type_registry.h"

#include "boat.h"
#include "car.h"
#include "exceptions.h"
#include "submarine.h"
#include "truck.h"

#include <QJsonValue>

#include <algorithm>

namespace {

int RequireInt(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (!value.isDouble()) {
        throw SerializationException("missing integer payload field: " + key.toStdString());
    }
    return value.toInt();
}

double RequireDouble(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (!value.isDouble()) {
        throw SerializationException("missing numeric payload field: " + key.toStdString());
    }
    return value.toDouble();
}

VehicleTypeRegistration MakeCarRegistration() {
    VehicleTypeRegistration registration;
    registration.type_id = "builtin.car";
    registration.display_name = "Легковой (Car)";
    registration.factory_name = "CarFactory";
    registration.card_color = QColor("#4CAF50");
    registration.owner_plugin_id = VehicleTypeRegistry::kBuiltinOwner;
    registration.create_default = [](const std::string& name) {
        return std::make_unique<Car>(name, 200, 150, Engine::FuelType::kPetrol, 5);
    };
    registration.serialize_payload = [](const Vehicle& vehicle) {
        const auto* car = dynamic_cast<const Car*>(&vehicle);
        if (!car) {
            throw SerializationException("vehicle is not Car");
        }
        QJsonObject payload;
        payload["seat_count"] = car->GetSeatCount();
        return payload;
    };
    registration.deserialize = [](const VehicleSerializationData& data) {
        return std::make_unique<Car>(data.name, data.max_speed,
                                     data.engine_horsepower, data.fuel_type,
                                     RequireInt(data.payload, "seat_count"));
    };
    return registration;
}

VehicleTypeRegistration MakeTruckRegistration() {
    VehicleTypeRegistration registration;
    registration.type_id = "builtin.truck";
    registration.display_name = "Грузовик (Truck)";
    registration.factory_name = "TruckFactory";
    registration.card_color = QColor("#FF9800");
    registration.owner_plugin_id = VehicleTypeRegistry::kBuiltinOwner;
    registration.create_default = [](const std::string& name) {
        return std::make_unique<Truck>(name, 110, 400, Engine::FuelType::kDiesel, 20);
    };
    registration.serialize_payload = [](const Vehicle& vehicle) {
        const auto* truck = dynamic_cast<const Truck*>(&vehicle);
        if (!truck) {
            throw SerializationException("vehicle is not Truck");
        }
        QJsonObject payload;
        payload["cargo_capacity"] = truck->GetCargoCapacity();
        return payload;
    };
    registration.deserialize = [](const VehicleSerializationData& data) {
        return std::make_unique<Truck>(data.name, data.max_speed,
                                       data.engine_horsepower, data.fuel_type,
                                       RequireDouble(data.payload, "cargo_capacity"));
    };
    return registration;
}

VehicleTypeRegistration MakeBoatRegistration() {
    VehicleTypeRegistration registration;
    registration.type_id = "builtin.boat";
    registration.display_name = "Катер (Boat)";
    registration.factory_name = "BoatFactory";
    registration.card_color = QColor("#2196F3");
    registration.owner_plugin_id = VehicleTypeRegistry::kBuiltinOwner;
    registration.create_default = [](const std::string& name) {
        return std::make_unique<Boat>(name, 70, 250, Engine::FuelType::kPetrol, 40, 8);
    };
    registration.serialize_payload = [](const Vehicle& vehicle) {
        const auto* boat = dynamic_cast<const Boat*>(&vehicle);
        if (!boat) {
            throw SerializationException("vehicle is not Boat");
        }
        QJsonObject payload;
        payload["displacement"] = boat->GetDisplacement();
        payload["passenger_capacity"] = boat->GetPassengerCapacity();
        return payload;
    };
    registration.deserialize = [](const VehicleSerializationData& data) {
        return std::make_unique<Boat>(data.name, data.max_speed,
                                      data.engine_horsepower, data.fuel_type,
                                      RequireDouble(data.payload, "displacement"),
                                      RequireInt(data.payload, "passenger_capacity"));
    };
    return registration;
}

VehicleTypeRegistration MakeSubmarineRegistration() {
    VehicleTypeRegistration registration;
    registration.type_id = "builtin.submarine";
    registration.display_name = "Подводная лодка (Submarine)";
    registration.factory_name = "SubmarineFactory";
    registration.card_color = QColor("#9C27B0");
    registration.owner_plugin_id = VehicleTypeRegistry::kBuiltinOwner;
    registration.create_default = [](const std::string& name) {
        return std::make_unique<Submarine>(name, 45, 800,
                                           Engine::FuelType::kNuclear, 5000, 400);
    };
    registration.serialize_payload = [](const Vehicle& vehicle) {
        const auto* submarine = dynamic_cast<const Submarine*>(&vehicle);
        if (!submarine) {
            throw SerializationException("vehicle is not Submarine");
        }
        QJsonObject payload;
        payload["displacement"] = submarine->GetDisplacement();
        payload["max_depth"] = submarine->GetMaxDepth();
        return payload;
    };
    registration.deserialize = [](const VehicleSerializationData& data) {
        return std::make_unique<Submarine>(data.name, data.max_speed,
                                           data.engine_horsepower, data.fuel_type,
                                           RequireDouble(data.payload, "displacement"),
                                           RequireDouble(data.payload, "max_depth"));
    };
    return registration;
}

} // namespace

VehicleTypeRegistry& VehicleTypeRegistry::Instance() {
    static VehicleTypeRegistry registry;
    return registry;
}

VehicleTypeRegistry::VehicleTypeRegistry() {
    RegisterBuiltinTypes();
}

bool VehicleTypeRegistry::RegisterType(const VehicleTypeRegistration& registration,
                                       QString* error) {
    if (registration.type_id.empty()) {
        if (error) *error = "Пустой type_id транспорта";
        return false;
    }
    if (registration.display_name.empty()) {
        if (error) *error = "Пустое отображаемое имя транспорта";
        return false;
    }
    if (!registration.create_default) {
        if (error) *error = "Не задана фабрика создания транспорта";
        return false;
    }
    if (!registration.serialize_payload || !registration.deserialize) {
        if (error) *error = "Не задан контракт сериализации транспорта";
        return false;
    }

    const auto duplicate = std::find_if(types_.begin(), types_.end(),
        [&](const VehicleTypeRegistration& existing) {
            return existing.type_id == registration.type_id;
        });
    if (duplicate != types_.end()) {
        if (error) {
            *error = "Тип транспорта уже зарегистрирован: " +
                     QString::fromStdString(registration.type_id);
        }
        return false;
    }

    types_.push_back(registration);
    return true;
}

void VehicleTypeRegistry::UnregisterOwner(const std::string& owner_plugin_id) {
    if (owner_plugin_id == kBuiltinOwner) {
        return;
    }
    types_.erase(std::remove_if(types_.begin(), types_.end(),
        [&](const VehicleTypeRegistration& registration) {
            return registration.owner_plugin_id == owner_plugin_id;
        }), types_.end());
}

void VehicleTypeRegistry::SetOwnerEnabled(const std::string& owner_plugin_id,
                                          bool enabled) {
    for (auto& registration : types_) {
        if (registration.owner_plugin_id == owner_plugin_id) {
            registration.enabled = enabled;
        }
    }
}

std::vector<VehicleTypeRegistration> VehicleTypeRegistry::AvailableTypes() const {
    std::vector<VehicleTypeRegistration> result;
    for (const auto& registration : types_) {
        if (registration.enabled) {
            result.push_back(registration);
        }
    }
    return result;
}

std::vector<VehicleTypeRegistration> VehicleTypeRegistry::AllTypes() const {
    return types_;
}

const VehicleTypeRegistration* VehicleTypeRegistry::Find(const std::string& type_id) const {
    const auto it = std::find_if(types_.begin(), types_.end(),
        [&](const VehicleTypeRegistration& registration) {
            return registration.type_id == type_id;
        });
    return it == types_.end() ? nullptr : &(*it);
}

const VehicleTypeRegistration* VehicleTypeRegistry::FindEnabled(
    const std::string& type_id) const {
    const auto* registration = Find(type_id);
    if (!registration || !registration->enabled) {
        return nullptr;
    }
    return registration;
}

std::string VehicleTypeRegistry::OwnerForType(const std::string& type_id) const {
    const auto* registration = Find(type_id);
    return registration ? registration->owner_plugin_id : std::string();
}

void VehicleTypeRegistry::RegisterBuiltinTypes() {
    RegisterType(MakeCarRegistration());
    RegisterType(MakeTruckRegistration());
    RegisterType(MakeBoatRegistration());
    RegisterType(MakeSubmarineRegistration());
}
