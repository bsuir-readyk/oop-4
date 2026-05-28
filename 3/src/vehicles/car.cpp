#include "car.h"
#include <iostream>

Car::Car(const std::string& name, double max_speed,
         double engine_hp, Engine::FuelType fuel_type, int seat_count)
    : LandVehicle(name, max_speed, engine_hp, fuel_type, 4), seat_count_(seat_count) {
    std::cout << "[Car] Создан: " << name_ << "\n";
}

Car::~Car() {
    std::cout << "[Car] Уничтожен: " << name_ << "\n";
}

std::string Car::GetType() const { return "Легковой автомобиль"; }
std::string Car::Move() const { return name_ + " едет по дороге 🚗"; }

std::string Car::GetInfo() const {
    return LandVehicle::GetInfo() +
           "\n  Мест: " + std::to_string(seat_count_);
}

int Car::GetSeatCount() const { return seat_count_; }
