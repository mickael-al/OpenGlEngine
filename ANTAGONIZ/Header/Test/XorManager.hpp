#ifndef __XOR_MANAGER__
#define __XOR_MANAGER__

#include "GameEngine.hpp"
#include <map>
#include <vector>
#include "ShaderUtil.hpp"

struct NeuralNetworkData
{
	int nb_input_layer;
	int nb_output_layer;
	int nb_hiden_layer;
	int nb_col_hiden_layer;	
	int activationSize;
	int weightSize;	
	int select_sub_best_neural;
	float mutation_rate;
	float mutation_multiplayer;
};

struct NeuralSwapData
{
	int layerId;
	int size;
	int seed;
};

struct ScoreData
{
	int scorePerThread;
	int initDecompose;
};

class XorManager : public Behaviour
{
public:
	void start();
	void fixedUpdate();
	void update();
	void stop();
	void onGUI();
private:
	ptrClass m_pc;

	NeuralNetworkData m_nnd;
	NeuralSwapData m_nld;
	ScoreData m_sd;
	int nb_neural_network = 1000;
	int mutation_gen_loop = 1000;
	int max_best_neural = 20;

	ComputeShader * m_initXor;
	ComputeShader * m_initWeight;
	ComputeShader * m_resetScore;
	ComputeShader * m_xorScore;
	ComputeShader * m_bestScore;
	ComputeShader * m_cmpBest;
	ComputeShader * m_updateNeuralNetwork;
	ComputeShader * m_neuralNetwork;		

	ComputeBuffer * nndBuffer			= nullptr;
	ComputeBuffer * nldBuffer			= nullptr;	
	ComputeBuffer * bestWeight			= nullptr;
	ComputeBuffer * sBuffer				= nullptr;
	ComputeBuffer * weightBuffer		= nullptr;
	ComputeBuffer * activationBuffer	= nullptr;
	ComputeBuffer * scoreBuffer			= nullptr;
	ComputeBuffer * scoreIndexBuffer	= nullptr;
	ComputeBuffer * scoreIndexSubBuffer = nullptr;
};
#endif //__XOR_MANAGER__