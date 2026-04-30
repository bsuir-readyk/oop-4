#ifndef DRIVER_H
#define DRIVER_H

#include <string>

// Класс Driver — используется в агрегации с Vehicle.
// Существует независимо от Vehicle.
class Driver {
public:
    Driver(const std::string& name, int experience_years);
    ~Driver();

    std::string GetName() const;
    int GetExperienceYears() const;
    std::string GetInfo() const;

private:
    std::string name_;          // Имя водителя
    int experience_years_;      // Стаж в годах
};

#endif // DRIVER_H
