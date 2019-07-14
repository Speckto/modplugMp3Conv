#ifndef _MODCVT_SOUND_FILE_H
#define _MODCVT_SOUND_FILE_H

#include <string>
#include <iostream>

# include <inttypes.h>
//#include <stdint.h>

class CSoundFileWrapData;

/**
 * @CSoundFileWrapper
 * @title Wrapper class for CSoundFile
 *
 * This class acts as a wrapper to CSoundFile removing the need
 * to include the libModPlug header files into our own header files.
 *
 * This prevents naming conflicts as libModPlug has nasty hacks for various
 * types that are needed on different platforms. We don't really care about
 * them, and incuding 'stdafx' needed by libModPlug can cause conflicts with
 * say the BladeEnc headers or windows.h !
 *
 * Wrapping like this sucks (why implement your own interface to a library.
 * but I can't see any easy way round this.
 *
 */

class CSoundFileWrapper
{
public:
    static void Initialise();
    
    CSoundFileWrapper();
    ~CSoundFileWrapper();
    bool Create(std::istream & fileStream,
                std::string & errorMessage);

    static uint32_t gnBitsPerSample();
    static uint32_t gnChannels();
    uint32_t GetCurrentPos();
    uint32_t GetMaxPosition();
    uint32_t Read(void * audioBuffer,
                  const uint32_t bufferSize);
    void SetCurrentPos(uint32_t position);
    void SetRepeatCount(uint32_t repeatCount);
    void ResetChannels();

    uint32_t m_nInstruments();
    uint32_t m_nSamples();
    const char * GetTitle();
    uint32_t GetInstrumentName (const uint32_t nInstr,
                                char * s);

    uint32_t GetSampleName (const uint32_t nInstr,
                            char * s);

private:
    CSoundFileWrapData * data;

};

#endif
