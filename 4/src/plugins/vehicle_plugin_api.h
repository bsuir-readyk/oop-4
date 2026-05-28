#ifndef VEHICLE_PLUGIN_API_H
#define VEHICLE_PLUGIN_API_H

#include "engine.h"
#include "vehicle.h"

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QWidget>
#include <QtPlugin>

#include <functional>
#include <memory>
#include <string>

struct VehicleSerializationData {
    std::string name;
    double max_speed = 0.0;
    double current_speed = 0.0;
    double engine_horsepower = 0.0;
    Engine::FuelType fuel_type = Engine::FuelType::kPetrol;
    bool engine_running = false;
    int driver_id = 0;
    QJsonObject payload;
};

struct VehicleTypeRegistration {
    std::string type_id;
    std::string display_name;
    std::string factory_name;
    QColor card_color;
    std::function<std::unique_ptr<Vehicle>(const std::string& name)> create_default;
    std::function<QJsonObject(const Vehicle& vehicle)> serialize_payload;
    std::function<std::unique_ptr<Vehicle>(const VehicleSerializationData& data)> deserialize;
    std::string owner_plugin_id;
    bool enabled = true;
};

struct PluginTabRegistration {
    std::string tab_id;
    QString title;
    std::function<QWidget*(QWidget* parent)> create_widget;
    std::string owner_plugin_id;
};

struct FleetCardStyle {
    QString background_color;
    QString subtitle;
};

struct FleetUiExtensionRegistration {
    std::string extension_id;
    std::function<bool(const Vehicle& vehicle)> supports;
    std::function<void(const Vehicle& vehicle, FleetCardStyle& style)> customize_card;
    std::function<QString(const Vehicle& vehicle)> extra_details;
    std::string owner_plugin_id;
};

class PluginHost {
public:
    virtual ~PluginHost() = default;

    virtual bool RegisterVehicleType(const VehicleTypeRegistration& registration,
                                     QString* error) = 0;
    virtual bool RegisterTab(const PluginTabRegistration& registration,
                             QString* error) = 0;
    virtual bool RegisterFleetUiExtension(
        const FleetUiExtensionRegistration& registration,
        QString* error) = 0;
};

class VehiclePluginInterface {
public:
    virtual ~VehiclePluginInterface() = default;

    virtual QString PluginId() const = 0;
    virtual QString PluginName() const = 0;
    virtual QString PluginVersion() const = 0;
    virtual bool RegisterPlugin(PluginHost& host, QString* error) = 0;
    virtual void BeforeUnload() {}
};

#define VehiclePluginInterface_iid "oop.vehicles.VehiclePluginInterface/1.0"

Q_DECLARE_INTERFACE(VehiclePluginInterface, VehiclePluginInterface_iid)

#endif // VEHICLE_PLUGIN_API_H
