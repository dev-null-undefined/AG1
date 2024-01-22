#ifndef __PROGTEST__

#include <cassert>
#include <iomanip>
#include <cstdint>
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
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>

using Price = unsigned long long;
using Employee = size_t;
inline constexpr Employee NO_EMPLOYEE = -1;
using Gift = size_t;

#endif

struct Solution {
    Gift gift;
    Price sum;
};

struct DP {
    bool visited = false;
    std::array<Solution, 2> best_solution;
};

template<typename T, size_t N, typename Compare>
class BestN {
    Compare _compare;
    std::array<T, N> _best;
    size_t _size = 0;

public:
    explicit BestN(Compare compare) : _compare(compare) {}

    void insert(T t) {
        if (_size < N) {
            _best[_size++] = t;
            std::push_heap(_best.begin(), _best.begin() + _size, _compare);
        } else if (_compare(t, _best[0])) {
            std::pop_heap(_best.begin(), _best.end(), _compare);
            _best[N - 1] = t;
            std::push_heap(_best.begin(), _best.end(), _compare);
        }
    }

    const std::array<T, N> &best() {
        std::sort(_best.begin(), _best.end(), _compare);
        return _best;
    }
};

template<typename T, size_t N, typename Compare>
BestN<T, N, Compare> make_best_n(Compare compare) {
    return BestN<T, N, Compare>(compare);
}

void optimize_tree(Employee start, const std::vector<std::vector<Employee>> &subordinates,
                   const std::vector<Price> &gift_price, std::vector<DP> &dp) {
    std::stack<Employee> q;
    q.push(start);
    while (!q.empty()) {
        Employee c = q.top();
        if (dp[c].visited) {
            q.pop();
            auto compare = [](const Solution &a, const Solution &b) { return a.sum < b.sum; };
            auto solutions = make_best_n<Solution, 2>(compare);
            for (size_t gift = 0; gift < gift_price.size(); ++gift) {
                Price sum = gift_price[gift];
                for (Employee e: subordinates[c]) {
                    size_t non_colliding_gift = 0;
                    while (dp[e].best_solution[non_colliding_gift].gift == gift) non_colliding_gift++;
                    sum += dp[e].best_solution[non_colliding_gift].sum;
                }
                solutions.insert({gift, sum});
            }
            dp[c].best_solution = solutions.best();
        } else {
            dp[c].visited = true;
            for (Employee e: subordinates[c]) {
                q.push(e);
            }
        }
    }
}

std::pair<Price, std::vector<Gift>> optimize_gifts(
        const std::vector<Employee> &boss,
        const std::vector<Price> &gift_price
) {
    std::vector<std::vector<Employee>> subordinates(boss.size());
    std::vector<Employee> bosses;
    for (Employee e = 0; e < boss.size(); e++) {
        if (boss[e] != NO_EMPLOYEE) {
            subordinates[boss[e]].push_back(e);
        } else {
            bosses.push_back(e);
        }
    }
    std::vector<DP> dp(boss.size());
    for (Employee e: bosses) {
        optimize_tree(e, subordinates, gift_price, dp);
    }

    Price sum = 0;

    std::queue<std::pair<Employee, Gift>> q;
    for (Employee e: bosses) {
        q.emplace(e, dp[e].best_solution[0].gift);
        sum += dp[e].best_solution[0].sum;
    }
    std::vector<Gift> gifts(boss.size());
    while (!q.empty()) {
        auto [empl, gift] = q.front();
        q.pop();
        gifts[empl] = gift;
        for (Employee s: subordinates[empl]) {
            size_t non_colliding_gift = 0;
            while (dp[s].best_solution[non_colliding_gift].gift == gift) non_colliding_gift++;
            q.emplace(s, dp[s].best_solution[non_colliding_gift].gift);
        }
    }

    return {sum, gifts};
}

#ifndef __PROGTEST__

const std::tuple<Price, std::vector<Employee>, std::vector<Price>> EXAMPLES[] = {
        {17, {1, 2, 3, 4, NO_EMPLOYEE},       {25, 4, 18, 3}},
        {16, {4, 4, 4, 4, NO_EMPLOYEE},       {25, 4, 18, 3}},
        {17, {4, 4, 3, 4, NO_EMPLOYEE},       {25, 4, 18, 3}},
        {24, {4, 4, 3, 4, NO_EMPLOYEE, 3, 3}, {25, 4, 18, 3}},
};

#define CHECK(cond, ...) do { \
    if (cond) break; \
    printf("Test failed: " __VA_ARGS__); \
    printf("\n"); \
    return false; \
  } while (0)

bool test(Price p, const std::vector<Employee> &boss, const std::vector<Price> &gp) {
    auto &&[sol_p, sol_g] = optimize_gifts(boss, gp);
    CHECK(sol_g.size() == boss.size(),
          "Size of the solution: expected %zu but got %zu.", boss.size(), sol_g.size());

    Price real_p = 0;
    for (Gift g: sol_g) real_p += gp[g];
    CHECK(real_p == sol_p, "Sum of gift prices is %llu but reported price is %llu.", real_p, sol_p);

    if (0) {
        for (Employee e = 0; e < boss.size(); e++) printf(" (%zu)%zu", e, sol_g[e]);
        printf("\n");
    }

    for (Employee e = 0; e < boss.size(); e++)
        CHECK(boss[e] == NO_EMPLOYEE || sol_g[boss[e]] != sol_g[e],
              "Employee %zu and their boss %zu has same gift %zu.", e, boss[e], sol_g[e]);

    CHECK(p == sol_p, "Wrong price: expected %llu got %llu.", p, sol_p);

    return true;
}

#undef CHECK

int main() {
    int ok = 0, fail = 0;
    for (auto &&[p, b, gp]: EXAMPLES) (test(p, b, gp) ? ok : fail)++;

    if (!fail) printf("Passed all %d tests!\n", ok);
    else printf("Failed %d of %d tests.", fail, fail + ok);
}

#endif


