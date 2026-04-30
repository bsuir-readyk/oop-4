#include "main_window.h"
#include "vehicle_factory.h"
#include "car.h"
#include "truck.h"
#include "boat.h"
#include "submarine.h"
#include "exceptions.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QFont>
#include <QPalette>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QDialog>
#include <chrono>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("ООП: Иерархия транспортных средств — Лаб. 3 (Сериализация)");
    setMinimumSize(1000, 650);

    tabs_ = new QTabWidget(this);
    setCentralWidget(tabs_);

    SetupFleetTab();
    SetupSerializationTab();
    SetupFactoryTab();
    SetupPolymorphismTab();
    SetupExceptionsTab();
    SetupAboutTab();

    PopulateDefaultFleet();
    RefreshFleetView();
}

MainWindow::~MainWindow() = default;

// --- Вкладка "Флот" ---
void MainWindow::SetupFleetTab() {
    auto* tab = new QWidget();
    auto* main_layout = new QHBoxLayout(tab);

    // Левая часть — квадраты с объектами
    auto* left = new QVBoxLayout();

    fleet_scroll_ = new QScrollArea();
    fleet_scroll_->setWidgetResizable(true);
    fleet_container_ = new QWidget();
    fleet_layout_ = new QVBoxLayout(fleet_container_);
    fleet_scroll_->setWidget(fleet_container_);
    left->addWidget(fleet_scroll_);

    auto* btn_layout = new QHBoxLayout();
    auto* add_btn = new QPushButton("Добавить ТС");
    auto* remove_btn = new QPushButton("Удалить выбранное");
    auto* edit_btn = new QPushButton("Редактировать");
    btn_layout->addWidget(add_btn);
    btn_layout->addWidget(edit_btn);
    btn_layout->addWidget(remove_btn);
    left->addLayout(btn_layout);

    connect(add_btn, &QPushButton::clicked, this, &MainWindow::OnAddVehicle);
    connect(remove_btn, &QPushButton::clicked, this, &MainWindow::OnRemoveVehicle);
    connect(edit_btn, &QPushButton::clicked, this, &MainWindow::OnEditVehicle);

    // Правая часть — детали
    detail_text_ = new QTextEdit();
    detail_text_->setReadOnly(true);
    detail_text_->setFont(QFont("Menlo", 12));

    main_layout->addLayout(left, 2);
    main_layout->addWidget(detail_text_, 3);

    tabs_->addTab(tab, "Флот");
}

