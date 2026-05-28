#include "vehicle_serializer.h"

#include "boat.h"
#include "car.h"
#include "exceptions.h"
#include "submarine.h"
#include "truck.h"

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
#include <memory>
#include <sstream>
#include <stdexcept>
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
constexpr int kMaxSeatCount = 1000;
constexpr int kMaxPassengerCapacity = 100000;
constexpr double kMaxSpeed = 10000.0;
constexpr double kMaxEngineHorsepower = 100000.0;
constexpr double kMaxCargoCapacity = 10000.0;
constexpr double kMaxDisplacement = 1000000.0;
constexpr double kMaxDepth = 20000.0;

struct DriverRecord {
    int id = 0;
    std::string name;
    int experience_years = 0;
};

struct EngineRecord {
    double horsepower = 0.0;
    Engine::FuelType fuel_type = Engine::FuelType::kPetrol;
    bool is_running = false;
};

struct VehicleRecord {
    int id = 0;
    std::string type;
    std::string name;
    double max_speed = 0.0;
    double current_speed = 0.0;
    EngineRecord engine;
    int driver_id = 0;

    int seat_count = 0;
    double cargo_capacity = 0.0;
    double displacement = 0.0;
    int passenger_capacity = 0;
    double max_depth = 0.0;
};

QString ToQString(const std::string& value) {
    return QString::fromUtf8(value.data(), static_cast<int>(value.size()));
}

std::string FromQString(const QString& value) {
    const QByteArray bytes = value.toUtf8();
    return std::string(bytes.constData(), static_cast<size_t>(bytes.size()));
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
    std::string field;
    std::istringstream input(line);
    while (std::getline(input, field, '|')) {
        fields.push_back(field);
    }
    if (!line.empty() && line.back() == '|') {
        fields.emplace_back();
    }
    return fields;
}

int ParseInt(const std::string& value, const std::string& field_name) {
    try {
        size_t parsed = 0;
        const long number = std::stol(value, &parsed);
        if (parsed != value.size() ||
            number < std::numeric_limits<int>::min() ||
            number > std::numeric_limits<int>::max()) {
            throw std::invalid_argument("not int");
        }
        return static_cast<int>(number);
    } catch (const std::exception&) {
        throw SerializationException("invalid integer field '" + field_name + "': " + value);
    }
}

double ParseDouble(const std::string& value, const std::string& field_name) {
    try {
        size_t parsed = 0;
        const double number = std::stod(value, &parsed);
        if (parsed != value.size() || !std::isfinite(number)) {
            throw std::invalid_argument("not finite double");
        }
        return number;
    } catch (const std::exception&) {
        throw SerializationException("invalid number field '" + field_name + "': " + value);
    }
}

bool ParseBool(const std::string& value, const std::string& field_name) {
    if (value == "1" || value == "true") return true;
    if (value == "0" || value == "false") return false;
    throw SerializationException("invalid bool field '" + field_name + "': " + value);
}

void EnsurePositiveId(int id, const std::string& kind) {
    if (id <= 0) {
        throw SerializationException(kind + " id must be positive");
    }
}

void ValidateIntRange(int value, int min_value, int max_value,
                      const std::string& field_name) {
    if (value < min_value || value > max_value) {
        throw SerializationException(
            field_name + " must be in range [" + std::to_string(min_value) +
            ", " + std::to_string(max_value) + "]");
    }
}

void ValidateDoubleRange(double value, double min_value, double max_value,
                         const std::string& field_name) {
    if (!std::isfinite(value) || value < min_value || value > max_value) {
        throw SerializationException(
            field_name + " must be in range [" + std::to_string(min_value) +
            ", " + std::to_string(max_value) + "]");
    }
}

QJsonArray RequireArray(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (!value.isArray()) {
        throw SerializationException("JSON field '" + FromQString(key) + "' must be an array");
    }
    return value.toArray();
}

QJsonObject RequireObject(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (!value.isObject()) {
        throw SerializationException("JSON field '" + FromQString(key) + "' must be an object");
    }
    return value.toObject();
}

QJsonObject RequireObject(const QJsonValue& value, const std::string& context) {
    if (!value.isObject()) {
        throw SerializationException("JSON value '" + context + "' must be an object");
    }
    return value.toObject();
}

