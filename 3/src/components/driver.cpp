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

void Driver::SetName(const std::string& name) { name_ = name; }
void Driver::SetExperienceYears(int years) { experience_years_ = years; }

std::string Driver::GetInfo() const {
    return name_ + ", стаж: " + std::to_string(experience_years_) + " лет";
}

std::string Driver::SerializeCustom() const {
    std::string result;
    result += "driver_name=" + name_ + "\n";
    result += "driver_exp=" + std::to_string(experience_years_) + "\n";
    return result;
}

void Driver::DeserializeCustom(const std::string& data) {
    auto kv = ParseKeyValue(data);
    if (kv.count("driver_name")) name_ = kv["driver_name"];
    if (kv.count("driver_exp"))  experience_years_ = std::stoi(kv["driver_exp"]);
}
