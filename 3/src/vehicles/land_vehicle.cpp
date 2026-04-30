#include "land_vehicle.h"
#include <iostream>

LandVehicle::LandVehicle(const std::string& name, double max_speed,
                         double engine_hp, Engine::FuelType fuel_type, int wheel_count)
    : Vehicle(name, max_speed, engine_hp, fuel_type), wheel_count_(wheel_count) {
    std::cout << "[LandVehicle] Создан наземный: " << name_ << "\n";
}

LandVehicle::~LandVehicle() {
    std::cout << "[LandVehicle] Уничтожен наземный: " << name_ << "\n";
}

int LandVehicle::GetWheelCount() const { return wheel_count_; }
void LandVehicle::SetWheelCount(int count) { wheel_count_ = count; }

std::string LandVehicle::GetInfo() const {
    return Vehicle::GetInfo() +
           "\n  Колёса: " + std::to_string(wheel_count_);
}
