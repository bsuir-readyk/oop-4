#include "plugin_manager.h"

#include "vehicle_type_registry.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLibrary>

#include <algorithm>

namespace {

constexpr const char* kLoadCopyMarker = ".reload-";

bool IsLoadCopy(const QString& path) {
    return QFileInfo(path).completeBaseName().contains(kLoadCopyMarker);
}

} // namespace

PluginManager::PluginManager(QString plugin_directory)
    : plugin_directory_(std::move(plugin_directory)) {}

PluginManager::~PluginManager() {
    for (auto& plugin : plugins_) {
        const QString load_path = plugin->load_path;
        plugin->tabs.clear();
        plugin->fleet_extensions.clear();
        if (HasActivePluginObjects(*plugin)) {
            VehicleTypeRegistry::Instance().SetOwnerEnabled(plugin->plugin_id.toStdString(), false);
            // The manager can be destroyed before external plugin objects in tests or tools.
            // In that case leaking the loader until process exit is safer than unloading
            // code that still owns live virtual objects.
            (void)plugin->loader.release();
            continue;
        }

        if (plugin->instance) {
            plugin->instance->BeforeUnload();
        }
        VehicleTypeRegistry::Instance().UnregisterOwner(plugin->plugin_id.toStdString());
        if (plugin->loader && plugin->loader->isLoaded()) {
            plugin->loader->unload();
        }
        RemoveLoadCopy(load_path);
    }
}

void PluginManager::SetPluginDirectory(const QString& plugin_directory) {
    plugin_directory_ = plugin_directory;
}

QString PluginManager::PluginDirectory() const {
    return plugin_directory_;
}

QStringList PluginManager::LoadPluginsFromDirectory() {
    failed_plugins_.clear();
    QStringList messages;

    const QStringList files = CandidatePluginFiles();
    for (const QString& path : files) {
        const auto already_loaded = std::any_of(plugins_.begin(), plugins_.end(),
            [&](const std::unique_ptr<LoadedPlugin>& plugin) {
                return QFileInfo(plugin->path).canonicalFilePath() ==
                       QFileInfo(path).canonicalFilePath();
            });
        if (already_loaded) {
            continue;
        }

        QString copy_error;
        const QString load_path = CreateLoadCopy(path, &copy_error);
        if (load_path.isEmpty()) {
            messages << FailLoading(path, copy_error);
            continue;
        }

        auto loader = std::make_unique<QPluginLoader>(load_path);
        loader->setLoadHints(QLibrary::LoadHints());
        QObject* object = loader->instance();
        if (!object) {
            messages << FailLoading(path, loader->errorString());
            RemoveLoadCopy(load_path);
            continue;
        }

        auto* plugin_interface = qobject_cast<VehiclePluginInterface*>(object);
        if (!plugin_interface) {
            messages << FailLoading(path, "Файл не реализует VehiclePluginInterface");
            loader->unload();
            RemoveLoadCopy(load_path);
            continue;
        }

        const QString plugin_id = plugin_interface->PluginId();
        if (plugin_id.isEmpty()) {
            messages << FailLoading(path, "Плагин вернул пустой PluginId");
            loader->unload();
            RemoveLoadCopy(load_path);
            continue;
        }
        if (IsPluginLoaded(plugin_id)) {
            messages << FailLoading(path, "Плагин уже загружен: " + plugin_id);
            loader->unload();
            RemoveLoadCopy(load_path);
            continue;
        }

        auto plugin = std::make_unique<LoadedPlugin>();
        plugin->plugin_id = plugin_id;
        plugin->name = plugin_interface->PluginName();
        plugin->version = plugin_interface->PluginVersion();
        plugin->path = path;
        plugin->load_path = load_path;
        plugin->status = "Загружен";
        plugin->lifetime_token = std::make_shared<int>(0);
        plugin->loader = std::move(loader);
        plugin->instance = plugin_interface;

        QString error;
        registering_plugin_ = plugin.get();
        const bool registered = plugin_interface->RegisterPlugin(*this, &error);
        registering_plugin_ = nullptr;

        if (!registered) {
            VehicleTypeRegistry::Instance().UnregisterOwner(plugin_id.toStdString());
            messages << FailLoading(path, error.isEmpty() ? "RegisterPlugin вернул false" : error);
            plugin->loader->unload();
            RemoveLoadCopy(load_path);
            continue;
        }

        messages << "Загружен плагин: " + plugin->name + " (" + plugin_id + ")";
        plugins_.push_back(std::move(plugin));
    }

    return messages;
}