std::string RequireString(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (!value.isString()) {
        throw SerializationException("JSON field '" + FromQString(key) + "' must be a string");
    }
    return FromQString(value.toString());
}

double RequireDouble(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (!value.isDouble() || !std::isfinite(value.toDouble())) {
        throw SerializationException("JSON field '" + FromQString(key) + "' must be a finite number");
    }
    return value.toDouble();
}

int RequireInt(const QJsonObject& object, const QString& key) {
    const double value = RequireDouble(object, key);
    if (std::floor(value) != value ||
        value < std::numeric_limits<int>::min() ||
        value > std::numeric_limits<int>::max()) {
        throw SerializationException("JSON field '" + FromQString(key) + "' must be an integer");
    }
    return static_cast<int>(value);
}

bool RequireBool(const QJsonObject& object, const QString& key) {
    const QJsonValue value = object.value(key);
    if (!value.isBool()) {
        throw SerializationException("JSON field '" + FromQString(key) + "' must be a bool");
    }
    return value.toBool();
}

class ISerializationContract {
public:
    virtual ~ISerializationContract() = default;
    virtual std::string ContractName() const = 0;
};

class EngineContract : public ISerializationContract {
public:
    std::string ContractName() const override { return "Engine"; }

    EngineRecord FromJson(const QJsonObject& object) const {
        EngineRecord record;
        record.horsepower = RequireDouble(object, "horsepower");
        record.fuel_type = FuelFromCode(RequireString(object, "fuel_type"));
        record.is_running = RequireBool(object, "is_running");
        return record;
    }

    EngineRecord FromCustom(const std::vector<std::string>& fields) const {
        EngineRecord record;
        record.horsepower = ParseDouble(fields[6], "engine_hp");
        record.fuel_type = FuelFromCode(fields[7]);
        record.is_running = ParseBool(fields[8], "engine_running");
        return record;
    }

    QJsonObject ToJson(const Engine& engine) const {
        QJsonObject object;
        object["horsepower"] = engine.GetHorsepower();
        object["fuel_type"] = ToQString(FuelToCode(engine.GetFuelType()));
        object["is_running"] = engine.IsRunning();
        return object;
    }
};

const EngineContract& EngineDataContract() {
    static const EngineContract contract;
    return contract;
}

class DriverContract : public ISerializationContract {
public:
    std::string ContractName() const override { return "Driver"; }

    std::vector<const Driver*> CollectReferencedDrivers(
        const Fleet& fleet,
        const std::vector<std::unique_ptr<Driver>>& drivers,
        std::unordered_map<const Driver*, int>& driver_ids) const {
        std::vector<const Driver*> ordered_drivers;
        auto add_driver = [&](const Driver* driver) {
            if (!driver || driver_ids.find(driver) != driver_ids.end()) {
                return;
            }
            const int id = static_cast<int>(ordered_drivers.size()) + 1;
            driver_ids.emplace(driver, id);
            ordered_drivers.push_back(driver);
        };

        for (const auto& driver : drivers) {
            add_driver(driver.get());
        }
        for (size_t i = 0; i < fleet.Size(); ++i) {
            add_driver(fleet[i].GetDriver());
        }

        return ordered_drivers;
    }

    void WriteCustom(std::ostream& out, const Driver& driver, int id) const {
        out << "DRIVER|" << id
            << '|' << EscapeText(driver.GetName())
            << '|' << driver.GetExperienceYears() << '\n';
    }

    DriverRecord FromCustom(const std::vector<std::string>& fields, int line_number) const {
        if (fields.size() != 4) {
            throw SerializationException("invalid DRIVER line at " + std::to_string(line_number));
        }

        DriverRecord record;
        record.id = ParseInt(fields[1], "driver.id");
        record.name = UnescapeText(fields[2]);
        record.experience_years = ParseInt(fields[3], "experience_years");
        return record;
    }

    QJsonObject ToJson(const Driver& driver, int id) const {
        QJsonObject object;
        object["id"] = id;
        object["name"] = ToQString(driver.GetName());
        object["experience_years"] = driver.GetExperienceYears();
        return object;
    }

