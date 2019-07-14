#if MP2MP3_USE_BLADE_ENC_DLL > 0

#ifndef __MP3C_ENCODER_WINDOWS_H__
#define __MP3C_ENCODER_WINDOWS_H__

#include <MP3Encoder.h>
#include <windows.h>
#include "BladeMP3EncDLL.h"
#include <string>

using std::string;
class CWinEncoderLibLoader;

class CWindowsMP3Encoder : public CMP3Encoder
{
public:
    CWindowsMP3Encoder();
    virtual ~CWindowsMP3Encoder();

    virtual string GetLibraryPath();
    virtual string GetLibraryName();
    virtual string GetLibraryMessage();
    virtual string GetLibraryVersion();

    virtual bool LoadEncoderLibrary(std::string & errorMessage);
    virtual bool ValidLibraryLoaded();

    virtual bool InitializeStream(const int bitRate,
                                  unsigned long & samplesPerChunk,
                                  std::string & errorMessage);

    /**
     * @return In bytes. must be called AFTER InitializeStream
     */
    virtual int GetOutBufferSize();
    /**
     * input is interleaved if stereo
     * @return the number of bytes written
     *         -1 if an error occured
     */
//    virtual int EncodeBuffer(short int inbuffer[],
//                             unsigned char outbuffer[],
//                             std::string & errorMessage);
    virtual bool EncodeBuffer(short int inbuffer[],
                              unsigned int nSamples,
                              unsigned char outbuffer[],
                              unsigned long & encodedBytes,
                              std::string & errorMessage);

    virtual bool FinishStream(unsigned char outbuffer[],
                              unsigned long & encodedBytes,
                              std::string & errorMessage);
    virtual void CancelEncoding();

private: // methods
    std::string LameMp3ErrToStr(BE_ERR e);

private: //members
    CWinEncoderLibLoader * lame;
    bool                   encodeInProgress;
    unsigned long          lameBufferSize;
    HBE_STREAM             lameStreamHandle;
	BE_CONFIG              beConfig;
    BE_VERSION             Version;
};

/**
 * @class CWinEncoderLibLoader
 * @title Lame library loading singleton
 *
 * Loads a sinlge instance of the lame DLL for use.
 */
class CWinEncoderLibLoader
{
public:
    static CWinEncoderLibLoader * Instance();
    bool LoadLameDll(std::string & errorMessage);
    bool LameLoaded();
    string GetLibraryName();
public:
    bool             lameLoaded;
    HINSTANCE        hLameDll;
    BEINITSTREAM     beInitStream;
	BEENCODECHUNK    beEncodeChunk;
	BEDEINITSTREAM   beDeinitStream;
	BECLOSESTREAM    beCloseStream;
	BEVERSION        beVersion;
	BEWRITEVBRHEADER beWriteVBRHeader;
private:
    static CWinEncoderLibLoader * pinstance;

    void FreeLameDll();
    CWinEncoderLibLoader();
    ~CWinEncoderLibLoader();
    CWinEncoderLibLoader(const CWinEncoderLibLoader&);
    CWinEncoderLibLoader& operator= (const CWinEncoderLibLoader&);
};

#endif

#endif