PluginUnloadResult PluginManager::UnloadPlugin(const QString& plugin_id, const Fleet& fleet) {
    for (size_t i = 0; i < plugins_.size(); ++i) {
        auto& plugin = plugins_[i];
        if (plugin->plugin_id != plugin_id) {
            continue;
        }

        (void)fleet;
        if (HasActivePluginObjects(*plugin)) {
            plugin->pending_unload = true;
            plugin->status = "Ожидает выгрузки: ещё существуют объекты этого типа";
            plugin->tabs.clear();
            plugin->fleet_extensions.clear();
            VehicleTypeRegistry::Instance().SetOwnerEnabled(plugin_id.toStdString(), false);
            return {false, true,
                    "Плагин оставлен в памяти: ещё существуют созданные им объекты"};
        }

        return FinalizeUnload(i);
    }

    return {false, false, "Плагин не найден"};
}

std::vector<PluginInfo> PluginManager::PluginInfos() const {
    std::vector<PluginInfo> result;
    for (const auto& plugin : plugins_) {
        result.push_back({plugin->plugin_id, plugin->name, plugin->version,
                          plugin->path, plugin->status, plugin->error,
                          plugin->pending_unload});
    }
    result.insert(result.end(), failed_plugins_.begin(), failed_plugins_.end());
    return result;
}

std::vector<PluginTabRegistration> PluginManager::TabRegistrations() const {
    std::vector<PluginTabRegistration> result;
    for (const auto& plugin : plugins_) {
        if (plugin->pending_unload) {
            continue;
        }
        result.insert(result.end(), plugin->tabs.begin(), plugin->tabs.end());
    }
    return result;
}

std::vector<FleetUiExtensionRegistration> PluginManager::FleetUiExtensions() const {
    std::vector<FleetUiExtensionRegistration> result;
    for (const auto& plugin : plugins_) {
        if (plugin->pending_unload) {
            continue;
        }
        result.insert(result.end(), plugin->fleet_extensions.begin(),
                      plugin->fleet_extensions.end());
    }
    return result;
}

bool PluginManager::RegisterVehicleType(const VehicleTypeRegistration& registration,
                                        QString* error) {
    if (!registering_plugin_) {
        if (error) *error = "RegisterVehicleType вызван вне загрузки плагина";
        return false;
    }

    VehicleTypeRegistration owned_registration = registration;
    owned_registration.owner_plugin_id = registering_plugin_->plugin_id.toStdString();
    owned_registration.enabled = true;
    const std::weak_ptr<void> plugin_lifetime = registering_plugin_->lifetime_token;
    const auto original_create = owned_registration.create_default;
    owned_registration.create_default = [original_create, plugin_lifetime](const std::string& name) {
        auto vehicle = original_create(name);
        if (vehicle) {
            vehicle->HoldPluginLifetime(plugin_lifetime.lock());
        }
        return vehicle;
    };
    const auto original_deserialize = owned_registration.deserialize;
    owned_registration.deserialize = [original_deserialize, plugin_lifetime](
        const VehicleSerializationData& data) {
        auto vehicle = original_deserialize(data);
        if (vehicle) {
            vehicle->HoldPluginLifetime(plugin_lifetime.lock());
        }
        return vehicle;
    };
    if (!VehicleTypeRegistry::Instance().RegisterType(owned_registration, error)) {
        return false;
    }

    registering_plugin_->vehicle_type_ids.push_back(owned_registration.type_id);
    return true;
}

bool PluginManager::RegisterTab(const PluginTabRegistration& registration,
                                QString* error) {
    if (!registering_plugin_) {
        if (error) *error = "RegisterTab вызван вне загрузки плагина";
        return false;
    }
    if (registration.tab_id.empty() || registration.title.isEmpty() ||
        !registration.create_widget) {
        if (error) *error = "Некорректная регистрация вкладки плагина";
        return false;
    }

    PluginTabRegistration owned_registration = registration;
    owned_registration.owner_plugin_id = registering_plugin_->plugin_id.toStdString();
    registering_plugin_->tabs.push_back(std::move(owned_registration));
    return true;
}

