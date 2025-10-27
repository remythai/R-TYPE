#pragma once
#include <string>

#include "../../../ecs/Component.hpp"
#include "../../../ecs/utils.hpp"

namespace GameEngine {
struct Text : public Component<Text>
{
    std::string content;
    int fontSize;

    Text(const std::string& val_content = "", int val_fontSize = 12)
        : content(val_content), fontSize(val_fontSize)
    {
    }

    static constexpr const char* Name = "Text";
    static constexpr const char* Version = "1.0.0";
};
}  // namespace GameEngine
