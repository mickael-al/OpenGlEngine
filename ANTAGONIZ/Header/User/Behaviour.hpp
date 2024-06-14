#ifndef __ENGINE_BEHAVIOUR__
#define __ENGINE_BEHAVIOUR__

#include "Component.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <typeinfo>
#include "Factory.hpp"

class Behaviour : public Component
{
public:
	virtual void start() = 0;
	virtual void fixedUpdate() = 0;
	virtual void update() = 0;
	virtual void stop() = 0;	
	inline std::string className()
	{
		return typeid(this).name();
	}
};


#endif // __ENGINE_BEHAVIOUR__


