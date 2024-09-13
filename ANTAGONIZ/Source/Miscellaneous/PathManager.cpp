#include "PathManager.hpp"

#ifdef _WIN32
#include <Windows.h>

#include <filesystem>

namespace fs = std::filesystem;

#elif __linux__
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <iostream>
#include <cstring>
#endif

std::string PathManager::homeProjectPath = "";

std::string PathManager::getHomeDirectory()
{
	std::string homeDir;

#ifdef _WIN32
	char homeDrive[MAX_PATH];
	if (GetEnvironmentVariable("HOMEDRIVE", homeDrive, MAX_PATH) > 0)
	{
		homeDir = std::string(homeDrive) + "/Users/Public/Antagoniz";
	}
#elif __linux__
	const char* homePath = getenv("HOME");
	if (homePath != nullptr)
	{
		homeDir = std::string(homePath) + "/.Antagoniz";
	}
#endif

	return homeDir;
}

void PathManager::initDirectory()
{
#ifdef _WIN32
	fs::create_directories(getHomeDirectory());
#elif __linux__
	mkdir(getHomeDirectory().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

void PathManager::workDirectory(const char * path)
{
#ifdef _WIN32

#elif __linux__
	size_t length = std::strlen(path);
	if (length == 0)
	{
		return;
	}

	size_t lastSlash = length - 1;
	while (lastSlash > 0 && path[lastSlash] != '/')
	{
		lastSlash--;
	}

	char* result = new char[lastSlash + 2];
	std::strncpy(result, path, lastSlash + 1);
	result[lastSlash + 1] = '\0';
	chdir(result);
#endif
}

std::string & PathManager::getHomeProjectPath()
{
	return homeProjectPath;
}

void PathManager::setHomeProjectPath(std::string path)
{
	homeProjectPath = path;
}
