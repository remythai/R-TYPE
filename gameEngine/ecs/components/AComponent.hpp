#include <iostream>

namespace GameEngine {
class AComponent {
public:
	AComponent ();
	~AComponent ();
    std::string get_tag() const;

private:
    std::string _tag;
};
}
