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

std::uniform_int_distribution<size_t> distribution{0, 1000};

std::mt19937 * mt;

size_t rng() {
    return distribution(*mt);
}

size_t TEST_CONSTANT = 500;

namespace Color {
    namespace FG {
        enum Code {
            DEFAULT = 39,
            RED = 31,
            GREEN = 32,
            BLUE = 34,
            CYAN = 36,
            YELLOW = 33,
            MAGENTA = 35,
            BLACK = 30,
            WHITE = 37,
        };
    }
    namespace BG {
        enum Code {
            DEFAULT = 49,
            RED = 41,
            GREEN = 42,
            BLUE = 44,
            CYAN = 46,
            YELLOW = 43,
            MAGENTA = 45,
            BLACK = 40,
            WHITE = 47,
        };
    }
}

std::ostream & operator<<(std::ostream & os, const Color::FG::Code & mod) {
    return os << "\033[" << std::to_string((int) mod) << "m";
}

std::ostream & operator<<(std::ostream & os, const Color::BG::Code & mod) {
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

struct Stat {

    size_t constructor_count;
    size_t destructor_count;
    size_t copy_constructor_count;
    size_t move_constructor_count;
    size_t copy_assignment_count;
    size_t move_assignment_count;
    size_t operator_less_count;

    Stat() {
        constructor_count = 0;
        destructor_count = 0;
        copy_constructor_count = 0;
        move_constructor_count = 0;
        copy_assignment_count = 0;
        move_assignment_count = 0;
        operator_less_count = 0;
    }

    Stat & operator+=(const Stat & other) {
        constructor_count += other.constructor_count;
        destructor_count += other.destructor_count;
        copy_constructor_count += other.copy_constructor_count;
        move_constructor_count += other.move_constructor_count;
        copy_assignment_count += other.copy_assignment_count;
        move_assignment_count += other.move_assignment_count;
        operator_less_count += other.operator_less_count;
        return *this;
    }

    void print() const {
        cout << "constructor_count: " << constructor_count << endl;
        cout << "destructor_count: " << destructor_count << endl;
        cout << "copy_constructor_count: " << copy_constructor_count << endl;
        cout << "move_constructor_count: " << move_constructor_count << endl;
        cout << "copy_assignment_count: " << copy_assignment_count << endl;
        cout << "move_assignment_count: " << move_assignment_count << endl;
        cout << "operator_less_count: " << operator_less_count << endl;
    }

};


struct Tester {
    size_t value;
    static Stat stat;
    static Stat stat_ref;
    static Stat * current;
    static bool disable_stat;


    explicit Tester(size_t value) {
        if (!disable_stat) current->constructor_count++;
        this->value = value;
        if (ENABLE_PRINT > 5) {
            cout << Color::FG::GREEN << "C: " << this->value << Color::FG::DEFAULT << endl;
        }
    }

    Tester(const Tester & other) {
        if (!disable_stat) current->copy_constructor_count++;
        this->value = other.value;
        if (ENABLE_PRINT > 5) {
            cout << Color::FG::BLUE << "C&: " << value << Color::FG::DEFAULT << endl;
        }
    }

    Tester(Tester && other) noexcept {
        if (!disable_stat) current->move_constructor_count++;
        this->value = other.value;
        other.value = -1;
        if (ENABLE_PRINT > 5) {
            cout << Color::FG::CYAN << "C&&: " << value << Color::FG::DEFAULT << endl;
        }
    }

    ~Tester() {
        if (!disable_stat) current->destructor_count++;
        if (ENABLE_PRINT > 5) {
            cout << Color::FG::RED << "~: " << value << Color::FG::DEFAULT << endl;
        }
        value = -1;
    }

    Tester & operator=(const Tester & other) {
        if (!disable_stat) current->copy_assignment_count++;
        this->value = other.value;
        if (ENABLE_PRINT > 5) {
            cout << Color::FG::BLUE << "=&: " << value << Color::FG::DEFAULT << endl;
        }
        return *this;
    }

    Tester & operator=(Tester && other) noexcept {
        if (!disable_stat) current->move_assignment_count++;
        this->value = other.value;
        other.value = -1;
        if (ENABLE_PRINT > 5) {
            cout << Color::FG::CYAN << "=&&: " << value << Color::FG::DEFAULT << endl;
        }
        return *this;
    }

    bool operator<(const Tester & other) const {
        if (!disable_stat) current->operator_less_count++;
        if (ENABLE_PRINT > 5) {
            cout << Color::FG::MAGENTA << other.value << "<" << value << Color::FG::DEFAULT << endl;
        }
        return value < other.value;
    }

    // only used for checking if the tree is correct
    bool operator!=(const Tester & other) const {
        return value != other.value;
    }

    friend ostream & operator<<(ostream & os, const Tester & tester) {
        return os << tester.value;
    }

    static void reset() {
        set_ref(false);
        stat = Stat();
        stat_ref = Stat();
    }

    static void set_ref(bool value) {
        is_ref = value;
        current = is_ref ? &stat_ref : &stat;
    }

    static size_t ENABLE_PRINT;
    static bool is_ref;
};

Stat Tester::stat = Stat();
bool Tester::disable_stat = false;
Stat Tester::stat_ref = Stat();
Stat * Tester::current = &Tester::stat;
bool Tester::is_ref = false;

size_t Tester::ENABLE_PRINT = 0;


template<typename A, typename B>
bool iterative_data_test_iterators(A b1, A e1, B b2, B e2, const string & test_name) {
    for (; b1 != e1;) {
        if (b2 == e2) {
            tests[test_name] = "Failed iterator of reference ended before us";
            return false;
        }
        Tester::set_ref(false);
        typename A::reference a = *b1;
        Tester::set_ref(true);
        typename B::reference b = *b2;
        if (a != b) {
            if constexpr (std::is_trivial_v<decltype(*b1)>) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
            } else {
                tests[test_name] = string_format("Non trivial %s != %s", toString(*b1).c_str(), toString(*b2).c_str());
            }
            return false;
        }

        Tester::set_ref(false);
        ++b1;
        Tester::set_ref(true);
        ++b2;
    }
    if (b2 != e2) {
        tests[test_name] = "failed not at the end of set";
        return false;
    }
    return true;
}


