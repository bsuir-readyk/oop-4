# Отчёт по лабораторной работе 1

**Тема:** основы объектно-ориентированного программирования на C++ с графическим интерфейсом Qt 6.

**Проект:** приложение для работы с иерархией транспортных средств.

## Цель работы

Разработать программу, демонстрирующую основные принципы ООП: инкапсуляцию, наследование, полиморфизм и абстракцию. В проекте также должны быть показаны композиция, агрегация, работа с коллекцией объектов, индексатор через `operator[]`, обработка исключений и графический интерфейс.

## Общая структура программы

В проекте реализована трёхуровневая иерархия классов:

```text
Vehicle (абстрактный базовый класс)
├── LandVehicle (наземный транспорт, абстрактный)
│   ├── Car (легковой автомобиль)
│   └── Truck (грузовик)
└── WaterVehicle (водный транспорт, абстрактный)
    ├── Boat (катер)
    └── Submarine (подводная лодка)
```

Дополнительные классы:

- `Engine` — двигатель, используется как пример композиции;
- `Driver` — водитель, используется как пример агрегации;
- `Fleet` — коллекция транспортных средств;
- `MainWindow` — главное окно Qt-приложения;
- `VehicleException`, `SpeedLimitException`, `FleetIndexException`, `EngineException` — иерархия пользовательских исключений.

Исходные файлы разнесены по каталогам:

- `src/vehicles/` — классы транспортных средств;
- `src/components/` — вспомогательные классы и исключения;
- `src/ui/` — графический интерфейс;
- `src/main.cpp` — точка входа в приложение.

## Описание классов

### Vehicle

`Vehicle` является абстрактным базовым классом. Он содержит общие данные для всех транспортных средств:

- `name_` — название;
- `max_speed_` — максимальная скорость;
- `current_speed_` — текущая скорость;
- `engine_` — двигатель;
- `driver_` — указатель на назначенного водителя.

Класс объявляет чисто виртуальные методы:

```cpp
virtual std::string GetType() const = 0;
virtual std::string Move() const = 0;
```

Из-за этих методов нельзя создать объект `Vehicle` напрямую. Конкретные классы обязаны реализовать тип транспорта и способ движения.

Метод `GetInfo()` реализован в базовом классе и переиспользуется потомками. Он собирает общую информацию: тип, название, максимальную скорость, данные двигателя и водителя.

### LandVehicle и WaterVehicle

`LandVehicle` и `WaterVehicle` являются промежуточными абстрактными классами.

`LandVehicle` добавляет поле `wheel_count_` и метод `GetWheelCount()`. Его потомки используют это поле для наземного транспорта.

`WaterVehicle` добавляет поле `displacement_` и метод `GetDisplacement()`. Его потомки используют это поле для водного транспорта.

Оба класса переопределяют `GetInfo()`, добавляя свои характеристики к базовой информации из `Vehicle`.

### Car, Truck, Boat, Submarine

Конкретные классы третьего уровня реализуют собственное поведение:

| Класс | Базовый класс | Дополнительное поле | Метод `Move()` |
| --- | --- | --- | --- |
| `Car` | `LandVehicle` | `seat_count_` | едет по дороге |
| `Truck` | `LandVehicle` | `cargo_capacity_` | перевозит груз |
| `Boat` | `WaterVehicle` | `passenger_capacity_` | плывёт по воде |
| `Submarine` | `WaterVehicle` | `max_depth_` | погружается под воду |

Каждый класс переопределяет методы `GetType()`, `Move()` и `GetInfo()`.

### Engine

Класс `Engine` описывает двигатель:

- мощность в лошадиных силах;
- тип топлива через перечисление `FuelType`;
- состояние работы двигателя.

Поддерживаются типы топлива:

```cpp
enum class FuelType { kPetrol, kDiesel, kElectric, kNuclear };
```

Методы `Start()` и `Stop()` меняют состояние двигателя. При некорректных действиях выбрасывается `EngineException`: например, если запустить уже работающий двигатель или создать двигатель с неположительной мощностью.

### Driver

`Driver` хранит имя водителя и стаж. Он не принадлежит транспортному средству, а только назначается через указатель:

```cpp
void AssignDriver(Driver* driver);
void RemoveDriver();
```

Это демонстрирует агрегацию: водитель может существовать независимо от транспорта.

### Fleet

`Fleet` хранит коллекцию транспортных средств:

```cpp
std::vector<std::unique_ptr<Vehicle>> vehicles_;
```

Использование `std::unique_ptr<Vehicle>` позволяет хранить объекты разных производных классов в одной коллекции и автоматически управлять их временем жизни.

Класс предоставляет:

- `Add()` — добавление объекта;
- `Remove()` — удаление по индексу;
- `Size()` и `IsEmpty()` — получение состояния коллекции;
- `operator[]` — индексатор с проверкой границ;
- `GetAllInfo()` — получение информации обо всех объектах;
- `MoveAll()` — вызов полиморфного метода `Move()` для всех объектов.

При обращении за пределы коллекции выбрасывается `FleetIndexException`.

## Принципы ООП в проекте

### Инкапсуляция

Поля классов скрыты от внешнего кода. Доступ к ним выполняется через публичные методы:

- `GetName()`;
- `GetMaxSpeed()`;
- `GetEngine()`;
- `SetCurrentSpeed()`;
- `GetCurrentSpeed()`.

Метод `SetCurrentSpeed()` не просто записывает значение, а проверяет корректность скорости. Если скорость отрицательная или превышает максимум, выбрасывается `SpeedLimitException`.

### Наследование

Наследование используется для построения общей модели транспорта. Например:

```text
Vehicle -> LandVehicle -> Car
Vehicle -> WaterVehicle -> Submarine
```

Базовый класс содержит общее состояние и методы. Промежуточные классы добавляют признаки группы транспорта. Конкретные классы добавляют собственные характеристики.

### Полиморфизм

Полиморфизм реализован через виртуальные методы `Move()`, `GetType()`, `GetInfo()` и `GetMaxSpeed()`.

