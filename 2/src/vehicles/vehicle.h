#ifndef VEHICLE_H
#define VEHICLE_H

#include "engine.h"
#include "driver.h"
#include <string>

class Vehicle {
public:
    Vehicle(const std::string& name, double max_speed,
            double engine_hp, Engine::FuelType fuel_type);
    virtual ~Vehicle();

    virtual std::string GetType() const = 0;
    virtual std::string Move() const = 0;

    virtual std::string GetInfo() const;
    virtual double GetMaxSpeed() const;

    std::string GetName() const;
    const Engine& GetEngine() const;
    Engine& GetEngine();
    const Driver* GetDriver() const;

    void AssignDriver(Driver* driver);
    void RemoveDriver();

    // для исключений
    void SetCurrentSpeed(double speed);
    double GetCurrentSpeed() const;

protected:
    std::string name_;
    double max_speed_;
    double current_speed_ = 0.0;

private:
    Engine engine_;             // composition
    Driver* driver_ = nullptr;  // aggregation
};

#endif // VEHICLE_H
