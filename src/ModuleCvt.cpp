
#include <ModuleCvt.h>

// Tag Lib Headers
#include <fileref.h>
#include <mpegfile.h>

//#ifndef __WIN32__
//#include <dlfcn.h>
//#endif

#include <SoundFileWrap.h>
#include <MP3EncoderFactory.h>
#include <WaveFile.h>

#include <iostream>
#include <ctime>
#include <cstring>

using std::endl;
using std::flush;
bool CModuleCvt::modPlugInitialised = false;
bool CModuleCvt::initialiseInvoked = false;
CMP3Encoder * CModuleCvt::mp3Encoder = NULL;


// -----------------------------------------------------------------------------
// CLASS: CConversionListener
CModuleCvt::CConversionListener::CConversionListener()
{
}

CModuleCvt::CConversionListener::~CConversionListener()
{
}


// -----------------------------------------------------------------------------
// STATIC METHOD

void CModuleCvt::Initialise(bool initaliseLame)
{
    initialiseInvoked = true;
    CSoundFileWrapper::Initialise();
    modPlugInitialised = true;

    if (initaliseLame)
    {
        InitialiseLame();
    }
}

void CModuleCvt::Cleanup()
{
    if (initialiseInvoked)
    {
        // Reset a few things
        initialiseInvoked = false;
    }
}

void CModuleCvt::InitialiseLame()
{
    std::string errorMessage;
    CMP3EncoderFactory * enc = CMP3EncoderFactory::Instance();
    mp3Encoder = enc->GetEncoder();

    if (!mp3Encoder->LoadEncoderLibrary(errorMessage))
    {
        throw CModuleCvt_Exception(errorMessage);
    }
}


// -----------------------------------------------------------------------------
// PER-INSTANCE METHOD
CModuleCvt::CModuleCvt(CConversionListener & externalListener)
    : listener(externalListener)
{
}

CModuleCvt::~CModuleCvt()
{

}

void CModuleCvt::LoadModFile(std::string modFilePath)
{
    ifstream sourceFile;

    // Open the file and read contents into an inmemory buffer
    sourceFile.open (modFilePath.c_str(), ios::in | ios::binary);
    if (!sourceFile)
    {
        throw CModuleCvt_Exception("Unable to opened source module file");
    }

    std::string errorMessage;
    modFile.reset(new CSoundFileWrapper);
    if (!modFile->Create(sourceFile,
                         errorMessage))
    {
        throw CModuleCvt_Exception(errorMessage);
    }

    sourceFile.close();
}

// --------------------------- WAV RELATED -------------------------------------

void CModuleCvt::ConvertModuleToWav(
    std::string sourceModuleFile,
    CFileName destinationWaveFile)
{
    listener.OnConversionStarted(sourceModuleFile,
                                 destinationWaveFile,
                                 false);

    // Ensure Modplug was initialised
    if (!modPlugInitialised)
    {
        throw CModuleCvt_Exception(
            "Cannot convert: ModPlug must be initialised first.");
    }

    // Prepare files
    LoadModFile(sourceModuleFile);
    modFile->SetCurrentPos(0);
    modFile->SetRepeatCount(0);
    modFile->ResetChannels();

    ofstream destinationFile;
    destinationFile.open (destinationWaveFile.Full().c_str(),
                          ios::out | ios::binary);
    if (!destinationFile)
    {
        throw CModuleCvt_Exception(
            "Unable to convert module to WAV - could not open.output file");
    }

    // Start the conversion
    const unsigned int BUF_SIZE = 4096;
    char audio_buffer[BUF_SIZE];
    unsigned int samplesRead       = 0;
    unsigned int totalBytesWritten = 0;
    unsigned int nBytesPerSample   = (CSoundFileWrapper::gnBitsPerSample() *
                                      CSoundFileWrapper::gnChannels()) / 8;
    WriteWaveHeader(*(modFile.get()), destinationFile, 0);

    double percentPos = 0.0;

    // Read returns the number of SAMPLES rather than the number of bytes
    // (a sample includes both stereo channels)
    samplesRead = modFile->Read(audio_buffer, BUF_SIZE);
    while (samplesRead > 0)
    {

        // Write any read data to the file, obviously the buffer may not be
        // completely full
        destinationFile.write(audio_buffer, samplesRead*nBytesPerSample);
        totalBytesWritten +=         samplesRead*nBytesPerSample;

        percentPos = (static_cast<float> (modFile->GetCurrentPos())
                      / static_cast<float> (modFile->GetMaxPosition()) * 100.0f);

        listener.OnConversionStatus(sourceModuleFile,
                                    destinationWaveFile,
                                    false,
                                    CConversionListener::CS_CONVERTING,
                                    percentPos,
                                    std::string(""));

        // Next block
        samplesRead = modFile->Read(audio_buffer, BUF_SIZE);
    }

    // Rewrite the wave header now we know the size of the
    // data block
    WriteWaveHeader(*(modFile.get()), destinationFile, totalBytesWritten);
    destinationFile.close();

    listener.OnConversionComplete(sourceModuleFile,
                                  destinationWaveFile,
                                  false);
}