// --- Вкладка "Сериализация" ---
void MainWindow::SetupSerializationTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    auto* btn_row = new QHBoxLayout();
    auto* save_custom_btn = new QPushButton("Сохранить (текст)");
    auto* load_custom_btn = new QPushButton("Загрузить (текст)");
    auto* save_json_btn = new QPushButton("Сохранить (JSON)");
    auto* load_json_btn = new QPushButton("Загрузить (JSON)");

    btn_row->addWidget(save_custom_btn);
    btn_row->addWidget(load_custom_btn);
    btn_row->addWidget(save_json_btn);
    btn_row->addWidget(load_json_btn);
    layout->addLayout(btn_row);

    connect(save_custom_btn, &QPushButton::clicked, this, &MainWindow::OnSaveCustom);
    connect(load_custom_btn, &QPushButton::clicked, this, &MainWindow::OnLoadCustom);
    connect(save_json_btn, &QPushButton::clicked, this, &MainWindow::OnSaveJson);
    connect(load_json_btn, &QPushButton::clicked, this, &MainWindow::OnLoadJson);

    serial_output_ = new QTextEdit();
    serial_output_->setReadOnly(true);
    serial_output_->setFont(QFont("Menlo", 11));

    // Теория по умолчанию
    serial_output_->setText(
        "=== СЕРИАЛИЗАЦИЯ ОБЪЕКТОВ ===\n\n"
        "Сериализация — процесс преобразования объекта в последовательность\n"
        "байтов (или текст) для сохранения или передачи.\n"
        "Десериализация — обратный процесс.\n\n"
        "ЧТО МОЖНО СЕРИАЛИЗОВАТЬ?\n"
        "• Состояние (state) — значения полей объекта. ДА.\n"
        "• Поведение (behavior) — методы, код. НЕТ напрямую.\n"
        "  Поведение восстанавливается через тип класса при десериализации.\n\n"
        "=== ГЛУБОКАЯ vs ПОВЕРХНОСТНАЯ СЕРИАЛИЗАЦИЯ ===\n\n"
        "Поверхностная (shallow): сохраняет только ссылки/указатели.\n"
        "  Проблема: при десериализации ссылки становятся невалидными.\n\n"
        "Глубокая (deep): сохраняет полные копии всех вложенных объектов.\n"
        "  Engine (композиция) → всегда сериализуется вместе с Vehicle.\n"
        "  Driver (агрегация) → при глубокой сериализации копируются данные.\n"
        "  ВАЖНО: после десериализации ссылки на оригинальные объекты\n"
        "  теряются — создаются новые копии.\n\n"
        "=== СРАВНЕНИЕ ФОРМАТОВ ===\n\n"
        "┌──────────────┬───────────┬──────────┬──────────┬──────────────┐\n"
        "│ Критерий     │ JSON      │ XML      │ Бинарная │ Своя текст.  │\n"
        "├──────────────┼───────────┼──────────┼──────────┼──────────────┤\n"
        "│ Читаемость   │ Высокая   │ Высокая  │ Нет      │ Высокая      │\n"
        "│ Размер       │ Средний   │ Большой  │ Малый    │ Средний      │\n"
        "│ Скорость     │ Средняя   │ Медленно │ Быстро   │ Быстро       │\n"
        "│ Стандарт     │ RFC 8259  │ W3C      │ Нет      │ Нет          │\n"
        "│ Типизация    │ Слабая    │ XSD      │ Строгая  │ Нет          │\n"
        "│ Совместим.   │ Высокая   │ Высокая  │ Низкая   │ Низкая       │\n"
        "│ Вложенность  │ Да        │ Да       │ Да       │ Ограничено   │\n"
        "│ Надёжность   │ Средняя   │ Высокая  │ Высокая  │ Низкая       │\n"
        "└──────────────┴───────────┴──────────┴──────────┴──────────────┘\n\n"
        "=== СПОСОБЫ СЕРИАЛИЗАЦИИ ===\n\n"
        "1. Кастомная (через интерфейс ISerializable)\n"
        "   + Полный контроль над форматом\n"
        "   + Максимальная производительность\n"
        "   - Нужно писать вручную для каждого класса\n"
        "   - Легко сломать при изменении класса\n\n"
        "2. JSON (nlohmann/json)\n"
        "   + Человекочитаемый формат\n"
        "   + Стандартный, поддержка во всех языках\n"
        "   + Легко отлаживать\n"
        "   - Избыточность (ключи повторяются)\n"
        "   - Нет схемы по умолчанию\n\n"
        "3. XML (с Data Contract / схемами)\n"
        "   + Строгая типизация через XSD\n"
        "   + Контрактная сериализация (DataContract)\n"
        "   + Пространства имён, XSLT-трансформации\n"
        "   - Очень verbose, большой размер\n"
        "   - Медленный парсинг\n\n"
        "4. Бинарная (protobuf, msgpack, и т.д.)\n"
        "   + Минимальный размер\n"
        "   + Максимальная скорость\n"
        "   - Нечитаемый формат\n"
        "   - Сложнее отлаживать\n"
        "   - Зависимость от платформы/версии\n\n"
        "=== DATA CONTRACT (Контрактная сериализация) ===\n\n"
        "Data Contract — подход, при котором сериализуемые поля явно\n"
        "помечаются атрибутами (аннотациями). Только помеченные поля\n"
        "попадают в сериализованное представление.\n\n"
        "Аналог в C++: наш интерфейс ISerializable.\n"
        "В .NET: [DataContract] + [DataMember] атрибуты.\n"
        "В Java: Serializable интерфейс + transient модификатор.\n\n"
        "Преимущества:\n"
        "• Явный контроль — что сериализуется, а что нет\n"
        "• Версионирование — можно добавлять/удалять поля\n"
        "• Безопасность — скрытые поля не утекают\n\n"
        "Нажмите кнопки выше для сохранения/загрузки флота.\n"
        "Результат сериализации будет показан здесь.\n"
    );

    layout->addWidget(serial_output_);
    tabs_->addTab(tab, "Сериализация");
}

