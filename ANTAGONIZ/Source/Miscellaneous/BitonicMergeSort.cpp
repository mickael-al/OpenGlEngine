#include "glcore.hpp"
#include "BitonicMergeSort.hpp"
#include "ShaderUtil.hpp"
#include "Debug.hpp"

void BitonicMergeSort::Sort(ComputeShader * bms, ComputeShader * fill, unsigned int ssbo_index, unsigned int ssbo_value, int numElts, int sortSize)
{
	int x, y, z;
	fill->use();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_index);
	ShaderUtil::CalcWorkSize(sortSize, &x, &y, &z);
	fill->dispatch(x, y, z);
	glFinish();
	bms->use();
	glUniform1i(glGetUniformLocation(bms->getProgram(), "count"), sortSize);
	glUniform1i(glGetUniformLocation(bms->getProgram(), "subcount"), numElts);
	unsigned int block_location = glGetUniformLocation(bms->getProgram(), "block");
	unsigned int dim_location = glGetUniformLocation(bms->getProgram(), "dim");
	int dim = 0, block = 0;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_index);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_value);

	for (dim = 2; dim <= sortSize; dim <<= 1)
	{
		glUniform1i(dim_location, dim);
		for (block = dim >> 1; block > 0; block >>= 1)
		{
			glUniform1i(block_location, block);
			bms->dispatch(x, y, z);
			glFinish();
			//Debug::Log("x: %d y: %d z: %d dim: %d block: %d", x, y, z, dim, block);
		}
	}
}

void BitonicMergeSort::Sort(unsigned int ssbo_index, unsigned int ssbo_value, int numElts, int sortSize)
{
	ComputeShader * bms = new ComputeShader("../Asset/Shader/BitonicMergeSort.C.glsl");
	ComputeShader * fill = new ComputeShader("../Asset/Shader/Fill.C.glsl");

	int x, y, z;
	fill->use();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_index);
	ShaderUtil::CalcWorkSize(sortSize, &x, &y, &z);
	fill->dispatch(x,y,z);
	glFinish();
	bms->use();
	glUniform1i(glGetUniformLocation(bms->getProgram(), "count"), sortSize);
	glUniform1i(glGetUniformLocation(bms->getProgram(), "subcount"), numElts);
	unsigned int block_location = glGetUniformLocation(bms->getProgram(), "block");
	unsigned int dim_location = glGetUniformLocation(bms->getProgram(), "dim");
	int dim = 0, block = 0;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_index);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_value);

	for (dim = 2; dim <= sortSize; dim <<= 1)
	{		
		glUniform1i(dim_location, dim);
		for (block = dim >> 1; block > 0; block >>= 1)
		{
			glUniform1i(block_location, block);
			bms->dispatch(x, y, z);
			glFinish();
			//Debug::Log("x: %d y: %d z: %d dim: %d block: %d",x,y,z,dim,block);
		}
	}

	delete bms;
	delete fill;
}