    DriverRecord FromJson(const QJsonObject& object) const {
        DriverRecord record;
        record.id = RequireInt(object, "id");
        record.name = RequireString(object, "name");
        record.experience_years = RequireInt(object, "experience_years");
        return record;
    }

    void Restore(VehicleSerializer::FleetSnapshot& snapshot,
                 std::unordered_map<int, Driver*>& drivers_by_id,
                 const DriverRecord& record) const {
        EnsurePositiveId(record.id, "Driver");
        if (snapshot.drivers.size() >= kMaxSerializedDrivers) {
            throw SerializationException("too many drivers in serialized data");
        }
        ValidateIntRange(record.experience_years, 0, kMaxDriverExperienceYears,
                         "experience_years");
        if (drivers_by_id.find(record.id) != drivers_by_id.end()) {
            throw SerializationException("duplicate driver id: " + std::to_string(record.id));
        }

        auto driver = std::make_unique<Driver>(record.name, record.experience_years);
        drivers_by_id.emplace(record.id, driver.get());
        snapshot.drivers.push_back(std::move(driver));
    }
};

const DriverContract& DriverDataContract() {
    static const DriverContract contract;
    return contract;
}

class VehicleContract : public ISerializationContract {
public:
    virtual ~VehicleContract() = default;

    std::string ContractName() const override { return Type(); }

    virtual std::string Type() const = 0;
    virtual bool Matches(const Vehicle& vehicle) const = 0;
    virtual void WriteCustomSpecific(std::ostream& out, const Vehicle& vehicle) const = 0;
    virtual void WriteJsonSpecific(QJsonObject& object, const Vehicle& vehicle) const = 0;
    virtual void ReadCustomSpecific(const std::vector<std::string>& fields,
                                    VehicleRecord& record) const = 0;
    virtual void ReadJsonSpecific(const QJsonObject& object,
                                  VehicleRecord& record) const = 0;
    virtual std::unique_ptr<Vehicle> Create(const VehicleRecord& record) const = 0;

    void WriteCustom(std::ostream& out, const Vehicle& vehicle, int id,
                     const std::unordered_map<const Driver*, int>& driver_ids) const {
        const Driver* driver = vehicle.GetDriver();
        out << "VEHICLE|" << id
            << '|' << Type()
            << '|' << EscapeText(vehicle.GetName())
            << '|' << vehicle.GetMaxSpeed()
            << '|' << vehicle.GetCurrentSpeed()
            << '|' << vehicle.GetEngine().GetHorsepower()
            << '|' << FuelToCode(vehicle.GetEngine().GetFuelType())
            << '|' << (vehicle.GetEngine().IsRunning() ? 1 : 0)
            << '|' << (driver ? driver_ids.at(driver) : 0);
        WriteCustomSpecific(out, vehicle);
        out << '\n';
    }

    VehicleRecord FromCustom(const std::vector<std::string>& fields,
                             int line_number) const {
        if (fields.size() < 11) {
            throw SerializationException("invalid VEHICLE line at " + std::to_string(line_number));
        }

        VehicleRecord record;
        record.id = ParseInt(fields[1], "vehicle.id");
        record.type = fields[2];
        record.name = UnescapeText(fields[3]);
        record.max_speed = ParseDouble(fields[4], "max_speed");
        record.current_speed = ParseDouble(fields[5], "current_speed");
        record.engine = EngineDataContract().FromCustom(fields);
        record.driver_id = ParseInt(fields[9], "driver_id");
        ReadCustomSpecific(fields, record);
        return record;
    }

    QJsonObject ToJson(const Vehicle& vehicle, int id,
                       const std::unordered_map<const Driver*, int>& driver_ids) const {
        QJsonObject object;
        object["id"] = id;
        object["type"] = ToQString(Type());
        object["name"] = ToQString(vehicle.GetName());
        object["max_speed"] = vehicle.GetMaxSpeed();
        object["current_speed"] = vehicle.GetCurrentSpeed();
        object["engine"] = EngineDataContract().ToJson(vehicle.GetEngine());

        const Driver* driver = vehicle.GetDriver();
        if (driver) {
            object["driver_id"] = driver_ids.at(driver);
        } else {
            object["driver_id"] = QJsonValue(QJsonValue::Null);
        }

        WriteJsonSpecific(object, vehicle);
        return object;
    }

