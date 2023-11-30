#include <benchmark/benchmark.h>
#include <iostream>
#include <map>
#include <chrono>
#include <functional>
#include <random>
#include <queue>

auto prepare_data(benchmark::State &state) {
    int n_elements = state.range(0);
    auto data_generator_style = state.range(1);
    std::vector<int> v(n_elements);
    std::mt19937 gen(12345124); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> allNums(0, n_elements);
    std::uniform_int_distribution<> bigger(0, n_elements * 10'000);
    std::uniform_int_distribution<> smaller(0, n_elements / 1'000);
    std::normal_distribution<> normal(0, n_elements);
    auto normal_to_int = [&](double n) {
        return int((n - normal.min() / normal.max() - normal.min()) * n_elements);
    };
    std::generate(v.begin(), v.end(), [&]() -> int {
        switch (data_generator_style) {
            case 0:
                return allNums(gen);
            case 1:
                return normal_to_int(normal(gen));
            case 2:
                return smaller(gen);
            case 3:
                return bigger(gen);
            case 4:
                return 1000000;
            default:
                assert(false);
        }
    });
    return std::make_pair(v, state.range(2));
}

//#define mstatement(x) x
//#define massert(x) assert(x)

#define mstatement(x)
#define massert(x)

struct BinaryHeap {
    explicit BinaryHeap(std::vector<int> & data):_data(data) {
        for (int i = _data.size() / 2; i >= 0; --i) {
            _bubble_down(i);
        }
    }

    int extract_min() {
        int ret = std::move(_data[0]);
        _data[0] = std::move(_data.back());
        _data.pop_back();
        _bubble_down(0);
        return ret;
    }

    struct TestHelper {
        static size_t parent(size_t index) {
            return (index - 1) / 2;
        }

        static size_t child(size_t parent, size_t ith) {
            return 2 * parent + 1 + ith;
        }
    };

    void _bubble_down(size_t index) {
        size_t left = TestHelper::child(index, 0);
        size_t right = TestHelper::child(index, 1);
        size_t min = index;

        if (left < _data.size() && (_data[left] < _data[min]))
            min = left;
        if (right < _data.size() && (_data[right] < _data[min]))
            min = right;

        if (min != index) {
            std::swap(_data[index], _data[min]);
            _bubble_down(min);
        }
    }

    void _bubble_up(size_t index) {
        size_t parent = TestHelper::parent(index);
        if(index == 0) return;
        if (index != parent && (_data[index] < _data[parent])) {
            std::swap(_data[index], _data[parent]);
            _bubble_up(parent);
        }
    }

    std::vector<int> & _data;
};


static void heap(benchmark::State &state) {
    auto [data, smallest_count] = prepare_data(state);
    mstatement(auto sorted = data;)
    mstatement(std::sort(sorted.begin(), sorted.end());)
    for (auto _: state) {
        auto v = data;
        std::make_heap(v.begin(), v.end(), std::greater<>());
        for (int i = 0; i < smallest_count; ++i) {
            std::pop_heap(v.begin(), v.end(), std::greater<>());
            auto x = v.back();
            benchmark::DoNotOptimize(x);
            massert(x == sorted[i]);
            v.pop_back();
            benchmark::ClobberMemory();
        }
    }
}


static void my_heap(benchmark::State &state) {
    auto [data, smallest_count] = prepare_data(state);
    mstatement(auto sorted = data;)
    mstatement(std::sort(sorted.begin(), sorted.end());)
    for (auto _: state) {
        auto v = data;
        BinaryHeap heap(v);
        for (int i = 0; i < smallest_count; ++i) {
            auto x = heap.extract_min();
            benchmark::DoNotOptimize(x);
            massert(x == sorted[i]);
            benchmark::ClobberMemory();
        }
    }
}

static void priority_queue(benchmark::State &state) {
    auto [data, smallest_count] = prepare_data(state);
    mstatement(auto sorted = data;)
    mstatement(std::sort(sorted.begin(), sorted.end());)
    for (auto _: state) {
        auto v = data;
        std::priority_queue<int, std::vector<int>, std::greater<>> q(v.begin(), v.end());
        for (int i = 0; i < smallest_count; ++i) {
            auto x = q.top();
            massert(x == sorted[i]);
            benchmark::DoNotOptimize(x);
            q.pop();
            benchmark::ClobberMemory();
        }
    }
}

static void priority_queue_no_cpy(benchmark::State &state) {
    auto [data, smallest_count] = prepare_data(state);
    mstatement(auto sorted = data;)
    mstatement(std::sort(sorted.begin(), sorted.end());)
    for (auto _: state) {
        std::priority_queue<int, std::vector<int>, std::greater<>> q(data.begin(), data.end());
        for (int i = 0; i < smallest_count; ++i) {
            auto x = q.top();
            massert(x == sorted[i]);
            benchmark::DoNotOptimize(x);
            q.pop();
            benchmark::ClobberMemory();
        }
    }
}

static void sqrt(benchmark::State &state) {
    auto [data, smallest_count] = prepare_data(state);
    mstatement(auto sorted = data;)
    mstatement(std::sort(sorted.begin(), sorted.end());)
    auto sqrt_n = int(std::sqrt(data.size()));
    for (auto _: state) {
        auto v = data;
        std::vector<int> mins(sqrt_n);
        for (int part = 0; part < sqrt_n; ++part) {
            size_t segment_start = part * sqrt_n;
            size_t segment_end = part == sqrt_n - 1 ? v.size() : (part + 1) * sqrt_n;
            auto min = std::min_element(v.begin() + segment_start, v.begin() + segment_end);
            mins[part] = *min;
            *min = std::numeric_limits<int>::max();
            benchmark::ClobberMemory();
        }
        for (int i = 0; i < smallest_count; ++i) {
            auto x = std::min_element(mins.begin(), mins.end());
            auto min_value = *x;
            massert(min_value == sorted[i]);
            benchmark::DoNotOptimize(min_value);
            auto segment = x - mins.begin();
            auto segment_start = segment * sqrt_n;
            auto segment_end = segment == sqrt_n - 1 ? v.size() : (segment + 1) * sqrt_n;
            auto min = std::min_element(v.begin() + segment_start, v.begin() + segment_end);
            mins[segment] = *min;
            *min = std::numeric_limits<int>::max();
            benchmark::ClobberMemory();
        }
    }
}


static void sqrt_heap(benchmark::State &state) {
    auto [data, smallest_count] = prepare_data(state);
    mstatement(auto sorted = data;)
    mstatement(std::sort(sorted.begin(), sorted.end());)
    auto sqrt_n = int(std::sqrt(data.size()));
    auto compare = [&](auto a, auto b) {
        return a.first > b.first;
    };
    for (auto _: state) {
        auto v = data;
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, decltype(compare)> mins(compare);
        for (int part = 0; part < sqrt_n; ++part) {
            size_t segment_start = part * sqrt_n;
            size_t segment_end = part == sqrt_n - 1 ? v.size() : (part + 1) * sqrt_n;
            auto min = std::min_element(v.begin() + segment_start, v.begin() + segment_end);
            mins.emplace(*min, part);
            *min = std::numeric_limits<int>::max();
            benchmark::ClobberMemory();
        }
        for (int i = 0; i < smallest_count; ++i) {
            auto x = mins.top();
            mins.pop();
            auto min_value = x.first;
            massert(min_value == sorted[i]);
            benchmark::DoNotOptimize(min_value);
            auto segment = x.second;
            auto segment_start = segment * sqrt_n;
            auto segment_end = segment == sqrt_n - 1 ? v.size() : (segment + 1) * sqrt_n;
            auto min = std::min_element(v.begin() + segment_start, v.begin() + segment_end);
            mins.emplace(*min, segment);
            *min = std::numeric_limits<int>::max();
            benchmark::ClobberMemory();
        }
    }
}


static void giga_fast_boy(benchmark::State &state) {
    auto [data, smallest_count] = prepare_data(state);
    mstatement(auto sorted = data;)
    mstatement(std::sort(sorted.begin(), sorted.end());)
    auto sqrt_n = int(1024);
    auto part_count = data.size() / sqrt_n;
    auto compare = [&](auto a, auto b) {
        return a.first > b.first;
    };
    for (auto _: state) {
        auto v = data;
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, decltype(compare)> mins(compare);
        for (int part = 0; part < part_count; ++part) {
            size_t segment_start = part * sqrt_n;
            size_t segment_end = part == part_count - 1 ? v.size() : (part + 1) * sqrt_n;
            auto min = std::min_element(v.begin() + segment_start, v.begin() + segment_end);
            mins.emplace(*min, part);
            *min = std::numeric_limits<int>::max();
            benchmark::ClobberMemory();
        }
        for (int i = 0; i < smallest_count; ++i) {
            auto x = mins.top();
            mins.pop();
            auto min_value = x.first;
            massert(min_value == sorted[i]);
            benchmark::DoNotOptimize(min_value);
            auto segment = x.second;
            auto segment_start = segment * sqrt_n;
            auto segment_end = segment == part_count - 1 ? v.size() : (segment + 1) * sqrt_n;
            auto min = std::min_element(v.begin() + segment_start, v.begin() + segment_end);
            mins.emplace(*min, segment);
            *min = std::numeric_limits<int>::max();
            benchmark::ClobberMemory();
        }
    }
}


static void giga_fast_boy_2(benchmark::State &state) {
    auto [data, smallest_count] = prepare_data(state);
    mstatement(auto sorted = data;)
    mstatement(std::sort(sorted.begin(), sorted.end());)
    auto sqrt_n = int(64);
    auto part_count = data.size() / sqrt_n;
    auto compare = [&](auto a, auto b) {
        return a.first > b.first;
    };
    for (auto _: state) {
        auto v = data;
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, decltype(compare)> mins(compare);
        for (int part = 0; part < part_count; ++part) {
            size_t segment_start = part * sqrt_n;
            size_t segment_end = part == part_count - 1 ? v.size() : (part + 1) * sqrt_n;
            auto min = std::min_element(v.begin() + segment_start, v.begin() + segment_end);
            mins.emplace(*min, part);
            *min = std::numeric_limits<int>::max();
            benchmark::ClobberMemory();
        }
        for (int i = 0; i < smallest_count; ++i) {
            auto x = mins.top();
            mins.pop();
            auto min_value = x.first;
            massert(min_value == sorted[i]);
            benchmark::DoNotOptimize(min_value);
            auto segment = x.second;
            auto segment_start = segment * sqrt_n;
            auto segment_end = segment == part_count - 1 ? v.size() : (segment + 1) * sqrt_n;
            auto min = std::min_element(v.begin() + segment_start, v.begin() + segment_end);
            mins.emplace(*min, segment);
            *min = std::numeric_limits<int>::max();
            benchmark::ClobberMemory();
        }
    }
}

constexpr auto pow(auto x, int y) {
    decltype(x) result = 1;
    for (int i = 0; i < y; ++i) {
        result *= x;
    }
    return result;
}

static void CustomArguments(benchmark::internal::Benchmark *b) {
    for (int array_size = 1'000'000; array_size <= 10'000'000; array_size *= 10)
        for (int smallest_count = 1'000; smallest_count <= (array_size - 10); smallest_count *= 100)
            for (int generator_style = 0; generator_style < 5; ++generator_style)
                b->Args({array_size, generator_style, smallest_count});
}

// Register the function as a benchmark
BENCHMARK(my_heap)->Apply(CustomArguments);
BENCHMARK(heap)->Apply(CustomArguments);
BENCHMARK(priority_queue_no_cpy)->Apply(CustomArguments);
BENCHMARK(priority_queue)->Apply(CustomArguments);
BENCHMARK(sqrt)->Apply(CustomArguments);
BENCHMARK(sqrt_heap)->Apply(CustomArguments);
BENCHMARK(giga_fast_boy)->Apply(CustomArguments);
BENCHMARK(giga_fast_boy_2)->Apply(CustomArguments);

// Run the benchmark
BENCHMARK_MAIN();