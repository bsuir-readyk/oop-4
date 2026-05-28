#include "main_window.h"
#include "vehicle_factory.h"
#include "car.h"
#include "truck.h"
#include "boat.h"
#include "submarine.h"
#include "exceptions.h"
#include "vehicle_type_registry.h"
#include "serialization/vehicle_serializer.h"

#include <QByteArray>
#include <QColor>
#include <QCoreApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QFrame>
#include <QGridLayout>
#include <QFont>
#include <QListWidgetItem>
#include <QPalette>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    plugin_manager_.SetPluginDirectory(QCoreApplication::applicationDirPath() + "/plugins");

    setWindowTitle("ООП: Иерархия транспортных средств");
    setMinimumSize(900, 600);

    tabs_ = new QTabWidget(this);
    setCentralWidget(tabs_);

    SetupFleetTab();
    SetupFactoryTab();
    SetupPolymorphismTab();
    SetupExceptionsTab();
    SetupSerializationTab();
    SetupPluginsTab();
    SetupAboutTab();

    plugin_manager_.LoadPluginsFromDirectory();
    RebuildPluginTabs();
    PopulateDefaultFleet();
    RefreshFleetView();
    RefreshPluginsView();
}

MainWindow::~MainWindow() {
    RemovePluginTabs();
}

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
    btn_layout->addWidget(add_btn);
    btn_layout->addWidget(remove_btn);
    left->addLayout(btn_layout);

    connect(add_btn, &QPushButton::clicked, this, &MainWindow::OnAddVehicle);
    connect(remove_btn, &QPushButton::clicked, this, &MainWindow::OnRemoveVehicle);

    // Правая часть — детали
    detail_text_ = new QTextEdit();
    detail_text_->setReadOnly(true);
    detail_text_->setFont(QFont("Menlo", 12));

    main_layout->addLayout(left, 2);
    main_layout->addWidget(detail_text_, 3);

    tabs_->addTab(tab, "Флот");
}

// --- Вкладка "Фабрика" ---
void MainWindow::SetupFactoryTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    auto* btn = new QPushButton("Показать фабричный класс и фабричный метод");
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

// --- Вкладка "Сериализация" ---
void MainWindow::SetupSerializationTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    auto* hint = new QLabel(
        "Сериализация глубокая: Engine сохраняется внутри Vehicle, "
        "Driver сохраняется один раз и подключается к Vehicle по driver_id. "
        "Текст можно редактировать, затем десериализовать обратно во флот. "
        "При загрузке проверяются диапазоны чисел, например max_speed <= 10000.");
    hint->setWordWrap(true);

    auto* buttons = new QHBoxLayout();
    auto* serialize_custom_btn = new QPushButton("Сериализовать TEXT");
    auto* deserialize_custom_btn = new QPushButton("Десериализовать TEXT");
    auto* serialize_json_btn = new QPushButton("Сериализовать JSON");
    auto* deserialize_json_btn = new QPushButton("Десериализовать JSON");

    buttons->addWidget(serialize_custom_btn);
    buttons->addWidget(deserialize_custom_btn);
    buttons->addWidget(serialize_json_btn);
    buttons->addWidget(deserialize_json_btn);

    serialization_text_ = new QTextEdit();
    serialization_text_->setFont(QFont("Menlo", 11));

    connect(serialize_custom_btn, &QPushButton::clicked,
            this, &MainWindow::OnSerializeCustom);
    connect(deserialize_custom_btn, &QPushButton::clicked,
            this, &MainWindow::OnDeserializeCustom);
    connect(serialize_json_btn, &QPushButton::clicked,
            this, &MainWindow::OnSerializeJson);
    connect(deserialize_json_btn, &QPushButton::clicked,
            this, &MainWindow::OnDeserializeJson);

    layout->addWidget(hint);
    layout->addLayout(buttons);
    layout->addWidget(serialization_text_);
    tabs_->addTab(tab, "Сериализация");
}

