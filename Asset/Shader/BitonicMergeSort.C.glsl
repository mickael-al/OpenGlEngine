//User
#version 450

#define GROUP_SIZE 256
#define MAX_DIM_GROUPS 256
#define MAX_DIM_THREADS (GROUP_SIZE * MAX_DIM_GROUPS)
#define MAX_DIM_THREADS_THREADS (MAX_DIM_THREADS * MAX_DIM_GROUPS)

layout(std430, binding = 0) buffer IndexBuffer
{
	int indexData[];
}id;

layout(std430, binding = 1) buffer ValueBuffer
{
	int valueData[];
}vd;

#define INT_MAX 2147483647;

uniform int block;
uniform int dim;
uniform int count;
uniform int subcount;

layout(local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
	int i = int(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * MAX_DIM_THREADS + gl_GlobalInvocationID.z * MAX_DIM_THREADS_THREADS);
	int j = (i ^ block);

	if (j < i || i >= count)
	{
		return;
	}

	int value_i, value_j;
	int key_i = id.indexData[i];
	if (key_i < subcount)
	{
		value_i = vd.valueData[key_i];
	}
	else
	{
		value_i = INT_MAX;
	}

	int key_j = id.indexData[j];
	if (key_j < subcount)
	{
		value_j = vd.valueData[key_j];
	}
	else
	{
		value_j = INT_MAX;
	}

	int diff = (value_i - value_j) * ((i & dim) == 0 ? 1 : -1);
	if (diff > 0)
	{
		id.indexData[i] = key_j;
		id.indexData[j] = key_i;
	}

}