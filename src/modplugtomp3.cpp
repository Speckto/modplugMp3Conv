// General includes
#include <unistd.h>
#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <config.h>

// ModulePlug
#include <libmodplug/stdafx.h>
#include <libmodplug/modplug.h>
#include <libmodplug/sndfile.h>

// MP3
#include <BladeMP3EncDLL.h>

// Application
#include <MpToMp3Common.h>
#include <FileNameParser.h>
#include <ModuleCvt.h>

using std::string;
using std::vector;

static SProgramOptions programOptions;

static struct option long_options[] =
{
    /* These options set a flag. */
    {"help",       no_argument,       0, 'h'},
    {"helplong",   no_argument,       0, 'H'},
    {"version",    no_argument,       0, 'v'},
    {"waveonly",   no_argument,       0, 'w'},
    {"outputdir",  required_argument, 0, 'o'},
    {"filelist",   required_argument, 0, 'l'},
    {0, 0, 0, 0}
};

class CListener : public CModuleCvt::CConversionListener
{
public:
    CListener() {};
    virtual ~CListener() {};

    virtual void OnConversionStarted(const std::string &sourceModuleFile,
                                     const CFileName &destFile,
                                     bool  convertingToMp3)
    {
        cout << "Starting conversion of file "
             << sourceModuleFile
             << " to "
             << (convertingToMp3 ? "mp3" : "wav")
             << " as "
             << destFile.Full()
             << endl;
    };

    virtual void OnConversionStatus(
        const  std::string &sourceModuleFile,
        const  CFileName &destFile,
        bool   convertingToMp3,
        CModuleCvt::CConversionListener::TConversionStatus status,
        double percentDone,
        const std::string & message)
    {
        cout << "\r";
        cout << "> Converting file to "
             << (convertingToMp3 ? "mp3" : "wav")
             << " ("
             << std::fixed << std::setprecision(1)
             << std::setfill(' ') << std::setw(5)
             << percentDone
             << "%)";
    };

    virtual void OnConversionComplete(const  std::string &sourceModuleFile,
                                      const  CFileName &destFile,
                                      bool   convertingToMp3)
    {
        cout << "\r> Conversion Completed." << endl;
    };
};

void Usage(bool longHelp)
{
    PrintHeaders();

    cout << " Usage: ls [OPTION]... [FILE]..."
         << endl
         << "Converts MOD files to MP3 (or wave) files using libmodplug"
         << "and lame"
         << endl
         << "-v, --version   Displays version"
         << endl
         << "-h, --help      Displays this help text"
         << endl
         << "-H, --longhelp  Display extended help text"
         << endl
         << "-w, --waveonly"
         << " Converts specified file(s) only to a WAV file rather than MP3"
         << endl
         << "-o, --outputdir"
         << " Specifies destination directory for converted files."
         << endl
         << "                If unspecified the converted file is placed in the"
         << " same directory as the source file"
         << endl
         << "-l, --filelist  Reads list of files to convert from a text file"
         << " containing full file patsh"
         << endl
         << endl;
}


void ReadFileListOption()
{
    ifstream fileListStream;

    // Open the file and read contents into an inmemory buffer
    fileListStream.open(programOptions.fileList.c_str(), ios::in);
    if (!fileListStream)
    {
        throw CCommandLineParseException(
                      "Invalid file name specified for file listing parameter");
    }

    while ( !fileListStream.eof() ) {
        std::string line;

        getline(fileListStream, line);

        // Some validation is performed on the file name
        // - Ensure that a dot is present in the file name
        // - Ensre that there is a non-space character
        std::string::size_type dotIndex = line.find(".", 0);
        if (!line.empty() &&
            string::npos != line.find_first_not_of(' ') &&
            string::npos != dotIndex)

        {
            programOptions.filesToConvert.push_back(line);
        }
    }
}