// --- Вкладка "Модули" ---
void MainWindow::SetupPluginsTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    plugin_folder_label_ = new QLabel();
    plugin_folder_label_->setWordWrap(true);

    auto* hint = new QLabel(
        "Плагины загружаются из папки plugins рядом с исполняемым файлом. "
        "Новый модуль может добавить тип транспорта, вкладку приложения и "
        "расширение отображения карточек флота без изменения кода программы.");
    hint->setWordWrap(true);

    auto* buttons = new QHBoxLayout();
    auto* reload_btn = new QPushButton("Обновить модули");
    auto* unload_btn = new QPushButton("Выгрузить выбранный");
    buttons->addWidget(reload_btn);
    buttons->addWidget(unload_btn);

    plugins_list_ = new QListWidget();

    connect(reload_btn, &QPushButton::clicked, this, &MainWindow::OnReloadPlugins);
    connect(unload_btn, &QPushButton::clicked, this, &MainWindow::OnUnloadSelectedPlugin);

    layout->addWidget(plugin_folder_label_);
    layout->addWidget(hint);
    layout->addLayout(buttons);
    layout->addWidget(plugins_list_);
    tabs_->addTab(tab, "Модули");
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

    FleetCardStyle style;
    style.background_color = "#757575";
    if (const auto* type = VehicleTypeRegistry::Instance().Find(vehicle.GetTypeId())) {
        style.background_color = type->card_color.name();
    }
    for (const auto& extension : plugin_manager_.FleetUiExtensions()) {
        try {
            if (extension.supports && extension.supports(vehicle) &&
                extension.customize_card) {
                extension.customize_card(vehicle, style);
            }
        } catch (const std::exception&) {
            // Ошибка расширения UI не должна ломать базовое отображение флота.
        }
    }

    card->setStyleSheet(
        QString("QFrame { background-color: %1; border-radius: 8px; }"
                "QLabel { color: white; }").arg(style.background_color));

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
    if (!style.subtitle.isEmpty()) {
        auto* subtitle_label = new QLabel(style.subtitle);
        subtitle_label->setFont(QFont("", 8));
        subtitle_label->setAlignment(Qt::AlignCenter);
        layout->addWidget(subtitle_label);
    }

    // Клик — выбрать
    card->setProperty("index", index);
    card->installEventFilter(this);

    return card;
}

