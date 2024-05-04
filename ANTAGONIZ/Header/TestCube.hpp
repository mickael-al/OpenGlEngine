#ifndef __TEST_CUBE
#define __TEST_CUBE

#include "Engine.hpp"
#include "PointeurClass.hpp"
#include "Model.hpp"
using namespace Ge;

class TestCube : public Behaviour
{
public:
	TestCube(Model* m);
	void start();
	void fixedUpdate();
	void update();
	void stop();
	void onGUI();
private:
	const ptrClass* m_pc;
	Model* m_model;
	DirectionalLight* m_dir;

};

#endif//!__TEST_CUBE