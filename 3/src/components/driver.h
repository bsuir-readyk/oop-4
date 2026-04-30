#ifndef DRIVER_H
#define DRIVER_H

#include "serializable.h"
#include <string>

// Класс Driver — используется в агрегации с Vehicle.
// Существует независимо от Vehicle.
class Driver : public ISerializable {
public:
    Driver(const std::string& name, int experience_years);
    ~Driver() override;

    std::string GetName() const;
    int GetExperienceYears() const;
    std::string GetInfo() const;

    // Сеттеры
    void SetName(const std::string& name);
    void SetExperienceYears(int years);

    // ISerializable
    std::string SerializeCustom() const override;
    void DeserializeCustom(const std::string& data) override;

private:
    std::string name_;          // Имя водителя
    int experience_years_;      // Стаж в годах
};

#endif // DRIVER_H
