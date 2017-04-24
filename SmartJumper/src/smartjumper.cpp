/******************************************************************************
 *
 * SmartJumper
 *
 * This benchmark is based on the RandomAccess benchmark. RandomAccess is one
 * of the DARPA HPCS Discrete Math Benchmarks. It was initially developed by
 * David Koester and Bob Lucas.
 *
 * Changes log
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

#include <iostream>
#include <string>
#include <map>
#include <random>
#include <vector>
#include <exception>
#include <sys/time.h>
#include "timers.h"
#include "defines.h"
#include "settings.hpp"
#include "helper.hpp"
#include "generator.hpp"
#include "distribution.hpp"
#include "lcg.h"
#include "lcg_pow2.h"
#include "linear.h"
#include "linear_pow2.h"
#include "smartjumper.hpp"

using namespace std;

template<class T>
inline void SmartJumper<T>::kernel(int taskNum, SIZE_T iters, SIZE_T block, ARRAY_T * a, bool doConcurrently)
{
	if(doConcurrently)
	{
		SIZE_T a_size = settings->numArrayElements;
		#pragma omp task label(kernel)// reduction(^:[a_size]a)
		{
			Generator<T> gen(taskNum * iters, taskNum, settings);
			for(auto i = 0; i < iters; ++i)
			{
				SIZE_T pos = (*distro)(&gen);
				//#pragma omp atomic
				a[pos] ^= pos;
			}
	}
	}else
	{
		Generator<T> gen(taskNum * iters, taskNum, settings);
		for(auto i = 0; i < iters; ++i)
		{
			SIZE_T pos = (*distro)(&gen);
			a[pos] ^= pos;
		}
	}

}

template<class T>
void SmartJumper<T>::compute(bool doConcurrently)
{
	SIZE_T start    = 0;
	SIZE_T block    = settings->taskDataBlock;
	SIZE_T blockMod = settings->taskDataBlockMod;
	SIZE_T iters    = settings->numItersTask;
	SIZE_T itersMod = settings->numItersTaskMod;
	int nt          = settings->numTasks;

	ARRAY_T * a = array;
	SIZE_T a_size = settings->numArrayElements;

	for(auto i = 0; i < nt; i++) {
		if(i == nt-1) {
			block += blockMod;
			iters += itersMod;
		}
		kernel(i, iters, block, a, doConcurrently);
		start += block;
	}
	#pragma omp taskwait
}

#define PRECONDITIONER

int main(int argc, const char** argv)
{
	Settings * settings = new Settings();
	Helper::parseParams(argc, argv, settings);
	SmartJumper<GEN> * ra = new SmartJumper<GEN>(settings);
	ra->clear();

	//quadruple problem size every loop iteration\
	to reach a representative timing of >=3 sec.
	do{

#ifdef PRECONDITIONER
		ra->compute(true);
		ra->clear();
		//#pragma omp inspector_use(a)
#endif
		timer_clear(0);
		timer_start(0);
		ra->compute(true);
		timer_stop(0);

		if(timer_read(0) >3) break;
		ra->clear();
		settings->setNumIters(settings->numIters * 4);
		//#pragma omp inspector_free(a)

	}while (1);

	ra->printDistro();
	ra->printStats();
	return 1;
}
