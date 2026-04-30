#ifndef VEHICLE_H
#define VEHICLE_H

#include "engine.h"
#include "driver.h"
#include <string>
#include <memory>

// Абстрактный базовый класс — уровень 1 иерархии.
// Демонстрирует: абстракцию, инкапсуляцию, композицию (Engine), агрегацию (Driver).
class Vehicle {
public:
    Vehicle(const std::string& name, double max_speed,
            double engine_hp, Engine::FuelType fuel_type);
    virtual ~Vehicle();

    // Чисто виртуальные методы (абстракция) — должны быть реализованы в потомках
    virtual std::string GetType() const = 0;
    virtual std::string Move() const = 0;

    // Виртуальный метод с реализацией по умолчанию (полиморфизм)
    virtual std::string GetInfo() const;
    virtual double GetMaxSpeed() const;

    // Геттеры (инкапсуляция)
    std::string GetName() const;
    const Engine& GetEngine() const;
    Engine& GetEngine();
    const Driver* GetDriver() const;

    // Агрегация: назначить/снять водителя
    void AssignDriver(Driver* driver);
    void RemoveDriver();

    // Установить скорость с проверкой (исключения)
    void SetCurrentSpeed(double speed);
    double GetCurrentSpeed() const;

protected:
    std::string name_;              // Название ТС
    double max_speed_;              // Максимальная скорость
    double current_speed_ = 0.0;   // Текущая скорость

private:
    Engine engine_;                 // Композиция: двигатель принадлежит ТС
    Driver* driver_ = nullptr;     // Агрегация: водитель — внешний объект
};

#endif // VEHICLE_H
