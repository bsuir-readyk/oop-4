#ifndef VEHICLE_SERIALIZER_H
#define VEHICLE_SERIALIZER_H

#include "driver.h"
#include "fleet.h"

#include <memory>
#include <string>
#include <vector>

namespace VehicleSerializer {

struct FleetSnapshot {
    Fleet fleet;
    std::vector<std::unique_ptr<Driver>> drivers;

    FleetSnapshot() = default;
    FleetSnapshot(FleetSnapshot&&) noexcept = default;
    FleetSnapshot& operator=(FleetSnapshot&&) noexcept = default;
    FleetSnapshot(const FleetSnapshot&) = delete;
    FleetSnapshot& operator=(const FleetSnapshot&) = delete;
};

std::string SerializeCustom(const Fleet& fleet,
                            const std::vector<std::unique_ptr<Driver>>& drivers);
FleetSnapshot DeserializeCustom(const std::string& text);

std::string SerializeJson(const Fleet& fleet,
                          const std::vector<std::unique_ptr<Driver>>& drivers);
FleetSnapshot DeserializeJson(const std::string& json);

} // namespace VehicleSerializer

#endif // VEHICLE_SERIALIZER_H
