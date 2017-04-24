/******************************************************************************
 *
 * RandomAccess++
 *
 * This benchmark is based on the RandomAccess benchmark. RandomAccess is one
 * of the DARPA HPCS Discrete Math Benchmarks. It was initially developed by
 * David Koester and Bob Lucas.
 *
 * Change log
 * 1) written in C++
 * 2) Allows to configure the following benchmark properties
 *  a) Iteration size, array size
 *  b) Access sparsity
 *  c) Access distribution
 *
 ******************************************************************************
 * Copyright (C) 2015 Jan Ciesko <jciesko@ac.upc.edu>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef SMARTJUMPER_H
#define SMARTJUMPER_H

#include "defines.h"

template<class T>
class SmartJumper
{
	ARRAY_T * array;
	Settings * settings;

public:
	DISTRO<T> * distro;
	SmartJumper(Settings * s)
	{
		settings = s;
		distro   = new DISTRO<GEN>(settings);
		array    = new ARRAY_T[settings->numArrayElements];

		num_bins = 0;
		bin_size = 0;
	}

	void initDistribution();
	void printDistro();
	void printStats();
	void compute(bool);
	void kernel(int, SIZE_T, SIZE_T, ARRAY_T *, bool);
	void clear();
	ARRAY_T * get() {return array;};
	SIZE_T test();
	SIZE_T run();

	SIZE_T num_bins;
	SIZE_T bin_size;
};

template<class T>
void SmartJumper<T>::printStats()
{
	SIZE_T error = 0;
	error = (!settings->flags["test"]) ? 0 : test();
	cout << "SJ, " \
			//Name of distribution
			<< settings->nameDistro + "_" + settings->namePattern                    << ", " \
			//Time
			<< timer_read(0)                                                         << ", " \
			//Problem size
			<< settings->settings["S"]                                               << ", " \
			//Iters
		    << settings->numIters                                                    << ", " \
			//Locality
			<< settings->paramLocality                                               << ", " \
			//Connectivity
			<< settings->numIters / settings->numArrayElements                       << ", " \
			//GUPs
			<< 1e-9 * settings->numIters / timer_read(0)                              << ", " \
			//Normalized lat (ns)
			<< settings->numTasks * 1e9 * timer_read(0) \
			/ (settings->numIters * settings->numOpsPerUpdate)  << ", " \
			//Bandwidth
			<< ((settings->numIters * sizeof(ARRAY_T))>>20) / timer_read(0)          << ", " \
			//Number of tasks
			<< settings->numTasks                                                    << ", " \
			//Number of threads
			<< omp_get_num_threads()                                                 << ", " \
			//Number of errors
			<< error                                                                 << ", " \
			//Test pass
			<< ((error!=0)? "FAILED": "PASSED")                                      << ", " \
			//RT info
			<< num_bins << ", " << bin_size                                          <<      \
			endl;
}

template<class T>
SIZE_T SmartJumper<T>::test()
{
	SIZE_T error = 0;
	compute(false);

	for(auto i = 0; i < settings->numArrayElements; i++) {
		if(array[i] != i)
			error++;
	}
	return error;
}

template<class T>
void SmartJumper<T>::printDistro()
{
	distro->printDistro();
}

template<class T>
void SmartJumper<T>::clear()
{
	SIZE_T start    = 0;
	SIZE_T block    = settings->taskDataBlock;
	SIZE_T blockMod = settings->taskDataBlockMod;
	ARRAY_T * h     = distro->histo;
	int nt          = settings->numTasks;

	for(auto i = 0; i < nt; i++) {
		if(i == nt-1) {
			block += blockMod;
		}
		//#pragma omp task inout (array[start;block], h[start;block])
		{
			for(auto j = start; j < start + block; j++)
			{
				array[j] = j;
				#ifdef RECORD
					distro->histo[j] = 0;
				#endif
			}
		}
		start += block;
	}
	//#pragma omp taskwait noflush
}

#endif
