#include <iostream>
#ifdef _WIN32
	#include <windows.h>
	#include <direct.h>
	#define getcwd _getcwd
	#define dlsym GetProcAddress
	#define PATH_MAX MAX_PATH
#else
	#include <unistd.h>
	#include <dlfcn.h>
#endif
#include <limits.h>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using GRIDFunc = void (*)(const char* jsonPath, int argc, char* argv[]);

int main(int argc, char* argv[]) {
	std::cout << "(G)raphical(R)untime for (I)nteractive(D)evelopment" << std::endl;
	std::cout << "GRID Launcher v0.1" << std::endl;

	if(argc < 2) {
		std::cout << "No application config path provided" << std::endl;
		std::cout << "Run with 'GRID path/to/config.json'" << std::endl;
		return 0;
	}

	char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
	std::string jsonPath = std::string(cwd) + "/" + argv[1];
	std::string relativePath = jsonPath.substr(0, jsonPath.find_last_of("/"));

	std::ifstream f(jsonPath);
	json data;
	try {
		data = json::parse(f);
	}
	catch (const json::parse_error& e) {
		std::cout << "Invalid JSON config: " << jsonPath << std::endl;
		return 0;
	}

	std::string GRIDPath = relativePath + "/" + data["GRIDCoreLib"].get<std::string>();

	#ifdef _WIN32
		HMODULE hGRIDLib = LoadLibrary(GRIDPath.c_str());
	#else
		void* hGRIDLib = dlopen(GRIDPath.c_str(), RTLD_LAZY);
	#endif

    if (hGRIDLib == nullptr) {
	#ifdef _WIN32
		LPSTR msg = nullptr;
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(), 0, (LPSTR)&msg, 0, nullptr);
		std::cerr << "Failed to load library: " << GRIDPath << "\n" << "Windows error: " << (msg ? msg : "Unknown error") << std::endl;
		LocalFree(msg);
	#else
		std::cout << "Failed to load library: " << dlerror() << " " << GRIDPath.c_str() << std::endl;
	#endif
    }

	GRIDFunc GRIDStart = (GRIDFunc)dlsym(hGRIDLib, "start");

    if (GRIDStart == nullptr) {
        std::cout << "Failed to load function: start" << std::endl;
    }

	GRIDStart(jsonPath.c_str(), argc-2, argv+2);
}