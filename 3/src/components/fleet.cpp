#include "fleet.h"
#include "car.h"
#include "truck.h"
#include "boat.h"
#include "submarine.h"
#include "exceptions.h"
#include "json_serializer.h"
#include <iostream>
#include <fstream>
#include <sstream>

Fleet::~Fleet() {
    std::cout << "[Fleet] Уничтожен флот из " << vehicles_.size() << " ТС\n";
}

void Fleet::Add(std::unique_ptr<Vehicle> vehicle) {
    vehicles_.push_back(std::move(vehicle));
}

void Fleet::Remove(size_t index) {
    if (index >= vehicles_.size()) {
        throw FleetIndexException(index, vehicles_.size());
    }
    vehicles_.erase(vehicles_.begin() + static_cast<long>(index));
}

void Fleet::Clear() {
    vehicles_.clear();
}

size_t Fleet::Size() const { return vehicles_.size(); }
bool Fleet::IsEmpty() const { return vehicles_.empty(); }

Vehicle& Fleet::operator[](size_t index) {
    if (index >= vehicles_.size()) {
        throw FleetIndexException(index, vehicles_.size());
    }
    return *vehicles_[index];
}

const Vehicle& Fleet::operator[](size_t index) const {
    if (index >= vehicles_.size()) {
        throw FleetIndexException(index, vehicles_.size());
    }
    return *vehicles_[index];
}

std::string Fleet::GetAllInfo() const {
    std::string result;
    for (size_t i = 0; i < vehicles_.size(); ++i) {
        result += "[" + std::to_string(i) + "] " + vehicles_[i]->GetInfo() + "\n\n";
    }
    return result;
}

std::string Fleet::MoveAll() const {
    std::string result;
    for (const auto& v : vehicles_) {
        result += v->Move() + "\n";
    }
    return result;
}

// === Кастомная текстовая сериализация ===

std::string Fleet::SerializeCustom() const {
    std::string result;
    for (const auto& v : vehicles_) {
        result += v->SerializeCustom();
        result += "---\n";
    }
    return result;
}

void Fleet::DeserializeCustom(const std::string& data) {
    vehicles_.clear();

    // Разбиваем по разделителю "---"
    std::istringstream stream(data);
    std::string line;
    std::string block;

    while (std::getline(stream, line)) {
        if (line == "---") {
            if (!block.empty()) {
                // Определяем тип из заголовка [Type]
                std::string type;
                auto bracket_start = block.find('[');
                auto bracket_end = block.find(']');
                if (bracket_start != std::string::npos && bracket_end != std::string::npos) {
                    type = block.substr(bracket_start + 1, bracket_end - bracket_start - 1);
                }

                // Парсим key=value для создания объекта
                auto kv = ISerializable::ParseKeyValue(block);

                std::unique_ptr<Vehicle> vehicle;
                if (type == "Car") {
                    vehicle = std::make_unique<Car>(
                        kv["name"], std::stod(kv["max_speed"]),
                        std::stod(kv["engine_hp"]),
                        Engine::FuelTypeFromString(kv["engine_fuel"]),
                        kv.count("seat_count") ? std::stoi(kv["seat_count"]) : 5);
                } else if (type == "Truck") {
                    vehicle = std::make_unique<Truck>(
                        kv["name"], std::stod(kv["max_speed"]),
                        std::stod(kv["engine_hp"]),
                        Engine::FuelTypeFromString(kv["engine_fuel"]),
                        kv.count("cargo_capacity") ? std::stod(kv["cargo_capacity"]) : 10);
                } else if (type == "Boat") {
                    vehicle = std::make_unique<Boat>(
                        kv["name"], std::stod(kv["max_speed"]),
                        std::stod(kv["engine_hp"]),
                        Engine::FuelTypeFromString(kv["engine_fuel"]),
                        kv.count("displacement") ? std::stod(kv["displacement"]) : 50,
                        kv.count("passenger_capacity") ? std::stoi(kv["passenger_capacity"]) : 10);
                } else if (type == "Submarine") {
                    vehicle = std::make_unique<Submarine>(
                        kv["name"], std::stod(kv["max_speed"]),
                        std::stod(kv["engine_hp"]),
                        Engine::FuelTypeFromString(kv["engine_fuel"]),
                        kv.count("displacement") ? std::stod(kv["displacement"]) : 1000,
                        kv.count("max_depth") ? std::stod(kv["max_depth"]) : 100);
                }

                if (vehicle) {
                    // Восстанавливаем водителя (глубокая десериализация —
                    // создаём новый объект Driver, а не ссылку на существующий)
                    if (kv.count("driver_name") && !kv["driver_name"].empty()) {
                        // При десериализации Driver создаётся заново —
                        // это глубокая копия, ссылка на оригинал теряется
                        auto* driver = new Driver(
                            kv["driver_name"],
                            kv.count("driver_exp") ? std::stoi(kv["driver_exp"]) : 0);
                        vehicle->AssignDriver(driver);
                    }
                    vehicles_.push_back(std::move(vehicle));
                }
                block.clear();
            }
        } else {
            block += line + "\n";
        }
    }
}

void Fleet::SaveCustom(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw VehicleException("Не удалось открыть файл для записи: " + filepath);
    }
    file << SerializeCustom();
}

void Fleet::LoadCustom(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw VehicleException("Не удалось открыть файл для чтения: " + filepath);
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    DeserializeCustom(content);
}

// === JSON сериализация ===

std::string Fleet::SerializeJson() const {
    return JsonSerializer::SerializeFleet(*this);
}

void Fleet::DeserializeJson(const std::string& data) {
    JsonSerializer::DeserializeFleet(data, *this);
}

void Fleet::SaveJson(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw VehicleException("Не удалось открыть файл для записи: " + filepath);
    }
    file << SerializeJson();
}

void Fleet::LoadJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw VehicleException("Не удалось открыть файл для чтения: " + filepath);
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    DeserializeJson(content);
}
