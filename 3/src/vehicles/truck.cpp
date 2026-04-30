#include "truck.h"
#include <iostream>

Truck::Truck(const std::string& name, double max_speed,
             double engine_hp, Engine::FuelType fuel_type, double cargo_capacity)
    : LandVehicle(name, max_speed, engine_hp, fuel_type, 6), cargo_capacity_(cargo_capacity) {
    std::cout << "[Truck] Создан: " << name_ << "\n";
}

Truck::~Truck() {
    std::cout << "[Truck] Уничтожен: " << name_ << "\n";
}

std::string Truck::GetType() const { return "Грузовик"; }
std::string Truck::Move() const { return name_ + " перевозит груз"; }

std::string Truck::GetInfo() const {
    return LandVehicle::GetInfo() +
           "\n  Грузоподъёмность: " + std::to_string(static_cast<int>(cargo_capacity_)) + " т";
}

double Truck::GetCargoCapacity() const { return cargo_capacity_; }
void Truck::SetCargoCapacity(double capacity) { cargo_capacity_ = capacity; }

std::string Truck::SerializeCustom() const {
    std::string result;
    result += "[Truck]\n";
    result += "name=" + name_ + "\n";
    result += "max_speed=" + std::to_string(max_speed_) + "\n";
    result += GetEngine().SerializeCustom();
    result += "wheel_count=" + std::to_string(GetWheelCount()) + "\n";
    result += "cargo_capacity=" + std::to_string(cargo_capacity_) + "\n";
    if (GetDriver()) {
        result += GetDriver()->SerializeCustom();
    }
    return result;
}

void Truck::DeserializeCustom(const std::string& data) {
    auto kv = ParseKeyValue(data);
    if (kv.count("name"))           name_ = kv["name"];
    if (kv.count("max_speed"))      max_speed_ = std::stod(kv["max_speed"]);
    if (kv.count("wheel_count"))    wheel_count_ = std::stoi(kv["wheel_count"]);
    if (kv.count("cargo_capacity")) cargo_capacity_ = std::stod(kv["cargo_capacity"]);
    GetEngine().DeserializeCustom(data);
}
