
#ifndef _FILE_NAME_PARSER_H
#define _FILE_NAME_PARSER_H
#include <string>

using std::string;

class CFileName
{
public:
    CFileName();
    CFileName(const std::string filePathAndName,
              const char pathSeperator = '/');
    CFileName(const CFileName & c);

    void Reset(const std::string newFilePathAndName,
               const char pathSeperator = '/');

    bool SetPath(std::string newPathPart);

    std::string Full() const; // File path, Name, extension
    std::string Path() const; // Just Path
    std::string FileName() const; // Includes extension
    std::string Extension() const;
    char        PathSeperator() const;
private: //methods
    void SplitToComponents();
private: // members
    std::string fullFileNameAndPath;
    char        pathSep;
    std::string path;
    std::string fileName;
    std::string extension;
};

#endif
