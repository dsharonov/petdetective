#include "readlines.h"

#include <list>
#include <algorithm>


constexpr int readBufSize = 1024;

std::vector<std::string> readLines(std::istream& is, std::size_t maxlines)
{
    std::vector<std::string> lines;
    std::string part;
    std::size_t numlines = 0;

    while (is)
    {
        char buf[readBufSize];
        is.read(buf, sizeof(buf));

        char* pos = buf;
        char* prev = buf;
        char* const dataend = buf + is.gcount();

        while ((pos = std::find(prev, dataend, '\n')) != dataend && numlines < maxlines)
        {
            if (part.empty())
            {
                lines.push_back(std::string(prev, pos));
            }
            else
            {
                lines.push_back(part + std::string(prev, pos));
                part.clear();
            }

            prev = pos + 1;
            ++numlines;
        }

        // To get the last substring (or only, if delimiter is not found)
        if (prev < dataend) {
            if (part.empty())
                part.assign(prev, dataend);
            else
                part += std::string(prev, dataend);
        }

        if (numlines == maxlines) break;
    }

    if (!part.empty())
    {
        if (numlines == maxlines)
            is.seekg(-static_cast<long>(part.size()), std::ios_base::cur);
        else
            lines.push_back(std::move(part));
    }

    return lines;
}


