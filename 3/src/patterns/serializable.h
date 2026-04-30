#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include <string>
#include <sstream>
#include <map>

// Интерфейс кастомной сериализации.
// Каждый класс реализует сериализацию в текстовый формат key=value.
class ISerializable {
public:
    virtual ~ISerializable() = default;

    // Сериализовать объект в текстовый формат
    virtual std::string SerializeCustom() const = 0;

    // Десериализовать объект из текстового формата
    virtual void DeserializeCustom(const std::string& data) = 0;

    // Утилита: распарсить key=value строки в map
    static std::map<std::string, std::string> ParseKeyValue(const std::string& data) {
        std::map<std::string, std::string> result;
        std::istringstream stream(data);
        std::string line;
        while (std::getline(stream, line)) {
            if (line.empty() || line[0] == '[') continue;
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                result[line.substr(0, pos)] = line.substr(pos + 1);
            }
        }
        return result;
    }
};

#endif // SERIALIZABLE_H
