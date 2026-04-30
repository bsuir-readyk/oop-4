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
    Fleet() = default;
    ~Fleet();

    void Add(std::unique_ptr<Vehicle> vehicle);
    void Remove(size_t index);
    void Clear();
    size_t Size() const;
    bool IsEmpty() const;

    // Индексатор — operator[]
    Vehicle& operator[](size_t index);
    const Vehicle& operator[](size_t index) const;

    // Вызов полиморфных методов на всей коллекции
    std::string GetAllInfo() const;
    std::string MoveAll() const;

    // Сериализация — кастомный текстовый формат
    std::string SerializeCustom() const;
    void DeserializeCustom(const std::string& data);
    void SaveCustom(const std::string& filepath) const;
    void LoadCustom(const std::string& filepath);

    // Сериализация — JSON
    std::string SerializeJson() const;
    void DeserializeJson(const std::string& data);
    void SaveJson(const std::string& filepath) const;
    void LoadJson(const std::string& filepath);

    // Итерация (для range-for)
    auto begin() { return vehicles_.begin(); }
    auto end() { return vehicles_.end(); }
    auto begin() const { return vehicles_.begin(); }
    auto end() const { return vehicles_.end(); }

private:
    std::vector<std::unique_ptr<Vehicle>> vehicles_;
};

#endif // FLEET_H
