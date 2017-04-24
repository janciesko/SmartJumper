#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

//#define RECORD

template<class T>
class Distribution
{
public:
	Generator<T> gen;
	ARRAY_T * histo = NULL;
	Settings * settings;
	Distribution(Settings * s)
	{
		settings = s;
		#ifdef RECORD
			histo  = new ARRAY_T[s->numArrayElements];
		#endif

	}

	void printDistro()
	{
		#ifdef RECORD
			SIZE_T * access_histo_short = new SIZE_T[settings->settings["VRES"]];
			auto block    = settings->numArrayElements  / settings->settings["VRES"];
			auto blockMod = settings->numArrayElements  % settings->settings["VRES"];
			SIZE_T max = 0;
			for(auto i = 0; i < settings->settings["VRES"]; i++)
			{
				access_histo_short[i] = 0;
				block += (i == settings->settings["VRES"])? blockMod : 0;
				for(auto j = i * block; j < i * block + block; j++)
				{
					access_histo_short[i]+= histo[j];
				}
				max = (access_histo_short[i]> max)? access_histo_short[i]: max;
			}
			auto scale =  (float)settings->settings["HRES"] / (float)max;
			for(auto i = 0; i < settings->settings["VRES"]; i++)
			{
				auto len = access_histo_short[i]  * scale;
				cout << i << "[" << string(len,'*') << endl;
			}
		#endif
	}

	inline void record(SIZE_T pos)
	{
		#ifdef RECORD
			#pragma omp atomic
			histo[pos]++;
		#endif
	}
};

template<class T>
class PassThrough : public Distribution<T>
{
	Generator<T> gen;

public:
	inline SIZE_T operator()(Generator<T> * gen)
	{
		SIZE_T pos = gen->generator();
		Distribution<T>::record(pos);
		return pos;
	}
	PassThrough(Settings * s) : Distribution<T>(s)
	{
		s->nameDistro = "Uniform_PT";
	}
	inline void reset()
	{
	}
};


template<class T>
class LinearDistribution : public Distribution<T>
{
public:
	uniform_int_distribution<int> * distro;
	inline SIZE_T operator()(Generator<T> * gen)
	{
		SIZE_T pos = (*distro)(gen->generator);
		Distribution<T>::record(pos);
		return pos;
	}
	LinearDistribution(Settings * s) : Distribution<T>(s)
	{
		distro        = new uniform_int_distribution<int>((SIZE_T)0,(int)(s->numArrayElements - 1));
		s->nameDistro = "Uniform";
	}
	void reset()
	{
		distro->reset();
	}
};

template<class T>
class NormalDistribution : public Distribution<T>
{
	short int logTableSize;
	SIZE_T mask;
public:
	normal_distribution<double> * distro;
	inline SIZE_T operator()(Generator<T>  * gen)
	{
		SIZE_T pos = ((SIZE_T)(*distro)(gen->generator)) & mask;
		Distribution<T>::record(pos);
		return pos;
	}
	NormalDistribution(Settings * s) : Distribution<T>(s)
	{
		distro        = new normal_distribution<double>(s->numArrayElements/2,s->numArrayElements/6);
		logTableSize  = computeLog2(s->numArrayElements);
		mask          = (1  << logTableSize) - 1;
		s->nameDistro = "Normal";
	}
	void reset()
	{
		distro->reset();
	}
};

#endif
