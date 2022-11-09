#ifndef __PROGTEST__

#include <cassert>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <climits>

using Place = size_t;

struct Map {
    size_t places;
    Place start, end;
    std::vector<std::pair<Place, Place>> connections;
    std::vector<std::vector<Place>> items;
};

template<typename F, typename S>
struct std::hash<std::pair<F, S>> {
    std::size_t operator()(const std::pair<F, S> & p) const noexcept {
        // something like boost::combine would be much better
        return std::hash<F>()(p.first) ^ (std::hash<S>()(p.second) << 1);
    }
};


#endif

const size_t max_size = 12;

struct node {
    std::bitset<max_size> items;
    bool end = false;
};

struct node_state {
    std::pair<Place, std::bitset<max_size>> state;
    size_t length;

    bool operator<(const node_state & rhs) const {
        return length < rhs.length;
    }

    bool operator>(const node_state & rhs) const {
        return length > rhs.length;
    }

};

std::unordered_map<Place, size_t>
reduce_graph(Place start, std::vector<std::vector<size_t>> & graph, std::vector<node> & nodes) {
    std::unordered_map<Place, size_t> result;
    std::unordered_map<Place, size_t> distance;
    std::queue<Place> queue;
    queue.emplace(start);
    distance[start] = 0;
    while (!queue.empty()) {
        Place current = queue.front();
        queue.pop();
        size_t currentDistance = distance[current];
        for (auto neighbour : graph[current]) {
            if (!distance.count(neighbour)) {
                distance.emplace(neighbour, currentDistance + 1);
                if (nodes[neighbour].items.to_ulong() || nodes[neighbour].end) {
                    result[neighbour] = currentDistance + 1;
                }
                queue.push(neighbour);
            }
        }
    }
    return result;
}

void steper(std::list<Place> & result, Place from, Place to, const std::vector<std::vector<size_t>> & graph) {
    std::unordered_map<Place, Place> previous;
    std::queue<Place> queue;
    queue.push(from);
    previous[from] = from;
    while (!queue.empty()) {
        Place current = queue.front();
        queue.pop();
        for (auto neighbour : graph[current]) {
            if (!previous.count(neighbour)) {
                previous[neighbour] = current;
                if (neighbour == to) {
                    while (previous.count(neighbour)) {
                        if (neighbour == previous[neighbour]) {
                            break;
                        } // stfu martine
                        result.push_front(neighbour);
                        neighbour = previous[neighbour];
                    }
                    return;
                }
                queue.push(neighbour);
            }
        }
    }
}

