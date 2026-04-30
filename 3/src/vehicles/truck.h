#ifndef TRUCK_H
#define TRUCK_H

#include "land_vehicle.h"

// Уровень 3 — грузовик.
class Truck : public LandVehicle {
public:
    Truck(const std::string& name, double max_speed,
          double engine_hp, Engine::FuelType fuel_type, double cargo_capacity);
    ~Truck() override;

    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    double GetCargoCapacity() const;
    void SetCargoCapacity(double capacity);

    // ISerializable
    std::string SerializeCustom() const override;
    void DeserializeCustom(const std::string& data) override;

private:
    double cargo_capacity_;  // Грузоподъёмность в тоннах
};

#endif // TRUCK_H
