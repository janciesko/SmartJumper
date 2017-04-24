
#include "defines.h"
#include "helper.hpp"
#include "settings.hpp"
using namespace std;

const string Helper::paramErrorMsg = "Usage: ra -version -help -parameter value. Exiting...";
map<string, string> Helper::paramMap;
vector<string> Helper::flagsVector;

void Helper::parseParams (const int argc, const char ** argv, Settings * settings)
{
	for(auto i = 0; i < argc; ++i){
		string s(argv[i]);
		auto pos = s.find("-");
		if(pos == string::npos) continue;
		auto p  = s.substr(pos+1);
		if(i + 1 < argc) {
			string sNext (argv[i+1]);
			if(sNext.find("-") != string::npos)
				flagsVector.push_back(p);
			else
				paramMap.insert(pair<string, string> (p,sNext));
		}else
			flagsVector.push_back(p);
	}

	for(auto & i : paramMap){
		auto p = settings->settings.find(i.first);
		if (p != settings->settings.end())
			try{
			p->second = stol(i.second, nullptr);
			}catch (exception e) {
				cout << "Warning: Parameter " << i.first << " has a wrong numerical value." << endl;
			}
		else
			cout << "Warning: Parameter " << i.first << " was not recognized." << endl;
	}

	for(auto i : flagsVector){
		auto p = settings->flags.find(i);
		if (p != settings->flags.end())
			p->second = true;
		else
			cout << "Warning: Flag " << i << " was not recognized." << endl;
	}

	if(settings->flags["version"]){ cout << VERSION_STRING << endl; exit(1);}
	if(settings->flags["help"]){ cout << USAGE_STRING << endl; exit(1);}

	settings->setDefaultValues();
}
