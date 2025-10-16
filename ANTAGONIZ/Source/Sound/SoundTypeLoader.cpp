#include "SoundTypeLoader.hpp"
#include <cstring>
#include <vector>

namespace Ge
{
    double CalculateDuration(uint64_t data_size, uint32_t sampleRate, int32_t bitsPerSample, int32_t numChannels) 
    {
        // Calculer la durée du fichier WAV en secondes
        double duration = static_cast<double>(data_size) / (numChannels * (bitsPerSample / 8) * sampleRate);
        return duration;
    }

    int8_t* SoundTypeLoader::LoadWavFormat(const char* filePath, uint64_t* size, uint32_t* frequency, int32_t* format, double* time)
    {
        RIFF_Header riffHeader{};
        WAVE_Format waveFormat{};
        int8_t* buffer = nullptr;

        FILE* wavFile = fopen(filePath, "rb");
        if (!wavFile)
        {
            Debug::Error("Unable to open wave file: %s", filePath);
            return nullptr;
        }

        // ---- Lire l'entête RIFF ----
        fread(&riffHeader, 1, sizeof(RIFF_Header), wavFile);
        if (strncmp((char*)riffHeader.chunkID, "RIFF", 4) != 0 || strncmp((char*)riffHeader.format, "WAVE", 4) != 0)
        {
            Debug::Error("Invalid .wav file");
            fclose(wavFile);
            return nullptr;
        }

        // ---- Lecture du chunk "fmt " ----
        fread(&waveFormat, sizeof(uint8_t), 4, wavFile); // subChunkID
        if (strncmp((char*)waveFormat.subChunkID, "fmt ", 4) != 0)
        {
            Debug::Error("Invalid fmt chunk in %s", filePath);
            fclose(wavFile);
            return nullptr;
        }

        // Lire la taille du chunk fmt
        fread(&waveFormat.subChunkSize, sizeof(uint32_t), 1, wavFile);

        // Lire le reste du chunk dans un buffer temporaire
        std::vector<uint8_t> fmtData(waveFormat.subChunkSize);
        fread(fmtData.data(), 1, waveFormat.subChunkSize, wavFile);

        // Extraire les champs de base
        memcpy(&waveFormat.audioFormat, fmtData.data(), sizeof(uint16_t));
        memcpy(&waveFormat.numChannels, fmtData.data() + 2, sizeof(uint16_t));
        memcpy(&waveFormat.sampleRate, fmtData.data() + 4, sizeof(uint32_t));
        memcpy(&waveFormat.byteRate, fmtData.data() + 8, sizeof(uint32_t));
        memcpy(&waveFormat.blockAlign, fmtData.data() + 12, sizeof(uint16_t));
        memcpy(&waveFormat.bitsPerSample, fmtData.data() + 14, sizeof(uint16_t));

        // ---- Sauter jusqu'au chunk "data" ----
        char subChunkID[5] = { 0 };
        uint32_t chunkSize = 0;

        while (true)
        {
            if (fread(subChunkID, 1, 4, wavFile) != 4)
            {
                Debug::Error("Reached EOF before finding 'data' chunk: %s", filePath);
                fclose(wavFile);
                return nullptr;
            }

            if (fread(&chunkSize, sizeof(uint32_t), 1, wavFile) != 1)
            {
                Debug::Error("Unexpected EOF when reading chunk size: %s", filePath);
                fclose(wavFile);
                return nullptr;
            }

            if (strncmp(subChunkID, "data", 4) == 0)
            {
                break;
            }

            fseek(wavFile, chunkSize, SEEK_CUR);
            if (chunkSize % 2 == 1)
            {
                fseek(wavFile, 1, SEEK_CUR);
            }
        }

        uint32_t data_size = chunkSize;
        buffer = new int8_t[data_size];
        fread(buffer, 1, data_size, wavFile);

        *size = data_size;
        *frequency = waveFormat.sampleRate;

        if (waveFormat.bitsPerSample == 8)
        {
            *format = (waveFormat.numChannels == 1) ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
        }
        else if (waveFormat.bitsPerSample == 16)
        {
            *format = (waveFormat.numChannels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        }
        else
        {
            Debug::Warn("Unsupported WAV bit depth: %d bits per sample in %s",waveFormat.bitsPerSample, filePath);
            fclose(wavFile);
            delete[] buffer;
            return nullptr;
        }
        *time = CalculateDuration(data_size, waveFormat.sampleRate, waveFormat.bitsPerSample, waveFormat.numChannels);

        fclose(wavFile);
        return buffer;
    }


    int SoundTypeLoader::getFileSize(FILE* inFile)
    {
        int fileSize = 0;
        fseek(inFile, 0, SEEK_END);

        fileSize = ftell(inFile);

        fseek(inFile, 0, SEEK_SET);
        return fileSize;
    }
}