    VehicleRecord FromJson(const QJsonObject& object) const {
        VehicleRecord record;
        record.id = RequireInt(object, "id");
        record.type = RequireString(object, "type");
        record.name = RequireString(object, "name");
        record.max_speed = RequireDouble(object, "max_speed");
        record.current_speed = RequireDouble(object, "current_speed");
        record.driver_id = OptionalDriverId(object);
        record.engine = EngineDataContract().FromJson(RequireObject(object, "engine"));
        ReadJsonSpecific(object, record);
        return record;
    }

    std::unique_ptr<Vehicle> Restore(
        const VehicleRecord& record,
        const std::unordered_map<int, Driver*>& drivers_by_id) const {
        EnsurePositiveId(record.id, "Vehicle");
        Validate(record);

        auto vehicle = Create(record);
        if (record.driver_id != 0) {
            const auto driver_it = drivers_by_id.find(record.driver_id);
            if (driver_it == drivers_by_id.end()) {
                throw SerializationException("vehicle references unknown driver id: " +
                                             std::to_string(record.driver_id));
            }
            vehicle->AssignDriver(driver_it->second);
        }
        if (record.engine.is_running) {
            vehicle->GetEngine().Start();
        }
        vehicle->SetCurrentSpeed(record.current_speed);

        return vehicle;
    }

protected:
    template <typename T>
    const T& Cast(const Vehicle& vehicle) const {
        const auto* typed = dynamic_cast<const T*>(&vehicle);
        if (!typed) {
            throw SerializationException("vehicle object does not match contract " + Type());
        }
        return *typed;
    }

private:
    int OptionalDriverId(const QJsonObject& object) const {
        const QJsonValue value = object.value("driver_id");
        if (value.isUndefined() || value.isNull()) {
            return 0;
        }
        return RequireInt(object, "driver_id");
    }

    void Validate(const VehicleRecord& record) const {
        ValidateDoubleRange(record.max_speed, 1.0, kMaxSpeed, "max_speed");
        ValidateDoubleRange(record.current_speed, 0.0, record.max_speed, "current_speed");
        ValidateDoubleRange(record.engine.horsepower, 1.0, kMaxEngineHorsepower,
                            "engine.horsepower");
        ValidateIntRange(record.driver_id, 0, static_cast<int>(kMaxSerializedDrivers),
                         "driver_id");

        if (record.type == "Car") {
            ValidateIntRange(record.seat_count, 1, kMaxSeatCount, "seat_count");
        } else if (record.type == "Truck") {
            ValidateDoubleRange(record.cargo_capacity, 0.1, kMaxCargoCapacity,
                                "cargo_capacity");
        } else if (record.type == "Boat") {
            ValidateDoubleRange(record.displacement, 0.1, kMaxDisplacement,
                                "displacement");
            ValidateIntRange(record.passenger_capacity, 1, kMaxPassengerCapacity,
                             "passenger_capacity");
        } else if (record.type == "Submarine") {
            ValidateDoubleRange(record.displacement, 0.1, kMaxDisplacement,
                                "displacement");
            ValidateDoubleRange(record.max_depth, 0.1, kMaxDepth, "max_depth");
        } else {
            throw SerializationException("unknown vehicle type: " + record.type);
        }
    }
};

class CarContract final : public VehicleContract {
public:
    std::string Type() const override { return "Car"; }
    bool Matches(const Vehicle& vehicle) const override {
        return dynamic_cast<const Car*>(&vehicle) != nullptr;
    }
    void WriteCustomSpecific(std::ostream& out, const Vehicle& vehicle) const override {
        out << '|' << Cast<Car>(vehicle).GetSeatCount();
    }
    void WriteJsonSpecific(QJsonObject& object, const Vehicle& vehicle) const override {
        object["seat_count"] = Cast<Car>(vehicle).GetSeatCount();
    }
    void ReadCustomSpecific(const std::vector<std::string>& fields,
                            VehicleRecord& record) const override {
        if (fields.size() != 11) {
            throw SerializationException("Car VEHICLE line must have 11 fields");
        }
        record.seat_count = ParseInt(fields[10], "seat_count");
    }
    void ReadJsonSpecific(const QJsonObject& object, VehicleRecord& record) const override {
        record.seat_count = RequireInt(object, "seat_count");
    }
    std::unique_ptr<Vehicle> Create(const VehicleRecord& record) const override {
        return std::make_unique<Car>(record.name, record.max_speed,
                                     record.engine.horsepower, record.engine.fuel_type,
                                     record.seat_count);
    }
};

