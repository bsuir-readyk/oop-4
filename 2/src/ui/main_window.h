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
#include "vehicle_factory.h"
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
    void OnRunFactory();

private:
    void SetupFleetTab();
    void SetupFactoryTab();
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

    // Вкладка "Фабрика"
    QTextEdit* factory_output_;

    // Вкладка "Исключения"
    QTextEdit* exception_output_;

    // Данные
    Fleet fleet_;
    std::vector<std::unique_ptr<Driver>> drivers_;
    int selected_index_ = -1;
};

#endif // MAIN_WINDOW_H
