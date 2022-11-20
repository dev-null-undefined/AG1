#define AVL_TREE_TESTING 1

#include "./avl_tree.hpp"
#include <set>
#include <iostream>
#include <vector>
#include <variant>
#include <ctime>
#include <source_location>
#include <random>

using namespace stl;
using namespace std;

std::normal_distribution<> distribution{10, 10};

std::mt19937 * mt;

int rng() {
    return floor(distribution(*mt));
}

int TEST_CONSTANT = 500;

enum Color {
    FG_RED = 31,
    FG_GREEN = 32,
    FG_BLUE = 34,
    FG_DEFAULT = 39,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_BLUE = 44,
    BG_DEFAULT = 49
};

std::ostream & operator<<(std::ostream & os, const Color & mod) {
    return os << "\033[" << std::to_string((int) mod) << "m";
}

template<typename Tp>
inline void DoNotOptimize(Tp & value) {
    asm volatile("" : "+r,m"(value) : : "memory");
}


std::unordered_map<std::string, std::optional<std::string>> tests;

template<typename ... Args>
std::string string_format(const std::string & format, Args ... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return {buf.get(), buf.get() + size - 1}; // We don't want the '\0' inside
}

template<typename T>
struct has_operator_print {
    enum {
        value = !std::is_same<decltype(std::cout << declval<T>()), decltype(std::cout)>::value
    };
};

template<typename T>
inline constexpr bool has_operator_print_v = has_operator_print<T>::value;

template<typename A>
std::string toString(A a) {
    if constexpr (has_operator_print_v<A>) {
        std::ostringstream oss;
        oss << a;
        return oss.str();
    } else {
        return string_format("Failed: %p", &a);
    }
}


template<typename A, typename B>
void iterative_data_test_iterators(A b1, A e1, B b2, B e2, const string & test_name) {
    for (; b1 != e1; ++b1, ++b2) {
        if (*b1 != *b2) {
            if constexpr (std::is_trivial_v<decltype(*b1)>) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
            } else {
                tests[test_name] = string_format("Non trivial %s != %s", toString(*b1).c_str(), toString(*b2).c_str());
            }
            return;
        }
    }
    if (b2 != e2) {
        tests[test_name] = "failed not at the end of set";
        return;
    }
}


struct Tester {
    std::unique_ptr<int> value = std::make_unique<int>();

    Tester(int value) {
        *this->value = value;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "C" << " : " << *this->value << Color::FG_DEFAULT << endl;
        }
    }

    Tester(const Tester & other) {
        *this->value = *other.value;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "C&" << " : " << *value << Color::FG_DEFAULT << endl;
        }
    }

    Tester(Tester && other) noexcept {
        *this->value = *other.value;
        *other.value = -1;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "C&&" << " : " << *value << Color::FG_DEFAULT << endl;
        }
    }

    ~Tester() {
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "~" << " : " << *value << Color::FG_DEFAULT << endl;
        }
        *value = -1;
    }

    Tester & operator=(const Tester & other) {
        *this->value = *other.value;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "=&" << " : " << *value << Color::FG_DEFAULT << endl;
        }
        return *this;
    }

    Tester & operator=(Tester && other) noexcept {
        *this->value = *other.value;
        *other.value = -1;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "=&&" << " : " << *value << Color::FG_DEFAULT << endl;
        }
        return *this;
    }

    bool operator<(const Tester & other) const {
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << *other.value << "<" << *value << Color::FG_DEFAULT << endl;
        }
        return *value < *other.value;
    }

    bool operator!=(const Tester & other) const {
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << *other.value << "!=" << *value << Color::FG_DEFAULT << endl;
        }
        return *value != *other.value;
    }

    friend ostream & operator<<(ostream & os, const Tester & tester) {
        return os << *tester.value;
    }

    static bool ENABLE_PRINT;
};

bool Tester::ENABLE_PRINT = false;