В классе `Fleet` объекты хранятся как `std::unique_ptr<Vehicle>`, но при вызове:

```cpp
vehicles_[i]->GetInfo();
vehicles_[i]->Move();
```

выполняется метод реального класса: `Car`, `Truck`, `Boat` или `Submarine`.

### Абстракция

`Vehicle` задаёт общий интерфейс транспортного средства, но не описывает конкретный способ движения. Это позволяет работать с любым транспортом единообразно и не зависеть от деталей конкретного класса.

## Композиция и агрегация

### Композиция

Поле `engine_` хранится внутри `Vehicle` как объект:

```cpp
Engine engine_;
```

Двигатель создаётся в конструкторе транспорта и уничтожается вместе с ним. Это пример композиции: часть не существует отдельно от целого в рамках модели программы.

### Агрегация

Поле `driver_` хранится как указатель:

```cpp
Driver* driver_ = nullptr;
```

`Vehicle` не создаёт и не удаляет водителя. Водители хранятся отдельно в `MainWindow`:

```cpp
std::vector<std::unique_ptr<Driver>> drivers_;
```

Транспорт только получает адрес существующего водителя. Это пример агрегации.

## Исключения

В проекте реализована собственная иерархия исключений:

```text
std::runtime_error
└── VehicleException
    ├── SpeedLimitException
    ├── FleetIndexException
    └── EngineException
```

Примеры ситуаций:

- `SpeedLimitException` — установка скорости выше максимальной или ниже нуля;
- `FleetIndexException` — обращение к несуществующему элементу коллекции;
- `EngineException` — некорректная мощность двигателя или неверная операция запуска/остановки;
- `VehicleException` — базовый тип для ошибок предметной области.

Во вкладке «Исключения» пользователь может запустить демонстрацию обработки всех этих ошибок.

## Графический интерфейс

Интерфейс реализован на Qt 6 в классе `MainWindow`. Главное окно содержит вкладки:

1. **Флот** — карточки транспортных средств, просмотр подробной информации, добавление и удаление объектов.
2. **Полиморфизм** — демонстрация вызовов `Move()`, `GetInfo()` и `GetMaxSpeed()` для всех объектов коллекции.
3. **Исключения** — демонстрация обработки пользовательских исключений.
4. **О классах** — справочная информация об иерархии, принципах ООП, композиции, агрегации и передаче параметров.

При запуске создаётся начальный набор объектов:

- `Toyota Camry` — легковой автомобиль;
- `Volvo FH16` — грузовик;
- `Yamaha 242X` — катер;
- `Наутилус` — подводная лодка.

Также создаются два водителя, которые назначаются легковому автомобилю и катеру.

Добавление нового транспорта выполняется через диалог выбора типа и ввода названия. В зависимости от выбранного типа создаётся соответствующий объект:

```cpp
std::make_unique<Car>(...);
std::make_unique<Truck>(...);
std::make_unique<Boat>(...);
std::make_unique<Submarine>(...);
```

## Что содержит класс в C++

На примере проекта показаны основные элементы класса:

- **поля** — данные объекта, например `name_`, `max_speed_`, `engine_`;
- **методы** — поведение объекта, например `Move()`, `GetInfo()`, `Start()`;
- **свойства** — в C++ обычно реализуются через геттеры и сеттеры;
- **конструктор** — инициализирует объект при создании;
- **деструктор** — вызывается при уничтожении объекта;
- **индексатор** — реализован как `operator[]` в `Fleet`;
- **атрибуты и модификаторы** — `virtual`, `override`, `const`, `explicit`, `private`, `protected`, `public`.

## Класс и структура

В C++ `class` и `struct` имеют почти одинаковые возможности. Главное отличие — доступ по умолчанию:

- у `class` члены по умолчанию `private`;
- у `struct` члены по умолчанию `public`.

В проекте используются классы, потому что объекты имеют инварианты, скрытое состояние и поведение.

## Передача параметров

В коде используются разные способы передачи параметров:

- по значению — для простых типов вроде `double`, `int`, `size_t`;
- по константной ссылке — для строк: `const std::string& name`;
- по указателю — для агрегации: `Driver* driver`;
- через `std::unique_ptr` — для передачи владения объектом в `Fleet::Add()`.

Пример передачи владения:

```cpp
void Fleet::Add(std::unique_ptr<Vehicle> vehicle) {
    vehicles_.push_back(std::move(vehicle));
}
```

После `std::move()` объектом владеет коллекция `Fleet`.

## Нотации именования

В проекте используется стиль, близкий к Google C++ Style:

| Элемент | Нотация | Пример |
| --- | --- | --- |
| Классы | PascalCase | `LandVehicle` |
| Методы | PascalCase | `GetMaxSpeed()` |
| Приватные поля | snake_case_ | `max_speed_` |
| Локальные переменные | snake_case | `wheel_count` |
| Значения enum | kCamelCase | `kPetrol` |
| Файлы | snake_case | `water_vehicle.cpp` |

## Сборка и запуск

Проект собирается через CMake. В `CMakeLists.txt` подключается Qt 6 Widgets и стандарт C++17:

```cmake
set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
target_link_libraries(oop_vehicles PRIVATE Qt6::Widgets)
```

Также есть `Makefile` с командами:

```bash
make        # очистка, конфигурация и сборка
make run    # запуск приложения из build/
make clean  # удаление build/
```

## Вывод

В результате работы создано Qt-приложение, демонстрирующее базовые возможности ООП на C++: трёхуровневую иерархию классов, абстрактный базовый класс, виртуальные методы, полиморфную коллекцию, композицию, агрегацию, пользовательские исключения и графический интерфейс для работы с объектами.

## Приложение. Исходный код

Ниже приведены основные файлы проекта на момент подготовки отчёта.

