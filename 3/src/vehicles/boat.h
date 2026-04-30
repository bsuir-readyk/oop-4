#ifndef BOAT_H
#define BOAT_H

#include "water_vehicle.h"

// Уровень 3 — катер/лодка.
class Boat : public WaterVehicle {
public:
    Boat(const std::string& name, double max_speed,
         double engine_hp, Engine::FuelType fuel_type,
         double displacement, int passenger_capacity);
    ~Boat() override;

    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    int GetPassengerCapacity() const;
    void SetPassengerCapacity(int capacity);

    // ISerializable
    std::string SerializeCustom() const override;
    void DeserializeCustom(const std::string& data) override;

private:
    int passenger_capacity_;
};

#endif // BOAT_H
