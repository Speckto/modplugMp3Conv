#if MP2MP3_USE_BLADE_ENC_DLL > 0

#include <MP3EncoderWindows.h>
#include <iostream>
using std::endl;
using std::flush;
using std::cout;

//
// -------------------------------------------------------------------
//

CWinEncoderLibLoader * CWinEncoderLibLoader::pinstance = NULL;

//
// -------------------------------------------------------------------
//

CWinEncoderLibLoader * CWinEncoderLibLoader::Instance()
{
    if (pinstance == NULL)
    {
        pinstance = new CWinEncoderLibLoader();
    }

    return pinstance;
}

string CWinEncoderLibLoader::GetLibraryName()
{
    return "lame_enc.dll";
}

CWinEncoderLibLoader::CWinEncoderLibLoader()
    : lameLoaded(false),
      hLameDll(NULL),
      beInitStream(NULL),
      beEncodeChunk(NULL),
      beDeinitStream(NULL),
      beCloseStream(NULL),
      beVersion(NULL),
      beWriteVBRHeader(NULL)
{
}

CWinEncoderLibLoader::~CWinEncoderLibLoader()
{
    FreeLameDll();
}

void CWinEncoderLibLoader::FreeLameDll()
{
    if (hLameDll != NULL && lameLoaded)
    {
        FreeLibrary(hLameDll);

        lameLoaded       = false;
        hLameDll         = NULL;
        beInitStream     = NULL;
        beEncodeChunk    = NULL;
        beDeinitStream   = NULL;
        beCloseStream    = NULL;
        beVersion        = NULL;
        beWriteVBRHeader = NULL;
    }
}

bool CWinEncoderLibLoader::LameLoaded()
{
    return  (hLameDll != NULL && lameLoaded);
}

bool CWinEncoderLibLoader::LoadLameDll(std::string & errorMessage)
{
    bool libOpenedOk = true;
    if (hLameDll == NULL) // Don't open if already open
    {
        hLameDll=LoadLibrary(GetLibraryName().c_str());
        if(hLameDll==NULL)
        {
            libOpenedOk = false;
            errorMessage = "Error: Could not locate a valid installation of '";
            errorMessage += GetLibraryName();
            errorMessage += "'. Ensure LAME MP3 encoder is installed.";
        }
        else
        {
            beInitStream = (BEINITSTREAM) GetProcAddress(
                hLameDll, TEXT_BEINITSTREAM);
            beEncodeChunk = (BEENCODECHUNK) GetProcAddress(
                hLameDll, TEXT_BEENCODECHUNK);
            beDeinitStream = (BEDEINITSTREAM) GetProcAddress(
                hLameDll, TEXT_BEDEINITSTREAM);
            beCloseStream = (BECLOSESTREAM) GetProcAddress(
                hLameDll, TEXT_BECLOSESTREAM);
            beVersion = (BEVERSION) GetProcAddress(
                hLameDll, TEXT_BEVERSION);
            beWriteVBRHeader = (BEWRITEVBRHEADER) GetProcAddress(
                hLameDll,TEXT_BEWRITEVBRHEADER);

            //
            // Check if all interfaces are present
            //
            if(!beInitStream   ||
               !beEncodeChunk  ||
               !beDeinitStream ||
               !beCloseStream  ||
               !beVersion      ||
               !beWriteVBRHeader)
            {
                // Not valid so close it
                FreeLibrary(hLameDll);
                libOpenedOk = false;
                errorMessage = "Required lame interfaces not found";
            }
            else
            {
                // Get the version number, and display (legal requirement)
                libOpenedOk = true;
            }
        }
        lameLoaded = libOpenedOk;
    }

    return libOpenedOk;
}


//
// -------------------------------------------------------------------
// 

CWindowsMP3Encoder::CWindowsMP3Encoder()
    : encodeInProgress(false),
      lameBufferSize(0),
      lameStreamHandle(0)
{
    memset(&beConfig,
           0,
           sizeof(beConfig));
    memset(&Version,
           0,
           sizeof(Version));


    lame = CWinEncoderLibLoader::Instance();
}

CWindowsMP3Encoder::~CWindowsMP3Encoder()
{
}

string CWindowsMP3Encoder::GetLibraryPath()
{
    return "";
}

string CWindowsMP3Encoder::GetLibraryName()
{
    return lame->GetLibraryName();
}

string CWindowsMP3Encoder::GetLibraryMessage()
{
    std::string text;
    text = GetLibraryName();
    text += ":LAME MP4 encoding Library";
    text += "(Version ";
    text += GetLibraryVersion();
        
    if (Version.byMMXEnabled)
    {
        text += " MMX)";
    }
    else
    {
        text += ")";
    }
    text += " http://www.mp3dev.org";
    return text;
}

string CWindowsMP3Encoder::GetLibraryVersion()
{
    std::string text;
    text = Version.byDLLMajorVersion;
    text += ".";
    text += Version.byDLLMinorVersion;
    return text;
}


bool CWindowsMP3Encoder::LoadEncoderLibrary(std::string & errorMessage)
{
    bool loaded = lame->LoadLameDll(errorMessage);

    if (loaded)
    {
        lame->beVersion(&Version);
    }

    return loaded;
}

bool CWindowsMP3Encoder::ValidLibraryLoaded()
{
    return lame->LameLoaded();
}

