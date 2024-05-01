#ifndef __BITONIC_MERGE_SORT__
#define __BITONIC_MERGE_SORT__

#include "ComputeShader.hpp"
using namespace Ge;
class BitonicMergeSort
{
public:
	static void Sort(unsigned int ssbo_index,unsigned int ssbo_value, int numElts, int sortSize);
	static void Sort(ComputeShader * bms, ComputeShader * fill, unsigned int ssbo_index, unsigned int ssbo_value, int numElts, int sortSize);
};

#endif //!__BITONIC_MERGE_SORT__
