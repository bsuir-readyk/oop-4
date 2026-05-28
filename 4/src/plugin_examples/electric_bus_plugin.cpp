#include "electric_bus_plugin.h"

#include "exceptions.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

#include <iostream>

namespace {

double RequireDouble(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (!value.isDouble()) {
        throw SerializationException("electric bus payload misses numeric field: " +
                                     key.toStdString());
    }
    return value.toDouble();
}

int RequireInt(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (!value.isDouble()) {
        throw SerializationException("electric bus payload misses integer field: " +
                                     key.toStdString());
    }
    return value.toInt();
}

const ElectricBus& AsElectricBus(const Vehicle& vehicle) {
    const auto* bus = dynamic_cast<const ElectricBus*>(&vehicle);
    if (!bus) {
        throw VehicleException("Ожидался ElectricBus");
    }
    return *bus;
}

} // namespace


const Plane& AsPlane(const Vehicle& vehicle) {
    const auto* bus = dynamic_cast<const Plane*>(&vehicle);
    if (!bus) {
        throw VehicleException("Ожидался Plane");
    }
    return *bus;
}
// namespace

ElectricBus::ElectricBus(const std::string& name, double max_speed,
                         double engine_hp, Engine::FuelType fuel_type,
                         int wheel_count, double battery_capacity, int route_number)
    : LandVehicle(name, max_speed, engine_hp, fuel_type, wheel_count),
      battery_capacity_(battery_capacity), route_number_(route_number) {
    std::cout << "[ElectricBus] Создан: " << name_ << "\n";
}

ElectricBus::~ElectricBus() {
    std::cout << "[ElectricBus] Уничтожен: " << name_ << "\n";
}

std::string ElectricBus::GetTypeId() const {
    return "plugin.electric_bus";
}

std::string ElectricBus::GetType() const {
    return "Электробус";
}

std::string ElectricBus::Move() const {
    return name_ + " тихо движется по городскому маршруту";
}

std::string ElectricBus::GetInfo() const {
    return LandVehicle::GetInfo() +
           "\n  Ёмкость батареи: " +
           std::to_string(static_cast<int>(battery_capacity_)) + " кВт·ч" +
           "\n  Маршрут: " + std::to_string(route_number_);
}

double ElectricBus::GetBatteryCapacity() const {
    return battery_capacity_;
}

int ElectricBus::GetRouteNumber() const {
    return route_number_;
}

//plane

Plane::Plane(const std::string& name, double max_speed,
    double engine_hp, Engine::FuelType fuel_type,
    int wheel_count, double battery_capacity, int route_number)
: LandVehicle(name, max_speed, engine_hp, fuel_type, wheel_count),
battery_capacity_(battery_capacity), route_number_(route_number) {
std::cout << "[Plane] Создан: " << name_ << "\n";
}

Plane::~Plane() {
std::cout << "[Plane] Уничтожен: " << name_ << "\n";
}

std::string Plane::GetTypeId() const {
return "plugin.electric_bus";
}

std::string Plane::GetType() const {
return "Электробус";
}

std::string Plane::Move() const {
return name_ + " тихо движется по городскому маршруту";
}

std::string Plane::GetInfo() const {
return LandVehicle::GetInfo() +
"\n  Ёмкость батареи: " +
std::to_string(static_cast<int>(battery_capacity_)) + " кВт·ч" +
"\n  Маршрут: " + std::to_string(route_number_);
}

double Plane::GetBatteryCapacity() const {
return battery_capacity_;
}

int Plane::GetWings() const {
return route_number_;
}




QString ElectricBusPlugin::PluginId() const {
    return "electric_bus_plugin3";
}

QString ElectricBusPlugin::PluginName() const {
    return "Electric bus transport plugin";
}

QString ElectricBusPlugin::PluginVersion() const {
    return "1.0";
}