void CModuleCvt::WriteWaveHeader(CSoundFileWrapper & modFile,
                                 ofstream & destFile,
                                 const unsigned long dataLength)
{
    // Curse whoever designed the WAVE file format
    WAVEFILEHEADER   header;
    WAVEDATAHEADER   dataHdr;
    WAVEFORMATHEADER fmtHdr;

    destFile.seekp(0,
                   ios_base::beg);

    // File Header
    header.id_RIFF = IFFID_RIFF;
    // Size of file minus size of header
    // header.filesize = (sizeof(WAVEFILEHEADER)-8)
    // + (8+fmthdr.length) + (8+datahdr.length);
    header.filesize = sizeof(WAVEFILEHEADER) - 8 + 2;
    header.id_WAVE = IFFID_WAVE;

    // Wave Format Header
    fmtHdr.id_fmt   = IFFID_fmt;
    fmtHdr.hdrlen   = 16; // length of format subchunk
    fmtHdr.format   = 1;    //  (excl header)
    fmtHdr.channels = 2;
    fmtHdr.freqHz   = 44100;              // Sample rate
    fmtHdr.bytessec = 44100 * ((2 * 16) / 8); // Avg bytes/sec
                                //freqHz * channels * bitsPerSample/8
    fmtHdr.samplesize = (2 * 16) / 8; // Block Aligh
    // = wChannels * (wBitsPerSample / 8)
                                // NumChannels * BitsPerSample/8;
    fmtHdr.bitspersample = 16;
    header.filesize += sizeof(fmtHdr);

    // Data header
    dataHdr.id_data = IFFID_data;
    dataHdr.length  = dataLength;
    header.filesize += sizeof(dataHdr) + dataLength;

    // Writing Headers
    destFile.write(reinterpret_cast<char *> (&header), sizeof(header));
    destFile.write(reinterpret_cast<char *> (&fmtHdr), sizeof(fmtHdr));
    destFile.write(reinterpret_cast<char *> (&dataHdr), sizeof(dataHdr));
}

// --------------------------- MP3 RELATED -------------------------------------

void CModuleCvt::ConvertModuleToMP3(
    std::string sourceModuleFile,
    CFileName destinationMP3File)
{
    listener.OnConversionStarted(sourceModuleFile,
                                 destinationMP3File,
                                 true);

    // Ensure Modplug was initialised
    if (!modPlugInitialised)
    {
        throw CModuleCvt_Exception(
            "Cannot convert: ModPlug must be initialised first.");
    }

    // Ensure LAME was correctly initialised
    if (NULL == mp3Encoder)
    {
        throw CModuleCvt_Exception(
            "Cannot convert: LAME must be initialised first.");
    }

    // Prepare files
    LoadModFile(sourceModuleFile);
    modFile->SetCurrentPos(0);
    modFile->SetRepeatCount(0);
    modFile->ResetChannels();
    unsigned int nBytesPerSample = (CSoundFileWrapper::gnBitsPerSample() *
                                    CSoundFileWrapper::gnChannels()) / 8;

    ofstream destinationFile;
    destinationFile.open (destinationMP3File.Full().c_str(),
                          ios::out | ios::binary);
    if (!destinationFile)
    {
        throw CModuleCvt_Exception(
            "Unable to convert module to WAV - could not open.output file");
    }

    // Kick off the conversion process
    // @44100 - 4 bytes per sample (2 per channel)
    int16_t *          mod_audio_buffer;
    unsigned char  * lame_output_buffer;
    unsigned int     samplesRead;

    // Initialise LAME
    unsigned long lameSamplesPerChunk;
    unsigned int  lameBufferSize;
    std::string   errorMessage;

    if (!mp3Encoder->InitializeStream(
            128, // kbps
            lameSamplesPerChunk,
            errorMessage))
    {
        destinationFile.close();
        throw CModuleCvt_Exception(errorMessage);
    }

    lameBufferSize = mp3Encoder->GetOutBufferSize();

    // Read samples from modplug in about the size needed by lame
    unsigned int MOD_ARRAY_BYTE_SIZE = nBytesPerSample * lameSamplesPerChunk;
    unsigned int MOD_ARRAY_SIZE = (MOD_ARRAY_BYTE_SIZE/2);

    lame_output_buffer = new unsigned char[lameBufferSize];
    mod_audio_buffer   = new int16_t[MOD_ARRAY_SIZE];

    unsigned long    lameBytesEncoded = 0;
    double           percentPos       = 0.0;

    // Read returns the number of SAMPLES rather than the number of Bytes
    // For 16bit Stereo one "sample" is 4 bytes: 2 for left, 2 for right
    samplesRead = modFile->Read(mod_audio_buffer, MOD_ARRAY_BYTE_SIZE);

    bool encodingErrors           = false;
    while (!encodingErrors &&
           samplesRead > 0)
    {
        // just to confuse things a "sample" from lame's view point is
        // a single int16_t for a single channel.
        // That means each 1 modplug sample (4 bytes, 2 per chanel)
        // is 2 lame samples
        if (!mp3Encoder->EncodeBuffer(
                mod_audio_buffer,
                2*samplesRead,
                lame_output_buffer,
                lameBytesEncoded,
                errorMessage))
        {
            // Tidy up
            encodingErrors = true;
            mp3Encoder->CancelEncoding();
            destinationFile.close();
            // throw
            throw CModuleCvt_Exception(errorMessage);
        }

        // Flush lame buffer to file
        if (lameBytesEncoded > 0)
        {
            destinationFile.write(
                (reinterpret_cast <char *> (lame_output_buffer)),
                lameBytesEncoded);
        }

        percentPos = (static_cast<float> (modFile->GetCurrentPos())
                      / static_cast<float> (modFile->GetMaxPosition()) * 100.0f);

        listener.OnConversionStatus(sourceModuleFile,
                                    destinationMP3File,
                                    true,
                                    CConversionListener::CS_CONVERTING,
                                    percentPos,
                                    std::string(""));

        // Next block of mod samples
        samplesRead = modFile->Read(mod_audio_buffer, MOD_ARRAY_BYTE_SIZE);
    }

    // Finish up
    if (!mp3Encoder->FinishStream(lame_output_buffer,
                                  lameBytesEncoded,
                                  errorMessage))
    {
        // Tidy up
        destinationFile.close();
        // throw
        throw CModuleCvt_Exception(errorMessage);
    }

    if (lameBytesEncoded > 0)
    {
        // Deinitialisation might not return anything
        destinationFile.write(
            (reinterpret_cast <char *> (lame_output_buffer)),
            lameBytesEncoded);
    }

    delete [] lame_output_buffer;
    delete [] mod_audio_buffer;
    destinationFile.close();

    listener.OnConversionStatus(sourceModuleFile,
                                destinationMP3File,
                                true,
                                CConversionListener::CS_WRITE_TAGS,
                                100.0,
                                std::string(""));

    WriteMP3Tags(sourceModuleFile,
                 *(modFile.get()),
                 destinationMP3File);

    listener.OnConversionComplete(sourceModuleFile,
                                  destinationMP3File,
                                  true);
}


