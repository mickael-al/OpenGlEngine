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

layout(std430, binding = 2) buffer NNError
{
	float[] error;
}nne;

layout(std430, binding = 3) buffer NNResult
{
	float[] result;
}nnr;

float getValue(uint index, uint inn)
{
	if (index % nnd.sizeNN >= nnd.negativeInputSize)
	{
		return nnv.data[index - (nnd.negativeInputSize * (inn + 1))];
	}
}

layout(local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
	uint nid = uint(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * MAX_DIM_THREADS + gl_GlobalInvocationID.z * MAX_DIM_THREADS_THREADS);
	if (nid >= nnd.numberNN)
	{
		return;
	}		
	float cumule = 0.0f;
	uint off_inn = nid * nnd.sizeNN;
	uint off_link;
	for (int i = 0; i < nnd.sizePCurrent;i++)
	{
		off_link = ((i + 1) * nnd.sizePPrevious) + (i*2);
		//cumule = cumule + abs(nnv.data[off_inn + nnd.loc + off_link] - nnr.result[i]);		
		cumule = cumule + abs(getValue(off_inn + nnd.loc + off_link, nid)- nnr.result[i]);
	}	
	nne.error[nid] += cumule;
}