class TruckContract final : public VehicleContract {
public:
    std::string Type() const override { return "Truck"; }
    bool Matches(const Vehicle& vehicle) const override {
        return dynamic_cast<const Truck*>(&vehicle) != nullptr;
    }
    void WriteCustomSpecific(std::ostream& out, const Vehicle& vehicle) const override {
        out << '|' << Cast<Truck>(vehicle).GetCargoCapacity();
    }
    void WriteJsonSpecific(QJsonObject& object, const Vehicle& vehicle) const override {
        object["cargo_capacity"] = Cast<Truck>(vehicle).GetCargoCapacity();
    }
    void ReadCustomSpecific(const std::vector<std::string>& fields,
                            VehicleRecord& record) const override {
        if (fields.size() != 11) {
            throw SerializationException("Truck VEHICLE line must have 11 fields");
        }
        record.cargo_capacity = ParseDouble(fields[10], "cargo_capacity");
    }
    void ReadJsonSpecific(const QJsonObject& object, VehicleRecord& record) const override {
        record.cargo_capacity = RequireDouble(object, "cargo_capacity");
    }
    std::unique_ptr<Vehicle> Create(const VehicleRecord& record) const override {
        return std::make_unique<Truck>(record.name, record.max_speed,
                                       record.engine.horsepower, record.engine.fuel_type,
                                       record.cargo_capacity);
    }
};

class BoatContract final : public VehicleContract {
public:
    std::string Type() const override { return "Boat"; }
    bool Matches(const Vehicle& vehicle) const override {
        return dynamic_cast<const Boat*>(&vehicle) != nullptr;
    }
    void WriteCustomSpecific(std::ostream& out, const Vehicle& vehicle) const override {
        const auto& boat = Cast<Boat>(vehicle);
        out << '|' << boat.GetDisplacement()
            << '|' << boat.GetPassengerCapacity();
    }
    void WriteJsonSpecific(QJsonObject& object, const Vehicle& vehicle) const override {
        const auto& boat = Cast<Boat>(vehicle);
        object["displacement"] = boat.GetDisplacement();
        object["passenger_capacity"] = boat.GetPassengerCapacity();
    }
    void ReadCustomSpecific(const std::vector<std::string>& fields,
                            VehicleRecord& record) const override {
        if (fields.size() != 12) {
            throw SerializationException("Boat VEHICLE line must have 12 fields");
        }
        record.displacement = ParseDouble(fields[10], "displacement");
        record.passenger_capacity = ParseInt(fields[11], "passenger_capacity");
    }
    void ReadJsonSpecific(const QJsonObject& object, VehicleRecord& record) const override {
        record.displacement = RequireDouble(object, "displacement");
        record.passenger_capacity = RequireInt(object, "passenger_capacity");
    }
    std::unique_ptr<Vehicle> Create(const VehicleRecord& record) const override {
        return std::make_unique<Boat>(record.name, record.max_speed,
                                      record.engine.horsepower, record.engine.fuel_type,
                                      record.displacement, record.passenger_capacity);
    }
};

class SubmarineContract final : public VehicleContract {
public:
    std::string Type() const override { return "Submarine"; }
    bool Matches(const Vehicle& vehicle) const override {
        return dynamic_cast<const Submarine*>(&vehicle) != nullptr;
    }
    void WriteCustomSpecific(std::ostream& out, const Vehicle& vehicle) const override {
        const auto& submarine = Cast<Submarine>(vehicle);
        out << '|' << submarine.GetDisplacement()
            << '|' << submarine.GetMaxDepth();
    }
    void WriteJsonSpecific(QJsonObject& object, const Vehicle& vehicle) const override {
        const auto& submarine = Cast<Submarine>(vehicle);
        object["displacement"] = submarine.GetDisplacement();
        object["max_depth"] = submarine.GetMaxDepth();
    }
    void ReadCustomSpecific(const std::vector<std::string>& fields,
                            VehicleRecord& record) const override {
        if (fields.size() != 12) {
            throw SerializationException("Submarine VEHICLE line must have 12 fields");
        }
        record.displacement = ParseDouble(fields[10], "displacement");
        record.max_depth = ParseDouble(fields[11], "max_depth");
    }
    void ReadJsonSpecific(const QJsonObject& object, VehicleRecord& record) const override {
        record.displacement = RequireDouble(object, "displacement");
        record.max_depth = RequireDouble(object, "max_depth");
    }
    std::unique_ptr<Vehicle> Create(const VehicleRecord& record) const override {
        return std::make_unique<Submarine>(record.name, record.max_speed,
                                           record.engine.horsepower, record.engine.fuel_type,
                                           record.displacement, record.max_depth);
    }
};

