
#ifndef _MODCVT_HEADER_INCLUDED
#define _MODCVT_HEADER_INCLUDED

//#include <config.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

// libModplug stuff
//#include <stdafx.h>
//#include <modplug.h>
//#include <sndfile.h>

// MP3 conversion
#include <tag.h>

// Our own headers
#include <MpToMp3Common.h>
#include <FileNameParser.h>
#include <MP3EncoderFactory.h>
#include <SoundFileWrap.h>



using namespace std;

/**
 * @class CModuleCvt
 * @brief Module Convertor
 * @description  Coverts MOD files to WAV/MP3 using libModPlug and lame
 *
 * Converts MOD files to WAV or MP3 using the ModPlug library.
 * Lame is loaded and used for converting MP3s
 * TagLib is used to write out ID tag informations
 */
class CModuleCvt {
public:

    class CConversionListener;
    /**
     * @class CModuleCvt::CConversionListener
     * @brief Module conversion listener class
     *
     * OnConversionStarted is always invoked immediately after
     * one of the Convert* methods is called
     */
    class CConversionListener
    {
    public:
        CConversionListener();
        virtual ~CConversionListener();

        typedef enum {
            CS_CONVERTING = 1,
            CS_WRITE_TAGS = 1
        } TConversionStatus;
/*        static const char * ConversionStatusStr(
            CModuleCvt::CConversionListener::TConversionStatus status)
        {
            const char * str = NULL;
            switch (status)
            {
            CS_CONVERTING:
                str = "converting";
                break;
            CS_WRITE_TAGS:
                str = "writing tags";
                break;
            default:
                str = "";
                break;
            }

            return str;
        }*/

        /**
         * @method OnConversionStarted
         * @brief Invoked whenever conversion is requested
         */
        virtual void OnConversionStarted(const std::string &sourceModuleFile,
                                         const CFileName &destFile,
                                         bool  convertingToMp3) = 0;

        virtual void OnConversionStatus(const  std::string &sourceModuleFile,
                                        const  CFileName &destFile,
                                        bool   convertingToMp3,
                                        CModuleCvt::CConversionListener::TConversionStatus status,
                                        double percentDone,
                                        const std::string & message) = 0;

        virtual void OnConversionComplete(const  std::string &sourceModuleFile,
                                          const  CFileName &destFile,
                                          bool   convertingToMp3) = 0;
    };

    CModuleCvt(CConversionListener & listener);
    ~CModuleCvt();

    void RegisterListener(CConversionListener & listener);
    //void RemoveListener(CConversionListener & listener);

    void ConvertModuleToWav(std::string sourceModuleFile,
                            const CFileName destinationWaveFile);

    void ConvertModuleToMP3(std::string sourceModuleFile,
                            const CFileName destinationMP3File);


    /**
     * Initialises libModPlug and lame
     * @throws: Exceptions
     */
    static void Initialise(bool initaliseLame = true);

    /**
     * Final "safe" cleanup method
     * Really should get round to making this automatic
     */
    static void Cleanup();

private: // methods
    static void ConfigureModPlugStaticSettings();
    static void InitialiseLame();

    /**
     * Load in a MOD file ready to be converted
     */
    void LoadModFile(std::string modFilePath);
    void WriteWaveHeader(CSoundFileWrapper & modFile,
                         ofstream & destFile,
                         const unsigned long dataLength);

    void WriteMP3Tags(std::string sourceModuleFile,
                      CSoundFileWrapper & modFile,
                      const CFileName destinationMP3File);
private: // data
    static bool initialiseInvoked;
    static bool modPlugInitialised;
    static CMP3Encoder * mp3Encoder;
    CConversionListener & listener;
    auto_ptr<CSoundFileWrapper> modFile;     // Currently loaded mod file
};

class CModuleCvt_Exception {
public:
    CModuleCvt_Exception(string msg)  { message = msg; }


    string Report() { return message; }
private:
    string message;
};

#endif // MODCVT

