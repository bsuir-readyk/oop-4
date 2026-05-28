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

std::string Boat::GetTypeId() const { return "builtin.boat"; }
std::string Boat::GetType() const { return "Катер"; }
std::string Boat::Move() const { return name_ + " плывёт по воде ⛵"; }

std::string Boat::GetInfo() const {
    return WaterVehicle::GetInfo() +
           "\n  Пассажиров: " + std::to_string(passenger_capacity_);
}

int Boat::GetPassengerCapacity() const { return passenger_capacity_; }