// --- Вкладка "Фабрика" ---
void MainWindow::SetupFactoryTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    auto* btn = new QPushButton("Демонстрация паттерна Factory Method");
    factory_output_ = new QTextEdit();
    factory_output_->setReadOnly(true);
    factory_output_->setFont(QFont("Menlo", 11));

    connect(btn, &QPushButton::clicked, this, &MainWindow::OnRunFactory);

    layout->addWidget(btn);
    layout->addWidget(factory_output_);
    tabs_->addTab(tab, "Фабрика");
}

// --- Вкладка "Полиморфизм" ---
void MainWindow::SetupPolymorphismTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    auto* btn = new QPushButton("Вызвать Move() и GetInfo() на всех объектах");
    poly_output_ = new QTextEdit();
    poly_output_->setReadOnly(true);
    poly_output_->setFont(QFont("Menlo", 11));

    connect(btn, &QPushButton::clicked, this, &MainWindow::OnRunPolymorphism);

    layout->addWidget(btn);
    layout->addWidget(poly_output_);
    tabs_->addTab(tab, "Полиморфизм");
}

// --- Вкладка "Исключения" ---
void MainWindow::SetupExceptionsTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    auto* btn = new QPushButton("Демонстрация исключений");
    exception_output_ = new QTextEdit();
    exception_output_->setReadOnly(true);
    exception_output_->setFont(QFont("Menlo", 11));

    connect(btn, &QPushButton::clicked, this, &MainWindow::OnRunExceptions);

    layout->addWidget(btn);
    layout->addWidget(exception_output_);
    tabs_->addTab(tab, "Исключения");
}

// --- Вкладка "О классах" ---
void MainWindow::SetupAboutTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    auto* text = new QTextEdit();
    text->setReadOnly(true);
    text->setFont(QFont("Menlo", 11));
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
        "ISerializable — ИНТЕРФЕЙС кастомной сериализации\n"
        "JsonSerializer — JSON сериализация (nlohmann/json)\n\n"
        "=== ЧТО СОДЕРЖИТ КЛАСС ===\n\n"
        "• Поля (fields) — данные: max_speed_, name_, engine_\n"
        "• Методы (methods) — поведение: Move(), GetInfo()\n"
        "• Свойства (properties) — геттеры/сеттеры: GetName(), SetCurrentSpeed()\n"
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
        "• Файлы: snake_case.cpp (land_vehicle.cpp)\n"
    );

    layout->addWidget(text);
    tabs_->addTab(tab, "О классах");
}

// --- Заполнение флота ---
void MainWindow::PopulateDefaultFleet() {
    drivers_.push_back(std::make_unique<Driver>("Иванов И.И.", 10));
    drivers_.push_back(std::make_unique<Driver>("Петров П.П.", 5));

    auto car = std::make_unique<Car>("Toyota Camry", 210, 180, Engine::FuelType::kPetrol, 5);
    car->AssignDriver(drivers_[0].get());
    fleet_.Add(std::move(car));

    fleet_.Add(std::make_unique<Truck>("Volvo FH16", 120, 540, Engine::FuelType::kDiesel, 25));

    auto boat = std::make_unique<Boat>("Yamaha 242X", 80, 320, Engine::FuelType::kPetrol, 50, 12);
    boat->AssignDriver(drivers_[1].get());
    fleet_.Add(std::move(boat));

    fleet_.Add(std::make_unique<Submarine>("Наутилус", 55, 1000, Engine::FuelType::kNuclear, 8000, 500));
}

