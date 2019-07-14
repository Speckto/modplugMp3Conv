#ifndef __MP3C_ENCODER_FACTORY_H__
#define __MP3C_ENCODER_FACTORY_H__

#include <string>
#include <memory>
#include "MP3Encoder.h"
using std::string;

class CMP3EncoderFactory;

/**
 * @class CMP3EncoderFactory
 * @title Singleton to create MP3 encoders
 *
 */
class CMP3EncoderFactory
{
public:
    static CMP3EncoderFactory * Instance();
    CMP3Encoder * GetEncoder();
    void DestroyEncoder();

    ~CMP3EncoderFactory();
private:
    CMP3EncoderFactory();
    CMP3EncoderFactory(const CMP3EncoderFactory &);
    CMP3EncoderFactory& operator= (const CMP3EncoderFactory&);
    static CMP3EncoderFactory * pinstance;

    CMP3Encoder * encoder;
};

#endif
