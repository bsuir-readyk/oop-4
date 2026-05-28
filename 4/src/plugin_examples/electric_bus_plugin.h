#ifndef ELECTRIC_BUS_PLUGIN_H
#define ELECTRIC_BUS_PLUGIN_H

#include "land_vehicle.h"
#include "vehicle_plugin_api.h"

#include <QObject>

#include <string>

class ElectricBus : public LandVehicle {
public:
    ElectricBus(const std::string& name, double max_speed,
                double engine_hp, Engine::FuelType fuel_type,
                int wheel_count, double battery_capacity, int route_number);
    ~ElectricBus() override;

    std::string GetTypeId() const override;
    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    double GetBatteryCapacity() const;
    int GetRouteNumber() const;

private:
    double battery_capacity_;
    int route_number_;
};

class Plane : public LandVehicle {
    public:
        Plane(const std::string& name, double max_speed,
                    double engine_hp, Engine::FuelType fuel_type,
                    int wheel_count, double battery_capacity, int route_number);
        ~Plane() override;
    
        std::string GetTypeId() const override;
        std::string GetType() const override;
        std::string Move() const override;
        std::string GetInfo() const override;
    
        double GetBatteryCapacity() const;
        int GetWings() const;
    
    private:
        double battery_capacity_;
        int route_number_;
    };

class ElectricBusPlugin : public QObject, public VehiclePluginInterface {
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

#endif // ELECTRIC_BUS_PLUGIN_H