bool PluginManager::RegisterFleetUiExtension(
    const FleetUiExtensionRegistration& registration,
    QString* error) {
    if (!registering_plugin_) {
        if (error) *error = "RegisterFleetUiExtension вызван вне загрузки плагина";
        return false;
    }
    if (registration.extension_id.empty() || !registration.supports) {
        if (error) *error = "Некорректная регистрация расширения UI флота";
        return false;
    }

    FleetUiExtensionRegistration owned_registration = registration;
    owned_registration.owner_plugin_id = registering_plugin_->plugin_id.toStdString();
    registering_plugin_->fleet_extensions.push_back(std::move(owned_registration));
    return true;
}

bool PluginManager::IsPluginLoaded(const QString& plugin_id) const {
    return std::any_of(plugins_.begin(), plugins_.end(),
        [&](const std::unique_ptr<LoadedPlugin>& plugin) {
            return plugin->plugin_id == plugin_id;
        });
}

bool PluginManager::HasActivePluginObjects(const LoadedPlugin& plugin) const {
    return plugin.lifetime_token && plugin.lifetime_token.use_count() > 1;
}

PluginUnloadResult PluginManager::FinalizeUnload(size_t plugin_index) {
    auto& plugin = plugins_[plugin_index];
    const QString plugin_id = plugin->plugin_id;
    const QString plugin_name = plugin->name;
    const QString load_path = plugin->load_path;

    if (plugin->instance) {
        plugin->instance->BeforeUnload();
    }

    plugin->tabs.clear();
    plugin->fleet_extensions.clear();
    VehicleTypeRegistry::Instance().UnregisterOwner(plugin_id.toStdString());

    if (plugin->loader && plugin->loader->isLoaded() && !plugin->loader->unload()) {
        plugin->pending_unload = true;
        plugin->status = "Ошибка выгрузки";
        plugin->error = plugin->loader->errorString();
        return {false, true, "Qt не выгрузил плагин: " + plugin->error};
    }

    plugins_.erase(plugins_.begin() + static_cast<long>(plugin_index));
    RemoveLoadCopy(load_path);
    return {true, false, "Плагин выгружен: " + plugin_name + " (" + plugin_id + ")"};
}

QString PluginManager::CreateLoadCopy(const QString& path, QString* error) const {
    const QFileInfo source_info(path);
    const QString suffix = source_info.suffix();
    const QString base_name = source_info.completeBaseName();
    const QString pid = QString::number(QCoreApplication::applicationPid());
    const QString now = QString::number(QDateTime::currentMSecsSinceEpoch());

    QString last_error;
    for (int attempt = 0; attempt < 100; ++attempt) {
        const QString copy_name = base_name + kLoadCopyMarker + pid + "-" + now +
                                  "-" + QString::number(attempt) + "." + suffix;
        const QString copy_path = source_info.dir().absoluteFilePath(copy_name);
        if (QFileInfo::exists(copy_path)) {
            continue;
        }

        QFile source(path);
        if (source.copy(copy_path)) {
            QFile::setPermissions(copy_path, source_info.permissions());
            return copy_path;
        }
        last_error = source.errorString();
    }

    if (error) {
        *error = "Не удалось создать копию плагина для загрузки: " + last_error;
    }
    return QString();
}

void PluginManager::RemoveLoadCopy(const QString& path) const {
    if (!path.isEmpty() && IsLoadCopy(path)) {
        QFile::remove(path);
    }
}

QStringList PluginManager::CandidatePluginFiles() const {
    QStringList result;
    if (plugin_directory_.isEmpty()) {
        return result;
    }

    const QDir directory(plugin_directory_);
    if (!directory.exists()) {
        return result;
    }

    const QFileInfoList files = directory.entryInfoList(QDir::Files | QDir::Readable,
                                                        QDir::Name);
    for (const QFileInfo& file : files) {
        if (!IsLoadCopy(file.absoluteFilePath()) && IsPluginFile(file.absoluteFilePath())) {
            result << file.absoluteFilePath();
        }
    }
    return result;
}

bool PluginManager::IsPluginFile(const QString& path) const {
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == "dylib" || suffix == "so" || suffix == "dll";
}

QString PluginManager::FailLoading(const QString& path, const QString& error) {
    PluginInfo info;
    info.path = path;
    info.name = QFileInfo(path).fileName();
    info.status = "Ошибка загрузки";
    info.error = error;
    failed_plugins_.push_back(info);
    return "Не загружен " + path + ": " + error;
}
