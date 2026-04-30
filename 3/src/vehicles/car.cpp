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
std::string Car::Move() const { return name_ + " едет по дороге"; }

std::string Car::GetInfo() const {
    return LandVehicle::GetInfo() +
           "\n  Мест: " + std::to_string(seat_count_);
}

int Car::GetSeatCount() const { return seat_count_; }
void Car::SetSeatCount(int count) { seat_count_ = count; }

std::string Car::SerializeCustom() const {
    std::string result;
    result += "[Car]\n";
    result += "name=" + name_ + "\n";
    result += "max_speed=" + std::to_string(max_speed_) + "\n";
    result += GetEngine().SerializeCustom();
    result += "wheel_count=" + std::to_string(GetWheelCount()) + "\n";
    result += "seat_count=" + std::to_string(seat_count_) + "\n";
    if (GetDriver()) {
        result += GetDriver()->SerializeCustom();
    }
    return result;
}

void Car::DeserializeCustom(const std::string& data) {
    auto kv = ParseKeyValue(data);
    if (kv.count("name"))        name_ = kv["name"];
    if (kv.count("max_speed"))   max_speed_ = std::stod(kv["max_speed"]);
    if (kv.count("wheel_count")) wheel_count_ = std::stoi(kv["wheel_count"]);
    if (kv.count("seat_count"))  seat_count_ = std::stoi(kv["seat_count"]);
    GetEngine().DeserializeCustom(data);
}