template<typename A, typename B>
bool iterative_data_test(A a, B b, const string & test_name, bool reverse = false) {
    if (!reverse) {
        if constexpr (std::is_const_v<std::remove_reference_t<A>>) {
            return iterative_data_test_iterators(a.cbegin(), a.cend(), b.cbegin(), b.cend(), test_name);
        } else {
            return iterative_data_test_iterators(a.begin(), a.end(), b.begin(), b.end(), test_name);
        }
    } else {
        if constexpr (std::is_const_v<std::remove_reference_t<A>>) {
            return iterative_data_test_iterators(a.crbegin(), a.crend(), b.crbegin(), b.crend(), test_name);
        } else {
            return iterative_data_test_iterators(a.rbegin(), a.rend(), b.rbegin(), b.rend(), test_name);
        }
    }
}

template<typename A, typename B>
void insert_random(A & a, B & b, size_t count) {
    if (Tester::ENABLE_PRINT > 5)
        cout << endl << endl << "Inserting " << count << " elements" << endl;
    for (size_t i = 0; i < count; ++i) {
        size_t random = rng();
        if (Tester::ENABLE_PRINT > 5)
            cout << "inserting " << random << endl;
        if (Tester::ENABLE_PRINT > 5)
            cout << "AVLTree insert" << endl;
        Tester::set_ref(false);
        a.insert(random);
        if (Tester::ENABLE_PRINT > 5)
            cout << "----------------" << endl;
        if (Tester::ENABLE_PRINT > 5)
            cout << "Set insert" << endl;
        Tester::set_ref(true);
        b.emplace(random);
        if (Tester::ENABLE_PRINT > 5)
            cout << "----------------" << endl << endl;
    }
    if (Tester::ENABLE_PRINT > 5) {
        cout << Color::FG::BLUE << "insert_stat" << Color::FG::YELLOW << endl;
        Tester::stat.print();
        cout << endl << Color::FG::BLUE << "insert_stat_ref" << Color::FG::YELLOW << endl;
        Tester::stat_ref.print();
        cout << Color::FG::DEFAULT << endl;
    }
}