### `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.16)
project(oop_vehicles LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(oop_vehicles
    src/main.cpp
    src/ui/main_window.cpp
    src/ui/main_window.h
    src/components/engine.cpp
    src/components/driver.cpp
    src/components/fleet.cpp
    src/vehicles/vehicle.cpp
    src/vehicles/land_vehicle.cpp
    src/vehicles/water_vehicle.cpp
    src/vehicles/car.cpp
    src/vehicles/truck.cpp
    src/vehicles/boat.cpp
    src/vehicles/submarine.cpp
)

target_include_directories(oop_vehicles PRIVATE
    src
    src/vehicles
    src/components
    src/ui
)
target_link_libraries(oop_vehicles PRIVATE Qt6::Widgets)
```

### `Makefile`

```makefile
BUILD_DIR := build
TARGET := oop_vehicles

.PHONY: all configure build run clean

all: clean configure build

configure:
	@cmake -S . -B $(BUILD_DIR)

build:
	@cmake --build $(BUILD_DIR)

run:
	@./$(BUILD_DIR)/$(TARGET)

clean:
	@rm -rf $(BUILD_DIR)
```

### `src/main.cpp`

```cpp
#include <QApplication>
#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
```

### `src/components/engine.h`

```cpp
#ifndef ENGINE_H
#define ENGINE_H

#include <string>

// Класс Engine — используется в композиции с Vehicle.
// Создаётся и уничтожается вместе с Vehicle.
class Engine {
public:
    // Перечисление типов топлива
    enum class FuelType { kPetrol, kDiesel, kElectric, kNuclear };

    Engine(double horsepower, FuelType fuel_type);
    ~Engine();

    // Геттеры (инкапсуляция)
    double GetHorsepower() const;
    FuelType GetFuelType() const;
    bool IsRunning() const;

    // Сеттеры
    void SetHorsepower(double hp);

    // Методы
    void Start();
    void Stop();
    std::string GetInfo() const;

    static std::string FuelTypeToString(FuelType type);

private:
    double horsepower_;        // Мощность в л.с.
    FuelType fuel_type_;       // Тип топлива
    bool is_running_ = false;  // Состояние двигателя
};

#endif // ENGINE_H
```

### `src/components/engine.cpp`

```cpp
#include "engine.h"
#include "exceptions.h"
#include <iostream>

Engine::Engine(double horsepower, FuelType fuel_type)
    : horsepower_(horsepower), fuel_type_(fuel_type) {
    if (horsepower <= 0) {
        throw EngineException("Мощность должна быть положительной");
    }
    std::cout << "[Engine] Создан двигатель " << horsepower_ << " л.с.\n";
}

Engine::~Engine() {
    std::cout << "[Engine] Уничтожен двигатель " << horsepower_ << " л.с.\n";
}

double Engine::GetHorsepower() const { return horsepower_; }
Engine::FuelType Engine::GetFuelType() const { return fuel_type_; }
bool Engine::IsRunning() const { return is_running_; }

void Engine::SetHorsepower(double hp) {
    if (hp <= 0) {
        throw EngineException("Мощность должна быть положительной");
    }
    horsepower_ = hp;
}

void Engine::Start() {
    if (is_running_) {
        throw EngineException("Двигатель уже запущен");
    }
    is_running_ = true;
}

void Engine::Stop() {
    if (!is_running_) {
        throw EngineException("Двигатель уже остановлен");
    }
    is_running_ = false;
}

std::string Engine::GetInfo() const {
    return std::to_string(static_cast<int>(horsepower_)) + " л.с., " +
           FuelTypeToString(fuel_type_) +
           (is_running_ ? ", работает" : ", выключен");
}

std::string Engine::FuelTypeToString(FuelType type) {
    switch (type) {
        case FuelType::kPetrol:   return "Бензин";
        case FuelType::kDiesel:   return "Дизель";
        case FuelType::kElectric: return "Электро";
        case FuelType::kNuclear:  return "Ядерный";
    }
    return "Неизвестно";
}
```

### `src/components/driver.h`

```cpp
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
```

### `src/components/driver.cpp`

```cpp
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
```

### `src/components/exceptions.h`

```cpp
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

// Базовое исключение для транспорта
class VehicleException : public std::runtime_error {
public:
    explicit VehicleException(const std::string& message)
        : std::runtime_error(message) {}
};

// Исключение превышения скорости
class SpeedLimitException : public VehicleException {
public:
    SpeedLimitException(double speed, double limit)
        : VehicleException("Скорость " + std::to_string(static_cast<int>(speed)) +
                           " км/ч превышает лимит " + std::to_string(static_cast<int>(limit)) + " км/ч"),
          speed_(speed), limit_(limit) {}

    double GetSpeed() const { return speed_; }
    double GetLimit() const { return limit_; }

private:
    double speed_;
    double limit_;
};

// Исключение индекса флота
class FleetIndexException : public VehicleException {
public:
    FleetIndexException(size_t index, size_t size)
        : VehicleException("Индекс " + std::to_string(index) +
                           " вне диапазона [0, " + std::to_string(size) + ")"),
          index_(index), size_(size) {}

    size_t GetIndex() const { return index_; }
    size_t GetSize() const { return size_; }

private:
    size_t index_;
    size_t size_;
};

// Исключение пустого двигателя
class EngineException : public VehicleException {
public:
    explicit EngineException(const std::string& message)
        : VehicleException("Ошибка двигателя: " + message) {}
};

#endif // EXCEPTIONS_H
```

### `src/components/fleet.h`

```cpp
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
    size_t Size() const;
    bool IsEmpty() const;

    // Индексатор — operator[]
    Vehicle& operator[](size_t index);
    const Vehicle& operator[](size_t index) const;

    // Вызов полиморфных методов на всей коллекции
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
```

### `src/components/fleet.cpp`

```cpp
#include "fleet.h"
#include "exceptions.h"
#include <iostream>

Fleet::~Fleet() {
    std::cout << "[Fleet] Уничтожен флот из " << vehicles_.size() << " ТС\n";
}

void Fleet::Add(std::unique_ptr<Vehicle> vehicle) {
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
```

### `src/vehicles/vehicle.h`

```cpp
#ifndef VEHICLE_H
#define VEHICLE_H

#include "engine.h"
#include "driver.h"
#include <string>
#include <memory>

// Абстрактный базовый класс — уровень 1 иерархии.
// Демонстрирует: абстракцию, инкапсуляцию, композицию (Engine), агрегацию (Driver).
class Vehicle {
public:
    Vehicle(const std::string& name, double max_speed,
            double engine_hp, Engine::FuelType fuel_type);
    virtual ~Vehicle();

    // Чисто виртуальные методы (абстракция) — должны быть реализованы в потомках
    virtual std::string GetType() const = 0;
    virtual std::string Move() const = 0;

    // Виртуальный метод с реализацией по умолчанию (полиморфизм)
    virtual std::string GetInfo() const;
    virtual double GetMaxSpeed() const;

    // Геттеры (инкапсуляция)
    std::string GetName() const;
    const Engine& GetEngine() const;
    Engine& GetEngine();
    const Driver* GetDriver() const;

    // Агрегация: назначить/снять водителя
    void AssignDriver(Driver* driver);
    void RemoveDriver();

    // Установить скорость с проверкой (исключения)
    void SetCurrentSpeed(double speed);
    double GetCurrentSpeed() const;

protected:
    std::string name_;              // Название ТС
    double max_speed_;              // Максимальная скорость
    double current_speed_ = 0.0;   // Текущая скорость

private:
    Engine engine_;                 // Композиция: двигатель принадлежит ТС
    Driver* driver_ = nullptr;     // Агрегация: водитель — внешний объект
};

#endif // VEHICLE_H
```

### `src/vehicles/vehicle.cpp`

```cpp
#include "vehicle.h"
#include "exceptions.h"
#include <iostream>

Vehicle::Vehicle(const std::string& name, double max_speed,
                 double engine_hp, Engine::FuelType fuel_type)
    : name_(name), max_speed_(max_speed), engine_(engine_hp, fuel_type) {
    std::cout << "[Vehicle] Создан: " << name_ << "\n";
}

Vehicle::~Vehicle() {
    std::cout << "[Vehicle] Уничтожен: " << name_ << "\n";
}

std::string Vehicle::GetInfo() const {
    std::string info = GetType() + ": " + name_ +
                       "\n  Макс. скорость: " + std::to_string(static_cast<int>(max_speed_)) + " км/ч" +
                       "\n  Двигатель: " + engine_.GetInfo();
    if (driver_) {
        info += "\n  Водитель: " + driver_->GetInfo();
    } else {
        info += "\n  Водитель: не назначен";
    }
    return info;
}

double Vehicle::GetMaxSpeed() const { return max_speed_; }
std::string Vehicle::GetName() const { return name_; }
const Engine& Vehicle::GetEngine() const { return engine_; }
Engine& Vehicle::GetEngine() { return engine_; }
const Driver* Vehicle::GetDriver() const { return driver_; }

void Vehicle::AssignDriver(Driver* driver) { driver_ = driver; }
void Vehicle::RemoveDriver() { driver_ = nullptr; }

void Vehicle::SetCurrentSpeed(double speed) {
    if (speed < 0) {
        throw SpeedLimitException(speed, max_speed_);
    }
    if (speed > max_speed_) {
        throw SpeedLimitException(speed, max_speed_);
    }
    current_speed_ = speed;
}

double Vehicle::GetCurrentSpeed() const { return current_speed_; }
```

### `src/vehicles/land_vehicle.h`

```cpp
#ifndef LAND_VEHICLE_H
#define LAND_VEHICLE_H

#include "vehicle.h"

// Уровень 2 иерархии — наземный транспорт (абстрактный).
class LandVehicle : public Vehicle {
public:
    LandVehicle(const std::string& name, double max_speed,
                double engine_hp, Engine::FuelType fuel_type, int wheel_count);
    ~LandVehicle() override;

    int GetWheelCount() const;
    std::string GetInfo() const override;

    // Move() остаётся чисто виртуальным — конкретные классы реализуют

protected:
    int wheel_count_;  // Количество колёс
};

#endif // LAND_VEHICLE_H
```

### `src/vehicles/land_vehicle.cpp`

```cpp
#include "land_vehicle.h"
#include <iostream>

LandVehicle::LandVehicle(const std::string& name, double max_speed,
                         double engine_hp, Engine::FuelType fuel_type, int wheel_count)
    : Vehicle(name, max_speed, engine_hp, fuel_type), wheel_count_(wheel_count) {
    std::cout << "[LandVehicle] Создан наземный: " << name_ << "\n";
}

LandVehicle::~LandVehicle() {
    std::cout << "[LandVehicle] Уничтожен наземный: " << name_ << "\n";
}

int LandVehicle::GetWheelCount() const { return wheel_count_; }

std::string LandVehicle::GetInfo() const {
    return Vehicle::GetInfo() +
           "\n  Колёса: " + std::to_string(wheel_count_);
}
```

### `src/vehicles/water_vehicle.h`

```cpp
#ifndef WATER_VEHICLE_H
#define WATER_VEHICLE_H

#include "vehicle.h"

class WaterVehicle : public Vehicle {
public:
  WaterVehicle(const std::string &name, double max_speed, double engine_hp,
               Engine::FuelType fuel_type, double displacement);
  ~WaterVehicle() override;

  double GetDisplacement() const;
  std::string GetInfo() const override;

protected:
  double displacement_; // Водоизмещение в тоннах
};

#endif // WATER_VEHICLE_H
```

### `src/vehicles/water_vehicle.cpp`

```cpp
#include "water_vehicle.h"
#include <iostream>

WaterVehicle::WaterVehicle(const std::string& name, double max_speed,
                           double engine_hp, Engine::FuelType fuel_type, double displacement)
    : Vehicle(name, max_speed, engine_hp, fuel_type), displacement_(displacement) {
    std::cout << "[WaterVehicle] Создан водный: " << name_ << "\n";
}

WaterVehicle::~WaterVehicle() {
    std::cout << "[WaterVehicle] Уничтожен водный: " << name_ << "\n";
}

double WaterVehicle::GetDisplacement() const { return displacement_; }

std::string WaterVehicle::GetInfo() const {
    return Vehicle::GetInfo() +
           "\n  Водоизмещение: " + std::to_string(static_cast<int>(displacement_)) + " т";
}
```

### `src/vehicles/car.h`

```cpp
#ifndef CAR_H
#define CAR_H

#include "land_vehicle.h"

// Уровень 3 — конкретный класс легкового автомобиля.
class Car : public LandVehicle {
public:
    Car(const std::string& name, double max_speed,
        double engine_hp, Engine::FuelType fuel_type, int seat_count);
    ~Car() override;

    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    int GetSeatCount() const;

private:
    int seat_count_;
};

#endif // CAR_H
```

### `src/vehicles/car.cpp`

```cpp
#include "car.h"
#include <iostream>

Car::Car(const std::string& name, double max_speed,
         double engine_hp, Engine::FuelType fuel_type, int seat_count)
    : LandVehicle(name, max_speed, engine_hp, fuel_type, 4), seat_count_(seat_count) {
    std::cout << "[Car] Создан: " << name_ << "\n";
}

Car::~Car() {
    std::cout << "[Car] Уничтожен: " << name_ << "\n";
}

std::string Car::GetType() const { return "Легковой автомобиль"; }
std::string Car::Move() const { return name_ + " едет по дороге 🚗"; }

std::string Car::GetInfo() const {
    return LandVehicle::GetInfo() +
           "\n  Мест: " + std::to_string(seat_count_);
}

int Car::GetSeatCount() const { return seat_count_; }
```

### `src/vehicles/truck.h`

```cpp
#ifndef TRUCK_H
#define TRUCK_H

#include "land_vehicle.h"

// Уровень 3 — грузовик.
class Truck : public LandVehicle {
public:
    Truck(const std::string& name, double max_speed,
          double engine_hp, Engine::FuelType fuel_type, double cargo_capacity);
    ~Truck() override;

    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    double GetCargoCapacity() const;

private:
    double cargo_capacity_;  // Грузоподъёмность в тоннах
};

#endif // TRUCK_H
```

### `src/vehicles/truck.cpp`

```cpp
#include "truck.h"
#include <iostream>

Truck::Truck(const std::string& name, double max_speed,
             double engine_hp, Engine::FuelType fuel_type, double cargo_capacity)
    : LandVehicle(name, max_speed, engine_hp, fuel_type, 6), cargo_capacity_(cargo_capacity) {
    std::cout << "[Truck] Создан: " << name_ << "\n";
}

Truck::~Truck() {
    std::cout << "[Truck] Уничтожен: " << name_ << "\n";
}

std::string Truck::GetType() const { return "Грузовик"; }
std::string Truck::Move() const { return name_ + " перевозит груз 🚛"; }

std::string Truck::GetInfo() const {
    return LandVehicle::GetInfo() +
           "\n  Грузоподъёмность: " + std::to_string(static_cast<int>(cargo_capacity_)) + " т";
}

double Truck::GetCargoCapacity() const { return cargo_capacity_; }
```

### `src/vehicles/boat.h`

```cpp
#ifndef BOAT_H
#define BOAT_H

#include "water_vehicle.h"

// Уровень 3 — катер/лодка.
class Boat : public WaterVehicle {
public:
    Boat(const std::string& name, double max_speed,
         double engine_hp, Engine::FuelType fuel_type,
         double displacement, int passenger_capacity);
    ~Boat() override;

    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    int GetPassengerCapacity() const;

private:
    int passenger_capacity_;
};

#endif // BOAT_H
```

### `src/vehicles/boat.cpp`

```cpp
#include "boat.h"
#include <iostream>

Boat::Boat(const std::string& name, double max_speed,
           double engine_hp, Engine::FuelType fuel_type,
           double displacement, int passenger_capacity)
    : WaterVehicle(name, max_speed, engine_hp, fuel_type, displacement),
      passenger_capacity_(passenger_capacity) {
    std::cout << "[Boat] Создан: " << name_ << "\n";
}

Boat::~Boat() {
    std::cout << "[Boat] Уничтожен: " << name_ << "\n";
}

std::string Boat::GetType() const { return "Катер"; }
std::string Boat::Move() const { return name_ + " плывёт по воде ⛵"; }

std::string Boat::GetInfo() const {
    return WaterVehicle::GetInfo() +
           "\n  Пассажиров: " + std::to_string(passenger_capacity_);
}

int Boat::GetPassengerCapacity() const { return passenger_capacity_; }
```

### `src/vehicles/submarine.h`

```cpp
#ifndef SUBMARINE_H
#define SUBMARINE_H

#include "water_vehicle.h"

// Уровень 3 — подводная лодка.
class Submarine : public WaterVehicle {
public:
    Submarine(const std::string& name, double max_speed,
              double engine_hp, Engine::FuelType fuel_type,
              double displacement, double max_depth);
    ~Submarine() override;

    std::string GetType() const override;
    std::string Move() const override;
    std::string GetInfo() const override;

    double GetMaxDepth() const;

private:
    double max_depth_;  // Максимальная глубина погружения
};

#endif // SUBMARINE_H
```

### `src/vehicles/submarine.cpp`

```cpp
#include "submarine.h"
#include <iostream>

Submarine::Submarine(const std::string& name, double max_speed,
                     double engine_hp, Engine::FuelType fuel_type,
                     double displacement, double max_depth)
    : WaterVehicle(name, max_speed, engine_hp, fuel_type, displacement),
      max_depth_(max_depth) {
    std::cout << "[Submarine] Создан: " << name_ << "\n";
}

Submarine::~Submarine() {
    std::cout << "[Submarine] Уничтожен: " << name_ << "\n";
}

std::string Submarine::GetType() const { return "Подводная лодка"; }
std::string Submarine::Move() const { return name_ + " погружается под воду 🚢"; }

std::string Submarine::GetInfo() const {
    return WaterVehicle::GetInfo() +
           "\n  Макс. глубина: " + std::to_string(static_cast<int>(max_depth_)) + " м";
}

double Submarine::GetMaxDepth() const { return max_depth_; }
```

### `src/ui/main_window.h`

```cpp
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QLineEdit>

#include "fleet.h"
#include "driver.h"
#include <vector>
#include <memory>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void OnAddVehicle();
    void OnRemoveVehicle();
    void OnRunPolymorphism();
    void OnRunExceptions();

private:
    void SetupFleetTab();
    void SetupPolymorphismTab();
    void SetupExceptionsTab();
    void SetupAboutTab();
    void PopulateDefaultFleet();
    void RefreshFleetView();

    // Виджет с квадратами для отображения объектов
    QWidget* CreateVehicleCard(const Vehicle& vehicle, int index);

    QTabWidget* tabs_;

    // Вкладка "Флот"
    QWidget* fleet_container_;
    QVBoxLayout* fleet_layout_;
    QScrollArea* fleet_scroll_;
    QTextEdit* detail_text_;

    // Вкладка "Полиморфизм"
    QTextEdit* poly_output_;

    // Вкладка "Исключения"
    QTextEdit* exception_output_;

    // Данные
    Fleet fleet_;
    std::vector<std::unique_ptr<Driver>> drivers_;
    int selected_index_ = -1;
};

#endif // MAIN_WINDOW_H
```

### `src/ui/main_window.cpp`

```cpp
#include "main_window.h"
#include "boat.h"
#include "car.h"
#include "exceptions.h"
#include "submarine.h"
#include "truck.h"

#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("ООП: Иерархия транспортных средств");
  setMinimumSize(900, 600);

  tabs_ = new QTabWidget(this);
  setCentralWidget(tabs_);

  SetupFleetTab();
  SetupPolymorphismTab();
  SetupExceptionsTab();
  SetupAboutTab();

  PopulateDefaultFleet();
  RefreshFleetView();
}

