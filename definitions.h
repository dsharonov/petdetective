#pragma once

#include <vector>
#include <memory>

using Position = std::pair<int, int>;
using Car = Position;

constexpr Position INVALID_POSITION{ -1, -1 };
constexpr char INVALID_NAME = '\0';

constexpr char ROAD = '*';
constexpr char WAY  = '+';
constexpr char NOWAY= ' ';
constexpr char CAR  = '@';

constexpr int MAX_CAPTURED = 4;

inline bool isAnimalOrHouse(char c) { return std::isalpha(c); }


using Streets = std::vector<std::vector<char>>;
using StreetsPtr = std::shared_ptr<Streets>;