void ParseCommandLine(int argc, char* argv[])
{
    bool continueParse = true;
    int option_index   = -1;
    int c              = -1;
    while (continueParse)
    {
        c = getopt_long (argc, argv, "hHw",
                         long_options, &option_index);
        if (c != -1)
        {
            switch (c)
            {
            case 'h':
                programOptions.helpFlag = true;
                break;
            case 'H':
                programOptions.longHelpFlag = true;
                break;
            case 'w':
                programOptions.waveOnly = true;
                break;
            case 'o':
                programOptions.outputDir = optarg;
                break;
            case 'l':
                programOptions.fileList = optarg;
                ReadFileListOption();
                break;
            default:
                throw CCommandLineParseException();
                break;
            }
        }
        else
        {
            continueParse = false;
        }
    }

    if (optind < argc)
    {
        int curOptInd = optind;
        while (curOptInd < argc)
        {
            // More arguments
            programOptions.filesToConvert.push_back(argv[curOptInd]);
            curOptInd++;
        }
    }
    else if (!programOptions.helpFlag &&
             !programOptions.longHelpFlag &&
             programOptions.filesToConvert.size() <= 0)
    {
        throw CCommandLineParseException("No files specified to be converted.");
    }
}

void GenerateDestFilePath(const bool convertToMP3,
                          const string sourceFile,
                          CFileName &destFile)
{
    string tmpStr;
    tmpStr = sourceFile;
    if (convertToMP3)
    {
        tmpStr += ".mp3";
    }
    else
    {
        tmpStr += ".wav";
    }
    destFile.Reset(tmpStr);

    if (programOptions.outputDir != "")
    {
        destFile.SetPath(programOptions.outputDir);
    }
}

void ConvertSpecifiedFiles ()
{
    bool initialised = true;
    PrintHeaders();
    bool convertToMP3 = !programOptions.waveOnly;
    try
    {
        if (convertToMP3)
        {
            CModuleCvt::Initialise(true);
        }
        else
        {
            CModuleCvt::Initialise(false);
        }
    }
    catch (CModuleCvt_Exception e)
    {
        initialised = false;
        cout << "Unable to initialise MP3 encoding library."
             << " The error was" << endl
             << e.Report() << endl;
        cout << "Unable to proceed. Terminating." << endl;
        exit(1);
    }

    if (initialised)
    {
        CListener listener;
        std::auto_ptr<CModuleCvt> moduleConvert;
        moduleConvert.reset(new CModuleCvt(listener));

        int index = 1;
        for (vector<string>::iterator it =
                 programOptions.filesToConvert.begin();
             it!=programOptions.filesToConvert.end();
             ++it)
        {
            // Work out the target filename
            CFileName destFile;
            GenerateDestFilePath(convertToMP3,
                                 *it,
                                 destFile);

            try
            {
                if (convertToMP3)
                {
                    cout << "["
                         << index
                         << "/"
                         << programOptions.filesToConvert.size()
                         << "] ";
                    moduleConvert->ConvertModuleToMP3(*it,
                                                      destFile);
                }
                else
                {
                    moduleConvert->ConvertModuleToWav(*it,
                                                      destFile);
                }
            }
            catch (CModuleCvt_Exception e)
            {
                cout << e.Report() << endl << flush;
            }
            catch (char * str)
            {
                cout << "STR: " << str << endl << flush;
            }
            index++;
        }
    }
    CModuleCvt::Cleanup();
}

int main(int argc, char* argv[])
{
    // Initalisation
    try
    {
        ParseCommandLine(argc, argv);

        // Implement the logic
        if (programOptions.helpFlag ||
            programOptions.longHelpFlag)
        {
            Usage(programOptions.longHelpFlag);
        }
        else
        {
            ConvertSpecifiedFiles();
        }
    }
    catch (CCommandLineParseException e)
    {
        // nothing to do
        PrintHeaders();
        e.Report();
    }

    return 0;
}

