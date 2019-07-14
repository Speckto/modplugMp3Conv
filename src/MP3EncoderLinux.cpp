#include <MP3EncoderLinux.h>
#include <iostream>
using std::endl;
using std::flush;
using std::cout;

//
// -------------------------------------------------------------------
// 

void lame_errorf(const char *format, va_list ap)
{
    (void) vfprintf(stdout, format, ap);
}

void lame_debugf(const char *format, va_list ap)
{
    (void) vfprintf(stdout, format, ap);
}

void lame_msgf(const char *format, va_list ap)
{
    (void) vfprintf(stdout, format, ap);
}

CLinuxMP3Encoder::CLinuxMP3Encoder()
    : encodeInProgress(false)
{
}

CLinuxMP3Encoder::~CLinuxMP3Encoder()
{
}

string CLinuxMP3Encoder::GetLibraryPath()
{
    return "";
}

string CLinuxMP3Encoder::GetLibraryName()
{
    return "LAME MP3 Encoding Library";
}

string CLinuxMP3Encoder::GetLibraryMessage()
{
    std::string text;
    text =  "LAME MP3 encoding Library";
    text += "(Version ";
    text += GetLibraryVersion();
        
    text += get_lame_url();
    return text;
}

string CLinuxMP3Encoder::GetLibraryVersion()
{
    return get_lame_version();
}


bool CLinuxMP3Encoder::LoadEncoderLibrary(std::string & errorMessage)
{
    return true;
}

bool CLinuxMP3Encoder::ValidLibraryLoaded()
{
    return true;
}

bool CLinuxMP3Encoder::InitializeStream(
    const int bitRate,
    unsigned long & samplesPerChunk,
    std::string & errorMessage)
{
    // TODO: Set error callback routines for lame
    bool okSoFar = true;
    errorMessage = "";

    lameGlobalFlags = lame_init();
    lame_set_brate(lameGlobalFlags, bitRate);
    lame_set_quality(lameGlobalFlags, 2); // near-best quality
    lame_set_bWriteVbrTag(lameGlobalFlags, 0);
    lame_set_errorf(lameGlobalFlags, lame_errorf);
    lame_set_msgf(lameGlobalFlags, lame_msgf);
    lame_set_debugf(lameGlobalFlags, lame_debugf);

    if (lame_init_params(lameGlobalFlags) < 0)
    {
        okSoFar = false;
        errorMessage = "Failed to initialise lame encoding parameters";
    }
    else
    {
        //lame_print_config(lameGlobalFlags);
        // 2 channels (stereo) so double samples per channel
        samplesPerChunk = mSamplesPerChannelInChunk*2;
        encodeInProgress = true;

    }

    return okSoFar;
}

int CLinuxMP3Encoder::GetOutBufferSize()
{
    return mOutBufferSize;
}

bool CLinuxMP3Encoder::EncodeBuffer(
    short int inbuffer[],
    unsigned int nSamples,
    unsigned char outbuffer[],
    unsigned long & encodedBytes,
    std::string & errorMessage)
{
    bool encodedOk = true;
    // Convert total sample count to the number for each channel
    int samplesPerChannel = static_cast<int> (nSamples/2);
    encodedBytes = 0;

    if (encodeInProgress)
    {
        int encodeRet = lame_encode_buffer_interleaved(lameGlobalFlags,
                                                       inbuffer,
                                                       samplesPerChannel,
                                                       outbuffer,
                                                       mOutBufferSize);

        if (encodeRet >= 0)
        {
            encodedOk = true;
            encodedBytes = encodeRet;
        }
        else if (encodeRet == -1)
        {
            encodedOk = false;
            errorMessage = "Output buffer too small";
        }
        else if (encodeRet == -2)
        {
            encodedOk = false;
            errorMessage = "Unable to allocated memory";
        }
        else if (encodeRet == -3)
        {
            encodedOk = false;
            errorMessage = "lame was not previously initialised";
        }
        else if (encodeRet == -4)
        {
            encodedOk = false;
            errorMessage = "Psycho acoustic problems in lame";
        }
        else
        {
            encodedOk = false;
            errorMessage = "Unknown lame error: ";
            errorMessage += std::to_string(encodeRet);
        }
    }
    else
    {
        encodedOk    = false;
        errorMessage = "No encode in progress";
    }

    return encodedOk;
}


bool CLinuxMP3Encoder::FinishStream(
    unsigned char outbuffer[],
    unsigned long & encodedBytes,
    std::string & errorMessage)
{
    bool encodedOk = true;
    encodedBytes = 0;
    if (encodeInProgress)
    {
        int lameRet = lame_encode_flush(lameGlobalFlags,
                                        outbuffer,
                                        mOutBufferSize);
        if (lameRet >= 0)
        {
            encodedBytes = lameRet;
            lame_mp3_tags_fid(lameGlobalFlags, NULL);

            lame_close(lameGlobalFlags);
            encodeInProgress = false;
        }
        else
        {
            encodedOk = false;
            errorMessage = "Unknown lame error (flush)";
        }
    }
    else
    {
        encodedOk = false;
        errorMessage = "No encode in progress";
    }

    return encodedOk;
}

void CLinuxMP3Encoder::CancelEncoding()
{
    if (encodeInProgress)
    {
        lame_close(lameGlobalFlags);
        encodeInProgress = false;
    }
}

