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

float getValue(uint index, uint inn)
{
	if (index % nnd.sizeNN >= nnd.negativeInputSize)
	{
		return nnv.data[index - (nnd.negativeInputSize * (inn + 1))];
	}
	else
	{
		return 0.0f;
	}
}

void setValue(uint index, uint inn, float val)
{
	if (index % nnd.sizeNN >= nnd.negativeInputSize)
	{
		nnv.data[index - (nnd.negativeInputSize * (inn + 1))] = val;
	}
}

uniform uint numberOfBest;

layout(local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
	uint nid = uint(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * MAX_DIM_THREADS + gl_GlobalInvocationID.z * MAX_DIM_THREADS_THREADS);
	if (nid >= numberOfBest * nnd.sizeNN)
	{
		return;
	}		
	uint nnid =	uint(double(nid) / double(nnd.sizeNN));
	uint loid = nid % nnd.sizeNN;
	//float data = nnv.data[nni.index[nnid]* nnd.sizeNN + loid];
	float data = getValue(nni.index[nnid] * nnd.sizeNN + loid, nni.index[nnid]);
	for (uint i = nnid + numberOfBest; i < nnd.numberNN; i += numberOfBest)
	{
		//nnv.data[nni.index[i] * nnd.sizeNN + loid] = data;
		setValue(nni.index[i] * nnd.sizeNN + loid, nni.index[i], data);
	}
}