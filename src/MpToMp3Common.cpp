
#include <MpToMp3Common.h>

std::string AppVersion()
{
    return "0.5";
}

std::string AppTitle()
{
    return "Modplug To MP3 Convertor";
}


void PrintHeaders()
{
    cout << AppTitle() << " version " << AppVersion()
         << " - (c) Neil Potter 2011" << endl;
    cout << "libmodplug (XMMS) v0.8.8.1 - based on ModPlug sound engine by Olivier Lapicque " << endl;
    cout << "LAME MP3 Encoding Library - http://www.mp3dev.org/" << endl;
    cout << "TagLib Audio Meta-Data Library - http://developer.kde.org/~wheeler/taglib.html" << endl;
    cout << endl;
}

std::string TrimString(std::string s)
{
    std::string trimmed = s;
    if (s.length() > 0)
    {
        unsigned int begin = s.find_first_not_of(' ');
        unsigned int end = s.find_last_not_of(' ');
        trimmed = s.substr( begin, end-begin+1);
    }
    return trimmed;
}