MainWindow::~MainWindow() = default;

// --- Вкладка "Флот" ---
void MainWindow::SetupFleetTab() {
  auto *tab = new QWidget();
  auto *main_layout = new QHBoxLayout(tab);

  // Левая часть — квадраты с объектами
  auto *left = new QVBoxLayout();

  fleet_scroll_ = new QScrollArea();
  fleet_scroll_->setWidgetResizable(true);
  fleet_container_ = new QWidget();
  fleet_layout_ = new QVBoxLayout(fleet_container_);
  fleet_scroll_->setWidget(fleet_container_);
  left->addWidget(fleet_scroll_);

  auto *btn_layout = new QHBoxLayout();
  auto *add_btn = new QPushButton("Добавить ТС");
  auto *remove_btn = new QPushButton("Удалить выбранное");
  btn_layout->addWidget(add_btn);
  btn_layout->addWidget(remove_btn);
  left->addLayout(btn_layout);

  connect(add_btn, &QPushButton::clicked, this, &MainWindow::OnAddVehicle);
  connect(remove_btn, &QPushButton::clicked, this,
          &MainWindow::OnRemoveVehicle);

  detail_text_ = new QTextEdit();
  detail_text_->setReadOnly(true);
  detail_text_->setFont(QFont("Courier", 12));

  main_layout->addLayout(left, 2);
  main_layout->addWidget(detail_text_, 3);

  tabs_->addTab(tab, "Флот");
}

