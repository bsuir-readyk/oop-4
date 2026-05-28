#include "vehicle_factory.h"
#include "car.h"
#include "truck.h"
#include "boat.h"
#include "submarine.h"
#include "exceptions.h"

std::unique_ptr<Vehicle> CarFactory::Create(const std::string& name) const {
    return std::make_unique<Car>(name, 200, 150, Engine::FuelType::kPetrol, 5);
}

std::unique_ptr<Vehicle> TruckFactory::Create(const std::string& name) const {
    return std::make_unique<Truck>(name, 110, 400, Engine::FuelType::kDiesel, 20);
}

std::unique_ptr<Vehicle> BoatFactory::Create(const std::string& name) const {
    return std::make_unique<Boat>(name, 70, 250, Engine::FuelType::kPetrol, 40, 8);
}

std::unique_ptr<Vehicle> SubmarineFactory::Create(const std::string& name) const {
    return std::make_unique<Submarine>(name, 45, 800, Engine::FuelType::kNuclear, 5000, 400);
}

std::unique_ptr<VehicleFactory> GetFactory(VehicleType type) {
    switch (type) {
        case VehicleType::kCar:       return std::make_unique<CarFactory>();
        case VehicleType::kTruck:     return std::make_unique<TruckFactory>();
        case VehicleType::kBoat:      return std::make_unique<BoatFactory>();
        case VehicleType::kSubmarine: return std::make_unique<SubmarineFactory>();
    }

    throw VehicleException("Неизвестный тип транспорта");
}