template<typename Function>
void testbed(const Function & test, const source_location & location) {
    string test_name = location.function_name();
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    Tester::disable_stat = true;
    for (size_t j = TEST_CONSTANT - 1; j < TEST_CONSTANT; ++j) {
        if (j == TEST_CONSTANT - 1) {
            Tester::disable_stat = false;
            Tester::reset();
        }
        if (!test(j, test_name)) {
            assert(tests[test_name].has_value());
            return;
        } else {
            assert(!tests[test_name].has_value());
        }
        if (j == TEST_CONSTANT - 1) {
            cout << Color::FG::BLUE << "insert_stat" << Color::FG::YELLOW << endl;
            Tester::stat.print();
            cout << endl << Color::FG::BLUE << "insert_stat_ref" << Color::FG::YELLOW << endl;
            Tester::stat_ref.print();
            cout << Color::FG::DEFAULT << endl;

            Tester::disable_stat = true;
        }
    }
    Tester::ENABLE_PRINT = 0;
}

void test_iterators() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}


void test_reverse_iterators() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name, true)) return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}

void test_const_iterators() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<const AVLTree<Tester> &, const std::set<Tester> &>(a, b, test_name)) return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}

void test_const_reverse_iterators() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<const AVLTree<Tester> &, const std::set<Tester> &>(a, b, test_name, true))
            return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}


void test_copy() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        Tester::set_ref(false);
        AVLTree<Tester> a_copy(a);
        Tester::set_ref(true);
        set<Tester> b_copy(b);
        insert_random(a_copy, b_copy, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_copy, b_copy, test_name)) return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}

void test_move() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;

        Tester::set_ref(false);
        AVLTree<Tester> a_copy(a);
        Tester::set_ref(true);
        set<Tester> b_copy(b);
        insert_random(a_copy, b_copy, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_copy, b_copy, test_name)) return false;
        Tester::set_ref(false);
        AVLTree<Tester> a_move(std::move(a_copy));
        Tester::set_ref(true);
        set<Tester> b_move(std::move(b_copy));
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_move, b_move, test_name)) return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}

void test_assign_move() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        Tester::set_ref(false);
        AVLTree<Tester> a_copy(a);
        Tester::set_ref(true);
        set<Tester> b_copy(b);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_copy, b_copy, test_name)) return false;
        Tester::set_ref(false);
        AVLTree<Tester> a_move;
        Tester::set_ref(true);
        set<Tester> b_move;
        insert_random(a_move, b_move, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_move, b_move, test_name)) return false;
        Tester::set_ref(false);
        a_move = std::move(a_copy);
        Tester::set_ref(true);
        b_move = std::move(b_copy);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_move, b_move, test_name)) return false;
        Tester::disable_stat = true;
        if (!iterative_data_test<std::set<Tester> &, std::set<Tester> &>(b, b_move, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, AVLTree<Tester> &>(a, a_move, test_name)) return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}

void test_assign_copy() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        Tester::set_ref(false);
        AVLTree<Tester> a_copy(a);
        Tester::set_ref(true);
        set<Tester> b_copy(b);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_copy, b_copy, test_name)) return false;
        Tester::set_ref(false);
        AVLTree<Tester> a_help;
        Tester::set_ref(true);
        set<Tester> b_help;
        insert_random(a_help, b_help, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_help, b_help, test_name)) return false;
        Tester::set_ref(false);
        a_help = a;
        Tester::set_ref(true);
        b_help = b;
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_help, b_help, test_name)) return false;
        Tester::disable_stat = true;
        if (!iterative_data_test<std::set<Tester> &, std::set<Tester> &>(b, b_help, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, AVLTree<Tester> &>(a, a_help, test_name)) return false;
        Tester::disable_stat = false;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a_help, b_help, test_name)) return false;
        Tester::disable_stat = true;
        if (!iterative_data_test<std::set<Tester> &, std::set<Tester> &>(b_copy, b_help, test_name)) return false;
        if (!iterative_data_test<AVLTree<Tester> &, AVLTree<Tester> &>(a_copy, a_help, test_name)) return false;
        return true;
    }, std::source_location::current());
}

