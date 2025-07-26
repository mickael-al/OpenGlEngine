#ifndef __NEURAL_NETWORK__
#define __NEURAL_NETWORK__

#include <iostream>
#include <vector>
#include "LargeArray.hpp"
#include "Sorter.hpp"

struct NeuralLayer
{
	size_t size;
	unsigned int typeActivation;
};

struct NeuralGlobalData
{
	unsigned int numberNN;//nombre de reseaux de neuronne
	unsigned int sizeNN;//la taille d'un reseau de neuronne
	unsigned int sizePPold;//la taille de perceptrons dans la couche precedente precedente
	unsigned int sizePPrevious;//la taille de perceptrons dans la couche precedente
	unsigned int sizePCurrent;//la taille de perceptrons dans la couche actuel	
	unsigned int lop;//local previous offset
	unsigned int loc;//local offset current
	unsigned int activationType;//activation Type
	unsigned int negativeInputSize;//1 input for all neuralNetwork
};

struct NeuralMutate
{
	unsigned int numberOfBest;
	float weight;
	float weightScale;
	float activation;
};

struct NeuralInitData
{
	unsigned int numberNeuralNetwork;
	std::vector<NeuralLayer> neuralLayer;
	unsigned int numberOfBest;//doit etre inferieur au nombre de reseau de neuronne
	float mutationWeight;
	float mutationWeightScale;
	float mutationActivation;
	bool singleInput;
};

struct TrainingData
{
	std::vector<float> inputData;
	std::vector<float> expetedResult;
};

namespace Ge
{
	class ComputeShader;
	class NeuralNetwork
	{
	public:
		NeuralNetwork(NeuralInitData nid);		
		void propagateGlobal(std::vector<TrainingData> tds);
		void propagate();//WitoutTraining
		void mutate();
		void elitims();
		const NeuralGlobalData* getGlobalData() const;
		unsigned int getGlobalDataSSBO() const;
		unsigned int getInputDataSSBO() const;
		unsigned int getNNDataSSBO() const;
		unsigned int getErrorSSBO() const;
		void drawBestNeuralNetwork();
		void computeNetwork(std::vector<float>& data, std::vector<float>& result);
		~NeuralNetwork();
	private:
		unsigned int m_ssboGlobalData;		
		unsigned int m_ssboInputNNData;
		unsigned int m_ssboNNData;
		unsigned int m_ssboError;
		unsigned int m_ssboIndexError;
		unsigned int m_ssboExpectedResult;
		unsigned int m_ssboMutate;
		std::vector<unsigned int> m_layerOffset;
		NeuralGlobalData m_ngd;		
		NeuralInitData m_nid;
		unsigned int m_globalSize;
		LargeArray<float>* m_nndata;
		ComputeShader* m_initLayer;
		ComputeShader* m_computeLayer;
		ComputeShader* m_computeError;

		ComputeShader* m_computeCopyElitims;
		unsigned int m_nbBestlocation;
		ComputeShader* m_computeMutate;
		unsigned int m_randomlocation;

		Sorter* m_sorter = nullptr;

		int m_index_best = -1;
	};
}

#endif //!__NEURAL_NETWORK__