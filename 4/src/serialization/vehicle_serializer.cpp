#include "vehicle_serializer.h"

#include "exceptions.h"
#include "vehicle_type_registry.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QString>

#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

constexpr const char* kCustomHeader = "FLEET_CUSTOM_V1";
constexpr const char* kJsonFormat = "oop_vehicles_fleet";
constexpr size_t kMaxSerializedDrivers = 1000;
constexpr size_t kMaxSerializedVehicles = 1000;
constexpr int kMaxDriverExperienceYears = 100;
constexpr double kMaxSpeed = 10000.0;
constexpr double kMaxEngineHorsepower = 100000.0;

struct DriverRecord {
    int id = 0;
    std::string name;
    int experience_years = 0;
};

struct VehicleRecord {
    int id = 0;
    std::string type_id;
    VehicleSerializationData data;
};

QString ToQString(const std::string& value) {
    return QString::fromUtf8(value.data(), static_cast<int>(value.size()));
}

std::string FromQString(const QString& value) {
    const QByteArray bytes = value.toUtf8();
    return std::string(bytes.constData(), static_cast<size_t>(bytes.size()));
}

std::string CanonicalTypeId(const std::string& type) {
    if (type == "Car") return "builtin.car";
    if (type == "Truck") return "builtin.truck";
    if (type == "Boat") return "builtin.boat";
    if (type == "Submarine") return "builtin.submarine";
    return type;
}

std::string FuelToCode(Engine::FuelType type) {
    switch (type) {
        case Engine::FuelType::kPetrol:   return "Petrol";
        case Engine::FuelType::kDiesel:   return "Diesel";
        case Engine::FuelType::kElectric: return "Electric";
        case Engine::FuelType::kNuclear:  return "Nuclear";
    }
    throw SerializationException("unknown fuel type");
}

Engine::FuelType FuelFromCode(const std::string& code) {
    if (code == "Petrol") return Engine::FuelType::kPetrol;
    if (code == "Diesel") return Engine::FuelType::kDiesel;
    if (code == "Electric") return Engine::FuelType::kElectric;
    if (code == "Nuclear") return Engine::FuelType::kNuclear;
    throw SerializationException("unknown fuel type: " + code);
}

int HexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

std::string EscapeText(const std::string& value) {
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setfill('0');
    for (unsigned char c : value) {
        if (c == '%' || c == '|' || c == '\n' || c == '\r' || c == '\t') {
            out << '%' << std::setw(2) << static_cast<int>(c);
        } else {
            out << static_cast<char>(c);
        }
    }
    return out.str();
}

std::string UnescapeText(const std::string& value) {
    std::string result;
    result.reserve(value.size());

    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] != '%') {
            result.push_back(value[i]);
            continue;
        }
        if (i + 2 >= value.size()) {
            throw SerializationException("invalid escape sequence in custom text");
        }
        const int hi = HexValue(value[i + 1]);
        const int lo = HexValue(value[i + 2]);
        if (hi < 0 || lo < 0) {
            throw SerializationException("invalid escape sequence in custom text");
        }
        result.push_back(static_cast<char>((hi << 4) | lo));
        i += 2;
    }

    return result;
}

