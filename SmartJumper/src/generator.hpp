#ifndef GENERATOR_H
#define GENERATOR_H
#include <random>

using namespace std;

template<class T>
class Generator
{

public:
	T  generator;

	Generator(){}
	Generator(SIZE_T seedOffset, int taskNum, Settings * settings)
	{
		generator.init(taskNum, settings);
		generator.seed(settings->initialSeed);
		generator.discard(seedOffset);
	}
};

#endif
