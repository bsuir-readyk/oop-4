#ifndef VEHICLE_FACTORY_H
#define VEHICLE_FACTORY_H

#include "vehicle.h"
#include <memory>
#include <string>

enum class VehicleType { kCar, kTruck, kBoat, kSubmarine };

class VehicleFactory {
public:
    virtual ~VehicleFactory() = default;

    virtual std::unique_ptr<Vehicle> Create(const std::string& name) const = 0;
    virtual std::string GetFactoryName() const = 0;
};

class CarFactory : public VehicleFactory {
public:
    std::unique_ptr<Vehicle> Create(const std::string& name) const override;
    std::string GetFactoryName() const override { return "CarFactory"; }
};

class TruckFactory : public VehicleFactory {
public:
    std::unique_ptr<Vehicle> Create(const std::string& name) const override;
    std::string GetFactoryName() const override { return "TruckFactory"; }
};

class BoatFactory : public VehicleFactory {
public:
    std::unique_ptr<Vehicle> Create(const std::string& name) const override;
    std::string GetFactoryName() const override { return "BoatFactory"; }
};

class SubmarineFactory : public VehicleFactory {
public:
    std::unique_ptr<Vehicle> Create(const std::string& name) const override;
    std::string GetFactoryName() const override { return "SubmarineFactory"; }
};

std::unique_ptr<VehicleFactory> GetFactory(VehicleType type);

#endif // VEHICLE_FACTORY_H
