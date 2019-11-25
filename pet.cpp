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


