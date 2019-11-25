#include "state.h"

#include <boost/container_hash/hash.hpp>

StatePtr State::addState(StateRegistryPtr statereg, StreetsPtr streets, StateKey&& key)
{
    auto found = statereg->find(key);

    if (found != statereg->end())
        return found->second;

    StatePtr newstate(new State(statereg, streets, std::move(key)));
    statereg->insert({ newstate->key(), newstate });

    return newstate;
}

auto State::adjacent() -> AdjContainerPtr&
{
    if (!m_adjacent)
    {
        Position const incs[] =
        {
            { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 },
        };

        m_adjacent.reset(new AdjContainer());

        for (auto const& inc : incs)
        {
            Car newcar = std::get<1>(key());

            newcar.first += inc.first;
            newcar.second += inc.second;

            if (!isValidCarPosition(newcar))
                continue;

            newcar.first += inc.first;
            newcar.second += inc.second;

            Pets pets = std::get<0>(key());
            int num_captured = 0;
            for (auto it = pets.begin(), endit = pets.end(); it != endit; ++it) {
                num_captured += it->isCaptured() ? 1 : 0;
                it->followCar(newcar, false);
            }

            m_adjacent->push_back({ shared_from_this(), addState(m_statereg, m_streets, { pets, newcar }) });

            if (num_captured < MAX_CAPTURED)
            {
                if (auto found = std::find_if(pets.begin(), pets.end(), [newcar](Pet const& pet) {
                                              return !pet.isHome() && !pet.isCaptured() && newcar == pet.animalPosition(); });
                        found != pets.end())
                {
                    // newcar is on pos of some animal - can capture
                    found->followCar(newcar, true);
                    m_adjacent->push_back({ shared_from_this(), addState(m_statereg, m_streets, { pets, newcar }) });
                }
            }
        }
    }

    return m_adjacent;
}

std::ostream& operator<<(std::ostream& os, State const& s)
{
    auto streets = s.m_streets;

    auto const& pets = std::get<0>(s.key());
    auto const& car = std::get<1>(s.key());

    for (int row = 0, rowcount = static_cast<int>(streets->size()); row < rowcount; ++row)
    {
        auto const& line = streets->at(static_cast<std::size_t>(row));

        for (int col = 0, colcount = static_cast<int>(line.size()); col < colcount; ++col)
        {
            auto item = line[static_cast<std::size_t>(col)];
            Position const pos(row, col);

            if (Position(row, col) == car)
                os << CAR;
            else
            {
                auto found = std::find_if(pets.begin(), pets.end(), [pos](auto const& pet) {
                    return pos == pet.housePosition() || pos == pet.animalPosition();
                });

                if (found == pets.end())
                    os << item;
                else if (found->housePosition() == pos)
                    os << found->houseName();
                else
                    os << found->animalName();
            }
        }

        os << std::endl;
    }

    os << "Captured: ";
    for (auto const& pet : pets)
    {
        if (pet.isCaptured())
            os << pet.animalName() << ' ';
    }

    os << std::endl;

    return os;
}

std::size_t hash_value(State const& s)
{
    boost::hash<StateKey> hasher;
    return hasher(s.key());
}

#ifdef TEST

#ifdef NDEBUG
    #error NDEBUG should not be defined
#endif

#include <iostream>


int main()
{
//    { 'a','+','*','+','F','+','f','+','D' },
//    { '+',' ','+',' ',' ',' ',' ',' ','+' },
//    { 'c','+','*','+','@',' ','d','+','e' },
//    { '+',' ',' ',' ','+',' ','+',' ','+' },
//    { 'b','+','E','+','A','+','B','+','C' },

    StreetsPtr streets = std::make_shared<Streets>(Streets{
        { '*','+','*','+','*','+','*','+','*' },
        { '+',' ','+',' ',' ',' ',' ',' ','+' },
        { '*','+','*','+','*',' ','*','+','*' },
        { '+',' ',' ',' ','+',' ','+',' ','+' },
        { '*','+','*','+','*','+','*','+','*' },
    });

    constexpr int NUMPETS = 6;

    Car car(2, 4);

    Position animalpos[NUMPETS] = {
        {0,0}, {4, 0}, {2, 0}, {2, 6}, {2, 8}, {0, 6},
    };

    Position housepos[NUMPETS] = {
        {4,4}, {4, 6}, {4, 8}, {0, 8}, {4, 2}, {0, 4},
    };

    Pet::Builder builders[NUMPETS];
    Pets pets;
    Pets pets2;

    for (std::size_t i = 0; i < std::size(builders); ++i)
    {
        auto& b = builders[i];
        b.addPos('a' + static_cast<char>(i), animalpos[i]);
        b.addPos('A' + static_cast<char>(i), housepos[i]);
        pets.push_back(b.build());
        pets2.push_back(b.build());
    }

    assert(pets == pets2);

    for (std::size_t i = 0; i < NUMPETS; ++i)
    {
        auto const& pet = pets[i];
        assert(pet.animalName() == 'a' + static_cast<char>(i));
        assert(pet.houseName() == 'A' + static_cast<char>(i));
        assert(pet.animalPosition() == animalpos[i]);
        assert(pet.housePosition() == housepos[i]);
        assert(!pet.isCaptured());
        assert(!pet.isHome());
    }

    StateRegistryPtr statereg = std::make_shared<StateRegistry>();

//    auto state = State::addState(statereg, streets, { pets, {0,0} });
//    auto& adj = state->adjacent();
//    for(auto it = adj->begin(); it != adj->end(); ++it)
//    {
//        std::cout << *(it->second) << std::endl;
//    }

//    pets[0].followCar(pets[0].animalPosition(), true);
//    pets[0].followCar(car, false);
//    auto state = State::addState(statereg, streets, { pets, car });
//    auto& adj = state->adjacent();
//    for(auto it = adj->begin(); it != adj->end(); ++it)
//    {
//        std::cout << *(it->second) << std::endl;
//    }

    //    { 'a','+','*','+','F','+','f','+','D' },
    //    { '+',' ','+',' ',' ',' ',' ',' ','+' },
    //    { 'c','+','*','+','@',' ','d','+','e' },
    //    { '+',' ',' ',' ','+',' ','+',' ','+' },
    //    { 'b','+','E','+','A','+','B','+','C' },

    Position steps[] = {
        {1,0},{0,1},{-1,0},{0,1},{-1,0},{0,-1},{0,-1},{0,-1},{0,-1},
        {1,0},{1,0},{0,1},{0,1},{0,1},{0,1},

    };

    auto state = State::addState(statereg, streets, { pets, car });

    for(auto const& step : steps)
    {
        state->adjacent();

        car.first += 2 * step.first;
        car.second += 2 * step.second;
        std::for_each(pets.begin(), pets.end(), [car](auto& pet){ pet.followCar(car, true); });

        assert(statereg->find({pets, car}) != statereg->end());

        auto n = statereg->size();
        state = State::addState(statereg, streets, { pets, car });

        assert(n = statereg->size());
    }

    assert(std::all_of(pets.begin(), pets.end(), [](auto const& pet){ return pet.isHome(); }));

    return 0;
}


#endif
