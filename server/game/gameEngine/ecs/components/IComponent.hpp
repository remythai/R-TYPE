#include <iostream>

class IComponent {
public:
	IComponent ();
	~IComponent ();

private:
    std::string _tag;
};

IComponent ::IComponent () {
}

IComponent ::~IComponent () {
}
