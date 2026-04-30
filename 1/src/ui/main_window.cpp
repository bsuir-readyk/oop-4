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