// --- Обновление вида флота ---
void MainWindow::RefreshFleetView() {
    QLayoutItem* item;
    while ((item = fleet_layout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

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

QString MainWindow::BuildVehicleDetails(const Vehicle& vehicle) const {
    QString details = QString::fromStdString(vehicle.GetInfo());
    for (const auto& extension : plugin_manager_.FleetUiExtensions()) {
        try {
            if (extension.supports && extension.supports(vehicle) &&
                extension.extra_details) {
                const QString extra = extension.extra_details(vehicle);
                if (!extra.isEmpty()) {
                    details += "\n\n=== Расширение плагина ===\n" + extra;
                }
            }
        } catch (const std::exception& e) {
            details += "\n\n[Ошибка расширения UI: ";
            details += QString::fromUtf8(e.what());
            details += "]";
        }
    }
    return details;
}

void MainWindow::RefreshPluginsView() {
    if (!plugins_list_ || !plugin_folder_label_) {
        return;
    }

    plugin_folder_label_->setText("Папка модулей: " + plugin_manager_.PluginDirectory());
    plugins_list_->clear();

    const auto infos = plugin_manager_.PluginInfos();
    if (infos.empty()) {
        auto* item = new QListWidgetItem("Модули не загружены");
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        plugins_list_->addItem(item);
        return;
    }

    for (const auto& info : infos) {
        QString text;
        if (info.plugin_id.isEmpty()) {
            text = info.name + " — " + info.status;
        } else {
            text = info.name + " " + info.version + " [" + info.plugin_id + "] — " +
                   info.status;
        }
        if (!info.error.isEmpty()) {
            text += "\n" + info.error;
        }
        text += "\n" + info.path;

        auto* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, info.plugin_id);
        if (!info.error.isEmpty()) {
            item->setBackground(QColor("#FFCDD2"));
        } else if (info.pending_unload) {
            item->setBackground(QColor("#FFF9C4"));
        }
        plugins_list_->addItem(item);
    }
}

void MainWindow::RemovePluginTabs(const QString& plugin_id) {
    for (auto it = plugin_tab_widgets_.begin(); it != plugin_tab_widgets_.end();) {
        const bool remove = plugin_id.isEmpty() ||
            QString::fromStdString(it->plugin_id) == plugin_id;
        if (!remove) {
            ++it;
            continue;
        }

        if (it->widget) {
            const int index = tabs_->indexOf(it->widget);
            if (index >= 0) {
                tabs_->removeTab(index);
            }
            delete it->widget;
        }
        it = plugin_tab_widgets_.erase(it);
    }
}

void MainWindow::RebuildPluginTabs() {
    RemovePluginTabs();
    for (const auto& registration : plugin_manager_.TabRegistrations()) {
        try {
            QWidget* widget = registration.create_widget(tabs_);
            if (!widget) {
                continue;
            }
            tabs_->addTab(widget, registration.title);
            plugin_tab_widgets_.push_back({registration.owner_plugin_id, widget});
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Модуль",
                                 "Плагин не смог создать вкладку: " +
                                 QString::fromUtf8(e.what()));
        }
    }
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
                detail_text_->setText(BuildVehicleDetails(fleet_[idx]));
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// --- Слоты ---

void MainWindow::OnAddVehicle() {
    const auto vehicle_types = VehicleTypeRegistry::Instance().AvailableTypes();
    if (vehicle_types.empty()) {
        QMessageBox::warning(this, "Добавление", "Нет доступных типов транспорта");
        return;
    }

    QStringList types;
    for (const auto& vehicle_type : vehicle_types) {
        QString label = QString::fromStdString(vehicle_type.display_name);
        if (vehicle_type.owner_plugin_id != VehicleTypeRegistry::kBuiltinOwner) {
            label += " [плагин: " + QString::fromStdString(vehicle_type.owner_plugin_id) + "]";
        }
        types << label;
    }

    bool ok;
    QString type = QInputDialog::getItem(this, "Добавить ТС", "Тип:", types, 0, false, &ok);
    if (!ok) return;

    QString name = QInputDialog::getText(this, "Название", "Введите название:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    try {
        const int type_index = types.indexOf(type);
        if (type_index < 0 || static_cast<size_t>(type_index) >= vehicle_types.size()) {
            throw VehicleException("Выбран неизвестный тип транспорта");
        }

        const auto& registration = vehicle_types[static_cast<size_t>(type_index)];
        fleet_.Add(registration.create_default(name.toStdString()));
        RefreshFleetView();
    } catch (const VehicleException& e) {
        QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
    } catch (const std::exception& e) {
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

    // 6. FleetCapacityException
    output += "\n=== Тест 6: Превышение размера флота ===\n";
    try {
        Fleet full_fleet;
        for (size_t i = 0; i <= Fleet::kMaxSize; ++i) {
            full_fleet.Add(std::make_unique<Car>(
                "Test car " + std::to_string(i + 1),
                180, 120, Engine::FuelType::kPetrol, 5));
        }
    } catch (const FleetCapacityException& e) {
        output += QString("ПОЙМАНО: %1\n").arg(e.what());
        output += QString("  Размер: %1, Максимум: %2\n")
                      .arg(e.GetSize())
                      .arg(e.GetLimit());
    }

    exception_output_->setText(output);
}

void MainWindow::OnRunFactory() {
    QString output;

    output += "=== Динамический Factory Method + реестр типов ===\n\n";
    output += "VehicleTypeRegistry хранит встроенные типы и типы из плагинов.\n";
    output += "Каждая запись содержит фабричный класс, фабричный метод create_default,\n";
    output += "контракт сериализации payload и владельца: builtin или id плагина.\n";
    output += "Добавление нового модуля обновляет список без изменения MainWindow.\n\n";

    output += "=== Создание объектов через фабрики ===\n\n";

    const auto vehicle_types = VehicleTypeRegistry::Instance().AvailableTypes();
    for (size_t i = 0; i < vehicle_types.size(); ++i) {
        const auto& registration = vehicle_types[i];
        const auto factory_name = QString::fromStdString(registration.factory_name);
        output += QString("Тип: %1\n").arg(QString::fromStdString(registration.type_id));
        output += QString("Владелец: %1\n").arg(QString::fromStdString(registration.owner_plugin_id));
        output += QString("Фабричный класс: %1\n").arg(factory_name);
        output += QString("Фабричный метод: %1::create_default(name)\n").arg(factory_name);

        auto vehicle = registration.create_default("Demo " + std::to_string(i + 1));
        output += QString("  Создано: %1\n").arg(QString::fromStdString(vehicle->GetInfo()));
        output += QString("  Move(): %1\n\n").arg(QString::fromStdString(vehicle->Move()));
    }

    factory_output_->setText(output);
}

void MainWindow::OnSerializeCustom() {
    try {
        const std::string text = VehicleSerializer::SerializeCustom(fleet_, drivers_);
        serialization_text_->setPlainText(
            QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Сериализация", QString::fromUtf8(e.what()));
    }
}

void MainWindow::OnDeserializeCustom() {
    try {
        const QByteArray bytes = serialization_text_->toPlainText().toUtf8();
        auto snapshot = VehicleSerializer::DeserializeCustom(
            std::string(bytes.constData(), static_cast<size_t>(bytes.size())));
        fleet_ = std::move(snapshot.fleet);
        drivers_ = std::move(snapshot.drivers);
        selected_index_ = -1;
        detail_text_->clear();
        RefreshFleetView();
        QMessageBox::information(this, "Сериализация", "TEXT загружен во флот");
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Десериализация TEXT", QString::fromUtf8(e.what()));
    }
}

void MainWindow::OnSerializeJson() {
    try {
        const std::string text = VehicleSerializer::SerializeJson(fleet_, drivers_);
        serialization_text_->setPlainText(
            QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Сериализация", QString::fromUtf8(e.what()));
    }
}

void MainWindow::OnDeserializeJson() {
    try {
        const QByteArray bytes = serialization_text_->toPlainText().toUtf8();
        auto snapshot = VehicleSerializer::DeserializeJson(
            std::string(bytes.constData(), static_cast<size_t>(bytes.size())));
        fleet_ = std::move(snapshot.fleet);
        drivers_ = std::move(snapshot.drivers);
        selected_index_ = -1;
        detail_text_->clear();
        RefreshFleetView();
        QMessageBox::information(this, "Сериализация", "JSON загружен во флот");
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Десериализация JSON", QString::fromUtf8(e.what()));
    }
}

void MainWindow::OnReloadPlugins() {
    const QStringList messages = plugin_manager_.LoadPluginsFromDirectory();
    RebuildPluginTabs();
    RefreshFleetView();
    RefreshPluginsView();

    if (!messages.isEmpty()) {
        QMessageBox::information(this, "Модули", messages.join('\n'));
    }
}

void MainWindow::OnUnloadSelectedPlugin() {
    const QList<QListWidgetItem*> selected = plugins_list_->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Модули", "Выберите загруженный плагин");
        return;
    }

    const QString plugin_id = selected.first()->data(Qt::UserRole).toString();
    if (plugin_id.isEmpty()) {
        QMessageBox::information(this, "Модули", "Эту строку нельзя выгрузить");
        return;
    }

    RemovePluginTabs(plugin_id);
    const PluginUnloadResult result = plugin_manager_.UnloadPlugin(plugin_id, fleet_);
    RebuildPluginTabs();
    RefreshFleetView();
    RefreshPluginsView();

    if (!result.message.isEmpty()) {
        QMessageBox::information(this, "Модули", result.message);
    }
}
