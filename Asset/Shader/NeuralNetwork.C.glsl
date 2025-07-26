#version 450

#define GROUP_SIZE 256
#define MAX_DIM_GROUPS 256
#define MAX_DIM_THREADS (GROUP_SIZE * MAX_DIM_GROUPS)
#define MAX_DIM_THREADS_THREADS (MAX_DIM_THREADS * MAX_DIM_GROUPS)

#define SINGLE 0
#define MULTI 1

float tanh_activation(float x)
{
	return (exp(x) - exp(-x)) / (exp(x) + exp(-x));
}

float relu(float x) 
{
	return max(0.0, x);
}

float leaky_relu(float x) 
{
	return (x > 0.0) ? x : 0.01 * x;
}

float sigmoid(float x) 
{
	return 1.0 / (1.0 + exp(-x));
}

float elu(float x) 
{
	return (x >= 0.0) ? x : (exp(x) - 1.0);
}

float swish(float x) 
{
	return x / (1.0 + exp(-x));  // x * sigmoid(x)
}

float prelu(float x, float alpha) 
{
	return (x >= 0.0) ? x : alpha * x;
}

float step_function(float x) 
{
	return (x >= 0.0) ? 1.0 : 0.0;
}

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

layout(std430, binding = 2) buffer NNInput
{
	float[] data;
}nni;


float activation_function(float data, uint id)
{
	if (id == 0)
	{
		return tanh_activation(data);  // tanh
	}
	else if (id == 1)
	{
		return relu(data);             // ReLU
	}
	else if (id == 2)
	{
		return leaky_relu(data);       // Leaky ReLU
	}
	else if (id == 3)
	{
		return sigmoid(data);          // Sigmoid
	}
	else if (id == 4)
	{
		return elu(data);              // ELU
	}
	else if (id == 5)
	{
		return swish(data);            // Swish
	}
	else if (id == 6)
	{
		return step_function(data);    // Step
	}
	else if (id == 7)
	{
		return 0.0;					   // Death
	}
	else if (id == 8)
	{
		return (data / float(nnd.sizePPrevious));// Moyenne
	}
	// Retour par défaut (au cas où id n'est pas valide)
	return data;
}

float getValue(uint index, uint inn)
{
	uint id = index % nnd.sizeNN;
	if (id >= nnd.negativeInputSize)
	{
		return nnv.data[index - (nnd.negativeInputSize * (inn + 1))];
	}
	else
	{
#if MODE == SINGLE
		return nni.data[id / 2];
#else
		return nni.data[(id / 2) + (index/nnd.sizeNN) * (nnd.negativeInputSize/2)];
#endif
	}
}

void setValue(uint index, uint inn, float val)
{
	if (index % nnd.sizeNN >= nnd.negativeInputSize)
	{
		if (isnan(val))
		{
			nnv.data[index - (nnd.negativeInputSize * (inn + 1))] = 0;
		}
		else
		{
			nnv.data[index - (nnd.negativeInputSize * (inn + 1))] = val;
		}
	}
}

layout(local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
	uint nid = uint(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * MAX_DIM_THREADS + gl_GlobalInvocationID.z * MAX_DIM_THREADS_THREADS);
	if (nid >= nnd.sizePCurrent * nnd.numberNN)
	{
		return;
	}
	uint lpi = nid % nnd.sizePCurrent;//localPerceptronsIndex
	uint inn = uint(double(nid) / double(nnd.sizePCurrent));
	uint off_inn = inn * nnd.sizeNN;//indexNeuralNetwork l'indice qui correspond au block du reseaux de neuronne
	uint off_link = (nnd.sizePPrevious * lpi) + (2 * lpi);//l'offset de decallage des weight
	uint fid = off_inn + nnd.loc + off_link;
	float cummule = 0.0f;
	for (uint i = 0; i < nnd.sizePPrevious; i++)//weight from current layer and use size of previous layer = number weight of current layer
	{
		//cummule = cummule + (nnv.data[off_inn + nnd.loc + off_link + i] * nnv.data[off_inn + nnd.lop + (nnd.sizePPold*(i+1)) + (i*2)]);
		cummule = cummule + (getValue(fid + i, inn) * getValue(off_inn + nnd.lop + (nnd.sizePPold * (i + 1)) + (i * 2), inn));
	}

	//nnv.data[off_inn + nnd.loc + off_link + nnd.sizePPrevious] = activation_function(cummule, uint(nnv.data[off_inn + nnd.loc + off_link + nnd.sizePPrevious + 1]));
	setValue(fid + nnd.sizePPrevious, inn, activation_function(cummule, uint(getValue(fid + nnd.sizePPrevious + 1,inn))));
}