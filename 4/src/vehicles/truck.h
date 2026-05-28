#ifndef TRUCK_H
#define TRUCK_H

#include "land_vehicle.h"

class Truck : public LandVehicle {
public:
    Truck(const std::string& name, double max_speed,
          double engine_hp, Engine::FuelType fuel_type, double cargo_capacity);
    ~Truck() override;

    std::string GetTypeId() const override;
    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    double GetCargoCapacity() const;

private:
    double cargo_capacity_;
};

#endif // TRUCK_H