void CModuleCvt::WriteMP3Tags(std::string sourceModuleFile,
                              CSoundFileWrapper & modFile,
                              const CFileName destinationMP3File)
{
    bool useInstComments = (modFile.m_nInstruments() > 0) ? true : false;
    // Rather than overrite, we copy and create a tagged version
    string actualDestFile = destinationMP3File.Full();

    // Version 1.4 of TagLib is broken - use v1.5
    // (by broken I mean most apps will not read all of the comment
    // block - including winamp and ID3Lib based apps)
    // Worse, winamp skips the start of the MP3 by a few seconds/ms)

    CFileName srcFile_fname(sourceModuleFile);
    TagLib::MPEG::File f(actualDestFile.c_str());
    stringstream msg;
    msg << "> Writing ID3 tags ("
        << "taking comments from "
        << (useInstComments ? "Instruments [" : "Samples [")
        << (useInstComments ? modFile.m_nInstruments() : modFile.m_nSamples())
        << " defined])"
        << "              "
        << endl << flush;

    listener.OnConversionStatus(sourceModuleFile,
                                destinationMP3File,
                                true,
                                CConversionListener::CS_WRITE_TAGS,
                                100.0,
                                msg.str());


    if (f.audioProperties() != NULL)
    {
        std::string newTitle;
        newTitle = TrimString(modFile.GetTitle());
        newTitle += " (";
        newTitle += srcFile_fname.FileName();
        newTitle += ")";
        f.tag()->setTitle(newTitle);

        // Assemble the comments from instruments or samples
        stringstream comments;
        const int MAX_INST_NAME = 100;
        char instName[MAX_INST_NAME]; // What is max length of samples/insts ?
        if (useInstComments)
        {
            // Use instruments

            // We can't use GetNumInstruments as it skips instruments
            // without samples!
            for (unsigned int i = 1; i <= modFile.m_nInstruments(); i++)
            {
                memset(instName, '\0', MAX_INST_NAME); // NULL terminator protection
                modFile.GetInstrumentName(i, instName);
                comments << std::setfill(' ') << std::setw(2)
                         << i
                         << ": " << instName << "\r\n";
            }
        }
        else
        {
            // Use Samples
            for (unsigned int i =1; i <= modFile.m_nSamples(); i++)
            {
                memset(instName, '\0', MAX_INST_NAME);
                modFile.GetSampleName(i, instName);
                comments << std::setfill(' ') << std::setw(2)
                         << i
                         << ": " << instName << "\r\n";
            }
        }
        f.tag()->setComment(comments.str());

        // We're done here - only write v3.2 tag
        f.save(TagLib::MPEG::File::ID3v2);
    }
    else
    {
        throw CModuleCvt_Exception("Cannot open destination file to write tags (missing or invalid format)");
    }
}