// --- Вкладка "Полиморфизм" ---
void MainWindow::SetupPolymorphismTab() {
  auto *tab = new QWidget();
  auto *layout = new QVBoxLayout(tab);

  auto *btn = new QPushButton("Вызвать Move() и GetInfo() на всех объектах");
  poly_output_ = new QTextEdit();
  poly_output_->setReadOnly(true);
  poly_output_->setFont(QFont("Courier", 11));

  connect(btn, &QPushButton::clicked, this, &MainWindow::OnRunPolymorphism);

  layout->addWidget(btn);
  layout->addWidget(poly_output_);
  tabs_->addTab(tab, "Полиморфизм");
}

// --- Вкладка "Исключения" ---
void MainWindow::SetupExceptionsTab() {
  auto *tab = new QWidget();
  auto *layout = new QVBoxLayout(tab);

  auto *btn = new QPushButton("Демонстрация исключений");
  exception_output_ = new QTextEdit();
  exception_output_->setReadOnly(true);
  exception_output_->setFont(QFont("Courier", 11));

  connect(btn, &QPushButton::clicked, this, &MainWindow::OnRunExceptions);

  layout->addWidget(btn);
  layout->addWidget(exception_output_);
  tabs_->addTab(tab, "Исключения");
}

// --- Вкладка "О классах" ---
void MainWindow::SetupAboutTab() {
  auto *tab = new QWidget();
  auto *layout = new QVBoxLayout(tab);

  auto *text = new QTextEdit();
  text->setReadOnly(true);
  text->setFont(QFont("Courier", 11));
  text->setText(
      "=== ИЕРАРХИЯ КЛАССОВ ===\n\n"
      "Vehicle (абстрактный базовый класс)\n"
      "├── LandVehicle (наземный, абстрактный)\n"
      "│   ├── Car (легковой автомобиль)\n"
      "│   └── Truck (грузовик)\n"
      "└── WaterVehicle (водный, абстрактный)\n"
      "    ├── Boat (катер)\n"
      "    └── Submarine (подводная лодка)\n\n"
      "Engine — КОМПОЗИЦИЯ (создаётся и уничтожается вместе с Vehicle)\n"
      "Driver — АГРЕГАЦИЯ (существует независимо от Vehicle)\n"
      "Fleet  — КОЛЛЕКЦИЯ с operator[] (индексатор)\n\n"
      "=== ЧТО СОДЕРЖИТ КЛАСС ===\n\n"
      "• Поля (fields) — данные: max_speed_, name_, engine_\n"
      "• Методы (methods) — поведение: Move(), GetInfo()\n"
      "• Свойства (properties) — геттеры/сеттеры: GetName(), "
      "SetCurrentSpeed()\n"
      "• Конструктор — инициализация объекта: Vehicle(...)\n"
      "• Деструктор — освобождение ресурсов: ~Vehicle()\n"
      "• Индексатор — operator[] в Fleet для доступа по индексу\n"
      "• Атрибуты — модификаторы: virtual, override, const, explicit\n\n"
      "=== ПРИНЦИПЫ ООП ===\n\n"
      "ИНКАПСУЛЯЦИЯ:\n"
      "  Поля private, доступ через public геттеры/сеттеры.\n"
      "  Пример: max_speed_ — private, GetMaxSpeed() — public.\n\n"
      "НАСЛЕДОВАНИЕ:\n"
      "  3 уровня: Vehicle → LandVehicle → Car\n"
      "  Переиспользование кода базовых классов.\n\n"
      "ПОЛИМОРФИЗМ:\n"
      "  virtual методы Move(), GetInfo() — разное поведение\n"
      "  для Car, Truck, Boat, Submarine при вызове через Vehicle*.\n\n"
      "АБСТРАКЦИЯ:\n"
      "  Vehicle — абстрактный класс (= 0 методы).\n"
      "  Нельзя создать экземпляр Vehicle напрямую.\n\n"
      "=== АГРЕГАЦИЯ vs КОМПОЗИЦИЯ ===\n\n"
      "КОМПОЗИЦИЯ (Engine внутри Vehicle):\n"
      "  Engine создаётся в конструкторе Vehicle.\n"
      "  При уничтожении Vehicle двигатель тоже уничтожается.\n"
      "  Engine не существует без Vehicle.\n\n"
      "АГРЕГАЦИЯ (Driver* в Vehicle):\n"
      "  Driver существует независимо от Vehicle.\n"
      "  Vehicle хранит указатель, но не владеет Driver.\n"
      "  При уничтожении Vehicle, Driver продолжает жить.\n\n"
      "=== КЛАСС vs СТРУКТУРА (C++) ===\n\n"
      "• class: по умолчанию private доступ\n"
      "• struct: по умолчанию public доступ\n"
      "• Оба поддерживают наследование, методы, конструкторы\n"
      "• Класс — для сложных объектов с инвариантами\n"
      "• Структура — для простых данных (POD)\n\n"
      "=== ПЕРЕДАЧА ПАРАМЕТРОВ ===\n\n"
      "• По значению (int x) — копия, изменения не видны\n"
      "• По ссылке (int& x) — оригинал, изменения видны\n"
      "• По const-ссылке (const string& s) — без копии, без изменений\n"
      "• По указателю (int* x) — адрес, можно nullptr\n"
      "• move-семантика (string&& s) — передача владения\n\n"
      "=== НОТАЦИИ ИМЕНОВАНИЯ ===\n\n"
      "• Классы: PascalCase (LandVehicle, WaterVehicle)\n"
      "• Методы: PascalCase (GetMaxSpeed, Move)\n"
      "• Приватные поля: snake_case_ (max_speed_, name_)\n"
      "• Локальные переменные: snake_case (wheel_count)\n"
      "• Константы: kConstantName (kMaxSpeed)\n"
      "• Файлы: snake_case.cpp (land_vehicle.cpp)\n");

  layout->addWidget(text);
  tabs_->addTab(tab, "О классах");
}

