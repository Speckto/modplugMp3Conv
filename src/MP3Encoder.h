#ifndef __MP3C_ENCODER_H__
#define __MP3C_ENCODER_H__

#include <string>
#include <memory>
using std::string;

class CMP3Encoder;

/**
 * @class CMP3Encoder
 * @title Abstract MP3 exporter base class
 *
 * This can dynamically load either a linux or windows SO/DLL of Lame
 * or bladeenc.dll and use it to process an audio file
 */
class CMP3Encoder {
public:
    CMP3Encoder() { };
    virtual ~CMP3Encoder() { };

    virtual string GetLibraryPath() = 0;
    virtual string GetLibraryName() = 0;
    virtual string GetLibraryMessage() = 0;
    virtual string GetLibraryVersion() = 0;

    virtual bool LoadEncoderLibrary(std::string & errorMessage) = 0;
    virtual bool ValidLibraryLoaded() = 0;

    /**
     * @returns the number of samples to send for each call to
     *           EncodeBuffer
     */
    virtual bool InitializeStream(const int bitRate,
                                  unsigned long & samplesPerChunk,
                                  std::string & errorMessage) = 0;

    /**
     * @return In bytes. must be called AFTER InitializeStream
     */
    virtual int GetOutBufferSize() = 0;

//    virtual int EncodeBuffer(short int inbuffer[],
//                             unsigned char outbuffer[],
//                             std::string & errorMessage) = 0;

    /* returns the number of bytes written. input is interleaved if stereo*/
    virtual bool EncodeBuffer(short int inbuffer[],
                             unsigned int nSamples,
                             unsigned char outbuffer[],
                             unsigned long & encodedBytes,
                             std::string & errorMessage) = 0;

    virtual bool FinishStream(unsigned char outbuffer[],
                             unsigned long & encodedBytes,
                             std::string & errorMessage) = 0;

    virtual void CancelEncoding() = 0;
};

#endif
