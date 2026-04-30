#ifndef WATER_VEHICLE_H
#define WATER_VEHICLE_H

#include "vehicle.h"

// Уровень 2 иерархии — водный транспорт (абстрактный).
class WaterVehicle : public Vehicle {
public:
    WaterVehicle(const std::string& name, double max_speed,
                 double engine_hp, Engine::FuelType fuel_type, double displacement);
    ~WaterVehicle() override;

    double GetDisplacement() const;
    std::string GetInfo() const override;

protected:
    double displacement_;  // Водоизмещение в тоннах
};

#endif // WATER_VEHICLE_H
