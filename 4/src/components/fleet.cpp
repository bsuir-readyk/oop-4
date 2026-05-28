#include "fleet.h"
#include "exceptions.h"
#include <iostream>

Fleet::~Fleet() {
    std::cout << "[Fleet] Уничтожен флот из " << vehicles_.size() << " ТС\n";
}

void Fleet::Add(std::unique_ptr<Vehicle> vehicle) {
    if (vehicles_.size() >= kMaxSize) {
        throw FleetCapacityException(vehicles_.size(), kMaxSize);
    }
    vehicles_.push_back(std::move(vehicle));
}

void Fleet::Remove(size_t index) {
    if (index >= vehicles_.size()) {
        throw FleetIndexException(index, vehicles_.size());
    }
    vehicles_.erase(vehicles_.begin() + static_cast<long>(index));
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
