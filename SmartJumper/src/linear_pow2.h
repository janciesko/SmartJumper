#ifndef LINEAR_POW2_H
#define LINEAR_POW2_H

#include <limits>
#include "defines.h"

#define LCG_MUL_64 2147483647ULL
#define LCG_ADD 48271

#define NUM_OPS_PER_UPDATE_LINEAR_POW2 3
//#define OVERFLOW_FOLD

class Linear_pow2
{
	SIZE_T mul_k, add_k, _seed;
	short int bitSize, logTableSize;
	SIZE_T mask;
	SIZE_T array_offset;

public:
	Linear_pow2(){}

	void init(int taskNum, Settings * settings)
	{
		settings->namePattern = "linear_pow2";
		mul_k         = LCG_MUL_64;
		add_k         = LCG_ADD;
		bitSize       = sizeof(SIZE_T) * 8;
		settings->numOpsPerUpdate = NUM_OPS_PER_UPDATE_LINEAR_POW2;
		logTableSize         = computeLog2(settings->numArrayElements);
		mask                 = (1<<logTableSize)-1;
	}

	void seed(SIZE_T s)
	{
		_seed = s;
	}

	void discard(SIZE_T seedOffset)
	{
		_seed += seedOffset;
	}

	inline SIZE_T getNextNumber()
	{
		return _seed++;
	}

	inline SIZE_T normalize(SIZE_T n)
	{
		return n & mask;
	}

	inline SIZE_T operator()()
	{
		return normalize( getNextNumber());

	}

	SIZE_T min(){return 1;}
	SIZE_T max(){return numeric_limits<SIZE_T>::max();}
};
#endif
