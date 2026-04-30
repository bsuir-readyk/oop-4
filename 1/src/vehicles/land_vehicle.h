#ifndef LAND_VEHICLE_H
#define LAND_VEHICLE_H

#include "vehicle.h"

// Уровень 2 иерархии — наземный транспорт (абстрактный).
class LandVehicle : public Vehicle {
public:
    LandVehicle(const std::string& name, double max_speed,
                double engine_hp, Engine::FuelType fuel_type, int wheel_count);
    ~LandVehicle() override;

    int GetWheelCount() const;
    std::string GetInfo() const override;

    // Move() остаётся чисто виртуальным — конкретные классы реализуют

protected:
    int wheel_count_;  // Количество колёс
};

#endif // LAND_VEHICLE_H
