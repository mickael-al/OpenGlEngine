#ifndef __SHADER_UTIL__
#define __SHADER_UTIL__

#include <vector>
#include <algorithm>

class ShaderUtil
{
public:   
    static void CalcWorkSize(int length, int * x, int * y, int * z)
    {
        int GROUP_SIZE = 256;
        int MAX_DIM_GROUPS = 256;
        int MAX_DIM_THREADS = (GROUP_SIZE * MAX_DIM_GROUPS);
        int MAX_DIM_THREADS_THREADS = (MAX_DIM_THREADS * MAX_DIM_GROUPS);
        if (length <= MAX_DIM_THREADS)
        {
            *x = (length - 1) / GROUP_SIZE + 1;
            *y = *z = 1;
        }
        else if (length <= MAX_DIM_THREADS_THREADS)
        {
            *x = MAX_DIM_GROUPS;
            *y = (length - 1) / MAX_DIM_THREADS + 1;
            *z = 1;
        }
        else
        {
            *x = *y = MAX_DIM_GROUPS;
            *z = (length - 1) / MAX_DIM_THREADS_THREADS + 1;
        }
    }

	static int PowTwoUp(int nb)
	{
		int p = 1;
		while (p < nb)
		{
			p <<= 1;
		}
		return p;
	}

    static std::vector<int> DecomposeFirstFactors(int n)
    {
        std::vector<int> facteursPremiers = std::vector<int>();

        while (n % 2 == 0)
        {
            facteursPremiers.push_back(2);
            n /= 2;
        }

        for (int i = 3; i <= sqrt(n); i += 2)
        {
            while (n % i == 0)
            {
                facteursPremiers.push_back(i);
                n /= i;
            }
        }

        if (n > 2)
        {
            facteursPremiers.push_back(n);
        }

        return facteursPremiers;
    }

	static std::vector<int> FindBestDecomposition(int n, int& reste, int min = 2, int max = 32, int baseMax = 512) 
	{
		std::vector<int> facteursPremiers = DecomposeFirstFactors(n);
		reste = 0;
		int countMax = 0;

		for (size_t i = 0; i < facteursPremiers.size(); ++i) 
		{
			if (facteursPremiers[i] > max) 
			{
				countMax++;
			}
			if (countMax >= 2 || facteursPremiers[i] > baseMax || facteursPremiers.size() == 1) 
			{
				i = -1;
				reste++;
				countMax = 0;
				facteursPremiers = DecomposeFirstFactors(n - reste);
			}
		}

		while (facteursPremiers.size() > 1 && *std::min_element(facteursPremiers.begin(), facteursPremiers.end()) < min) 
		{
			auto itMin = std::min_element(facteursPremiers.begin(), facteursPremiers.end());
			int indexMin = std::distance(facteursPremiers.begin(), itMin);
			int minElement = *itMin;

			facteursPremiers.erase(itMin);

			auto itNextMin = std::min_element(facteursPremiers.begin(), facteursPremiers.end());
			int indexNextMin = std::distance(facteursPremiers.begin(), itNextMin);
			int nextMinElement = *itNextMin;

			int newElement = minElement * nextMinElement;

			if (newElement <= max) {
				facteursPremiers.erase(itNextMin);
				facteursPremiers.push_back(newElement);
			}
			else {
				facteursPremiers.insert(facteursPremiers.begin() + indexMin, minElement);
				return facteursPremiers;
			}
		}

		int product = 0;
		std::sort(facteursPremiers.begin(), facteursPremiers.end());

		while (facteursPremiers.size() > 3 && facteursPremiers[facteursPremiers.size() - 1] <= baseMax / 2) 
		{
			int largest = facteursPremiers[facteursPremiers.size() - 1];
			int smallest = facteursPremiers[0];
			product = largest * smallest;
			facteursPremiers.erase(facteursPremiers.begin());
			facteursPremiers.pop_back();
			facteursPremiers.push_back(product);
			std::sort(facteursPremiers.begin(), facteursPremiers.end());
		}

		return facteursPremiers;
	}
};


#endif // !__SHADER_UTIL__
