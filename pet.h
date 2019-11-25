#pragma once

#include <boost/operators.hpp>

#include "definitions.h"

class Pet : boost::equality_comparable1<Pet>
{
public:

    struct Builder
    {
        Builder() = default;

        Builder& addPos(char c, Position const& pos);
        Pet build();

    private:
        char m_name = INVALID_NAME;
        Position m_animal = INVALID_POSITION;
        Position m_house = INVALID_POSITION;
    };

    friend bool operator==(Pet const& l, Pet const& r)
    {
        return l.m_name == r.m_name
                && l.m_animal == r.m_animal
                && l.m_house == r.m_house
                && l.m_captured == r.m_captured;
    }

    friend std::size_t hash_value(Pet const& p);

    static char asAnimalName(char c) { return static_cast<char>(std::tolower(c)); }

    char animalName() const { return m_name; }
    char houseName() const { return static_cast<char>(std::toupper(m_name)); }
    Position const& housePosition() const { return m_house; }
    Position const& animalPosition() const { return m_animal; }
    bool isHome() const { return !isCaptured() && m_animal == m_house; }
    bool isCaptured() const { return m_captured; }

    void followCar(Position const& pos, bool capture);

private:

    Pet(char name, Position const& animal, Position const& house)
        : m_name(name)
        , m_animal(animal)
        , m_house(house)
    {}

    void setCaptured(bool yes) { m_captured = yes; }

private:
    char m_name;
    Position m_animal;
    Position m_house;
    bool m_captured = false;
};

using Pets = std::vector<Pet>;