template<typename A, typename B>
void iterative_data_test(A a, B b, const string & test_name, bool reverse = false) {
    if (!reverse) {
        if constexpr (std::is_const_v<std::remove_reference_t<A>>) {
            iterative_data_test_iterators(a.cbegin(), a.cend(), b.cbegin(), b.cend(), test_name);
        } else {
            iterative_data_test_iterators(a.begin(), a.end(), b.begin(), b.end(), test_name);
        }
    } else {
        if constexpr (std::is_const_v<std::remove_reference_t<A>>) {
            iterative_data_test_iterators(a.crbegin(), a.crend(), b.crbegin(), b.crend(), test_name);
        } else {
            iterative_data_test_iterators(a.rbegin(), a.rend(), b.rbegin(), b.rend(), test_name);
        }
    }
}

template<typename A, typename B>
void insert_random(A a, B b, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        int random = rng();
        a.insert(random);
        b.emplace(random);
    }
}

void test_iterators() {
    string test_name = std::source_location::current().function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<int> int_tree;
        set<int> int_tree_ref;
        insert_random(int_tree, int_tree_ref, j);
        iterative_data_test<AVLTree<int> &>(int_tree, int_tree_ref, test_name);
    }
}


void test_reverse_iterators() {
    string test_name = std::source_location::current().function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<int> int_tree;
        set<int> int_tree_ref;
        insert_random(int_tree, int_tree_ref, j);
        iterative_data_test<AVLTree<int> &>(int_tree, int_tree_ref, test_name, true);
    }
}

void test_const_iterators() {
    string test_name = std::source_location::current().function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<int> int_tree;
        set<int> int_tree_ref;
        insert_random(int_tree, int_tree_ref, j);
        iterative_data_test<const AVLTree<int> &, const std::set<int> &>(int_tree, int_tree_ref, test_name);
    }
}

void test_const_reverse_iterators() {
    string test_name = std::source_location::current().function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<int> int_tree;
        set<int> int_tree_ref;
        insert_random(int_tree, int_tree_ref, j);
        iterative_data_test<const AVLTree<int> &, const std::set<int> &>(int_tree, int_tree_ref, test_name, true);
    }
}


void test_copy() {
    Tester::ENABLE_PRINT = false;
    string test_name = std::source_location::current().function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<Tester> int_tree;
        set<Tester> int_tree_ref;
        insert_random(int_tree, int_tree_ref, j);
        AVLTree<Tester> int_tree_copy(int_tree);
        set<Tester> int_tree_copy_ref(int_tree_ref);
        insert_random(int_tree_copy, int_tree_copy_ref, j);
        iterative_data_test<AVLTree<Tester> &>(int_tree, int_tree_ref, test_name);
        iterative_data_test<AVLTree<Tester> &>(int_tree_copy, int_tree_copy_ref, test_name);
    }
}

void test_move() {
    Tester::ENABLE_PRINT = false;
    string test_name = std::source_location::current().function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<Tester> int_tree;
        set<Tester> int_tree_ref;
        insert_random(int_tree, int_tree_ref, j);
        iterative_data_test<AVLTree<Tester> &>(int_tree, int_tree_ref, test_name);
        AVLTree<Tester> int_tree_copy(std::move(int_tree));
        set<Tester> int_tree_copy_ref(std::move(int_tree_ref));
        iterative_data_test<AVLTree<Tester> &>(int_tree_copy, int_tree_copy_ref, test_name);
        insert_random(int_tree_copy, int_tree_copy_ref, j);
        iterative_data_test<AVLTree<Tester> &>(int_tree_copy, int_tree_copy_ref, test_name);
    }
}

void test_assign_move() {
    Tester::ENABLE_PRINT = false;
    string test_name = std::source_location::current().function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<Tester> int_tree;
        set<Tester> int_tree_ref;
        insert_random(int_tree, int_tree_ref, j);
        iterative_data_test<AVLTree<Tester> &>(int_tree, int_tree_ref, test_name);
        AVLTree<Tester> int_tree_copy;
        set<Tester> int_tree_copy_ref;
        iterative_data_test<AVLTree<Tester> &>(int_tree_copy, int_tree_copy_ref, test_name);
        insert_random(int_tree_copy, int_tree_copy_ref, j);
        int_tree_copy = std::move(int_tree);
        int_tree_copy_ref = std::move(int_tree_ref);
        iterative_data_test<AVLTree<Tester> &>(int_tree_copy, int_tree_copy_ref, test_name);
        insert_random(int_tree_copy, int_tree_copy_ref, j);
        iterative_data_test<AVLTree<Tester> &>(int_tree_copy, int_tree_copy_ref, test_name);
    }
}

