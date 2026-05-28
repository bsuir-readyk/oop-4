#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "fleet.h"
#include "vehicle_plugin_api.h"

#include <QPluginLoader>
#include <QString>
#include <QStringList>

#include <memory>
#include <string>
#include <vector>

struct PluginInfo {
    QString plugin_id;
    QString name;
    QString version;
    QString path;
    QString status;
    QString error;
    bool pending_unload = false;
};

struct PluginUnloadResult {
    bool unloaded = false;
    bool pending = false;
    QString message;
};

class PluginManager : public PluginHost {
public:
    explicit PluginManager(QString plugin_directory = QString());
    ~PluginManager() override;

    void SetPluginDirectory(const QString& plugin_directory);
    QString PluginDirectory() const;

    QStringList LoadPluginsFromDirectory();
    PluginUnloadResult UnloadPlugin(const QString& plugin_id, const Fleet& fleet);

    std::vector<PluginInfo> PluginInfos() const;
    std::vector<PluginTabRegistration> TabRegistrations() const;
    std::vector<FleetUiExtensionRegistration> FleetUiExtensions() const;

    bool RegisterVehicleType(const VehicleTypeRegistration& registration,
                             QString* error) override;
    bool RegisterTab(const PluginTabRegistration& registration,
                     QString* error) override;
    bool RegisterFleetUiExtension(const FleetUiExtensionRegistration& registration,
                                  QString* error) override;

private:
    struct LoadedPlugin {
        QString plugin_id;
        QString name;
        QString version;
        QString path;
        QString load_path;
        QString status;
        QString error;
        bool pending_unload = false;
        std::shared_ptr<void> lifetime_token;
        std::unique_ptr<QPluginLoader> loader;
        VehiclePluginInterface* instance = nullptr;
        std::vector<std::string> vehicle_type_ids;
        std::vector<PluginTabRegistration> tabs;
        std::vector<FleetUiExtensionRegistration> fleet_extensions;
    };

    bool IsPluginLoaded(const QString& plugin_id) const;
    bool HasActivePluginObjects(const LoadedPlugin& plugin) const;
    PluginUnloadResult FinalizeUnload(size_t plugin_index);
    QString CreateLoadCopy(const QString& path, QString* error) const;
    void RemoveLoadCopy(const QString& path) const;
    QStringList CandidatePluginFiles() const;
    bool IsPluginFile(const QString& path) const;
    QString FailLoading(const QString& path, const QString& error);

    QString plugin_directory_;
    std::vector<std::unique_ptr<LoadedPlugin>> plugins_;
    std::vector<PluginInfo> failed_plugins_;
    LoadedPlugin* registering_plugin_ = nullptr;
};

#endif // PLUGIN_MANAGER_H