bool CWindowsMP3Encoder::InitializeStream(
    const int bitRate,
    unsigned long & samplesPerChunk,
    std::string & errorMessage)
{
    BE_ERR lameError;
    bool okSoFar = true;

    if (encodeInProgress)
    {
        errorMessage = "Encode already in progress";
        okSoFar = false;
    }
    memset(&beConfig,
           0,
           sizeof(beConfig));
    // Now setup what we want
    beConfig.dwConfig = BE_CONFIG_LAME;
	beConfig.format.LHV1.dwStructVersion = 1;
	beConfig.format.LHV1.dwStructSize    = sizeof(beConfig);
	beConfig.format.LHV1.dwSampleRate    = 44100;
    beConfig.format.LHV1.dwReSampleRate  = 0; // automatic
    beConfig.format.LHV1.nMode           = BE_MP3_MODE_STEREO;
    beConfig.format.LHV1.dwBitrate       = bitRate;
    beConfig.format.LHV1.nVbrMethod      = VBR_METHOD_NONE;
    beConfig.format.LHV1.bEnableVBR      = FALSE;
    beConfig.format.LHV1.nQuality        = 0xFC03;
    beConfig.format.LHV1.bNoRes          = TRUE;

    lameError = lame->beInitStream(
        &beConfig,
        static_cast<DWORD *> (&samplesPerChunk),
        &lameBufferSize,
        &lameStreamHandle);

    if (lameError != BE_ERR_SUCCESSFUL)
    {
        okSoFar = false;
        errorMessage += "Error Initialising LAME stream :";
        errorMessage += LameMp3ErrToStr(lameError);
    }
    else
    {
        encodeInProgress = true;
    }

    return okSoFar;
}

int CWindowsMP3Encoder::GetOutBufferSize()
{
    return lameBufferSize;
}

// int CWindowsMP3Encoder::EncodeBuffer(short int inbuffer[],
//                                      unsigned char outbuffer[],
//                                      std::string & errorMessage)
// {
//     BE_ERR     lameError;
//     int encodedBytes = -1;
//     if (encodeInProgress)
//     {
//         lameError = lame->beEncodeChunk(lameStreamHandle,
//                                         mInSampleNum,
//                                         inbuffer,
//                                         outbuffer,
//                                         &encodedBytes);

//         if (lameError != BE_ERR_SUCCESSFUL)
//         {
//             encodedBytes = -1;
//             errorMessage += "Error Encoding chunk LAME stream :";
//             errorMessage += LameMp3ErrToStr(lameError);
//         }       
//     }
//     else
//     {
//         errorMessage = "No encode in progress";
//     }

//     return encodedBytes;
// }

bool CWindowsMP3Encoder::EncodeBuffer(
    short int inbuffer[],
    unsigned int nSamples,
    unsigned char outbuffer[],
    unsigned long & encodedBytes,
    std::string & errorMessage)
{
    bool encodedOk = true;
    BE_ERR     lameError;
    encodedBytes = 0;
    if (encodeInProgress)
    {
        lameError = lame->beEncodeChunk(lameStreamHandle,
                                        nSamples,
                                        inbuffer,
                                        outbuffer,
                                        &encodedBytes);

        if (lameError != BE_ERR_SUCCESSFUL)
        {
            encodedOk    = false;
            encodedBytes = 0;
            errorMessage += "Error Encoding chunk LAME stream :";
            errorMessage += LameMp3ErrToStr(lameError);
        }       
    }
    else
    {
        encodedOk    = false;
        errorMessage = "No encode in progress";
    }

    return encodedOk;
}


bool CWindowsMP3Encoder::FinishStream(
    unsigned char outbuffer[],
    unsigned long & encodedBytes,
    std::string & errorMessage)
{
    bool encodedOk = true;
    BE_ERR     lameError;
    encodedBytes = 0;
    if (encodeInProgress)
    {
      lameError = lame->beDeinitStream(lameStreamHandle,
                                       outbuffer,
                                       &encodedBytes);
        if (lameError != BE_ERR_SUCCESSFUL)
        {
            encodedOk = false;
            encodedBytes = 0;
            errorMessage += "Error de-initialising LAME stream :";
            errorMessage += LameMp3ErrToStr(lameError);
        }
        else
        {
            lame->beCloseStream(lameStreamHandle);
            if (lameError != BE_ERR_SUCCESSFUL)
            {
                encodedOk = false;
                encodedBytes = 0;
                errorMessage += "Error closing LAME stream :";
                errorMessage += LameMp3ErrToStr(lameError);
            }
        }

        encodeInProgress = false;
    }
    else
    {
        encodedOk = false;
        errorMessage = "No encode in progress";
    }

    return encodedOk;
}

void CWindowsMP3Encoder::CancelEncoding()
{
    if (encodeInProgress)
    {
        lame->beCloseStream(lameStreamHandle);
        encodeInProgress = false;
    }
}

std::string CWindowsMP3Encoder::LameMp3ErrToStr(BE_ERR e)
{
    std::string err;
    switch (e)
    {
    case BE_ERR_SUCCESSFUL:
        err = "BE_ERR_SUCCESSFUL";
        break;
    case BE_ERR_INVALID_FORMAT:
        err = "BE_ERR_INVALID_FORMAT";
        break;
    case BE_ERR_INVALID_FORMAT_PARAMETERS:
        err = "BE_ERR_INVALID_FORMAT_PARAMETERS";
        break;
    case BE_ERR_NO_MORE_HANDLES:
        err = "BE_ERR_NO_MORE_HANDLES";
        break;
    case BE_ERR_INVALID_HANDLE:
        err = "BE_ERR_INVALID_HANDLE";
        break;
    case BE_ERR_BUFFER_TOO_SMALL  :
        err = "BE_ERR_BUFFER_TOO_SMALL";
        break;
    default:
        err = "(unknown)";
    }
    return err;
}

#endif
