#include <iostream>

class IComponent {
public:
	IComponent ();
	~IComponent ();

private:
    std::string _type;
};

IComponent ::IComponent () {
}

IComponent ::~IComponent () {
}
