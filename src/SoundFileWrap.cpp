
#include <SoundFileWrap.h>

// libModplug stuff
#include <stdafx.h>
#include <modplug.h>
#include <sndfile.h>

// Other
//#ifndef __WIN32__
//#include <dlfcn.h>
//#endif

#include <iostream>
#include <memory>

#include <cassert>

using std::cout;
using std::endl;
using std::flush;

//
// ------------------------------------------------------------------
//
class CSoundFileWrapData
{
public:
    CSoundFileWrapData() : modFileDataSize(0) { };
public: // data
    std::auto_ptr<CSoundFile> modFile;
    std::auto_ptr<char> modFileData;
    uint32_t modFileDataSize;
};

//
// ------------------------------------------------------------------
//

void CSoundFileWrapper::Initialise()
{
    CSoundFile::SetXBassParameters(0, 0);
    CSoundFile::SetWaveConfig(44100, //Note: tally these with WAV, LAME
                              16,
                              2);
    
    // gSampleSize = gSettings.mBits / 8 * gSettings.mChannels;
    CSoundFile::SetWaveConfigEx(false, // surround
                                false , // !oversampling
                                false, // reverb
                                true,  // HqIO
                                false, // megabase
                                true,  // Noise Reduction
                                false); // Equaliser

    CSoundFile::SetAGC(false);
    CSoundFile::SetResamplingMode(SRCMODE_POLYPHASE);
    CSoundFile::gdwSoundSetup |= SNDMIX_DIRECTTODISK;
    CSoundFile::gdwSoundSetup |= SNDMIX_NOBACKWARDJUMPS;
    CSoundFile::gdwMixingFreq   = 44100;
    CSoundFile::gnBitsPerSample = 16;
    CSoundFile::gnChannels      = 2;
    CSoundFile::InitPlayer(true);
}

CSoundFileWrapper::CSoundFileWrapper()
{
    data = new CSoundFileWrapData();
}

CSoundFileWrapper::~CSoundFileWrapper()
{
    delete data;
}

bool CSoundFileWrapper::Create(std::istream & fileStream,
                               std::string & errorMessage)
{
    bool loadedOk = true;

    fileStream.seekg(0, std::ios::end);
    data->modFileDataSize = fileStream.tellg();
    // Allocate memory for buffer
    data->modFileData.reset(new char[data->modFileDataSize]);


    fileStream.seekg(0, std::ios::beg);
    fileStream.read (data->modFileData.get(),
                     data->modFileDataSize);

    data->modFile.reset(new CSoundFile());

    if (! (data->modFile->Create(
               reinterpret_cast<BYTE *> (data->modFileData.get()),
               data->modFileDataSize)))
    {
        loadedOk = false;
        errorMessage = "Not a valid/supported module file";
    }

    return loadedOk;
}
    
uint32_t CSoundFileWrapper::gnBitsPerSample()
{
    return CSoundFile::gnBitsPerSample;
}

uint32_t CSoundFileWrapper::gnChannels()
{
    return CSoundFile::gnChannels;
}

uint32_t CSoundFileWrapper::GetCurrentPos()
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->GetCurrentPos();
}

uint32_t CSoundFileWrapper::GetMaxPosition()
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->GetMaxPosition();
}

uint32_t CSoundFileWrapper::Read(
    void * audioBuffer,
    const uint32_t bufferSize)
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->Read(audioBuffer, bufferSize);
}

void CSoundFileWrapper::SetCurrentPos(uint32_t position)
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->SetCurrentPos(position);
}

void CSoundFileWrapper::SetRepeatCount(uint32_t repeatCount)
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->SetRepeatCount(repeatCount);
}

void CSoundFileWrapper::ResetChannels()
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->ResetChannels();
}

uint32_t CSoundFileWrapper::m_nInstruments()
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->m_nInstruments;
}

uint32_t CSoundFileWrapper::m_nSamples()
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->m_nSamples;
}

const char * CSoundFileWrapper::GetTitle()
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->GetTitle();
}

uint32_t CSoundFileWrapper::GetInstrumentName (
    const uint32_t nInstr,
    char * s)
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->GetInstrumentName(nInstr, s);
}

uint32_t CSoundFileWrapper::GetSampleName (
    const uint32_t nInstr,
    char * s)
{
    assert(data != NULL && "data is NULL");
    assert(data->modFile.get() != NULL && "data.modFile is NULL");
    return data->modFile->GetSampleName(nInstr, s);
}
