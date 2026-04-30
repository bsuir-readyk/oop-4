#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include <string>

class Fleet;

// JSON сериализация флота с использованием nlohmann/json.
// Второй вид сериализации (альтернатива кастомному текстовому формату).
class JsonSerializer {
public:
    static std::string SerializeFleet(const Fleet& fleet);
    static void DeserializeFleet(const std::string& json_str, Fleet& fleet);
};

#endif // JSON_SERIALIZER_H
