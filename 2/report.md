**Отчёт по лабораторной работе 2 (ООП, C++/Qt)**

**гр. ______, ФИО ______**

Реализована иерархия транспортных средств: абстрактный `Vehicle`, промежуточные `LandVehicle` и `WaterVehicle`, конкретные классы `Car`, `Truck`, `Boat`, `Submarine`. Дополнительно: `Engine` (композиция внутри `Vehicle`), `Driver` (агрегация по указателю), `Fleet` — коллекция `std::vector<std::unique_ptr<Vehicle>>` с `operator[]`, полиморфными `GetAllInfo()` и `MoveAll()`.

Паттерн **Factory Method**: абстрактный класс `VehicleFactory` объявляет `Create(const std::string& name)` и `GetFactoryName()`; для каждого вида ТС есть конкретная фабрика (`CarFactory`, `TruckFactory`, `BoatFactory`, `SubmarineFactory`). Тип выбирается перечислением `VehicleType`; функция `GetFactory(VehicleType)` возвращает `std::unique_ptr<VehicleFactory>`. В `Create()` задаются фиксированные параметры конструктора (скорость, мощность, топливо, грузоподъёмность и т.д.) — пользователь при добавлении вводит только имя и тип из списка.

Интерфейс на **Qt 6** (`MainWindow`): вкладки «Флот» (карточки, детали по клику, добавление через фабрику, удаление), «Фабрика» (демонстрация создания четырёх типов через фабрики), «Полиморфизм» (`Move()`, `GetInfo()`, `GetMaxSpeed()` по коллекции), «Исключения» (ловля `SpeedLimitException`, `FleetIndexException`, `EngineException`, базового `VehicleException`, `std::exception`), «О классах» (текстовое описание иерархии и принципов ООП). Точка входа — `main.cpp`: `QApplication`, показ `MainWindow`.

**Как работает:**

1) Пользователь на вкладке «Флот» выбирает тип ТС и имя; `OnAddVehicle` сопоставляет строку из `QInputDialog` с `VehicleType`, вызывает `GetFactory(vtype)` и `factory->Create(name)`; объект передаётся в `Fleet::Add(std::unique_ptr<Vehicle>)`.

2) Вкладка «Фабрика» по кнопке перебирает все `VehicleType`, для каждого создаёт фабрику через `GetFactory`, выводит `GetFactoryName()`, `GetInfo()` и `Move()` созданного объекта (объекты локальные, только для демонстрации).

3) Вкладка «Полиморфизм» вызывает `fleet_.MoveAll()` и `fleet_.GetAllInfo()` — внутри `Fleet` итерация по `unique_ptr<Vehicle>` и виртуальные методы базового интерфейса.

4) Исключения: `SetCurrentSpeed` проверяет лимит; `Fleet::operator[]` бросает `FleetIndexException` при выходе за границы; некорректные параметры двигателя — `EngineException`; `GetFactory` при неизвестном значении — `VehicleException`.

**Файл `src/patterns/vehicle_factory.h`:**

```cpp
enum class VehicleType { kCar, kTruck, kBoat, kSubmarine };

class VehicleFactory {
public:
    virtual ~VehicleFactory() = default;
    virtual std::unique_ptr<Vehicle> Create(const std::string& name) const = 0;
    virtual std::string GetFactoryName() const = 0;
};

// CarFactory, TruckFactory, BoatFactory, SubmarineFactory — наследники с override

std::unique_ptr<VehicleFactory> GetFactory(VehicleType type);
```

**Файл `src/patterns/vehicle_factory.cpp` (фрагмент `GetFactory`):**

```cpp
std::unique_ptr<VehicleFactory> GetFactory(VehicleType type) {
    switch (type) {
        case VehicleType::kCar:       return std::make_unique<CarFactory>();
        case VehicleType::kTruck:     return std::make_unique<TruckFactory>();
        case VehicleType::kBoat:      return std::make_unique<BoatFactory>();
        case VehicleType::kSubmarine: return std::make_unique<SubmarineFactory>();
    }
    throw VehicleException("Неизвестный тип транспорта");
}
```

**Пример создания ТС через фабрику (`src/ui/main_window.cpp`, `OnAddVehicle`):**

```cpp
VehicleType vtype;
if (type.startsWith("Легковой"))       vtype = VehicleType::kCar;
else if (type.startsWith("Грузовик"))  vtype = VehicleType::kTruck;
else if (type.startsWith("Катер"))     vtype = VehicleType::kBoat;
else                                    vtype = VehicleType::kSubmarine;

auto factory = GetFactory(vtype);
fleet_.Add(factory->Create(name.toStdString()));
```

**Файл `src/components/fleet.h` (коллекция и индексатор):**

```cpp
void Add(std::unique_ptr<Vehicle> vehicle);
Vehicle& operator[](size_t index);
const Vehicle& operator[](size_t index) const;
std::string GetAllInfo() const;
std::string MoveAll() const;

private:
    std::vector<std::unique_ptr<Vehicle>> vehicles_;
```

**Сборка:** CMake (`CMakeLists.txt`), подключение Qt Widgets, исходники в `src/` (подкаталоги `vehicles/`, `components/`, `patterns/`, `ui/`).
