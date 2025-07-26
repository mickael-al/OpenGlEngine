#include "FolderDialog.hpp"
#include <algorithm> // Pour std::transform
#include <cctype>    // Pour std::tolower, std::toupper

namespace Ge
{
	std::string FolderDialog::openDialog()
	{
#ifdef _WIN32
		return openWindowsDialog();
#elif __linux__
		return openLinuxDialog();
#else
		return "Plateforme non supportée";
#endif
	}

	std::string FolderDialog::openFileDialog()
	{
#ifdef _WIN32
		return openWindowsFileDialog();
#elif __linux__
		return openLinuxFileDialog();
#else
		return "Plateforme non supportée";
#endif
	}

	std::string FolderDialog::savePngFileDialog()
	{
#ifdef _WIN32
		return savePngFileWindowsDialog();
#elif __linux__
		return savePngFileLinuxDialog();
#else
		return "Plateforme non supportée";
#endif
	}

	std::string toLower(const std::string& str) {
		std::string lowerStr = str;
		std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return lowerStr;
	}

	std::string toUpper(const std::string& str) {
		std::string upperStr = str;
		std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(),
			[](unsigned char c) { return std::toupper(c); });
		return upperStr;
	}


	std::string FolderDialog::saveFileWindowsDialog(std::string ext)
	{
		std::string filePath;
		OPENFILENAME ofn;
		TCHAR szFile[MAX_PATH] = { 0 };

		// Convertion de l'extension en minuscule et majuscule
		std::string lowerExt = toLower(ext);
		std::string upperExt = toUpper(ext);

		TCHAR currentDir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, currentDir);

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		// Modification du titre de la fenêtre et des filtres avec l'extension
		std::string txt = ("Enregistrer un fichier" + upperExt);
		ofn.lpstrTitle = TEXT(txt.c_str());

		// Remplacement de "png" par l'extension spécifiée
		std::string filter = upperExt + " files (*." + lowerExt + ")\0*." + lowerExt + "\0All files (*.*)\0*.*\0";
		ofn.lpstrFilter = filter.c_str();

		// Remplacement de l'extension par défaut
		ofn.lpstrDefExt = lowerExt.c_str();

		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

		if (GetSaveFileName(&ofn) == TRUE) {
			filePath = ofn.lpstrFile;
		}
		SetCurrentDirectory(currentDir);
		return filePath;
	}

	void FolderDialog::openFolder(std::string path)
	{
#ifdef _WIN32
		ShellExecute(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#elif __linux__

#endif
	}

#ifdef _WIN32
	std::string FolderDialog::openWindowsFileDialog()
	{
		std::string folderPath;
		OPENFILENAME ofn;
		TCHAR szFile[MAX_PATH] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrTitle = TEXT("Choisir un dossier");
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = NULL;

		if (GetOpenFileName(&ofn) == TRUE) {
			folderPath = ofn.lpstrFile;
		}

		return folderPath;
	}

	std::string FolderDialog::savePngFileWindowsDialog()
	{
		std::string filePath;
		OPENFILENAME ofn;
		TCHAR szFile[MAX_PATH] = { 0 };

		TCHAR currentDir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, currentDir);

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrTitle = TEXT("Enregistrer un fichier PNG");
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = TEXT("png");
		ofn.lpstrFilter = TEXT("PNG files (*.png)\0*.png\0All files (*.*)\0*.*\0");

		if (GetSaveFileName(&ofn) == TRUE) {
			filePath = ofn.lpstrFile;
		}
		SetCurrentDirectory(currentDir);
		return filePath;
	}

	std::string FolderDialog::openWindowsDialog() 
	{
		std::wstring folderPath;
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr)) 
		{
			IFileOpenDialog *pFileOpen;

			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

			if (SUCCEEDED(hr)) 
			{
				DWORD dwOptions;
				hr = pFileOpen->GetOptions(&dwOptions);
				if (SUCCEEDED(hr)) 
				{
					hr = pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
				}

				if (SUCCEEDED(hr)) 
				{
					hr = pFileOpen->Show(NULL);

					if (SUCCEEDED(hr)) 
					{
						IShellItem *pItem;
						hr = pFileOpen->GetResult(&pItem);

						if (SUCCEEDED(hr)) 
						{
							PWSTR pszFolderPath;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);

							if (SUCCEEDED(hr)) 
							{
								folderPath = pszFolderPath;
								CoTaskMemFree(pszFolderPath);
							}

							pItem->Release();
						}
					}
				}

				pFileOpen->Release();
			}

			CoUninitialize();
		}
		return std::string(folderPath.begin(), folderPath.end());
	}
#elif __linux__
	std::string FolderDialog::openLinuxDialog()
	{
		std::string folderPath;

		// Utilisation de zenity pour ouvrir la boîte de dialogue de sélection de dossier
		FILE* pipe = popen("zenity --file-selection --directory", "r");
		if (!pipe) {
			return "Erreur lors de l'exécution de zenity";
		}

		char buffer[128];
		std::string result = "";
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL) {
				result += buffer;
			}
}
		pclose(pipe);

		// Nettoyer le résultat pour supprimer les retours à la ligne
		size_t pos = result.find_last_not_of("\n\r");
		if (pos != std::string::npos) {
			folderPath = result.substr(0, pos + 1);
		}
		else {
			folderPath = result;
		}

		return folderPath;
	}

	std::string FolderDialog::openLinuxFileDialog()
	{
		std::string folderPath;

		// Utilisation de zenity pour ouvrir la boîte de dialogue de sélection de dossier
		FILE* pipe = popen("zenity --file-selection", "r");
		if (!pipe) {
			return "Erreur lors de l'exécution de zenity";
		}

		char buffer[128];
		std::string result = "";
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL) {
				result += buffer;
			}
		}
		pclose(pipe);

		// Nettoyer le résultat pour supprimer les retours à la ligne
		size_t pos = result.find_last_not_of("\n\r");
		if (pos != std::string::npos) {
			folderPath = result.substr(0, pos + 1);
		}
		else {
			folderPath = result;
		}

		return folderPath;
	}

	std::string FolderDialog::savePngFileLinuxDialog()
	{
		std::string filePath;

		// Utilisation de zenity pour ouvrir la boîte de dialogue de sauvegarde de fichier
		FILE* pipe = popen("zenity --file-selection --save --file-filter='PNG files ( *.png ) | *.png'", "r");
		if (!pipe) {
			return "Erreur lors de l'exécution de zenity";
		}

		char buffer[128];
		std::string result = "";
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL) {
				result += buffer;
			}
		}
		pclose(pipe);

		// Nettoyer le résultat pour supprimer les retours à la ligne
		size_t pos = result.find_last_not_of("\n\r");
		if (pos != std::string::npos) {
			filePath = result.substr(0, pos + 1);
		}
		else {
			filePath = result;
		}

		return filePath;
	}
#endif
}