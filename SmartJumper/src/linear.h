#ifndef LINEAR_H
#define LINEAR_H

#include <limits>
#include <stdio.h>

#define LCG_MUL_64_LIN 1
#define LCG_ADD_LIN 1

#define NUM_OPS_PER_UPDATE_LINEAR 7
#define OVERFLOW_FOLD

extern SIZE_T dummy();

class Linear
{
	SIZE_T mul_k, add_k;
	SIZE_T volatile _seed;
	short int bitSize, logTableSize;
	SIZE_T mask;
	SIZE_T array_offset, overflow_fold_offset, ABS_shift;
	float scale_factor;

public:
	Linear(){}

	void init(int taskNum, Settings * settings)
	{
		settings->namePattern = "linear";
		mul_k         = LCG_MUL_64_LIN + dummy(); //avoid compiler optimization
		add_k         = LCG_ADD_LIN + dummy();
		bitSize       = sizeof(SIZE_T) * 8;
		settings->numOpsPerUpdate = NUM_OPS_PER_UPDATE_LINEAR;

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
			overflow_fold_offset = (one << (logTableSize+1)) - numArrayElementsTask;
			scale_factor         = (float)numArrayElementsTask / (float)( one <<(logTableSize+1) );
			mask                 = ( one <<(logTableSize+1))-1;
		}
		else
		{
			overflow_fold_offset = 0;
			scale_factor         = 1.0;
			mask                 = ( one <<logTableSize)-1;
		}
	}

	void seed(SIZE_T s)
	{
		_seed = s;
	}

	void discard(SIZE_T seedOffset)
	{
		_seed += seedOffset;
	}


	inline SIZE_T  getNextNumber()
	{
		_seed = (_seed * mul_k) + add_k;
		return _seed;
	}

	inline SIZE_T normalizeNumberShift(SIZE_T n)
	{
		return ((n & mask) - overflow_fold_offset);
	}

	inline SIZE_T normalizeNumberScale(SIZE_T n)
	{
		//type conversions kill performance
		float nn = (float) (n & mask);
		return (SIZE_T) (nn * scale_factor);
	}

	inline SIZE_T abs(SIZE_T n)
	{
		SIZE_T sign = n >> ABS_shift;
		return -sign ^ (n - sign);
	}

	inline SIZE_T operator()()
	{
		SIZE_T r = getNextNumber(); //1 OP
		#ifdef OVERFLOW_FOLD
			SIZE_T rn  = normalizeNumberShift(r);
 			SIZE_T rna = abs(rn);
			return array_offset + rna;
		#else
			SIZE_T rn  =  normalizeNumberScale(r); //2 OPS
			return array_offset + rn; //1 OP
		#endif
	}

	SIZE_T min(){return 1;}
	SIZE_T max(){return numeric_limits<SIZE_T>::max();}
};
#endif
