#pragma once
#include <string>
#include "../../../utils.hpp"

namespace GameEngine {
struct Text {
    std::string content;
    int fontSize;

    Text(const std::string& val_content = "", int val_fontSize = 12) : content(val_content), fontSize(val_fontSize) {}
};
}
