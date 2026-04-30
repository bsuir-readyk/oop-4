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

std::string Submarine::GetType() const { return "Подводная лодка"; }
std::string Submarine::Move() const { return name_ + " погружается под воду"; }

std::string Submarine::GetInfo() const {
    return WaterVehicle::GetInfo() +
           "\n  Макс. глубина: " + std::to_string(static_cast<int>(max_depth_)) + " м";
}

double Submarine::GetMaxDepth() const { return max_depth_; }
void Submarine::SetMaxDepth(double depth) { max_depth_ = depth; }

std::string Submarine::SerializeCustom() const {
    std::string result;
    result += "[Submarine]\n";
    result += "name=" + name_ + "\n";
    result += "max_speed=" + std::to_string(max_speed_) + "\n";
    result += GetEngine().SerializeCustom();
    result += "displacement=" + std::to_string(displacement_) + "\n";
    result += "max_depth=" + std::to_string(max_depth_) + "\n";
    if (GetDriver()) {
        result += GetDriver()->SerializeCustom();
    }
    return result;
}

void Submarine::DeserializeCustom(const std::string& data) {
    auto kv = ParseKeyValue(data);
    if (kv.count("name"))         name_ = kv["name"];
    if (kv.count("max_speed"))    max_speed_ = std::stod(kv["max_speed"]);
    if (kv.count("displacement")) displacement_ = std::stod(kv["displacement"]);
    if (kv.count("max_depth"))    max_depth_ = std::stod(kv["max_depth"]);
    GetEngine().DeserializeCustom(data);
}
