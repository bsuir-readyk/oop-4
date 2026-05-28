#include "submarine.h"
#include <iostream>

Submarine::Submarine(const std::string& name, double max_speed,
                     double engine_hp, Engine::FuelType fuel_type,
                     double displacement, double max_depth)
    : WaterVehicle(name, max_speed, engine_hp, fuel_type, displacement),
      max_depth_(max_depth) {
    std::cout << "[Submarine] Создан: " << name_ << "\n";
}

Submarine::~Submarine() {
    std::cout << "[Submarine] Уничтожен: " << name_ << "\n";
}

std::string Submarine::GetTypeId() const { return "builtin.submarine"; }
std::string Submarine::GetType() const { return "Подводная лодка"; }
std::string Submarine::Move() const { return name_ + " погружается под воду 🚢"; }

std::string Submarine::GetInfo() const {
    return WaterVehicle::GetInfo() +
           "\n  Макс. глубина: " + std::to_string(static_cast<int>(max_depth_)) + " м";
}

double Submarine::GetMaxDepth() const { return max_depth_; }
