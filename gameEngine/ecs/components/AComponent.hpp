#include "IComponent.hpp"

namespace GameEngine {
class AComponent : public IComponent {
public:
	AComponent(std::string tag);
	~AComponent();
    std::string get_tag() const;

private:
    std::string _tag;
};
}
