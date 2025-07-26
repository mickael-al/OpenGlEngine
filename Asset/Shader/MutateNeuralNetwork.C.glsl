#version 450

#define GROUP_SIZE 256
#define MAX_DIM_GROUPS 256
#define MAX_DIM_THREADS (GROUP_SIZE * MAX_DIM_GROUPS)
#define MAX_DIM_THREADS_THREADS (MAX_DIM_THREADS * MAX_DIM_GROUPS)

layout(std430, binding = 0) buffer NNData
{
	uint numberNN;//nombre de reseaux de neuronne
	uint sizeNN;//la taille d'un reseau de neuronne(le nombre de float entre chaque)
	uint sizePPold;//la taille de perceptrons dans la couche precedente precedente
	uint sizePPrevious;//la taille de perceptrons dans la couche precedente
	uint sizePCurrent;//la taille de perceptrons dans la couche actuel	
	uint lop;//local previous offset
	uint loc;//local offset current
	uint activationType;//activation Type
	uint negativeInputSize;//negativeInputSize
}nnd;

layout(std430, binding = 1) buffer NNValue
{
	float[] data;
}nnv;

layout(std430, binding = 2) buffer NNIndex
{
	uint[] index;
}nni;

layout(std430, binding = 3) buffer NNMutate
{
	uint numberOfBest;
	float weight;
	float weightScale;
	float activation;
}nnm;

uint hash(uint x) {
	x ^= x >> 16;
	x *= 0x7feb352du;
	x ^= x >> 15;
	x *= 0x846ca68bu;
	x ^= x >> 16;
	return x;
}

float randomInRange(uint value)
{
	uint hashedValue = hash(value);
	return (float(hashedValue % 200000u) / 100000.0) - 1.0;
}

void addValue(uint index, uint inn, float val)
{
	if (index % nnd.sizeNN >= nnd.negativeInputSize)
	{
		nnv.data[index - (nnd.negativeInputSize * (inn + 1))] += val;
	}	
}

void setValue(uint index, uint inn, float val)
{
	if (index % nnd.sizeNN >= nnd.negativeInputSize)
	{
		nnv.data[index - (nnd.negativeInputSize * (inn + 1))] = val;
	}
}

uniform uint random_uniform;

layout(local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
	uint nid = uint(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * MAX_DIM_THREADS + gl_GlobalInvocationID.z * MAX_DIM_THREADS_THREADS);
	if (nid >= nnd.sizePCurrent * nnd.numberNN)
	{
		return;
	}
	uint inn = uint(double(nid) / double(nnd.sizePCurrent));
	if (nni.index[inn] < nnm.numberOfBest)
	{
		return;
	}

	uint lpi = nid % nnd.sizePCurrent;//localPerceptronsIndex
	uint off_inn = inn * nnd.sizeNN;//indexNeuralNetwork l'indice qui correspond au block du reseaux de neuronne
	uint off_link = (nnd.sizePPrevious * lpi) + (2 * lpi);//l'offset de decallage des weight	
	uint weight_mutate = uint(double(nnm.weight) * 10000.0);
	uint fid = off_inn + nnd.loc + off_link;
	for (uint i = 0; i < nnd.sizePPrevious; i++)//weight from current layer and use size of previous layer = number weight of current layer
	{
		if (hash(nid + hash(i * nnd.sizeNN) + random_uniform) % 10000 < weight_mutate)
		{
			//nnv.data[fid + i] += randomInRange(nid + hash(i * nnd.sizeNN) + random_uniform) * nnm.weightScale;
			addValue(fid + i, inn, randomInRange(nid + hash(i * nnd.sizeNN) + random_uniform) * nnm.weightScale);
		}
	}
	uint activation_mutate = uint(double(nnm.activation) * 10000.0);
	if ((hash(nid) + random_uniform) % 10000 < activation_mutate)
	{
		//nnv.data[fid + nnd.sizePPrevious + 1] = float(hash(hash(nid) + random_uniform) % 9);
		setValue(fid + nnd.sizePPrevious + 1, inn, float(hash(hash(nid) + random_uniform) % 9));		
	}
}