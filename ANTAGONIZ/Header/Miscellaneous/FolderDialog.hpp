#ifndef __ENGINE_FOLDER_DIALOG__
#define __ENGINE_FOLDER_DIALOG__

#include <iostream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <ShlObj.h>
#elif __linux__
#include <cstdlib>
#endif

namespace Ge
{
	class FolderDialog
	{
	public:
		static std::string openFileDialog();
		static std::string openDialog();
		static std::string savePngFileDialog();
		static std::string saveFileWindowsDialog(std::string ext);
		static void openFolder(std::string path);
	private:
#ifdef _WIN32
		static std::string openWindowsFileDialog(); 
		static std::string openWindowsDialog();
		static std::string savePngFileWindowsDialog();
#elif __linux__
		static std::string openLinuxDialog();
		static std::string openLinuxFileDialog();
		static std::string savePngFileLinuxDialog();
#endif
	};
}

#endif //!__ENGINE_FOLDER_DIALOG__