// --- Заполнение флота ---
void MainWindow::PopulateDefaultFleet() {
  drivers_.push_back(std::make_unique<Driver>("Иванов И.И.", 10));
  drivers_.push_back(std::make_unique<Driver>("Петров П.П.", 5));

  auto car = std::make_unique<Car>("Toyota Camry", 210, 180,
                                   Engine::FuelType::kPetrol, 5);
  car->AssignDriver(drivers_[0].get());
  fleet_.Add(std::move(car));

  fleet_.Add(std::make_unique<Truck>("Volvo FH16", 120, 540,
                                     Engine::FuelType::kDiesel, 25));

  auto boat = std::make_unique<Boat>("Yamaha 242X", 80, 320,
                                     Engine::FuelType::kPetrol, 50, 12);
  boat->AssignDriver(drivers_[1].get());
  fleet_.Add(std::move(boat));

  fleet_.Add(std::make_unique<Submarine>(
      "Наутилус", 55, 1000, Engine::FuelType::kNuclear, 8000, 500));
}

// --- Создание карточки-квадрата ---
QWidget *MainWindow::CreateVehicleCard(const Vehicle &vehicle, int index) {
  auto *card = new QFrame();
  card->setFrameStyle(QFrame::Box | QFrame::Raised);
  card->setLineWidth(2);
  card->setFixedSize(180, 80);
  card->setCursor(Qt::PointingHandCursor);

  // Цвет зависит от типа
  QString color;
  if (dynamic_cast<const Car *>(&vehicle))
    color = "#4CAF50";
  else if (dynamic_cast<const Truck *>(&vehicle))
    color = "#FF9800";
  else if (dynamic_cast<const Boat *>(&vehicle))
    color = "#2196F3";
  else if (dynamic_cast<const Submarine *>(&vehicle))
    color = "#9C27B0";
  else
    color = "#757575";

  card->setStyleSheet(
      QString("QFrame { background-color: %1; border-radius: 8px; }"
              "QLabel { color: white; }")
          .arg(color));

  auto *layout = new QVBoxLayout(card);
  layout->setContentsMargins(8, 4, 8, 4);

  auto *type_label = new QLabel(QString::fromStdString(vehicle.GetType()));
  type_label->setFont(QFont("", 9, QFont::Bold));
  type_label->setAlignment(Qt::AlignCenter);

  auto *name_label = new QLabel(QString::fromStdString(vehicle.GetName()));
  name_label->setFont(QFont("", 10));
  name_label->setAlignment(Qt::AlignCenter);

  layout->addWidget(type_label);
  layout->addWidget(name_label);

  // Клик — выбрать
  card->setProperty("index", index);
  card->installEventFilter(this);

  return card;
}

