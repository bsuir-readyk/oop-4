#include "driver.h"
#include <iostream>

Driver::Driver(const std::string& name, int experience_years)
    : name_(name), experience_years_(experience_years) {
    std::cout << "[Driver] Создан водитель: " << name_ << "\n";
}

Driver::~Driver() {
    std::cout << "[Driver] Уничтожен водитель: " << name_ << "\n";
}

std::string Driver::GetName() const { return name_; }
int Driver::GetExperienceYears() const { return experience_years_; }

std::string Driver::GetInfo() const {
    return name_ + ", стаж: " + std::to_string(experience_years_) + " лет";
}
