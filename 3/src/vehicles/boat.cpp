#include "boat.h"
#include <iostream>

Boat::Boat(const std::string& name, double max_speed,
           double engine_hp, Engine::FuelType fuel_type,
           double displacement, int passenger_capacity)
    : WaterVehicle(name, max_speed, engine_hp, fuel_type, displacement),
      passenger_capacity_(passenger_capacity) {
    std::cout << "[Boat] Создан: " << name_ << "\n";
}

Boat::~Boat() {
    std::cout << "[Boat] Уничтожен: " << name_ << "\n";
}

std::string Boat::GetType() const { return "Катер"; }
std::string Boat::Move() const { return name_ + " плывёт по воде"; }

std::string Boat::GetInfo() const {
    return WaterVehicle::GetInfo() +
           "\n  Пассажиров: " + std::to_string(passenger_capacity_);
}

int Boat::GetPassengerCapacity() const { return passenger_capacity_; }
void Boat::SetPassengerCapacity(int capacity) { passenger_capacity_ = capacity; }

std::string Boat::SerializeCustom() const {
    std::string result;
    result += "[Boat]\n";
    result += "name=" + name_ + "\n";
    result += "max_speed=" + std::to_string(max_speed_) + "\n";
    result += GetEngine().SerializeCustom();
    result += "displacement=" + std::to_string(displacement_) + "\n";
    result += "passenger_capacity=" + std::to_string(passenger_capacity_) + "\n";
    if (GetDriver()) {
        result += GetDriver()->SerializeCustom();
    }
    return result;
}

void Boat::DeserializeCustom(const std::string& data) {
    auto kv = ParseKeyValue(data);
    if (kv.count("name"))               name_ = kv["name"];
    if (kv.count("max_speed"))          max_speed_ = std::stod(kv["max_speed"]);
    if (kv.count("displacement"))       displacement_ = std::stod(kv["displacement"]);
    if (kv.count("passenger_capacity")) passenger_capacity_ = std::stoi(kv["passenger_capacity"]);
    GetEngine().DeserializeCustom(data);
}
