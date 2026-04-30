#include "vehicle.h"
#include "exceptions.h"
#include <iostream>

Vehicle::Vehicle(const std::string& name, double max_speed,
                 double engine_hp, Engine::FuelType fuel_type)
    : name_(name), max_speed_(max_speed), engine_(engine_hp, fuel_type) {
    std::cout << "[Vehicle] Создан: " << name_ << "\n";
}

Vehicle::~Vehicle() {
    std::cout << "[Vehicle] Уничтожен: " << name_ << "\n";
}

std::string Vehicle::GetInfo() const {
    std::string info = GetType() + ": " + name_ +
                       "\n  Макс. скорость: " + std::to_string(static_cast<int>(max_speed_)) + " км/ч" +
                       "\n  Двигатель: " + engine_.GetInfo();
    if (driver_) {
        info += "\n  Водитель: " + driver_->GetInfo();
    } else {
        info += "\n  Водитель: не назначен";
    }
    return info;
}

double Vehicle::GetMaxSpeed() const { return max_speed_; }
std::string Vehicle::GetName() const { return name_; }
const Engine& Vehicle::GetEngine() const { return engine_; }
Engine& Vehicle::GetEngine() { return engine_; }
const Driver* Vehicle::GetDriver() const { return driver_; }

void Vehicle::AssignDriver(Driver* driver) { driver_ = driver; }
void Vehicle::RemoveDriver() { driver_ = nullptr; }

void Vehicle::SetCurrentSpeed(double speed) {
    if (speed < 0) {
        throw SpeedLimitException(speed, max_speed_);
    }
    if (speed > max_speed_) {
        throw SpeedLimitException(speed, max_speed_);
    }
    current_speed_ = speed;
}

double Vehicle::GetCurrentSpeed() const { return current_speed_; }
