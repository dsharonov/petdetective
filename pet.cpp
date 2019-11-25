#include "pet.h"

#include <boost/container_hash/hash.hpp>

std::size_t hash_value(Pet const& p)
{
    std::size_t h = 0;

    boost::hash_combine(h, p.m_name);
    boost::hash_combine(h, p.m_house);
    boost::hash_combine(h, p.m_animal);
    boost::hash_combine(h, p.m_captured);

    return h;
}

void Pet::followCar(Position const& pos, bool capture)
{
    if (isHome())
        return;

    if (isCaptured())
        m_animal = pos;
    else if(m_animal == pos && capture)
        setCaptured(true);

    if (m_animal == m_house) // home
        setCaptured(false);
}


auto Pet::Builder::addPos(char c, Position const& pos) -> Builder&
{
    if (!std::isalpha(c))
        throw std::invalid_argument("Invalid item name");

    auto name = Pet::asAnimalName(c);

    if (m_name == INVALID_NAME)
        m_name = name;
    else if (m_name != name)
        throw std::invalid_argument("Wrong item name");

    if (pos.first % 2 != 0 || pos.second % 2 != 0)
        throw std::runtime_error(std::string("Invalid position of ") + c);

    if (std::isupper(c))
        m_house = pos;
    else
        m_animal = pos;

    return *this;
}

Pet Pet::Builder::build()
{
    if (m_name == INVALID_NAME || m_animal == INVALID_POSITION || m_house == INVALID_POSITION)
        throw std::runtime_error("Insufficient data to build a Pet");

    return { m_name, m_animal, m_house };
}