std::list<Place> find_path(const Map & map) {
    std::vector<node> nodes;
    nodes.resize(map.places);
    std::bitset<max_size> end_state = (1 << map.items.size()) - 1;
    std::vector<std::vector<size_t>> graph;
    nodes[map.end].end = true;
    graph.resize(map.places);
    for (size_t i = 0; i < map.places; ++i) {
        graph[i].push_back(i);
    }
    for (auto [from, to] : map.connections) {
        graph[from].push_back(to);
        graph[to].push_back(from);
    }
    for (size_t i = 0; i < map.items.size(); ++i) {
        for (const auto & room_id : map.items[i]) {
            nodes[room_id].items |= 1 << i;
        }
    }
    if (map.items.size() <= 6) {
        std::queue<node_state> queue;
        std::unordered_map<Place, std::vector<std::bitset<max_size>>> visited;
        std::unordered_map<std::pair<Place, std::bitset<12>>, std::pair<Place, std::bitset<12>>> previous_state;
        queue.push({{map.start, nodes[map.start].items}, 0});
        visited[map.start].push_back(nodes[map.start].items);
        while (!queue.empty()) {
            node_state current = queue.front();
            if (nodes[current.state.first].end && current.state.second == end_state) {
                std::list<Place> result;
                std::pair<Place, std::bitset<12>> current_state = current.state;
                while (previous_state.count(current_state)) {
                    result.push_front(current_state.first);
                    current_state = previous_state[current_state];
                }
                result.emplace_front(current_state.first);
                return result;
            }
            queue.pop();
            for (auto neighbour : graph[current.state.first]) {
                bool already_visited = false;
                for (const auto & visited_state : visited[neighbour]) {
                    if ((visited_state | current.state.second) == visited_state) {
                        already_visited = true;
                        break;
                    }
                }
                if (!already_visited) {
                    queue.push({{neighbour, (current.state.second | nodes[neighbour].items)}, 0});
                    visited[neighbour].push_back(current.state.second | nodes[neighbour].items);
                    previous_state[{neighbour, current.state.second | nodes[neighbour].items}] = current.state;
                }
            }
        }

        return {};
    } else {
        std::unordered_map<Place, std::unordered_map<Place, size_t>> reduced_graph;
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (i == map.start || i == map.end || nodes[i].items.to_ulong()) {
                reduced_graph[i] = reduce_graph(i, graph, nodes);
            }
        }


        std::priority_queue<node_state, std::vector<node_state>, std::greater<>> queue;
        std::unordered_map<std::pair<Place, std::bitset<12>>, std::pair<Place, std::bitset<12>>> previous_state;
        std::unordered_map<std::pair<Place, std::bitset<12>>, size_t> distance_map;
        queue.push({{map.start, nodes[map.start].items}, 0});
        distance_map.emplace(std::make_pair(map.start, nodes[map.start].items), 0);
        while (!queue.empty()) {
            node_state current = queue.top();

            if (nodes[current.state.first].end && current.state.second == end_state) {
                std::list<Place> result;
                std::pair<Place, std::bitset<12>> current_state = current.state;
                while (previous_state.count(current_state)) {
                    std::pair<Place, std::bitset<12>> next_state = previous_state[current_state];
                    steper(result, next_state.first, current_state.first, graph);
                    current_state = next_state;
                }
                result.emplace_front(current_state.first);
                return result;
            }
            queue.pop();

            for (const auto [neighbour, distance] : reduced_graph[current.state.first]) {
                auto pair = std::make_pair(neighbour, current.state.second | nodes[neighbour].items);
                if (!distance_map.count(pair) || distance_map[pair] > current.length + distance) {
                    queue.push({{neighbour, (pair.second)}, current.length + distance});
                    distance_map[std::make_pair(neighbour, (pair.second))] =
                            current.length + distance;
                    previous_state[pair] = {current.state.first, current.state.second};
                }
            }
        }
        return {};
    }
}


#ifndef __PROGTEST__

using TestCase = std::pair<size_t, Map>;

// Class template argument deduction exists since C++17 :-)
const std::array examples = {
        TestCase{4, Map{4, 0, 1,
                        {{0, 2}, {2, 3}, {0, 3}, {3, 1}},
                        {{2}}
        }},
        TestCase{1, Map{2, 0, 0,
                        {{0, 1}},
                        {{0}}
        }},
        TestCase{3, Map{2, 0, 0,
                        {{0, 1}},
                        {{1}}
        }},
        TestCase{3, Map{4, 0, 1,
                        {{0, 2}, {2, 3}, {0, 3}, {3, 1}},
                        {}
        }},
        TestCase{0, Map{4, 0, 1,
                        {{0, 2}, {2, 3}, {0, 3}, {3, 1}},
                        {{2}, {}}
        }},
};

int main() {
    int fail = 0;
    for (size_t i = 0; i < examples.size(); i++) {
        auto sol = find_path(examples[i].second).size();
        if (sol != examples[i].first) {
            std::cout << "Wrong anwer for map " << i << std::endl;
            fail++;
        }
    }

    if (fail) std::cout << "Failed " << fail << " tests" << std::endl;
    else std::cout << "All tests completed" << std::endl;

    return 0;
}

#endif
