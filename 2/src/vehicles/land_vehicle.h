#ifndef LAND_VEHICLE_H
#define LAND_VEHICLE_H

#include "vehicle.h"

class LandVehicle : public Vehicle {
public:
    LandVehicle(const std::string& name, double max_speed,
                double engine_hp, Engine::FuelType fuel_type, int wheel_count);
    ~LandVehicle() override;

    int GetWheelCount() const;
    std::string GetInfo() const override;

protected:
    int wheel_count_;
};

#endif // LAND_VEHICLE_H
