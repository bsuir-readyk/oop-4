#ifndef ENGINE_H
#define ENGINE_H

#include <string>

class Engine {
public:
    enum class FuelType { kPetrol, kDiesel, kElectric, kNuclear };

    Engine(double horsepower, FuelType fuel_type);
    ~Engine();

    double GetHorsepower() const;
    FuelType GetFuelType() const;
    bool IsRunning() const;

    void SetHorsepower(double hp);

    void Start();
    void Stop();
    std::string GetInfo() const;

    static std::string FuelTypeToString(FuelType type);

private:
    double horsepower_;
    FuelType fuel_type_;
    bool is_running_ = false;
};

#endif // ENGINE_H
