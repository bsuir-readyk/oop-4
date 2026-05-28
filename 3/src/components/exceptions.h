#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <cstddef>
#include <stdexcept>
#include <string>

// Базовое исключение для транспорта
class VehicleException : public std::runtime_error {
public:
    explicit VehicleException(const std::string& message)
        : std::runtime_error(message) {}
};

// Исключение превышения скорости
class SpeedLimitException : public VehicleException {
public:
    SpeedLimitException(double speed, double limit)
        : VehicleException("Скорость " + std::to_string(static_cast<int>(speed)) +
                           " км/ч превышает лимит " + std::to_string(static_cast<int>(limit)) + " км/ч"),
          speed_(speed), limit_(limit) {}

    double GetSpeed() const { return speed_; }
    double GetLimit() const { return limit_; }

private:
    double speed_;
    double limit_;
};

// Исключение индекса флота
class FleetIndexException : public VehicleException {
public:
    FleetIndexException(size_t index, size_t size)
        : VehicleException("Индекс " + std::to_string(index) +
                           " вне диапазона [0, " + std::to_string(size) + ")"),
          index_(index), size_(size) {}

    size_t GetIndex() const { return index_; }
    size_t GetSize() const { return size_; }

private:
    size_t index_;
    size_t size_;
};

// Исключение переполнения коллекции транспорта
class FleetCapacityException : public VehicleException {
public:
    FleetCapacityException(size_t size, size_t limit)
        : VehicleException("Флот заполнен: " + std::to_string(size) +
                           " ТС, максимум " + std::to_string(limit)),
          size_(size), limit_(limit) {}

    size_t GetSize() const { return size_; }
    size_t GetLimit() const { return limit_; }

private:
    size_t size_;
    size_t limit_;
};

// Исключение пустого двигателя
class EngineException : public VehicleException {
public:
    explicit EngineException(const std::string& message)
        : VehicleException("Ошибка двигателя: " + message) {}
};

// Исключение сериализации/десериализации
class SerializationException : public VehicleException {
public:
    explicit SerializationException(const std::string& message)
        : VehicleException("Ошибка сериализации: " + message) {}
};

#endif // EXCEPTIONS_H
