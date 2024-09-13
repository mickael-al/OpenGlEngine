#ifndef __ENGINE_BEHAVIOUR__
#define __ENGINE_BEHAVIOUR__

#include "Factory.hpp"
#include "Component.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <typeinfo>
#include "imgui-cmake/Header/imgui.h"

class Behaviour : public Component
{	
public:
	virtual void start() = 0;
	virtual void fixedUpdate() = 0;
	virtual void update() = 0;
	virtual void stop() = 0;	

	virtual std::string serialize()
	{
		return "";
	}

	virtual void load(std::string data)
	{
		
	}

	inline std::string className()
	{
		return typeid(this).name();
	}
};

#endif // __ENGINE_BEHAVIOUR__


