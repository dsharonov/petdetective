#include <iostream>
#include <fstream>
#include <map>
#include <numeric>
#include <queue>
#include <future>

#include <boost/operators.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/coroutine2/all.hpp>

#include "readlines.h"
#include "state.h"

using StatePath = std::list<StatePtr>;
using SolCoro = boost::coroutines2::coroutine<StatePtr>;

struct Solution
{
    std::string filename;
    StatePath sol;
    std::exception_ptr error = nullptr;
};

struct StateGraph {
    using vertex_descriptor = StatePtr;
    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    using directed_category = boost::undirected_tag;
    using edge_parallel_category = boost::disallow_parallel_edge_tag;
    using traversal_category = boost::incidence_graph_tag;
    using out_edge_iterator = State::Iterator;
    using degree_size_type = std::size_t;

    static vertex_descriptor null_vertex()
    {
        return {};
    }

};

struct Predecessors
{
    using value_type = boost::graph_traits<StateGraph>::vertex_descriptor;
    using reference = value_type&;
    using key_type = boost::graph_traits<StateGraph>::vertex_descriptor;
    using category = boost::writable_property_map_tag;
    using data_type = std::map<key_type, value_type>;

    friend void put(Predecessors& pmap, key_type k, value_type v)
    {
        (*pmap.data).insert({k, v});

        if (k->isFinal())
            pmap.sink(k);
    }

    Predecessors(data_type* dptr, SolCoro::push_type& sink) : data(dptr), sink(sink) {}

private:
    data_type* data;
    SolCoro::push_type& sink;
};


struct Queue
{
    using value_type = 	boost::graph_traits<StateGraph>::vertex_descriptor;
    using size_type = std::size_t;

    void push(const value_type& t)	{ data.push(t); }
    void pop() { data.pop(); }
    value_type& top() { return data.front(); }
    const value_type& top() const { return data.front(); }
    size_type size() const { return data.size(); }
    bool empty() const { return data.empty(); }

private:
    std::queue<value_type> data;
};

struct Colors
{
    using value_type = boost::default_color_type;
    using reference = value_type&;
    using key_type = boost::graph_traits<StateGraph>::vertex_descriptor;
    using category = boost::read_write_property_map_tag;
    using data_type = std::map<key_type, value_type>;

    friend void put(Colors& pmap, key_type k, value_type v)
    {
        (*pmap.data)[k] = v;
    }

    friend value_type get(Colors& pmap, key_type k)
    {
        auto [it, ok] = pmap.data->insert({k, value_type::white_color});

        return it->second;
    }

    Colors(data_type* dptr) : data(dptr) {}

private:
    data_type* data;
};


boost::graph_traits<StateGraph>::vertex_descriptor
source(boost::graph_traits<StateGraph>::edge_descriptor e, StateGraph const& /*g*/)
{
    return e.first;
}

boost::graph_traits<StateGraph>::vertex_descriptor
target(boost::graph_traits<StateGraph>::edge_descriptor e, StateGraph const& /*g*/)
{
    return e.second;
}

std::pair<boost::graph_traits<StateGraph>::out_edge_iterator, boost::graph_traits<StateGraph>::out_edge_iterator>
out_edges(boost::graph_traits<StateGraph>::vertex_descriptor v, StateGraph const& /*g*/)
{
    return v->outEdges();
}

boost::graph_traits<StateGraph>::degree_size_type
out_degree(boost::graph_traits<StateGraph>::vertex_descriptor v, StateGraph const& /*g*/)
{
    return v->outDegree();
}

class Task
{
public:


    Task(std::string const& filename)
        : m_streets(std::make_shared<Streets>())
    {
        std::ifstream ifs(filename);

        if (!ifs)
            throw std::runtime_error("Can't open file " + filename);

        std::map<char, Pet::Builder> pet_builders;
        std::string::size_type line_size = 0;

        auto lines = readLines(ifs);

        for (decltype(lines)::size_type row = 0, numrows = lines.size(); row < numrows; ++row )
        {
            auto const& line = lines[row];

            if (line_size == 0)
                line_size = line.size();
            else if (line_size != line.size())
                throw std::runtime_error("Lines of different lengths in input file");

            std::vector<char> street;

            for (std::string::size_type col = 0; col < line_size; ++col)
            {
                auto c = line[col];

                if (isAnimalOrHouse(c))
                {
                    auto& pb = pet_builders[Pet::asAnimalName(c)];

                    pb.addPos(c, { row, col });

                    street.push_back(ROAD);
                }
                else if (c == CAR)
                {
                    m_car = {row, col};
                    street.push_back(ROAD);
                }
                else if (c != ROAD && c != WAY && c != NOWAY)
                    throw std::runtime_error("Bad char in input file");
                else
                    street.push_back(c);
            }

            m_streets->push_back(std::move(street));
        }

        m_pets.reserve(pet_builders.size());

        std::for_each(pet_builders.begin(), pet_builders.end(), [this](auto& pb) {
            m_pets.push_back(pb.second.build());
        });

        if (m_car == INVALID_POSITION)
            throw std::runtime_error("No car position specified.");
    }

    StatePath Solve() const
    {
        StatePath solpath;

        StateGraph g;
        StateRegistryPtr statereg = std::make_shared<StateRegistry>();
        StatePtr inistate = State::addState(statereg, m_streets, std::make_tuple(m_pets, m_car));
        Queue buf;
        Predecessors::data_type preds;
        Colors::data_type colors;

        SolCoro::pull_type solcoro([&](SolCoro::push_type& sink){
            boost::breadth_first_visit(g,
                                       inistate,
                                       buf,
                                       boost::make_bfs_visitor(boost::record_predecessors(Predecessors(&preds, sink),
                                                                                          boost::on_tree_edge())),
                                       Colors(&colors));

        });

        for (StatePtr s = solcoro.get(); s != inistate; s = preds[s])
            solpath.push_front(s);

        solpath.push_front(inistate);

        return solpath;
    }

private:
    StreetsPtr m_streets;
    Pets m_pets;
    Position m_car = INVALID_POSITION;
};

#ifndef TEST

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usasge: " << argv[0] << " <task_filename> [...]" << std::endl;
        return EXIT_SUCCESS;
    }

    std::vector<std::future<Solution>> futures(static_cast<std::size_t>(argc - 1));

    for (std::size_t i = 1; i < static_cast<std::size_t>(argc); ++i)
    {
        futures[i - 1] = std::async(std::launch::async, [filename = argv[i]](){
            try {
                Task task(filename);
                return Solution{ filename, task.Solve() };
            } catch (...) {
                return Solution{filename, {}, std::current_exception() };
            }
        });
    }


    for (auto& ftr : futures)
    {
        Solution sol = ftr.get();

        std::cout << "--- " << sol.filename << " ---" << std::endl;

        try {
            if (sol.error)
                std::rethrow_exception(sol.error);
        } catch (std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        int i = 0;
        for (auto const& step : sol.sol)
            std::cout << "step #" << i++ << '\n' << *step << std::endl;

        std::cout << sol.filename << ": solved in " << i - 1 << '\n' << std::endl;
    }

    return EXIT_SUCCESS;
}

#endif // TEST
