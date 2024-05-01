#ifndef __ENGINE_COMPUTE_SHADER__
#define __ENGINE_COMPUTE_SHADER__

#include <string>

namespace Ge
{
	class ComputeShader
	{
	public:
		ComputeShader(std::string filename);
		~ComputeShader();
		void use();
		void dispatch(int num_groups_x, int num_groups_y, int num_groups_z);
		unsigned int getProgram() const;
	private:
		unsigned int m_computeShader;
		unsigned int m_computeProgram;
	};
}

#endif //!__ENGINE_COMPUTE_SHADER__