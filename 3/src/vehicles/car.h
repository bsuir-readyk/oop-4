#ifndef CAR_H
#define CAR_H

#include "land_vehicle.h"

class Car : public LandVehicle {
public:
    Car(const std::string& name, double max_speed,
        double engine_hp, Engine::FuelType fuel_type, int seat_count);
    ~Car() override;

    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    int GetSeatCount() const;

private:
    int seat_count_;
};

#endif // CAR_H
