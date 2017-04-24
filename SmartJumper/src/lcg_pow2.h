#ifndef LCG_POW2_H
#define LCG_POW2_H

#include <limits>
//#include "defines.h"

#define LCG_MUL_64 2147483647ULL
#define LCG_ADD 48271

#define NUM_OPS_PER_UPDATE_LCG_POW2 4
//#define OVERFLOW_FOLD

class LCG_pow2
{
	SIZE_T mul_k, add_k, _seed;
	short int bitSize, logTableSize;
	SIZE_T mask;
	SIZE_T array_offset, overflow_fold_offset;

public:
	LCG_pow2(){}

	void init(int taskNum, Settings * settings)
	{
		settings->namePattern = "lcg_pow2";
		mul_k         = LCG_MUL_64;
		add_k         = LCG_ADD;
		bitSize       = sizeof(SIZE_T) * 8;
		settings->numOpsPerUpdate = NUM_OPS_PER_UPDATE_LCG_POW2;
		logTableSize         = computeLog2(settings->numArrayElements);
		mask                 = bitSize - logTableSize;
	}

	void seed(SIZE_T s)
	{
		_seed = s;
	}

	void discard(SIZE_T seedOffset)
	{
		SIZE_T ran = 1, un, add_k_t = add_k, mul_k_t = mul_k;
		for (un = (SIZE_T) seedOffset; un; un >>= 1) {
			if (un & 1)
				ran = mul_k_t * ran + add_k_t;
			add_k_t *= (mul_k_t + 1);
			mul_k_t *= mul_k_t;
		}
		_seed = ran;
	}


	inline SIZE_T getNextNumber()
	{
		_seed = (_seed * mul_k) + add_k;
		return _seed;
	}

	inline SIZE_T normalizeNumber(SIZE_T n)
	{
		return n >> mask;
	}

	inline SIZE_T operator()()
	{
		return  normalizeNumber(getNextNumber());
	}

	SIZE_T min(){return 1;}
	SIZE_T max(){return numeric_limits<SIZE_T>::max();}
};

#endif
