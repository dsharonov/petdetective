#pragma once

#include <functional>
#include <numeric>
#include <list>
#include <unordered_map>
#include <tuple>
#include <cassert>

#include <boost/operators.hpp>
#include <boost/container_hash/hash.hpp>

#include "definitions.h"
#include "pet.h"

class State;

using StatePtr = std::shared_ptr<State>;
using StateKey = std::tuple<Pets, Car>;
using StateRegistry = std::unordered_map<StateKey, StatePtr, boost::hash<StateKey>>;
using StateRegistryPtr = std::shared_ptr<StateRegistry>;


class State : public std::enable_shared_from_this<State>,
              public boost::equality_comparable1<State>

{
public:

    using AdjContainer = std::list<std::pair<StatePtr, StatePtr>>;
    using AdjContainerPtr = std::unique_ptr<AdjContainer>;
    using Iterator = AdjContainer::iterator;

    AdjContainerPtr& adjacent();

    std::pair<Iterator, Iterator> outEdges();
    std::size_t outDegree();
    StateKey const& key() const;
    bool isFinal() const;

    static StatePtr addState(StateRegistryPtr statereg, StreetsPtr streets, StateKey&& key);

private:

    friend bool operator==(State const& l, State const& r);
    friend std::size_t hash_value(State const& s);
    friend std::ostream& operator<<(std::ostream& os, State const& s);

    State(StateRegistryPtr statereg, StreetsPtr streets, StateKey&& key);

    bool isValidCarPosition(Position const& to) const;

private:
    StateRegistryPtr m_statereg;
    StreetsPtr m_streets;
    StateKey m_key;

    AdjContainerPtr m_adjacent;
};

inline State::State(StateRegistryPtr statereg, StreetsPtr streets, StateKey&& key)
    : m_statereg(statereg)
    , m_streets(streets)
    , m_key(key)
{
}

inline bool State::isValidCarPosition(Position const& pos) const
{
    return !(pos.first < 0
             || pos.second < 0
             || pos.first >= static_cast<int>(m_streets->size())
             || pos.second >= static_cast<int>(m_streets->at(0).size())
             || m_streets->at(pos.first)[pos.second] == NOWAY);
}

inline bool operator==(State const& l, State const& r)
{
    return l.key() == r.key();
}

inline auto State::outEdges() -> std::pair<Iterator, Iterator>
{
    auto& adj = adjacent();
    return { adj->begin(), adj->end() };
}

inline std::size_t State::outDegree()
{
    return adjacent()->size();
}

inline StateKey const& State::key() const
{
    return m_key;
}

inline bool State::isFinal() const
{
    auto const& pets = std::get<0>(m_key);
    return std::all_of(pets.cbegin(), pets.cend(), [](auto const& pet) { return pet.isHome(); });
}