// --- Обновление вида флота ---
void MainWindow::RefreshFleetView() {
  // Удалить старые виджеты
  QLayoutItem *item;
  while ((item = fleet_layout_->takeAt(0)) != nullptr) {
    delete item->widget();
    delete item;
  }

  // Создать сетку квадратов
  auto *grid = new QGridLayout();
  int cols = 3;
  for (size_t i = 0; i < fleet_.Size(); ++i) {
    auto *card = CreateVehicleCard(fleet_[i], static_cast<int>(i));
    grid->addWidget(card, static_cast<int>(i) / cols,
                    static_cast<int>(i) % cols);
  }

  auto *grid_widget = new QWidget();
  grid_widget->setLayout(grid);
  fleet_layout_->addWidget(grid_widget);
  fleet_layout_->addStretch();
}

// --- eventFilter для кликов по карточкам ---
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    auto *frame = qobject_cast<QFrame *>(obj);
    if (frame) {
      bool ok;
      int idx = frame->property("index").toInt(&ok);
      if (ok && idx >= 0 && static_cast<size_t>(idx) < fleet_.Size()) {
        selected_index_ = idx;
        detail_text_->setText(QString::fromStdString(fleet_[idx].GetInfo()));
      }
      return true;
    }
  }
  return QMainWindow::eventFilter(obj, event);
}