bool inner_count_test(const std::vector<Tester> & values, const std::set<Tester> & tree_ref,
                      const stl::AVLTree<Tester> & tree) {
    bool before = Tester::disable_stat;
    string test_name = "find";
    if (values.empty()) return true;

    for (size_t j = 0; j < TEST_CONSTANT; ++j) {
        Tester::disable_stat = before;
        if (j == TEST_CONSTANT - 1) {
            Tester::ENABLE_PRINT = 1;
        }
        size_t random = rng();
        const Tester & find = values[random % values.size()];
        Tester::set_ref(false);
        unsigned long a = tree.count(find);
        Tester::set_ref(true);
        unsigned long b = tree_ref.count(find);
        DoNotOptimize(a);
        DoNotOptimize(b);
        if ((a) != (b)) {
            tests[test_name] = string_format("value list %d != %d", a, b);
            return false;
        }

        Tester::disable_stat = true;
        Tester to_find(random);
        Tester::disable_stat = before;

        Tester::set_ref(false);
        a = tree.count(to_find);
        Tester::set_ref(true);
        b = tree_ref.count(to_find);
        DoNotOptimize(a);
        DoNotOptimize(b);
        if (a != b) {
            tests[test_name] = string_format("random %d != %d", tree.count(to_find),
                                             tree_ref.count(to_find));
            return false;
        }
        Tester::disable_stat = true;
    }
    return true;
}

void test_find() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        std::vector<Tester> values;
        for (const Tester & t : b) {
            bool before = Tester::disable_stat;
            Tester::disable_stat = true;
            values.push_back(t);
            Tester::disable_stat = before;
        }
        if (!inner_count_test(values, b, a)) return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}

void test_delete() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        std::vector<Tester> values;
        for (const Tester & t : b) {
            bool before = Tester::disable_stat;
            Tester::disable_stat = true;
            values.push_back(t);
            Tester::disable_stat = before;
        }
        for (size_t j = 0; j < i * 2; ++j) {
            size_t random = rng();
            const Tester & find = values[random % values.size()];
            Tester::set_ref(false);
            bool remove_a = a.remove(find);
            Tester::set_ref(true);
            size_t remove_b = b.erase(find);
            if (remove_a != remove_b) {
                tests[test_name] = string_format("remove tree(%d) != ref(%d)", remove_a, remove_b);
                return false;
            }
            if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        }
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}

void test_random_access() {
    testbed([](size_t i, const string & test_name) {
        AVLTree<Tester> a;
        set<Tester> b;
        insert_random(a, b, i);
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;

        for (size_t j = 0; j < i; ++j) {
            size_t random = rng();

            unsigned long size = b.size();
            if (!b.empty()) {
                Tester::set_ref(false);
                auto test = a[random % size];
                Tester::set_ref(true);
                auto ref = std::next(b.begin(), (long) (random % size));

                bool before = Tester::disable_stat;
                Tester::disable_stat = true;
                if (*test != *ref) {
                    tests[test_name] = string_format("Non trivial %s != %s", toString(*test).c_str(),
                                                     toString(*ref).c_str());
                    return false;
                }
                Tester::disable_stat = before;
            }
        }
        if (!iterative_data_test<AVLTree<Tester> &, std::set<Tester> &>(a, b, test_name)) return false;
        Tester::disable_stat = true;
        return true;
    }, std::source_location::current());
}


int main() {
    std::random_device rd;
    mt = new std::mt19937(rd());

    using namespace stl;
    using namespace std;
//    AVLTree<int> compileall;
//    for (int i = 0; i < 10; ++i)
//        compileall.insert(i);
//    for (int i = 0; i < 10; ++i)
//        compileall.remove(i);
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
    test_delete();
    test_random_access();

    bool failed = false;
    for (auto & [key, error] : tests) {
        if (error) {
            failed = true;
            cout << Color::FG::RED << "Failed: " << Color::FG::DEFAULT << key << ", Error: " << error.value() << endl;
        } else {
            cout << Color::FG::GREEN << "Pog: " << Color::FG::DEFAULT << key << endl;
        }
    }
    if (failed) std::cout << "Failed at least one!" << endl;
    return failed;
}


