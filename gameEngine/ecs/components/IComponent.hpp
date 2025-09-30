#include <iostream>

namespace GameEngine {
class IComponent {
public:
  IComponent();
  ~IComponent();
    std::string get_tag();
private:
};
} // namespace GameEngine
