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
	static std::string & getHomeProjectPath();
	static void setHomeProjectPath(std::string path);	
private:
	static std::string homeProjectPath;
};

#endif//!__PATH_MANAGER__