std::vector<std::string> SplitFields(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    for (char c : line) {
        if (c == '|') {
            fields.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    fields.push_back(current);
    return fields;
}

int ParseInt(const std::string& value, const std::string& field) {
    try {
        size_t pos = 0;
        const int result = std::stoi(value, &pos);
        if (pos != value.size()) {
            throw std::invalid_argument("trailing characters");
        }
        return result;
    } catch (const std::exception&) {
        throw SerializationException("invalid integer for " + field + ": " + value);
    }
}

double ParseDouble(const std::string& value, const std::string& field) {
    try {
        size_t pos = 0;
        const double result = std::stod(value, &pos);
        if (pos != value.size() || !std::isfinite(result)) {
            throw std::invalid_argument("invalid double");
        }
        return result;
    } catch (const std::exception&) {
        throw SerializationException("invalid number for " + field + ": " + value);
    }
}

void ValidateIntRange(int value, int min, int max, const std::string& field) {
    if (value < min || value > max) {
        throw SerializationException(field + " out of range");
    }
}

void ValidateDoubleRange(double value, double min, double max, const std::string& field) {
    if (!std::isfinite(value) || value < min || value > max) {
        throw SerializationException(field + " out of range");
    }
}

std::string PayloadToText(const QJsonObject& payload) {
    const QByteArray bytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    return std::string(bytes.constData(), static_cast<size_t>(bytes.size()));
}

QJsonObject PayloadFromText(const std::string& text) {
    QJsonParseError error;
    const QByteArray bytes(text.data(), static_cast<int>(text.size()));
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        throw SerializationException("invalid vehicle payload JSON");
    }
    return document.object();
}

QJsonValue RequireValue(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (value.isUndefined()) {
        throw SerializationException("missing JSON field: " + key.toStdString());
    }
    return value;
}

std::string RequireString(const QJsonObject& object, const QString& key) {
    const QJsonValue value = RequireValue(object, key);
    if (!value.isString()) {
        throw SerializationException("JSON field must be string: " + key.toStdString());
    }
    return FromQString(value.toString());
}

int RequireInt(const QJsonObject& object, const QString& key) {
    const QJsonValue value = RequireValue(object, key);
    if (!value.isDouble()) {
        throw SerializationException("JSON field must be integer: " + key.toStdString());
    }
    return value.toInt();
}

double RequireDouble(const QJsonObject& object, const QString& key) {
    const QJsonValue value = RequireValue(object, key);
    if (!value.isDouble()) {
        throw SerializationException("JSON field must be number: " + key.toStdString());
    }
    const double result = value.toDouble();
    if (!std::isfinite(result)) {
        throw SerializationException("JSON field must be finite: " + key.toStdString());
    }
    return result;
}

bool RequireBool(const QJsonObject& object, const QString& key) {
    const QJsonValue value = RequireValue(object, key);
    if (!value.isBool()) {
        throw SerializationException("JSON field must be bool: " + key.toStdString());
    }
    return value.toBool();
}

QJsonObject RequireObject(const QJsonObject& object, const QString& key) {
    const QJsonValue value = RequireValue(object, key);
    if (!value.isObject()) {
        throw SerializationException("JSON field must be object: " + key.toStdString());
    }
    return value.toObject();
}

QJsonArray RequireArray(const QJsonObject& object, const QString& key) {
    const QJsonValue value = RequireValue(object, key);
    if (!value.isArray()) {
        throw SerializationException("JSON field must be array: " + key.toStdString());
    }
    return value.toArray();
}

int OptionalDriverId(const QJsonObject& object) {
    const QJsonValue value = object.value("driver_id");
    if (value.isUndefined() || value.isNull()) {
        return 0;
    }
    if (!value.isDouble()) {
        throw SerializationException("JSON field must be integer or null: driver_id");
    }
    return value.toInt();
}

QJsonObject LegacyPayloadFromCustom(const std::string& type_id,
                                    const std::vector<std::string>& fields) {
    QJsonObject payload;
    if (type_id == "builtin.car") {
        if (fields.size() != 11) {
            throw SerializationException("Car VEHICLE line must have 11 fields");
        }
        payload["seat_count"] = ParseInt(fields[10], "seat_count");
    } else if (type_id == "builtin.truck") {
        if (fields.size() != 11) {
            throw SerializationException("Truck VEHICLE line must have 11 fields");
        }
        payload["cargo_capacity"] = ParseDouble(fields[10], "cargo_capacity");
    } else if (type_id == "builtin.boat") {
        if (fields.size() != 12) {
            throw SerializationException("Boat VEHICLE line must have 12 fields");
        }
        payload["displacement"] = ParseDouble(fields[10], "displacement");
        payload["passenger_capacity"] = ParseInt(fields[11], "passenger_capacity");
    } else if (type_id == "builtin.submarine") {
        if (fields.size() != 12) {
            throw SerializationException("Submarine VEHICLE line must have 12 fields");
        }
        payload["displacement"] = ParseDouble(fields[10], "displacement");
        payload["max_depth"] = ParseDouble(fields[11], "max_depth");
    } else {
        throw SerializationException("unknown legacy vehicle type: " + type_id);
    }
    return payload;
}

QJsonObject LegacyPayloadFromJson(const std::string& type_id,
                                  const QJsonObject& object) {
    QJsonObject payload;
    if (type_id == "builtin.car") {
        payload["seat_count"] = RequireInt(object, "seat_count");
    } else if (type_id == "builtin.truck") {
        payload["cargo_capacity"] = RequireDouble(object, "cargo_capacity");
    } else if (type_id == "builtin.boat") {
        payload["displacement"] = RequireDouble(object, "displacement");
        payload["passenger_capacity"] = RequireInt(object, "passenger_capacity");
    } else if (type_id == "builtin.submarine") {
        payload["displacement"] = RequireDouble(object, "displacement");
        payload["max_depth"] = RequireDouble(object, "max_depth");
    } else {
        throw SerializationException("missing payload for plugin vehicle type: " + type_id);
    }
    return payload;
}

const VehicleTypeRegistration& RequireTypeRegistration(const std::string& type_id) {
    const auto* registration = VehicleTypeRegistry::Instance().Find(type_id);
    if (!registration) {
        throw SerializationException(
            "vehicle type is not registered: " + type_id +
            ". Load the required plugin from the plugins folder");
    }
    return *registration;
}

const VehicleTypeRegistration& RequireCreatableTypeRegistration(const std::string& type_id) {
    const auto& registration = RequireTypeRegistration(type_id);
    if (!registration.enabled) {
        throw SerializationException(
            "vehicle type is temporarily disabled: " + type_id +
            ". Finish unloading or reload the plugin before deserialization");
    }
    return registration;
}

void ValidateCommonVehicleRecord(const VehicleRecord& record) {
    ValidateIntRange(record.id, 1, static_cast<int>(kMaxSerializedVehicles), "vehicle.id");
    ValidateDoubleRange(record.data.max_speed, 1.0, kMaxSpeed, "max_speed");
    ValidateDoubleRange(record.data.current_speed, 0.0, record.data.max_speed,
                        "current_speed");
    ValidateDoubleRange(record.data.engine_horsepower, 1.0, kMaxEngineHorsepower,
                        "engine.horsepower");
    ValidateIntRange(record.data.driver_id, 0, static_cast<int>(kMaxSerializedDrivers),
                     "driver_id");
}

QJsonObject EngineToJson(const Engine& engine) {
    QJsonObject object;
    object["horsepower"] = engine.GetHorsepower();
    object["fuel_type"] = ToQString(FuelToCode(engine.GetFuelType()));
    object["is_running"] = engine.IsRunning();
    return object;
}

void ReadEngineFromJson(const QJsonObject& object, VehicleSerializationData& data) {
    data.engine_horsepower = RequireDouble(object, "horsepower");
    data.fuel_type = FuelFromCode(RequireString(object, "fuel_type"));
    data.engine_running = RequireBool(object, "is_running");
}

std::vector<const Driver*> CollectDrivers(
    const Fleet& fleet,
    const std::vector<std::unique_ptr<Driver>>& drivers,
    std::unordered_map<const Driver*, int>& driver_ids) {
    std::vector<const Driver*> ordered;
    const auto add_driver = [&](const Driver* driver) {
        if (!driver || driver_ids.find(driver) != driver_ids.end()) {
            return;
        }
        const int id = static_cast<int>(driver_ids.size() + 1);
        driver_ids.emplace(driver, id);
        ordered.push_back(driver);
    };

    for (const auto& driver : drivers) {
        add_driver(driver.get());
    }
    for (size_t i = 0; i < fleet.Size(); ++i) {
        add_driver(fleet[i].GetDriver());
    }
    return ordered;
}

void WriteDriverCustom(std::ostream& out, const Driver& driver, int id) {
    out << "DRIVER|" << id
        << '|' << EscapeText(driver.GetName())
        << '|' << driver.GetExperienceYears()
        << '\n';
}

DriverRecord DriverFromCustom(const std::vector<std::string>& fields, int line_number) {
    if (fields.size() != 4) {
        throw SerializationException("invalid DRIVER line at " + std::to_string(line_number));
    }
    DriverRecord record;
    record.id = ParseInt(fields[1], "driver.id");
    record.name = UnescapeText(fields[2]);
    record.experience_years = ParseInt(fields[3], "experience_years");
    ValidateIntRange(record.id, 1, static_cast<int>(kMaxSerializedDrivers), "driver.id");
    ValidateIntRange(record.experience_years, 0, kMaxDriverExperienceYears,
                     "experience_years");
    return record;
}

QJsonObject DriverToJson(const Driver& driver, int id) {
    QJsonObject object;
    object["id"] = id;
    object["name"] = ToQString(driver.GetName());
    object["experience_years"] = driver.GetExperienceYears();
    return object;
}

DriverRecord DriverFromJson(const QJsonObject& object) {
    DriverRecord record;
    record.id = RequireInt(object, "id");
    record.name = RequireString(object, "name");
    record.experience_years = RequireInt(object, "experience_years");
    ValidateIntRange(record.id, 1, static_cast<int>(kMaxSerializedDrivers), "driver.id");
    ValidateIntRange(record.experience_years, 0, kMaxDriverExperienceYears,
                     "experience_years");
    return record;
}

void WriteVehicleCustom(std::ostream& out, const Vehicle& vehicle, int id,
                        const std::unordered_map<const Driver*, int>& driver_ids) {
    const auto& registration = RequireTypeRegistration(vehicle.GetTypeId());
    const Driver* driver = vehicle.GetDriver();
    const QJsonObject payload = registration.serialize_payload(vehicle);
    out << "VEHICLE|" << id
        << '|' << registration.type_id
        << '|' << EscapeText(vehicle.GetName())
        << '|' << vehicle.GetMaxSpeed()
        << '|' << vehicle.GetCurrentSpeed()
        << '|' << vehicle.GetEngine().GetHorsepower()
        << '|' << FuelToCode(vehicle.GetEngine().GetFuelType())
        << '|' << (vehicle.GetEngine().IsRunning() ? 1 : 0)
        << '|' << (driver ? driver_ids.at(driver) : 0)
        << '|' << EscapeText(PayloadToText(payload))
        << '\n';
}

VehicleRecord VehicleFromCustom(const std::vector<std::string>& fields, int line_number) {
    if (fields.size() < 11) {
        throw SerializationException("invalid VEHICLE line at " + std::to_string(line_number));
    }

    const std::string raw_type = fields[2];
    const std::string type_id = CanonicalTypeId(raw_type);

    VehicleRecord record;
    record.id = ParseInt(fields[1], "vehicle.id");
    record.type_id = type_id;
    record.data.name = UnescapeText(fields[3]);
    record.data.max_speed = ParseDouble(fields[4], "max_speed");
    record.data.current_speed = ParseDouble(fields[5], "current_speed");
    record.data.engine_horsepower = ParseDouble(fields[6], "engine.horsepower");
    record.data.fuel_type = FuelFromCode(fields[7]);
    record.data.engine_running = ParseInt(fields[8], "engine.is_running") != 0;
    record.data.driver_id = ParseInt(fields[9], "driver_id");

    if (raw_type == type_id) {
        if (fields.size() != 11) {
            throw SerializationException("plugin VEHICLE line must have 11 fields");
        }
        record.data.payload = PayloadFromText(UnescapeText(fields[10]));
    } else {
        record.data.payload = LegacyPayloadFromCustom(type_id, fields);
    }

    ValidateCommonVehicleRecord(record);
    RequireTypeRegistration(type_id);
    return record;
}

QJsonObject VehicleToJson(const Vehicle& vehicle, int id,
                          const std::unordered_map<const Driver*, int>& driver_ids) {
    const auto& registration = RequireTypeRegistration(vehicle.GetTypeId());
    QJsonObject object;
    object["id"] = id;
    object["type_id"] = ToQString(registration.type_id);
    object["type_name"] = ToQString(registration.display_name);
    object["name"] = ToQString(vehicle.GetName());
    object["max_speed"] = vehicle.GetMaxSpeed();
    object["current_speed"] = vehicle.GetCurrentSpeed();
    object["engine"] = EngineToJson(vehicle.GetEngine());

    const Driver* driver = vehicle.GetDriver();
    object["driver_id"] = driver ? QJsonValue(driver_ids.at(driver))
                                  : QJsonValue(QJsonValue::Null);
    object["payload"] = registration.serialize_payload(vehicle);
    return object;
}

VehicleRecord VehicleFromJson(const QJsonObject& object) {
    VehicleRecord record;
    record.id = RequireInt(object, "id");

    if (!object.value("type_id").isUndefined()) {
        record.type_id = CanonicalTypeId(RequireString(object, "type_id"));
    } else {
        record.type_id = CanonicalTypeId(RequireString(object, "type"));
    }

    record.data.name = RequireString(object, "name");
    record.data.max_speed = RequireDouble(object, "max_speed");
    record.data.current_speed = RequireDouble(object, "current_speed");
    record.data.driver_id = OptionalDriverId(object);
    ReadEngineFromJson(RequireObject(object, "engine"), record.data);

    if (object.value("payload").isObject()) {
        record.data.payload = object.value("payload").toObject();
    } else {
        record.data.payload = LegacyPayloadFromJson(record.type_id, object);
    }

    ValidateCommonVehicleRecord(record);
    RequireTypeRegistration(record.type_id);
    return record;
}

class FleetRestorer {
public:
    void AddDriver(VehicleSerializer::FleetSnapshot& snapshot, const DriverRecord& record) {
        if (drivers_by_id_.find(record.id) != drivers_by_id_.end()) {
            throw SerializationException("duplicate driver id: " + std::to_string(record.id));
        }
        auto driver = std::make_unique<Driver>(record.name, record.experience_years);
        drivers_by_id_.emplace(record.id, driver.get());
        snapshot.drivers.push_back(std::move(driver));
    }

    void AddVehicle(VehicleSerializer::FleetSnapshot& snapshot, const VehicleRecord& record) {
        if (snapshot.fleet.Size() >= kMaxSerializedVehicles) {
            throw SerializationException("too many vehicles in serialized data");
        }
        if (vehicle_ids_.find(record.id) != vehicle_ids_.end()) {
            throw SerializationException("duplicate vehicle id: " + std::to_string(record.id));
        }
        vehicle_ids_.insert(record.id);

        const auto& registration = RequireCreatableTypeRegistration(record.type_id);
        auto vehicle = registration.deserialize(record.data);
        if (!vehicle) {
            throw SerializationException("vehicle deserializer returned null: " + record.type_id);
        }
        if (vehicle->GetTypeId() != record.type_id) {
            throw SerializationException("vehicle deserializer returned wrong type: " + record.type_id);
        }

        if (record.data.driver_id != 0) {
            const auto driver_it = drivers_by_id_.find(record.data.driver_id);
            if (driver_it == drivers_by_id_.end()) {
                throw SerializationException("vehicle references unknown driver id: " +
                                             std::to_string(record.data.driver_id));
            }
            vehicle->AssignDriver(driver_it->second);
        }
        if (record.data.engine_running) {
            vehicle->GetEngine().Start();
        }
        vehicle->SetCurrentSpeed(record.data.current_speed);
        snapshot.fleet.Add(std::move(vehicle));
    }

private:
    std::unordered_map<int, Driver*> drivers_by_id_;
    std::unordered_set<int> vehicle_ids_;
};

VehicleSerializer::FleetSnapshot RestoreSnapshot(
    const std::vector<DriverRecord>& driver_records,
    const std::vector<VehicleRecord>& vehicle_records) {
    VehicleSerializer::FleetSnapshot snapshot;
    FleetRestorer restorer;
    for (const DriverRecord& driver : driver_records) {
        restorer.AddDriver(snapshot, driver);
    }
    for (const VehicleRecord& vehicle : vehicle_records) {
        restorer.AddVehicle(snapshot, vehicle);
    }
    return snapshot;
}

} // namespace