// --- Создание карточки-квадрата ---
QWidget* MainWindow::CreateVehicleCard(const Vehicle& vehicle, int index) {
    auto* card = new QFrame();
    card->setFrameStyle(QFrame::Box | QFrame::Raised);
    card->setLineWidth(2);
    card->setFixedSize(180, 80);
    card->setCursor(Qt::PointingHandCursor);

    // Цвет зависит от типа
    QString color;
    if (dynamic_cast<const Car*>(&vehicle)) color = "#4CAF50";
    else if (dynamic_cast<const Truck*>(&vehicle)) color = "#FF9800";
    else if (dynamic_cast<const Boat*>(&vehicle)) color = "#2196F3";
    else if (dynamic_cast<const Submarine*>(&vehicle)) color = "#9C27B0";
    else color = "#757575";

    card->setStyleSheet(
        QString("QFrame { background-color: %1; border-radius: 8px; }"
                "QLabel { color: white; }").arg(color));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(8, 4, 8, 4);

    auto* type_label = new QLabel(QString::fromStdString(vehicle.GetType()));
    type_label->setFont(QFont("", 9, QFont::Bold));
    type_label->setAlignment(Qt::AlignCenter);

    auto* name_label = new QLabel(QString::fromStdString(vehicle.GetName()));
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
    QLayoutItem* item;
    while ((item = fleet_layout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Создать сетку квадратов
    auto* grid = new QGridLayout();
    int cols = 3;
    for (size_t i = 0; i < fleet_.Size(); ++i) {
        auto* card = CreateVehicleCard(fleet_[i], static_cast<int>(i));
        grid->addWidget(card, static_cast<int>(i) / cols, static_cast<int>(i) % cols);
    }

    auto* grid_widget = new QWidget();
    grid_widget->setLayout(grid);
    fleet_layout_->addWidget(grid_widget);
    fleet_layout_->addStretch();
}

// --- eventFilter для кликов по карточкам ---
bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        auto* frame = qobject_cast<QFrame*>(obj);
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
    QStringList types = {"Легковой (Car)", "Грузовик (Truck)", "Катер (Boat)", "Подводная лодка (Submarine)"};
    bool ok;
    QString type = QInputDialog::getItem(this, "Добавить ТС", "Тип:", types, 0, false, &ok);
    if (!ok) return;

    QString name = QInputDialog::getText(this, "Название", "Введите название:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    try {
        VehicleType vtype;
        if (type.startsWith("Легковой"))       vtype = VehicleType::kCar;
        else if (type.startsWith("Грузовик"))  vtype = VehicleType::kTruck;
        else if (type.startsWith("Катер"))     vtype = VehicleType::kBoat;
        else                                    vtype = VehicleType::kSubmarine;

        auto factory = GetFactory(vtype);
        fleet_.Add(factory->Create(name.toStdString()));
        RefreshFleetView();
    } catch (const VehicleException& e) {
        QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
    }
}

void MainWindow::OnRemoveVehicle() {
    if (selected_index_ < 0 || static_cast<size_t>(selected_index_) >= fleet_.Size()) {
        QMessageBox::information(this, "Удаление", "Выберите ТС кликом на карточку");
        return;
    }
    try {
        fleet_.Remove(static_cast<size_t>(selected_index_));
        selected_index_ = -1;
        detail_text_->clear();
        RefreshFleetView();
    } catch (const FleetIndexException& e) {
        QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
    }
}

void MainWindow::OnEditVehicle() {
    if (selected_index_ < 0 || static_cast<size_t>(selected_index_) >= fleet_.Size()) {
        QMessageBox::information(this, "Редактирование", "Выберите ТС кликом на карточку");
        return;
    }

    Vehicle& v = fleet_[static_cast<size_t>(selected_index_)];

    QDialog dialog(this);
    dialog.setWindowTitle("Редактировать: " + QString::fromStdString(v.GetName()));
    auto* form = new QFormLayout(&dialog);

    // Общие поля
    auto* name_edit = new QLineEdit(QString::fromStdString(v.GetName()));
    auto* speed_spin = new QDoubleSpinBox();
    speed_spin->setRange(1, 100000);
    speed_spin->setValue(v.GetMaxSpeed());
    auto* hp_spin = new QDoubleSpinBox();
    hp_spin->setRange(1, 100000);
    hp_spin->setValue(v.GetEngine().GetHorsepower());
    auto* fuel_combo = new QComboBox();
    fuel_combo->addItems({"Бензин", "Дизель", "Электро", "Ядерный"});
    fuel_combo->setCurrentIndex(static_cast<int>(v.GetEngine().GetFuelType()));

    form->addRow("Название:", name_edit);
    form->addRow("Макс. скорость:", speed_spin);
    form->addRow("Мощность двиг.:", hp_spin);
    form->addRow("Тип топлива:", fuel_combo);

    // Тип-специфичные поля
    QSpinBox* wheel_spin = nullptr;
    QSpinBox* seat_spin = nullptr;
    QDoubleSpinBox* cargo_spin = nullptr;
    QDoubleSpinBox* disp_spin = nullptr;
    QSpinBox* passenger_spin = nullptr;
    QDoubleSpinBox* depth_spin = nullptr;

    if (auto* car = dynamic_cast<Car*>(&v)) {
        wheel_spin = new QSpinBox(); wheel_spin->setRange(1, 20); wheel_spin->setValue(car->GetWheelCount());
        seat_spin = new QSpinBox(); seat_spin->setRange(1, 100); seat_spin->setValue(car->GetSeatCount());
        form->addRow("Колёса:", wheel_spin);
        form->addRow("Мест:", seat_spin);
    } else if (auto* truck = dynamic_cast<Truck*>(&v)) {
        wheel_spin = new QSpinBox(); wheel_spin->setRange(1, 20); wheel_spin->setValue(truck->GetWheelCount());
        cargo_spin = new QDoubleSpinBox(); cargo_spin->setRange(0.1, 1000); cargo_spin->setValue(truck->GetCargoCapacity());
        form->addRow("Колёса:", wheel_spin);
        form->addRow("Грузоподъёмность (т):", cargo_spin);
    } else if (auto* boat = dynamic_cast<Boat*>(&v)) {
        disp_spin = new QDoubleSpinBox(); disp_spin->setRange(0.1, 100000); disp_spin->setValue(boat->GetDisplacement());
        passenger_spin = new QSpinBox(); passenger_spin->setRange(1, 10000); passenger_spin->setValue(boat->GetPassengerCapacity());
        form->addRow("Водоизмещение (т):", disp_spin);
        form->addRow("Пассажиров:", passenger_spin);
    } else if (auto* sub = dynamic_cast<Submarine*>(&v)) {
        disp_spin = new QDoubleSpinBox(); disp_spin->setRange(0.1, 100000); disp_spin->setValue(sub->GetDisplacement());
        depth_spin = new QDoubleSpinBox(); depth_spin->setRange(1, 11000); depth_spin->setValue(sub->GetMaxDepth());
        form->addRow("Водоизмещение (т):", disp_spin);
        form->addRow("Макс. глубина (м):", depth_spin);
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        v.SetName(name_edit->text().toStdString());
        v.SetMaxSpeed(speed_spin->value());
        v.GetEngine().SetHorsepower(hp_spin->value());
        v.GetEngine().SetFuelType(static_cast<Engine::FuelType>(fuel_combo->currentIndex()));

        if (auto* car = dynamic_cast<Car*>(&v)) {
            if (wheel_spin) car->SetWheelCount(wheel_spin->value());
            if (seat_spin) car->SetSeatCount(seat_spin->value());
        } else if (auto* truck = dynamic_cast<Truck*>(&v)) {
            if (wheel_spin) truck->SetWheelCount(wheel_spin->value());
            if (cargo_spin) truck->SetCargoCapacity(cargo_spin->value());
        } else if (auto* boat = dynamic_cast<Boat*>(&v)) {
            if (disp_spin) boat->SetDisplacement(disp_spin->value());
            if (passenger_spin) boat->SetPassengerCapacity(passenger_spin->value());
        } else if (auto* sub = dynamic_cast<Submarine*>(&v)) {
            if (disp_spin) sub->SetDisplacement(disp_spin->value());
            if (depth_spin) sub->SetMaxDepth(depth_spin->value());
        }

        detail_text_->setText(QString::fromStdString(v.GetInfo()));
        RefreshFleetView();
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
        const auto& v = fleet_[i];
        output += QString::fromStdString(
            v.GetName() + ": " + std::to_string(static_cast<int>(v.GetMaxSpeed())) + " км/ч\n");
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
    } catch (const SpeedLimitException& e) {
        output += QString("ПОЙМАНО: %1\n").arg(e.what());
        output += QString("  Скорость: %1, Лимит: %2\n\n").arg(e.GetSpeed()).arg(e.GetLimit());
    }

    // 2. FleetIndexException
    output += "=== Тест 2: Индекс вне диапазона ===\n";
    try {
        auto& v = fleet_[999];
        (void)v;
    } catch (const FleetIndexException& e) {
        output += QString("ПОЙМАНО: %1\n").arg(e.what());
        output += QString("  Индекс: %1, Размер: %2\n\n").arg(e.GetIndex()).arg(e.GetSize());
    }

    // 3. EngineException
    output += "=== Тест 3: Ошибка двигателя ===\n";
    try {
        Engine bad_engine(-100, Engine::FuelType::kPetrol);
    } catch (const EngineException& e) {
        output += QString("ПОЙМАНО: %1\n\n").arg(e.what());
    }

    // 4. Общий catch
    output += "=== Тест 4: Базовый VehicleException ===\n";
    try {
        throw VehicleException("Тестовое исключение");
    } catch (const VehicleException& e) {
        output += QString("ПОЙМАНО (базовый тип): %1\n\n").arg(e.what());
    }

    // 5. std::exception catch
    output += "=== Тест 5: std::exception ===\n";
    try {
        throw SpeedLimitException(500, 200);
    } catch (const std::exception& e) {
        output += QString("ПОЙМАНО через std::exception: %1\n").arg(e.what());
    }

    exception_output_->setText(output);
}

void MainWindow::OnRunFactory() {
    QString output;

    output += "=== Паттерн Factory Method ===\n\n";
    output += "Суть: базовый класс VehicleFactory определяет интерфейс Create(),\n";
    output += "а конкретные фабрики (CarFactory, TruckFactory, ...) реализуют его.\n";
    output += "Клиентский код работает с абстрактной фабрикой, не зная конкретных классов.\n\n";

    VehicleType types[] = {VehicleType::kCar, VehicleType::kTruck,
                           VehicleType::kBoat, VehicleType::kSubmarine};
    std::string names[] = {"Tesla Model S", "MAN TGX", "Bayliner VR5", "Курск"};

    output += "=== Создание объектов через фабрики ===\n\n";

    for (int i = 0; i < 4; ++i) {
        auto factory = GetFactory(types[i]);
        output += QString("Фабрика: %1\n").arg(QString::fromStdString(factory->GetFactoryName()));

        auto vehicle = factory->Create(names[i]);
        output += QString("  Создано: %1\n").arg(QString::fromStdString(vehicle->GetInfo()));
        output += QString("  Move(): %1\n\n").arg(QString::fromStdString(vehicle->Move()));
    }

    output += "=== Преимущества Factory Method ===\n\n";
    output += "1. Отделяет создание объектов от их использования\n";
    output += "2. Легко добавить новый тип — достаточно новой фабрики\n";
    output += "3. Клиентский код не зависит от конкретных классов\n";
    output += "4. Соблюдает принцип Open/Closed (открыт для расширения,\n";
    output += "   закрыт для модификации)\n";

    factory_output_->setText(output);
}

// --- Сериализация ---

void MainWindow::OnSaveCustom() {
    QString path = QFileDialog::getSaveFileName(this, "Сохранить (текст)", "", "Текстовые файлы (*.txt)");
    if (path.isEmpty()) return;

    try {
        auto start = std::chrono::high_resolution_clock::now();
        std::string data = fleet_.SerializeCustom();
        fleet_.SaveCustom(path.toStdString());
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        serial_output_->setText(
            QString("=== Сохранено в текстовый формат ===\n"
                    "Файл: %1\n"
                    "Размер: %2 байт\n"
                    "Время: %3 мкс\n"
                    "Объектов: %4\n\n"
                    "--- Содержимое ---\n%5")
            .arg(path)
            .arg(data.size())
            .arg(ms)
            .arg(fleet_.Size())
            .arg(QString::fromStdString(data)));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
    }
}

void MainWindow::OnLoadCustom() {
    QString path = QFileDialog::getOpenFileName(this, "Загрузить (текст)", "", "Текстовые файлы (*.txt)");
    if (path.isEmpty()) return;

    try {
        auto start = std::chrono::high_resolution_clock::now();
        fleet_.LoadCustom(path.toStdString());
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        selected_index_ = -1;
        detail_text_->clear();
        RefreshFleetView();

        serial_output_->setText(
            QString("=== Загружено из текстового формата ===\n"
                    "Файл: %1\n"
                    "Время: %2 мкс\n"
                    "Загружено объектов: %3\n\n"
                    "ВАЖНО: При десериализации водители создаются заново (глубокая копия).\n"
                    "Ссылки на оригинальные объекты Driver теряются.\n")
            .arg(path)
            .arg(ms)
            .arg(fleet_.Size()));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
    }
}

void MainWindow::OnSaveJson() {
    QString path = QFileDialog::getSaveFileName(this, "Сохранить (JSON)", "", "JSON файлы (*.json)");
    if (path.isEmpty()) return;

    try {
        auto start = std::chrono::high_resolution_clock::now();
        std::string data = fleet_.SerializeJson();
        fleet_.SaveJson(path.toStdString());
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        serial_output_->setText(
            QString("=== Сохранено в JSON формат ===\n"
                    "Файл: %1\n"
                    "Размер: %2 байт\n"
                    "Время: %3 мкс\n"
                    "Объектов: %4\n\n"
                    "--- Содержимое ---\n%5")
            .arg(path)
            .arg(data.size())
            .arg(ms)
            .arg(fleet_.Size())
            .arg(QString::fromStdString(data)));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
    }
}

void MainWindow::OnLoadJson() {
    QString path = QFileDialog::getOpenFileName(this, "Загрузить (JSON)", "", "JSON файлы (*.json)");
    if (path.isEmpty()) return;

    try {
        auto start = std::chrono::high_resolution_clock::now();
        fleet_.LoadJson(path.toStdString());
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        selected_index_ = -1;
        detail_text_->clear();
        RefreshFleetView();

        serial_output_->setText(
            QString("=== Загружено из JSON формата ===\n"
                    "Файл: %1\n"
                    "Время: %2 мкс\n"
                    "Загружено объектов: %3\n\n"
                    "ВАЖНО: При десериализации водители создаются заново (глубокая копия).\n"
                    "Ссылки на оригинальные объекты Driver теряются.\n")
            .arg(path)
            .arg(ms)
            .arg(fleet_.Size()));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
    }
}
