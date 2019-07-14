
#include <cassert>
#include <MP3EncoderFactory.h>
#include <config.h>

#if MP2MP3_USE_BLADE_ENC_DLL > 0
#include <MP3EncoderWindows.h>
#else
#include <MP3EncoderLinux.h>
#endif

//
// ---------------------------------------------------------------------
// FACTORY
// ---------------------------------------------------------------------
//
CMP3EncoderFactory* CMP3EncoderFactory::pinstance = NULL;

CMP3EncoderFactory * CMP3EncoderFactory::Instance()
{
    if (pinstance == NULL)
    {
        pinstance = new CMP3EncoderFactory();
    }
    assert(pinstance!=NULL && "Factory Instance is NULL");
    return pinstance;
}


CMP3EncoderFactory::CMP3EncoderFactory()
    : encoder(NULL)
{
}

CMP3EncoderFactory::~CMP3EncoderFactory()
{
    delete encoder;
}

CMP3Encoder * CMP3EncoderFactory::GetEncoder()
{
#if MP2MP3_USE_BLADE_ENC_DLL > 0
    if (encoder == NULL)
    {
        encoder = new CWindowsMP3Encoder();
    }
#else
    if (encoder == NULL)
    {
        encoder = new CLinuxMP3Encoder();
    }
#endif

    assert(encoder!=NULL && "GetEncoder returning NULL");
    return encoder;
}

void CMP3EncoderFactory::DestroyEncoder()
{
    delete encoder;
    encoder = NULL;;
}