const std::vector<std::unique_ptr<VehicleContract>>& VehicleContracts() {
    static const auto contracts = [] {
        std::vector<std::unique_ptr<VehicleContract>> result;
        result.push_back(std::make_unique<CarContract>());
        result.push_back(std::make_unique<TruckContract>());
        result.push_back(std::make_unique<BoatContract>());
        result.push_back(std::make_unique<SubmarineContract>());
        return result;
    }();
    return contracts;
}

const VehicleContract& ContractFor(const Vehicle& vehicle) {
    for (const auto& contract : VehicleContracts()) {
        if (contract->Matches(vehicle)) {
            return *contract;
        }
    }
    throw SerializationException("unsupported vehicle type for serialization");
}

const VehicleContract& ContractForType(const std::string& type) {
    for (const auto& contract : VehicleContracts()) {
        if (contract->Type() == type) {
            return *contract;
        }
    }
    throw SerializationException("unknown vehicle type: " + type);
}

class FleetGraphRestorer {
public:
    void AddDriver(VehicleSerializer::FleetSnapshot& snapshot,
                   const DriverRecord& record) {
        DriverDataContract().Restore(snapshot, drivers_by_id_, record);
    }

    void AddVehicle(VehicleSerializer::FleetSnapshot& snapshot,
                    const VehicleRecord& record) {
        if (snapshot.fleet.Size() >= kMaxSerializedVehicles) {
            throw SerializationException("too many vehicles in serialized data");
        }
        if (vehicle_ids_.find(record.id) != vehicle_ids_.end()) {
            throw SerializationException("duplicate vehicle id: " + std::to_string(record.id));
        }
        vehicle_ids_.insert(record.id);

        snapshot.fleet.Add(ContractForType(record.type).Restore(record, drivers_by_id_));
    }

private:
    std::unordered_map<int, Driver*> drivers_by_id_;
    std::unordered_set<int> vehicle_ids_;
};

class CustomTextFleetSerializer {
public:
    std::string Serialize(const Fleet& fleet,
                          const std::vector<std::unique_ptr<Driver>>& drivers) const {
        std::unordered_map<const Driver*, int> driver_ids;
        const std::vector<const Driver*> ordered_drivers =
            DriverDataContract().CollectReferencedDrivers(fleet, drivers, driver_ids);

        std::ostringstream out;
        out << std::setprecision(15);
        out << kCustomHeader << '\n';
        for (const Driver* driver : ordered_drivers) {
            DriverDataContract().WriteCustom(out, *driver, driver_ids.at(driver));
        }
        for (size_t i = 0; i < fleet.Size(); ++i) {
            const Vehicle& vehicle = fleet[i];
            ContractFor(vehicle).WriteCustom(out, vehicle, static_cast<int>(i + 1), driver_ids);
        }
        out << "END\n";

        return out.str();
    }