bool ElectricBusPlugin::RegisterPlugin(PluginHost& host, QString* error) {
    VehicleTypeRegistration type;
    type.type_id = "plugin.electric_bus";
    type.display_name = "Электробус (ElectricBus)";
    type.factory_name = "ElectricBusPluginFactory";
    type.card_color = QColor("#2E7D32");
    type.create_default = [](const std::string& name) {
        return std::make_unique<ElectricBus>(name, 85, 320,
                                             Engine::FuelType::kElectric, 6, 410, 12);
    };
    type.serialize_payload = [](const Vehicle& vehicle) {
        const auto& bus = AsElectricBus(vehicle);
        QJsonObject payload;
        payload["wheel_count"] = bus.GetWheelCount();
        payload["battery_capacity"] = bus.GetBatteryCapacity();
        payload["route_number"] = bus.GetRouteNumber();
        return payload;
    };
    type.deserialize = [](const VehicleSerializationData& data) {
        return std::make_unique<ElectricBus>(
            data.name, data.max_speed, data.engine_horsepower, data.fuel_type,
            RequireInt(data.payload, "wheel_count"),
            RequireDouble(data.payload, "battery_capacity"),
            RequireInt(data.payload, "route_number"));
    };
    // if (!host.RegisterVehicleType(type, error)) {
    //     return false;
    // }


    { // plane
        VehicleTypeRegistration type;
        type.type_id = "plugin.plane";
        type.display_name = "Plane (Plane)";
        type.factory_name = "PlanePluginFactory";
        type.card_color = QColor("#ff0000");
        type.create_default = [](const std::string& name) {
            return std::make_unique<Plane>(name, 85, 320,
                                                Engine::FuelType::kElectric, 6, 410, 12);
        };
        type.serialize_payload = [](const Vehicle& vehicle) {
            const auto& bus = AsPlane(vehicle);
            QJsonObject payload;
            payload["wheel_count"] = bus.GetWheelCount();
            payload["battery_capacity"] = bus.GetBatteryCapacity();
            payload["wings"] = bus.GetWings();
            return payload;
        };
        type.deserialize = [](const VehicleSerializationData& data) {
            return std::make_unique<Plane>(
                data.name, data.max_speed, data.engine_horsepower, data.fuel_type,
                RequireInt(data.payload, "wheel_count"),
                RequireDouble(data.payload, "battery_capacity"),
                RequireInt(data.payload, "route_number"));
        };
        if (!host.RegisterVehicleType(type, error)) {
            return false;
        }
    }

    PluginTabRegistration tab;
    tab.tab_id = "electric_bus.about";
    tab.title = "ElectricBus";
    tab.create_widget = [](QWidget* parent) {
        auto* widget = new QWidget(parent);
        auto* layout = new QVBoxLayout(widget);
        auto* title = new QLabel("Плагин: городской электробус");
        title->setStyleSheet("font-weight: bold; font-size: 16px;");

        auto* text = new QTextEdit();
        text->setReadOnly(true);
        text->setText(
            "!!!! Этот плагин добавляет новый наземный транспорт ElectricBus.\n\n"
            "Класс наследуется от LandVehicle, имеет электрический двигатель, "
            "номер маршрута и ёмкость батареи.\n\n"
            "Модуль также регистрирует свою сериализацию и расширение UI "
            "для карточек во вкладке Флот.");

        layout->addWidget(title);
        layout->addWidget(text);
        return widget;
    };
    if (!host.RegisterTab(tab, error)) {
        return false;
    }

    // plane
        PluginTabRegistration tab1;
        tab1.tab_id = "electric_bus.plane";
        tab1.title = "Plane";
        tab1.create_widget = [](QWidget* parent) {
            auto* widget = new QWidget(parent);
            auto* layout = new QVBoxLayout(widget);
            auto* title = new QLabel("Plane");
            title->setStyleSheet("font-weight: bold; font-size: 16px;");

            auto* text = new QTextEdit();
            text->setReadOnly(true);
            text->setText(
                "!!! Этот плагин добавляет новый наземный транспорт ElectricBus.\n\n"
                "Класс наследуется от LandVehicle, имеет электрический двигатель, "
                "номер маршрута и ёмкость батареи.\n\n"
                "Модуль также регистрирует свою сериализацию и расширение UI "
                "для карточек во вкладке Флот.");

            layout->addWidget(title);
            layout->addWidget(text);
            return widget;
        };
        if (!host.RegisterTab(tab1, error)) {
            return false;
        }

    FleetUiExtensionRegistration fleet_ui;
    fleet_ui.extension_id = "electric_bus.fleet-card";
    fleet_ui.supports = [](const Vehicle& vehicle) {
        return vehicle.GetTypeId() == "plugin.electric_bus";
    };
    fleet_ui.customize_card = [](const Vehicle& vehicle, FleetCardStyle& style) {
        const auto& bus = AsElectricBus(vehicle);
        style.background_color = "#1B5E20";
        style.subtitle = QString("маршрут %1, %2 кВт·ч")
                             .arg(bus.GetRouteNumber())
                             .arg(static_cast<int>(bus.GetBatteryCapacity()));
    };
    fleet_ui.extra_details = [](const Vehicle& vehicle) {
        const auto& bus = AsElectricBus(vehicle);
        return QString("ElectricBus UI extension\nМаршрут: %1\nБатарея: %2 кВт·ч")
            .arg(bus.GetRouteNumber())
            .arg(static_cast<int>(bus.GetBatteryCapacity()));
    };
    return host.RegisterFleetUiExtension(fleet_ui, error);
}

void ElectricBusPlugin::BeforeUnload() {
    std::cout << "[ElectricBusPlugin] !!!!!!!new BeforeUnload\n";
}