void test_assign_copy() {
    Tester::ENABLE_PRINT = false;
    string test_name = std::source_location::current().function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<Tester> int_tree;
        set<Tester> int_tree_ref;
        insert_random(int_tree, int_tree_ref, j);
        iterative_data_test<AVLTree<Tester> &>(int_tree, int_tree_ref, test_name);
        AVLTree<Tester> int_tree_copy;
        set<Tester> int_tree_copy_ref;
        insert_random(int_tree_copy, int_tree_copy_ref, j);
        iterative_data_test<AVLTree<Tester> &>(int_tree, int_tree_ref, test_name);
        iterative_data_test<AVLTree<Tester> &>(int_tree_copy, int_tree_copy_ref, test_name);
        int_tree = int_tree_copy;
        int_tree_ref = int_tree_copy_ref;
        iterative_data_test<AVLTree<Tester> &>(int_tree, int_tree_ref, test_name);
        iterative_data_test<AVLTree<Tester> &>(int_tree_copy, int_tree_copy_ref, test_name);
        insert_random(int_tree_copy, int_tree_copy_ref, j);
        iterative_data_test<AVLTree<Tester> &>(int_tree, int_tree_ref, test_name);
        iterative_data_test<AVLTree<Tester> &>(int_tree_copy, int_tree_copy_ref, test_name);
    }
}

int inner_count_test(const std::vector<Tester> & values, const std::set<Tester> & tree_ref,
                     const stl::AVLTree<Tester> & tree) {
    string test_name = "find";
    if (values.empty()) return 0;
    for (int i = 0; i < TEST_CONSTANT; ++i) {
        int random = rng();
        const Tester & find = values[random % values.size()];
        unsigned long a = tree.count(find);
        unsigned long b = tree_ref.count(find);
        DoNotOptimize(a);
        DoNotOptimize(b);
        if ((a) != (b)) {
            tests[test_name] = string_format("value list %d != %d", a, b);
            return 1;
        }
        a = tree.count(random);
        b = tree_ref.count(random);
        DoNotOptimize(a);
        DoNotOptimize(b);
        if (a != b) {
            tests[test_name] = string_format("random %d != %d", tree.count(random), tree_ref.count(random));
            return 1;
        }
    }
    return 0;
}


void test_find() {
    Tester::ENABLE_PRINT = false;
    string test_name = std::source_location::current().function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; j += TEST_CONSTANT) {
        AVLTree<Tester> int_tree;
        set<Tester> int_tree_ref;
        vector<Tester> values;
        insert_random(int_tree, int_tree_ref, j);
        for (const auto & item : int_tree_ref) values.emplace_back(item);
        DoNotOptimize(int_tree);
        DoNotOptimize(int_tree_ref);
        DoNotOptimize(values);
        int error = inner_count_test(values, int_tree_ref, int_tree);
        if (error) return;
    }
}

int main() {
    std::random_device rd;
    mt = new std::mt19937(rd());

    using namespace stl;
    using namespace std;
//    AVLTree<int> compileall;
//    for (int i = 0; i < 200; ++i) {
//        compileall.insert(rng());
//        compileall.toGraphViz(string_format("grapth%d.dot", i));
//    }

    test_find();
    test_iterators();
    test_reverse_iterators();
    test_const_iterators();
    test_const_reverse_iterators();
    test_copy();
    test_move();
    test_assign_copy();
    test_assign_move();

    for (auto & [key, error] : tests) {
        if (error) {
            cout << Color::FG_RED << "Failed: " << Color::FG_DEFAULT << key << ", Error:" << error.value() << endl;
        } else {
            cout << Color::FG_GREEN << "Pog: " << Color::FG_DEFAULT << key << endl;
        }
    }

    return 0;
}


