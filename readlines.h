#ifndef READLINES_H
#define READLINES_H

#include <string>
#include <istream>
#include <vector>
#include <limits>

std::vector<std::string> readLines(std::istream& is, std::size_t maxlines = std::numeric_limits<std::size_t>::max());

#endif // READLINES_H
