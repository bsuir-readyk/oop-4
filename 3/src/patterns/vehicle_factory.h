#ifndef VEHICLE_FACTORY_H
#define VEHICLE_FACTORY_H

#include "vehicle.h"
#include <memory>
#include <string>

// Перечисление типов транспорта для фабрики
enum class VehicleType { kCar, kTruck, kBoat, kSubmarine };

// Абстрактная фабрика (Factory Method)
// Базовый класс определяет интерфейс создания, конкретные фабрики — реализацию.
class VehicleFactory {
public:
    virtual ~VehicleFactory() = default;
    virtual std::unique_ptr<Vehicle> Create(const std::string& name) const = 0;
    virtual std::string GetFactoryName() const = 0;
};

// Конкретная фабрика легковых автомобилей
class CarFactory : public VehicleFactory {
public:
    std::unique_ptr<Vehicle> Create(const std::string& name) const override;
    std::string GetFactoryName() const override { return "CarFactory"; }
};

// Конкретная фабрика грузовиков
class TruckFactory : public VehicleFactory {
public:
    std::unique_ptr<Vehicle> Create(const std::string& name) const override;
    std::string GetFactoryName() const override { return "TruckFactory"; }
};

// Конкретная фабрика катеров
class BoatFactory : public VehicleFactory {
public:
    std::unique_ptr<Vehicle> Create(const std::string& name) const override;
    std::string GetFactoryName() const override { return "BoatFactory"; }
};

// Конкретная фабрика подводных лодок
class SubmarineFactory : public VehicleFactory {
public:
    std::unique_ptr<Vehicle> Create(const std::string& name) const override;
    std::string GetFactoryName() const override { return "SubmarineFactory"; }
};

// Утилита: получить фабрику по типу
std::unique_ptr<VehicleFactory> GetFactory(VehicleType type);

#endif // VEHICLE_FACTORY_H
