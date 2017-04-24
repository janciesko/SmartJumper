#ifndef LCG_H
#define LCG_H

#include <limits>

#define LCG_MUL_64_LCG 2147483647ULL
#define LCG_ADD_LCG 48271

#define NUM_OPS_PER_UPDATE_LCG 7
#define OVERFLOW_FOLD

extern SIZE_T dummy();

class LCG
{
	SIZE_T mul_k, add_k;
	SIZE_T volatile _seed;
	short int bitSize, logTableSize;
	SIZE_T mask;
	SIZE_T array_offset, overflow_fold_offset, ABS_shift;
	float scale_factor;

public:
	LCG(){}

	void init(int taskNum, Settings * settings)
	{
		settings->namePattern = "lcg";
		mul_k         = LCG_MUL_64_LCG + dummy(); //avoid compiler optimization
		add_k         = LCG_ADD_LCG + dummy();
		bitSize       = sizeof(SIZE_T) * 8;
		settings->numOpsPerUpdate = NUM_OPS_PER_UPDATE_LCG;

		//setup array sizing depending of locality
		SIZE_T numArrayElementsTask;

		numArrayElementsTask = settings->numArrayElements \
							 / settings->numTasks;
		//apply locality parameter
		numArrayElementsTask += (settings->numArrayElements - numArrayElementsTask) \
				             * (1.0-settings->paramLocality);

		//compute the locality offset
		array_offset         = (settings->numTasks >1)?
				             (settings->numArrayElements - numArrayElementsTask) \
				             / (settings->numTasks-1):
				             0;
		array_offset *= taskNum;

		//this will be used to compute the absolute value for a position
		ABS_shift = bitSize - 1;

		//compute the table sizes and set it to the next bigger size
		//set offset to subtract from each index
		//apply later ABS(index) to avoid negative indexes

		SIZE_T one = 1;

		logTableSize  = computeLog2(numArrayElementsTask);
		if(((numArrayElementsTask - (one <<(logTableSize) )) > 0))
		{
			overflow_fold_offset = (one <<(logTableSize+1)) - numArrayElementsTask;
			scale_factor         = (float)numArrayElementsTask / (float)(one <<(logTableSize+1));
			mask                 = bitSize - logTableSize - 1;
		}
		else
		{
			overflow_fold_offset = 0;
			scale_factor         = 1.0;
			mask                 = bitSize - logTableSize;
		}
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

	inline SIZE_T normalizeNumberShift(SIZE_T n)
	{
		return ((n >> mask) - overflow_fold_offset);
	}

	inline SIZE_T normalizeNumberScale(SIZE_T n)
	{
		//type conversions kill performance
		float nn = (float) (n >> mask);
		return (SIZE_T) (nn * scale_factor);
	}

	inline SIZE_T abs(SIZE_T n)
	{
		SIZE_T sign = n >> ABS_shift;
		return -sign ^ (n - sign);
	}

	inline SIZE_T operator()()
	{
		SIZE_T r = getNextNumber(); //2 OPS
		#ifdef OVERFLOW_FOLD
			SIZE_T rn  = normalizeNumberShift(r);
 			SIZE_T rna = abs(rn);
			return array_offset + rna;
		#else
			SIZE_T rn  =  normalizeNumberScale(r); //2 OPS
			return array_offset + rn;    //1 OP
		#endif
	}


	SIZE_T min(){return 1;}
	SIZE_T max(){return numeric_limits<SIZE_T>::max();}
};

#endif
