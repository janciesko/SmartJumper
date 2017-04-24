#ifndef SETTINGS_H
#define SETTINGS_H

#include <map>
using namespace std;

class Settings{
public:
	map <string, SIZE_T> settings;
	map <string, bool> flags;
	int numTasks;
	SIZE_T numIters;
	SIZE_T initialSeed;
	SIZE_T numItersTask;
	float  paramLocality;
	SIZE_T taskDataBlock;
	SIZE_T numItersTaskMod;
	SIZE_T numOpsPerUpdate;
	SIZE_T numArrayElements;
	SIZE_T taskDataBlockMod;
	string nameDistro;
	string namePattern;

	Settings() : numTasks(1),\
			initialSeed(1),\
			numIters(0),\
			numItersTask(0),\
			taskDataBlock(0),\
			paramLocality(1),\
			numItersTaskMod(0),\
			numOpsPerUpdate(1),\
			numArrayElements(0),\
			taskDataBlockMod(0),\
			nameDistro("default"),\
			namePattern("default")
	{
		settings["S"]      = 65536;//array size in KB
		settings["N"]      = 0;    //number of iterations (will be set later)
		settings["NT"]     = 1;    //number of tasks to use
		settings["VRES"]   = 16;   //Resolution of access histogram output
		settings["HRES"]   = 16;   //Resolution of access histogram output
		settings["LOC"]    = 0;    //Locality
		flags["version"]   = false;
		flags["help"]      = false;
		flags["test"]      = false;
	}

	void setDefaultValues()
	{
		numTasks             = settings["NT"];
		numIters             = (settings["N"]==0)? settings["N"] = CONNECTIVITY * (settings["S"] << 10) / sizeof(ARRAY_T) :settings["N"];
		numItersTask         = numIters / settings["NT"];
		numItersTaskMod      = numIters % settings["NT"];
		numArrayElements     = (settings["S"] << 10) / (SIZE_T)sizeof(ARRAY_T); //KB to Byte
		taskDataBlock        = numArrayElements / settings["NT"];
		taskDataBlockMod     = numArrayElements % settings["NT"];
		paramLocality        = (settings["LOC"] > 100) ? 1 : settings["LOC"] / 100.0;
		paramLocality        = (settings["LOC"] < 0) ? 0 : paramLocality;
	}

	void setNumIters(SIZE_T it)
	{
		numIters          = it;
		numItersTask      = numIters / numTasks;
		numItersTaskMod   = numIters % numTasks;
	}
};

#endif

