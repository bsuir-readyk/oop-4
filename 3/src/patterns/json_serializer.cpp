#include "json_serializer.h"
#include "nlohmann/json.hpp"
#include "fleet.h"
#include "car.h"
#include "truck.h"
#include "boat.h"
#include "submarine.h"

using json = nlohmann::json;

// Сериализация одного Vehicle в JSON объект
static json VehicleToJson(const Vehicle& v) {
    json j;
    j["name"] = v.GetName();
    j["max_speed"] = v.GetMaxSpeed();
    j["engine"] = {
        {"horsepower", v.GetEngine().GetHorsepower()},
        {"fuel_type", Engine::FuelTypeToString(v.GetEngine().GetFuelType())}
    };

    // Глубокая сериализация водителя (агрегация → копия данных)
    if (v.GetDriver()) {
        j["driver"] = {
            {"name", v.GetDriver()->GetName()},
            {"experience_years", v.GetDriver()->GetExperienceYears()}
        };
    } else {
        j["driver"] = nullptr;
    }

    // Тип-специфичные поля
    if (auto* car = dynamic_cast<const Car*>(&v)) {
        j["type"] = "Car";
        j["wheel_count"] = car->GetWheelCount();
        j["seat_count"] = car->GetSeatCount();
    } else if (auto* truck = dynamic_cast<const Truck*>(&v)) {
        j["type"] = "Truck";
        j["wheel_count"] = truck->GetWheelCount();
        j["cargo_capacity"] = truck->GetCargoCapacity();
    } else if (auto* boat = dynamic_cast<const Boat*>(&v)) {
        j["type"] = "Boat";
        j["displacement"] = boat->GetDisplacement();
        j["passenger_capacity"] = boat->GetPassengerCapacity();
    } else if (auto* sub = dynamic_cast<const Submarine*>(&v)) {
        j["type"] = "Submarine";
        j["displacement"] = sub->GetDisplacement();
        j["max_depth"] = sub->GetMaxDepth();
    }

    return j;
}

// Десериализация одного Vehicle из JSON объекта
static std::unique_ptr<Vehicle> VehicleFromJson(const json& j) {
    std::string type = j["type"];
    std::string name = j["name"];
    double max_speed = j["max_speed"];
    double engine_hp = j["engine"]["horsepower"];
    Engine::FuelType fuel = Engine::FuelTypeFromString(j["engine"]["fuel_type"]);

    std::unique_ptr<Vehicle> vehicle;

    if (type == "Car") {
        int seat_count = j.value("seat_count", 5);
        vehicle = std::make_unique<Car>(name, max_speed, engine_hp, fuel, seat_count);
    } else if (type == "Truck") {
        double cargo = j.value("cargo_capacity", 10.0);
        vehicle = std::make_unique<Truck>(name, max_speed, engine_hp, fuel, cargo);
    } else if (type == "Boat") {
        double disp = j.value("displacement", 50.0);
        int passengers = j.value("passenger_capacity", 10);
        vehicle = std::make_unique<Boat>(name, max_speed, engine_hp, fuel, disp, passengers);
    } else if (type == "Submarine") {
        double disp = j.value("displacement", 1000.0);
        double depth = j.value("max_depth", 100.0);
        vehicle = std::make_unique<Submarine>(name, max_speed, engine_hp, fuel, disp, depth);
    }

    // Восстановить водителя (глубокая десериализация — новый объект)
    if (vehicle && !j["driver"].is_null()) {
        std::string d_name = j["driver"]["name"];
        int d_exp = j["driver"]["experience_years"];
        auto* driver = new Driver(d_name, d_exp);
        vehicle->AssignDriver(driver);
    }

    return vehicle;
}

std::string JsonSerializer::SerializeFleet(const Fleet& fleet) {
    json arr = json::array();
    for (size_t i = 0; i < fleet.Size(); ++i) {
        arr.push_back(VehicleToJson(fleet[i]));
    }
    return arr.dump(2);  // Pretty-print с отступом 2
}

void JsonSerializer::DeserializeFleet(const std::string& json_str, Fleet& fleet) {
    fleet.Clear();
    json arr = json::parse(json_str);
    for (const auto& j : arr) {
        auto vehicle = VehicleFromJson(j);
        if (vehicle) {
            fleet.Add(std::move(vehicle));
        }
    }
}
