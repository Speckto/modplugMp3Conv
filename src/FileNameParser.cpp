#include <FileNameParser.h>

CFileName::CFileName()
    : fullFileNameAndPath(""),
      pathSep('/'),
      path(""),
      fileName(""),
      extension("")
{
}

CFileName::CFileName(const std::string filePathAndName,
                     const char pathSeperator)
    : path(""),
      fileName(""),
      extension("")
{
    Reset(filePathAndName, pathSeperator);
}

CFileName::CFileName(const CFileName & c)
{
    // We could save some parsing by copying values I suppose
    // - maybe implement it later
    Reset(c.Full(), c.PathSeperator());
}

void CFileName::Reset(const std::string newFilePathAndName,
                      const char pathSeperator)
{
    pathSep             = pathSeperator;
    fullFileNameAndPath = newFilePathAndName;
    SplitToComponents();
}

void CFileName::SplitToComponents()
{
    // Split out path and filename
    string::size_type lastPathSep =
        fullFileNameAndPath.find_last_of(pathSep,
                                         fullFileNameAndPath.length());
    if (lastPathSep != string::npos &&
        (lastPathSep + 1) < fullFileNameAndPath.length())
    {
        // Found last path seperator
        path = fullFileNameAndPath.substr(0,
                                          lastPathSep + 1);
        fileName = fullFileNameAndPath.substr(lastPathSep + 1,
                                              fullFileNameAndPath.length());
    }
    else
    {
        // No path seperator
        path = "";
        fileName = fullFileNameAndPath;
    }

    // Now figure out the file extension
    string::size_type lastExtSep =
        fileName.find_last_of(".",
                              fileName.length());
    if (lastExtSep != string::npos &&
        (lastExtSep+1) < fileName.length())
    {
        extension = fileName.substr(
            lastExtSep + 1,
            fileName.length());
    }
    else
    {
        extension = "";
    }
}

string CFileName::Full() const
{
    return fullFileNameAndPath;
}

string CFileName::Path() const
{
    return path;
}

string CFileName::FileName() const
{
    return fileName;
}

string CFileName::Extension() const
{
    return extension;
}

char CFileName::PathSeperator() const
{
    return pathSep;
}

bool CFileName::SetPath(std::string newPathPart)
{
    bool pathSet = true;
    // Use pathSep to join if required
    if (newPathPart.length() > 0)
    {
        // decrement so it points to last character
        int plen = newPathPart.length();
        plen--;
        
        if (newPathPart.at(plen) == pathSep)
        {
            // Don't need joining slash
            path = newPathPart;
        }
        else
        {
            path = newPathPart + pathSep;
        }
        fullFileNameAndPath = path + fileName;
    }
    else
    {
        pathSet = false;
    }
    return pathSet;
}