    VehicleSerializer::FleetSnapshot Deserialize(const std::string& text) const {
        std::istringstream input(text);
        std::string line;
        int line_number = 0;
        bool seen_header = false;
        bool seen_end = false;

        std::vector<DriverRecord> driver_records;
        std::vector<VehicleRecord> vehicle_records;

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
                driver_records.push_back(DriverDataContract().FromCustom(fields, line_number));
            } else if (fields[0] == "VEHICLE") {
                if (fields.size() < 3) {
                    throw SerializationException("invalid VEHICLE line at " +
                                                 std::to_string(line_number));
                }
                vehicle_records.push_back(
                    ContractForType(fields[2]).FromCustom(fields, line_number));
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

        return Restore(driver_records, vehicle_records);
    }

private:
    VehicleSerializer::FleetSnapshot Restore(
        const std::vector<DriverRecord>& driver_records,
        const std::vector<VehicleRecord>& vehicle_records) const {
        VehicleSerializer::FleetSnapshot snapshot;
        FleetGraphRestorer restorer;
        for (const DriverRecord& record : driver_records) {
            restorer.AddDriver(snapshot, record);
        }
        for (const VehicleRecord& record : vehicle_records) {
            restorer.AddVehicle(snapshot, record);
        }
        return snapshot;
    }
};

class JsonFleetSerializer {
public:
    std::string Serialize(const Fleet& fleet,
                          const std::vector<std::unique_ptr<Driver>>& drivers) const {
        std::unordered_map<const Driver*, int> driver_ids;
        const std::vector<const Driver*> ordered_drivers =
            DriverDataContract().CollectReferencedDrivers(fleet, drivers, driver_ids);

        QJsonArray drivers_array;
        for (const Driver* driver : ordered_drivers) {
            drivers_array.append(DriverDataContract().ToJson(*driver, driver_ids.at(driver)));
        }

        QJsonArray vehicles_array;
        for (size_t i = 0; i < fleet.Size(); ++i) {
            const Vehicle& vehicle = fleet[i];
            vehicles_array.append(
                ContractFor(vehicle).ToJson(vehicle, static_cast<int>(i + 1), driver_ids));
        }

        QJsonObject data;
        data["drivers"] = drivers_array;
        data["vehicles"] = vehicles_array;

        QJsonObject root;
        root["format"] = kJsonFormat;
        root["version"] = 1;
        root["data"] = data;

        const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
        return std::string(bytes.constData(), static_cast<size_t>(bytes.size()));
    }

    VehicleSerializer::FleetSnapshot Deserialize(const std::string& json) const {
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
        if (RequireInt(root, "version") != 1) {
            throw SerializationException("unsupported JSON version");
        }

        const QJsonObject data = RequireObject(root, "data");
        const QJsonArray drivers_array = RequireArray(data, "drivers");
        const QJsonArray vehicles_array = RequireArray(data, "vehicles");

        std::vector<DriverRecord> driver_records;
        driver_records.reserve(static_cast<size_t>(drivers_array.size()));
        for (const QJsonValue& value : drivers_array) {
            driver_records.push_back(
                DriverDataContract().FromJson(RequireObject(value, "driver")));
        }

        std::vector<VehicleRecord> vehicle_records;
        vehicle_records.reserve(static_cast<size_t>(vehicles_array.size()));
        for (const QJsonValue& value : vehicles_array) {
            const QJsonObject object = RequireObject(value, "vehicle");
            vehicle_records.push_back(
                ContractForType(RequireString(object, "type")).FromJson(object));
        }

        return Restore(driver_records, vehicle_records);
    }

private:
    VehicleSerializer::FleetSnapshot Restore(
        const std::vector<DriverRecord>& driver_records,
        const std::vector<VehicleRecord>& vehicle_records) const {
        VehicleSerializer::FleetSnapshot snapshot;
        FleetGraphRestorer restorer;
        for (const DriverRecord& record : driver_records) {
            restorer.AddDriver(snapshot, record);
        }
        for (const VehicleRecord& record : vehicle_records) {
            restorer.AddVehicle(snapshot, record);
        }
        return snapshot;
    }
};

} // namespace

namespace VehicleSerializer {

std::string SerializeCustom(const Fleet& fleet,
                            const std::vector<std::unique_ptr<Driver>>& drivers) {
    return CustomTextFleetSerializer().Serialize(fleet, drivers);
}

FleetSnapshot DeserializeCustom(const std::string& text) {
    return CustomTextFleetSerializer().Deserialize(text);
}

std::string SerializeJson(const Fleet& fleet,
                          const std::vector<std::unique_ptr<Driver>>& drivers) {
    return JsonFleetSerializer().Serialize(fleet, drivers);
}

FleetSnapshot DeserializeJson(const std::string& json) {
    return JsonFleetSerializer().Deserialize(json);
}

} // namespace VehicleSerializer
