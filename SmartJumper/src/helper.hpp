#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <exception>
#include "omp.h"
#include "settings.hpp"

using namespace std;

#define VERSION_STRING "RandomAccess++ V.1.0"
#define USAGE_STRING "Usage: ra++ options \n\
  -N <value>           Iteration size \n\
  -S <value>           Problem size \n\
  -NT <value>          Number of tasks \n\
  -LOC <value>         Locality [0-100] \n\
  -version             Display version information\n\
  -help                Display this help \
  -test                Test output"

class Helper{
public:
	static int const numParams = 6;
	static vector<string> flagsVector;
	static string const paramErrorMsg;
	static map<string, string> paramMap;
	static vector<string> mendatoryParamsVector;
	static void parseParams (const int argc, const char ** argv, Settings * settings);

};

static SIZE_T computeLog2(SIZE_T in) {
	SIZE_T out;
	for (in *= 0.5, out = 0; in >= 1.0; in *= 0.5, out++)
		;
	return out;
}

#endif

