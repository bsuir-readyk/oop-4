#ifndef DRIVER_H
#define DRIVER_H

#include <string>

class Driver {
public:
    Driver(const std::string& name, int experience_years);
    ~Driver();

    std::string GetName() const;
    int GetExperienceYears() const;
    std::string GetInfo() const;

private:
    std::string name_;
    int experience_years_;
};

#endif // DRIVER_H
