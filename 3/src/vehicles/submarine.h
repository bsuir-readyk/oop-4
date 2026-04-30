#ifndef SUBMARINE_H
#define SUBMARINE_H

#include "water_vehicle.h"

// Уровень 3 — подводная лодка.
class Submarine : public WaterVehicle {
public:
    Submarine(const std::string& name, double max_speed,
              double engine_hp, Engine::FuelType fuel_type,
              double displacement, double max_depth);
    ~Submarine() override;

    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    double GetMaxDepth() const;
    void SetMaxDepth(double depth);

    // ISerializable
    std::string SerializeCustom() const override;
    void DeserializeCustom(const std::string& data) override;

private:
    double max_depth_;  // Максимальная глубина погружения
};

#endif // SUBMARINE_H
