#ifndef ENGINE_H
#define ENGINE_H

#include "serializable.h"
#include <string>

// Класс Engine — используется в композиции с Vehicle.
// Создаётся и уничтожается вместе с Vehicle.
class Engine : public ISerializable {
public:
    // Перечисление типов топлива
    enum class FuelType { kPetrol, kDiesel, kElectric, kNuclear };

    Engine(double horsepower, FuelType fuel_type);
    ~Engine() override;

    // Геттеры (инкапсуляция)
    double GetHorsepower() const;
    FuelType GetFuelType() const;
    bool IsRunning() const;

    // Сеттеры
    void SetHorsepower(double hp);
    void SetFuelType(FuelType type);

    // Методы
    void Start();
    void Stop();
    std::string GetInfo() const;

    static std::string FuelTypeToString(FuelType type);
    static FuelType FuelTypeFromString(const std::string& str);

    // ISerializable
    std::string SerializeCustom() const override;
    void DeserializeCustom(const std::string& data) override;

private:
    double horsepower_;        // Мощность в л.с.
    FuelType fuel_type_;       // Тип топлива
    bool is_running_ = false;  // Состояние двигателя
};

#endif // ENGINE_H
