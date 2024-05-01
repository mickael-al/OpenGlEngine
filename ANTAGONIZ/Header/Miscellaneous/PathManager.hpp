#ifndef __PATH_MANAGER__
#define __PATH_MANAGER__

#include <iostream>
#include <string>

class PathManager
{
public:
	static std::string getHomeDirectory();
	static void initDirectory();
	static void workDirectory(const char * path);
};

#endif//!__PATH_MANAGER__