namespace VehicleSerializer {

std::string SerializeCustom(const Fleet& fleet,
                            const std::vector<std::unique_ptr<Driver>>& drivers) {
    std::unordered_map<const Driver*, int> driver_ids;
    const std::vector<const Driver*> ordered_drivers =
        CollectDrivers(fleet, drivers, driver_ids);

    std::ostringstream out;
    out << std::setprecision(15);
    out << kCustomHeader << '\n';
    for (const Driver* driver : ordered_drivers) {
        WriteDriverCustom(out, *driver, driver_ids.at(driver));
    }
    for (size_t i = 0; i < fleet.Size(); ++i) {
        WriteVehicleCustom(out, fleet[i], static_cast<int>(i + 1), driver_ids);
    }
    out << "END\n";
    return out.str();
}

FleetSnapshot DeserializeCustom(const std::string& text) {
    std::istringstream input(text);
    std::string line;
    int line_number = 0;
    bool seen_header = false;
    bool seen_end = false;
    std::vector<DriverRecord> drivers;
    std::vector<VehicleRecord> vehicles;

    while (std::getline(input, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (!seen_header) {
            if (line != kCustomHeader) {
                throw SerializationException("custom text must start with FLEET_CUSTOM_V1");
            }
            seen_header = true;
            continue;
        }
        if (line == "END") {
            seen_end = true;
            break;
        }

        const std::vector<std::string> fields = SplitFields(line);
        if (fields.empty()) {
            continue;
        }
        if (fields[0] == "DRIVER") {
            drivers.push_back(DriverFromCustom(fields, line_number));
        } else if (fields[0] == "VEHICLE") {
            vehicles.push_back(VehicleFromCustom(fields, line_number));
        } else {
            throw SerializationException("unknown custom record at line " +
                                         std::to_string(line_number));
        }
    }

    if (!seen_header) {
        throw SerializationException("custom text is empty");
    }
    if (!seen_end) {
        throw SerializationException("custom text must end with END");
    }

    return RestoreSnapshot(drivers, vehicles);
}

std::string SerializeJson(const Fleet& fleet,
                          const std::vector<std::unique_ptr<Driver>>& drivers) {
    std::unordered_map<const Driver*, int> driver_ids;
    const std::vector<const Driver*> ordered_drivers =
        CollectDrivers(fleet, drivers, driver_ids);

    QJsonArray drivers_array;
    for (const Driver* driver : ordered_drivers) {
        drivers_array.append(DriverToJson(*driver, driver_ids.at(driver)));
    }

    QJsonArray vehicles_array;
    for (size_t i = 0; i < fleet.Size(); ++i) {
        vehicles_array.append(VehicleToJson(fleet[i], static_cast<int>(i + 1), driver_ids));
    }

    QJsonObject data;
    data["drivers"] = drivers_array;
    data["vehicles"] = vehicles_array;

    QJsonObject root;
    root["format"] = kJsonFormat;
    root["version"] = 2;
    root["data"] = data;

    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    return std::string(bytes.constData(), static_cast<size_t>(bytes.size()));
}

FleetSnapshot DeserializeJson(const std::string& json) {
    QJsonParseError error;
    const QByteArray bytes(json.data(), static_cast<int>(json.size()));
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError) {
        throw SerializationException("JSON parse error: " + FromQString(error.errorString()));
    }
    if (!document.isObject()) {
        throw SerializationException("JSON root must be an object");
    }

    const QJsonObject root = document.object();
    if (RequireString(root, "format") != kJsonFormat) {
        throw SerializationException("unsupported JSON format");
    }

    const QJsonObject data = RequireObject(root, "data");
    const QJsonArray driver_array = RequireArray(data, "drivers");
    const QJsonArray vehicle_array = RequireArray(data, "vehicles");

    if (static_cast<size_t>(driver_array.size()) > kMaxSerializedDrivers) {
        throw SerializationException("too many drivers in JSON");
    }
    if (static_cast<size_t>(vehicle_array.size()) > kMaxSerializedVehicles) {
        throw SerializationException("too many vehicles in JSON");
    }

    std::vector<DriverRecord> drivers;
    std::vector<VehicleRecord> vehicles;
    for (const QJsonValue& value : driver_array) {
        if (!value.isObject()) {
            throw SerializationException("driver JSON item must be object");
        }
        drivers.push_back(DriverFromJson(value.toObject()));
    }
    for (const QJsonValue& value : vehicle_array) {
        if (!value.isObject()) {
            throw SerializationException("vehicle JSON item must be object");
        }
        vehicles.push_back(VehicleFromJson(value.toObject()));
    }

    return RestoreSnapshot(drivers, vehicles);
}

} // namespace VehicleSerializer
