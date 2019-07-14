#include <MP3Encoder.h>
#include <string>
#include <lame/lame.h>

using std::string;
class CLinEncoderLibLoader;

class CLinuxMP3Encoder : public CMP3Encoder{
public:
    CLinuxMP3Encoder();
    virtual ~CLinuxMP3Encoder();

    virtual string GetLibraryPath() override;
    virtual string GetLibraryName() override;
    virtual string GetLibraryMessage() override;
    virtual string GetLibraryVersion() override;

    virtual bool LoadEncoderLibrary(std::string & errorMessage) override;
    virtual bool ValidLibraryLoaded() override;

    virtual bool InitializeStream(const int bitRate,
                                  unsigned long & samplesPerChunk,
                                  std::string & errorMessage) override;

    /**
     * @return In bytes. must be called AFTER InitializeStream
     */
    virtual int GetOutBufferSize() override;
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
                              std::string & errorMessage) override;

    virtual bool FinishStream(unsigned char outbuffer[],
                              unsigned long & encodedBytes,
                              std::string & errorMessage) override;
    virtual void CancelEncoding() override;

private: // methods
    // Number of samples for each channel in a single encoding chunk
    // *2 to get total number of channels for a stereo sample
    static const int mSamplesPerChannelInChunk = 220500;
    static const int mOutBufferSize = 
                                   int(1.25 * mSamplesPerChannelInChunk + 7200);

private: //members
    bool                   encodeInProgress;
    lame_global_flags *    lameGlobalFlags;   
};

