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
std::string Truck::Move() const { return name_ + " перевозит груз 🚛"; }

std::string Truck::GetInfo() const {
    return LandVehicle::GetInfo() +
           "\n  Грузоподъёмность: " + std::to_string(static_cast<int>(cargo_capacity_)) + " т";
}

double Truck::GetCargoCapacity() const { return cargo_capacity_; }
