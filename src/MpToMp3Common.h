
#ifndef _INCLUDE_MPTOMP3_COMMON_H
#define _INCLUDE_MPTOMP3_COMMON_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <FileNameParser.h>


using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::flush;

class CMpTpMp3Exception {
public:
    CMpTpMp3Exception()  {}
    CMpTpMp3Exception(string msg)  { message = msg; }
    virtual ~CMpTpMp3Exception() {}
    virtual void Report()
    {
        if (message.length() > 0)
        {
            cout << message << endl << flush;
        }
        else
        {
            cout << "Uknown error occured" << endl << flush;
        }
    };
private:
    string message;
};


class CCommandLineParseException : public CMpTpMp3Exception {
public:
    CCommandLineParseException()
    : CMpTpMp3Exception() {}
    CCommandLineParseException(string msg)
    : CMpTpMp3Exception(msg) { }
};


typedef struct SProgOpts
{
    bool helpFlag;
    bool longHelpFlag;
    bool waveOnly;
    std::string outputDir;
    std::string fileList;
    vector<string> filesToConvert;
} SProgramOptions;


std::string TrimString(std::string s);

std::string AppVersion();
std::string AppTitle();

void PrintHeaders();

#endif
