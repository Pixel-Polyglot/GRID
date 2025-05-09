#include <iostream>
#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using GRIDFunc = void (*)(const char* jsonPath, int argc, char* argv[]);

int main(int argc, char* argv[]) {
	std::cout << "GRID Launcher v0.1" << std::endl;

	if(argc < 2) {
		std::cout << "No application config path provided" << std::endl;
		std::cout << "Run with 'GRID path/to/config.json'" << std::endl;
		return 0;
	}

	char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
	std::string jsonPath = std::string(cwd) + "/" + argv[1];

	std::ifstream f(jsonPath);
	json data;
	try {
		data = json::parse(f);
	}
	catch (const json::parse_error& e) {
		std::cout << "Invalid JSON config: " << jsonPath << std::endl;
		return 0;
	}

	std::string GRIDPath = std::string(cwd) + "/" + data["GRIDCoreLib"].get<std::string>();

	void* hGRIDLib = dlopen(GRIDPath.c_str(), RTLD_LAZY);
    if (hGRIDLib == nullptr) {
        std::cout << "failed to load library: " << dlerror() << std::endl;
    }

	GRIDFunc GRIDStart = (GRIDFunc) dlsym(hGRIDLib, "start");

    if (GRIDStart == nullptr) {
        std::cout << "failed to load function: start" << std::endl;
    }

	GRIDStart(jsonPath.c_str(), argc-2, argv+2);
}