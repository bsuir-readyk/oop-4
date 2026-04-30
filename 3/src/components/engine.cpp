#include "engine.h"
#include "exceptions.h"
#include <iostream>

Engine::Engine(double horsepower, FuelType fuel_type)
    : horsepower_(horsepower), fuel_type_(fuel_type) {
    if (horsepower <= 0) {
        throw EngineException("Мощность должна быть положительной");
    }
    std::cout << "[Engine] Создан двигатель " << horsepower_ << " л.с.\n";
}

Engine::~Engine() {
    std::cout << "[Engine] Уничтожен двигатель " << horsepower_ << " л.с.\n";
}

double Engine::GetHorsepower() const { return horsepower_; }
Engine::FuelType Engine::GetFuelType() const { return fuel_type_; }
bool Engine::IsRunning() const { return is_running_; }

void Engine::SetHorsepower(double hp) {
    if (hp <= 0) {
        throw EngineException("Мощность должна быть положительной");
    }
    horsepower_ = hp;
}

void Engine::SetFuelType(FuelType type) {
    fuel_type_ = type;
}

void Engine::Start() {
    if (is_running_) {
        throw EngineException("Двигатель уже запущен");
    }
    is_running_ = true;
}

void Engine::Stop() {
    if (!is_running_) {
        throw EngineException("Двигатель уже остановлен");
    }
    is_running_ = false;
}

std::string Engine::GetInfo() const {
    return std::to_string(static_cast<int>(horsepower_)) + " л.с., " +
           FuelTypeToString(fuel_type_) +
           (is_running_ ? ", работает" : ", выключен");
}

std::string Engine::FuelTypeToString(FuelType type) {
    switch (type) {
        case FuelType::kPetrol:   return "Бензин";
        case FuelType::kDiesel:   return "Дизель";
        case FuelType::kElectric: return "Электро";
        case FuelType::kNuclear:  return "Ядерный";
    }
    return "Неизвестно";
}

Engine::FuelType Engine::FuelTypeFromString(const std::string& str) {
    if (str == "Бензин" || str == "Petrol")   return FuelType::kPetrol;
    if (str == "Дизель" || str == "Diesel")   return FuelType::kDiesel;
    if (str == "Электро" || str == "Electric") return FuelType::kElectric;
    if (str == "Ядерный" || str == "Nuclear")  return FuelType::kNuclear;
    return FuelType::kPetrol;
}

std::string Engine::SerializeCustom() const {
    std::string result;
    result += "engine_hp=" + std::to_string(horsepower_) + "\n";
    result += "engine_fuel=" + FuelTypeToString(fuel_type_) + "\n";
    return result;
}

void Engine::DeserializeCustom(const std::string& data) {
    auto kv = ParseKeyValue(data);
    if (kv.count("engine_hp"))   horsepower_ = std::stod(kv["engine_hp"]);
    if (kv.count("engine_fuel")) fuel_type_ = FuelTypeFromString(kv["engine_fuel"]);
}
