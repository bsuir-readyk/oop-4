#include "hovercraft_plugin.h"

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
        throw SerializationException("hovercraft payload misses numeric field: " +
                                     key.toStdString());
    }
    return value.toDouble();
}

const Hovercraft& AsHovercraft(const Vehicle& vehicle) {
    const auto* hovercraft = dynamic_cast<const Hovercraft*>(&vehicle);
    if (!hovercraft) {
        throw VehicleException("Ожидался Hovercraft");
    }
    return *hovercraft;
}

} // namespace

Hovercraft::Hovercraft(const std::string& name, double max_speed,
                       double engine_hp, Engine::FuelType fuel_type,
                       double displacement, double cushion_pressure)
    : WaterVehicle(name, max_speed, engine_hp, fuel_type, displacement),
      cushion_pressure_(cushion_pressure) {
    std::cout << "[Hovercraft] Создан: " << name_ << "\n";
}

Hovercraft::~Hovercraft() {
    std::cout << "[Hovercraft] Уничтожен: " << name_ << "\n";
}

std::string Hovercraft::GetTypeId() const {
    return "plugin.hovercraft";
}

std::string Hovercraft::GetType() const {
    return "Судно на воздушной подушке";
}

std::string Hovercraft::Move() const {
    return name_ + " скользит над водой на воздушной подушке";
}

std::string Hovercraft::GetInfo() const {
    return WaterVehicle::GetInfo() +
           "\n  Давление воздушной подушки: " +
           std::to_string(static_cast<int>(cushion_pressure_)) + " кПа";
}

double Hovercraft::GetCushionPressure() const {
    return cushion_pressure_;
}

QString HovercraftPlugin::PluginId() const {
    return "hovercraft_plugin";
}

QString HovercraftPlugin::PluginName() const {
    return "Hovercraft transport plugin";
}

QString HovercraftPlugin::PluginVersion() const {
    return "1.0";
}

bool HovercraftPlugin::RegisterPlugin(PluginHost& host, QString* error) {
    VehicleTypeRegistration type;
    type.type_id = "plugin.hovercraft";
    type.display_name = "Судно на воздушной подушке (Hovercraft)";
    type.factory_name = "HovercraftPluginFactory";
    type.card_color = QColor("#00838F");
    type.create_default = [](const std::string& name) {
        return std::make_unique<Hovercraft>(name, 95, 600,
                                            Engine::FuelType::kDiesel, 15, 42);
    };
    type.serialize_payload = [](const Vehicle& vehicle) {
        const auto& hovercraft = AsHovercraft(vehicle);
        QJsonObject payload;
        payload["displacement"] = hovercraft.GetDisplacement();
        payload["cushion_pressure"] = hovercraft.GetCushionPressure();
        return payload;
    };
    type.deserialize = [](const VehicleSerializationData& data) {
        return std::make_unique<Hovercraft>(
            data.name, data.max_speed, data.engine_horsepower, data.fuel_type,
            RequireDouble(data.payload, "displacement"),
            RequireDouble(data.payload, "cushion_pressure"));
    };
    if (!host.RegisterVehicleType(type, error)) {
        return false;
    }

    PluginTabRegistration tab;
    tab.tab_id = "hovercraft.about";
    tab.title = "Hovercraft";
    tab.create_widget = [](QWidget* parent) {
        auto* widget = new QWidget(parent);
        auto* layout = new QVBoxLayout(widget);
        auto* title = new QLabel("Плагин: судно на воздушной подушке");
        title->setStyleSheet("font-weight: bold; font-size: 16px;");

        auto* text = new QTextEdit();
        text->setReadOnly(true);
        text->setText(
            "Этот модуль загружается из папки plugins как отдельная "
            "динамическая библиотека.\n\n"
            "Он добавляет новый класс Hovercraft, фабрику создания, "
            "контракт сериализации payload и расширение UI карточек флота.\n\n"
            "Основная программа для появления этого типа не меняется и "
            "не перекомпилируется.");

        layout->addWidget(title);
        layout->addWidget(text);
        return widget;
    };
    if (!host.RegisterTab(tab, error)) {
        return false;
    }

    FleetUiExtensionRegistration fleet_ui;
    fleet_ui.extension_id = "hovercraft.fleet-card";
    fleet_ui.supports = [](const Vehicle& vehicle) {
        return vehicle.GetTypeId() == "plugin.hovercraft";
    };
    fleet_ui.customize_card = [](const Vehicle& vehicle, FleetCardStyle& style) {
        const auto& hovercraft = AsHovercraft(vehicle);
        style.background_color = "#006064";
        style.subtitle = QString("подушка: %1 кПа")
                             .arg(static_cast<int>(hovercraft.GetCushionPressure()));
    };
    fleet_ui.extra_details = [](const Vehicle& vehicle) {
        const auto& hovercraft = AsHovercraft(vehicle);
        return QString("Hovercraft UI extension\nДавление подушки: %1 кПа\n"
                       "Карточка перекрашена самим плагином.")
            .arg(static_cast<int>(hovercraft.GetCushionPressure()));
    };
    return host.RegisterFleetUiExtension(fleet_ui, error);
}

void HovercraftPlugin::BeforeUnload() {
    std::cout << "[HovercraftPlugin] BeforeUnload\n";
}
