#ifndef HOVERCRAFT_PLUGIN_H
#define HOVERCRAFT_PLUGIN_H

#include "vehicle_plugin_api.h"
#include "water_vehicle.h"

#include <QObject>

#include <string>

class Hovercraft : public WaterVehicle {
public:
    Hovercraft(const std::string& name, double max_speed,
               double engine_hp, Engine::FuelType fuel_type,
               double displacement, double cushion_pressure);
    ~Hovercraft() override;

    std::string GetTypeId() const override;
    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    double GetCushionPressure() const;

private:
    double cushion_pressure_;
};

class HovercraftPlugin : public QObject, public VehiclePluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID VehiclePluginInterface_iid)
    Q_INTERFACES(VehiclePluginInterface)

public:
    QString PluginId() const override;
    QString PluginName() const override;
    QString PluginVersion() const override;
    bool RegisterPlugin(PluginHost& host, QString* error) override;
    void BeforeUnload() override;
};

#endif // HOVERCRAFT_PLUGIN_H
