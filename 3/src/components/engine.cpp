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
