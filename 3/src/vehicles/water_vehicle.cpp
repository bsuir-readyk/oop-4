#include "water_vehicle.h"
#include <iostream>

WaterVehicle::WaterVehicle(const std::string& name, double max_speed,
                           double engine_hp, Engine::FuelType fuel_type, double displacement)
    : Vehicle(name, max_speed, engine_hp, fuel_type), displacement_(displacement) {
    std::cout << "[WaterVehicle] Создан водный: " << name_ << "\n";
}

WaterVehicle::~WaterVehicle() {
    std::cout << "[WaterVehicle] Уничтожен водный: " << name_ << "\n";
}

double WaterVehicle::GetDisplacement() const { return displacement_; }
void WaterVehicle::SetDisplacement(double displacement) { displacement_ = displacement; }

std::string WaterVehicle::GetInfo() const {
    return Vehicle::GetInfo() +
           "\n  Водоизмещение: " + std::to_string(static_cast<int>(displacement_)) + " т";
}
