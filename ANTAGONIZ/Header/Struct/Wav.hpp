#ifndef __WAV__
#define __WAV__

#include <iostream>

struct RIFF_Header 
{
    uint8_t chunkID[4];
    uint32_t chunkSize;
    uint8_t format[4];
};

#pragma pack(push, 1) // Pour d�sactiver l'alignement m�moire automatique

struct BEXT_Chunk
{
    char description[256];         // Description ou titre du contenu
    char originator[32];           // Nom de la source (ex: ing�nieur son)
    char originatorReference[32];  // R�f�rence d�identification unique
    char originationDate[10];      // Date de cr�ation: "YYYY-MM-DD"
    char originationTime[8];       // Heure de cr�ation: "HH:MM:SS"
    uint64_t timeReferenceLow;     // R�f�rence temporelle (low bits)
    uint64_t timeReferenceHigh;    // R�f�rence temporelle (high bits)
    uint16_t version;              // Version du chunk bext (ex: 1)
    char umid[64];                 // UMID (Unique Material Identifier), optionnel
    uint8_t reserved[190];         // R�serv� pour usage futur
    uint8_t codingHistory[1];      // Champ de texte extensible (null-terminated)
};
#pragma pack(pop)

struct WAVE_Format 
{
    uint8_t subChunkID[4];
    uint32_t subChunkSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

#endif //!__WAV__