// --- Слоты ---

void MainWindow::OnAddVehicle() {
  QStringList types = {"Легковой (Car)", "Грузовик (Truck)", "Катер (Boat)",
                       "Подводная лодка (Submarine)"};
  bool ok;
  QString type =
      QInputDialog::getItem(this, "Добавить ТС", "Тип:", types, 0, false, &ok);
  if (!ok)
    return;

  QString name = QInputDialog::getText(
      this, "Название", "Введите название:", QLineEdit::Normal, "", &ok);
  if (!ok || name.isEmpty())
    return;

  std::string n = name.toStdString();

  try {
    if (type.startsWith("Легковой")) {
      fleet_.Add(
          std::make_unique<Car>(n, 200, 150, Engine::FuelType::kPetrol, 5));
    } else if (type.startsWith("Грузовик")) {
      fleet_.Add(
          std::make_unique<Truck>(n, 110, 400, Engine::FuelType::kDiesel, 20));
    } else if (type.startsWith("Катер")) {
      fleet_.Add(
          std::make_unique<Boat>(n, 70, 250, Engine::FuelType::kPetrol, 40, 8));
    } else {
      fleet_.Add(std::make_unique<Submarine>(
          n, 45, 800, Engine::FuelType::kNuclear, 5000, 400));
    }
    RefreshFleetView();
  } catch (const VehicleException &e) {
    QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
  }
}

void MainWindow::OnRemoveVehicle() {
  if (selected_index_ < 0 ||
      static_cast<size_t>(selected_index_) >= fleet_.Size()) {
    QMessageBox::information(this, "Удаление",
                             "Выберите ТС кликом на карточку");
    return;
  }
  try {
    fleet_.Remove(static_cast<size_t>(selected_index_));
    selected_index_ = -1;
    detail_text_->clear();
    RefreshFleetView();
  } catch (const FleetIndexException &e) {
    QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
  }
}

void MainWindow::OnRunPolymorphism() {
  QString output;
  output += "=== Полиморфизм: вызов Move() через Vehicle* ===\n\n";
  output += QString::fromStdString(fleet_.MoveAll());
  output += "\n=== Полиморфизм: вызов GetInfo() через Vehicle* ===\n\n";
  output += QString::fromStdString(fleet_.GetAllInfo());
  output += "=== Полиморфизм: GetMaxSpeed() ===\n\n";
  for (size_t i = 0; i < fleet_.Size(); ++i) {
    const auto &v = fleet_[i];
    output += QString::fromStdString(
        v.GetName() + ": " + std::to_string(static_cast<int>(v.GetMaxSpeed())) +
        " км/ч\n");
  }
  poly_output_->setText(output);
}

void MainWindow::OnRunExceptions() {
  QString output;

  // 1. SpeedLimitException
  output += "=== Тест 1: Превышение скорости ===\n";
  try {
    if (fleet_.Size() > 0) {
      fleet_[0].SetCurrentSpeed(9999);
    }
  } catch (const SpeedLimitException &e) {
    output += QString("ПОЙМАНО: %1\n").arg(e.what());
    output += QString("  Скорость: %1, Лимит: %2\n\n")
                  .arg(e.GetSpeed())
                  .arg(e.GetLimit());
  }

  // 2. FleetIndexException
  output += "=== Тест 2: Индекс вне диапазона ===\n";
  try {
    auto &v = fleet_[999];
    (void)v;
  } catch (const FleetIndexException &e) {
    output += QString("ПОЙМАНО: %1\n").arg(e.what());
    output += QString("  Индекс: %1, Размер: %2\n\n")
                  .arg(e.GetIndex())
                  .arg(e.GetSize());
  }

  // 3. EngineException
  output += "=== Тест 3: Ошибка двигателя ===\n";
  try {
    Engine bad_engine(-100, Engine::FuelType::kPetrol);
  } catch (const EngineException &e) {
    output += QString("ПОЙМАНО: %1\n\n").arg(e.what());
  }

  // 4. Общий catch
  output += "=== Тест 4: Базовый VehicleException ===\n";
  try {
    throw VehicleException("Тестовое исключение");
  } catch (const VehicleException &e) {
    output += QString("ПОЙМАНО (базовый тип): %1\n\n").arg(e.what());
  }

  // 5. std::exception catch
  output += "=== Тест 5: std::exception ===\n";
  try {
    throw SpeedLimitException(500, 200);
  } catch (const std::exception &e) {
    output += QString("ПОЙМАНО через std::exception: %1\n").arg(e.what());
  }

  exception_output_->setText(output);
}
```
