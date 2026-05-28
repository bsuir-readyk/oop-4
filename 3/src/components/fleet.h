#ifndef FLEET_H
#define FLEET_H

#include "vehicle.h"
#include <vector>
#include <memory>
#include <string>

// Класс-коллекция с индексатором (operator[]).
// Содержит std::vector<std::unique_ptr<Vehicle>>.
class Fleet {
public:
    static constexpr size_t kMaxSize = 10;

    Fleet() = default;
    ~Fleet();

    Fleet(Fleet&&) noexcept = default;
    Fleet& operator=(Fleet&&) noexcept = default;
    Fleet(const Fleet&) = delete;
    Fleet& operator=(const Fleet&) = delete;

    void Add(std::unique_ptr<Vehicle> vehicle);
    void Remove(size_t index);
    size_t Size() const;
    bool IsEmpty() const;

    // Индексатор — operator[]
    Vehicle& operator[](size_t index);
    const Vehicle& operator[](size_t index) const;

    std::string GetAllInfo() const;
    std::string MoveAll() const;

    // Итерация (для range-for)
    auto begin() { return vehicles_.begin(); }
    auto end() { return vehicles_.end(); }
    auto begin() const { return vehicles_.begin(); }
    auto end() const { return vehicles_.end(); }

private:
    std::vector<std::unique_ptr<Vehicle>> vehicles_;
};

#endif // FLEET_H
