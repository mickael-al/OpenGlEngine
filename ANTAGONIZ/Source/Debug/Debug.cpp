#include "Debug.hpp"
#include "termcolor/termcolor.hpp"
#include <fstream>
#include <cstdarg>

namespace Ge
{
        char Debug::buffer[BUFFER_SIZE];
        char Debug::tempbuffer[BUFFER_SIZE];
        Console* Debug::console = nullptr;
        void Debug::Log(const char *format, ...)
        {
                va_list argList;
                va_start(argList, format);
#ifdef _WIN32
                vsprintf(buffer, format, argList);
                sprintf_s(tempbuffer, "[ENGINE][USER] : %s\n", buffer);
#elif __unix__
                vsprintf(buffer, format, argList);
                sprintf(tempbuffer, "[ENGINE][USER] : %s\n", buffer);
#endif
                va_end(argList);

                std::cout << tempbuffer;
        }

        void Debug::Error(const char *format, ...)
        {
                va_list argList;
                va_start(argList, format);
#ifdef _WIN32
                vsprintf(buffer, format, argList);
                sprintf_s(tempbuffer, "[ENGINE][ERROR] : %s\n", buffer);
#elif __unix__
                vsprintf(buffer, format, argList);
                sprintf(tempbuffer, "[ENGINE][ERROR] : %s\n", buffer);
#endif
                va_end(argList);

                std::cerr << termcolor::red << tempbuffer << termcolor::reset;
                std::cin.ignore();
        }

        void Debug::INITSUCCESS(const char *format)
        {
            Debug::Info("Initialisation : %s", format);
        }

        void Debug::RELEASESUCCESS(const char *format)
        {
            Debug::Info("Liberation : %s", format);
        }

        void Debug::INITFAILED(const char *format)
        {
            Debug::Error("Erreur lors de l'initialisation : %s", format);
        }

        void Debug::Warn(const char *format, ...)
        {
                va_list argList;
                va_start(argList, format);
#ifdef _WIN32
                vsprintf(buffer, format, argList);
                sprintf_s(tempbuffer, "[ENGINE][WARN] : %s\n", buffer);
#elif __unix__
                vsprintf(buffer, format, argList);
                sprintf(tempbuffer, "[ENGINE][WARN] : %s\n", buffer);
#endif
                va_end(argList);

                std::cout << termcolor::yellow << tempbuffer << termcolor::reset;
        }

        void Debug::Info(const char *format, ...)
        {
                va_list argList;
                va_start(argList, format);
#ifdef _WIN32
                vsprintf(buffer, format, argList);
                sprintf_s(tempbuffer, "[ENGINE][INFO] : %s\n", buffer);
#elif __unix__
                vsprintf(buffer, format, argList);
                sprintf(tempbuffer, "[ENGINE][INFO] : %s\n", buffer);
#endif
                va_end(argList);

                std::cout << termcolor::cyan << tempbuffer << termcolor::reset;
        }

        void Debug::VLayer(const char *format, ...)
        {
                va_list argList;
                va_start(argList, format);
#ifdef _WIN32
                vsprintf(buffer, format, argList);
                sprintf_s(tempbuffer, "[ENGINE][VLAYER] : %s\n", buffer);
#elif __unix__
                vsprintf(buffer, format, argList);
                sprintf(tempbuffer, "[ENGINE][VLAYER] : %s\n", buffer);
#endif
                va_end(argList);

                std::cout << termcolor::magenta << tempbuffer << termcolor::reset;
        }

        void Debug::TempFile(std::string data, const char* path)
        {
            try
            {
                std::ofstream tempFile(path, std::ios::out | std::ios::app);

                if (!tempFile.is_open())
                {
                    std::cerr << "Impossible d'ouvrir le fichier temporaire pour ecriture." << std::endl;
                    return;
                }

                tempFile << data;
                tempFile.close();

                std::cout << "Fichier temporaire cree avec succes a : " << path << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cerr << "Erreur lors de la cr�ation du fichier temporaire : " << e.what() << std::endl;
            }
        }
}