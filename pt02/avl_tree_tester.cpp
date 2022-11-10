#include "./avl_tree.cpp"
#include <set>
#include <map>
#include <iostream>
#include <vector>
#include <variant>
#include <ctime>

using namespace stl;
using namespace std;

int TEST_CONSTANT = 100;

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

std::unordered_map<std::string, std::optional<std::string>> tests;

template<typename ... Args>
std::string string_format(const std::string & format, Args ... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

void test_iterators() {
    string test_name = "reverse iterators";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<int> int_tree;
        set<int> int_tree_ref;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            int_tree.insert(random);
            int_tree_ref.emplace(random);
        }
        auto b2 = int_tree_ref.begin(), e2 = int_tree_ref.end();
        for (auto b1 = int_tree.begin(), e1 = int_tree.end(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
        if (b2 != e2) {
            tests[test_name] = "failed not at the end of set";
            return;
        }
    }
}


void test_reverse_iterators() {
    string test_name = "iterators";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<int> int_tree;
        set<int> int_tree_ref;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            int_tree.insert(random);
            int_tree_ref.emplace(random);
        }
        auto b2 = int_tree_ref.rbegin(), e2 = int_tree_ref.rend();
        for (auto b1 = int_tree.rbegin(), e1 = int_tree.rend(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
        if (b2 != e2) {
            tests[test_name] = "failed not at the end of set";
            return;
        }
    }
}

void test_const_reverse_iterators() {
    string test_name = "cosnt reverse iterators";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<int> int_tree;
        set<int> int_tree_ref;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            int_tree.insert(random);
            int_tree_ref.emplace(random);
        }
        auto b2 = int_tree_ref.crbegin(), e2 = int_tree_ref.crend();
        for (auto b1 = int_tree.crbegin(), e1 = int_tree.crend(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
        if (b2 != e2) {
            tests[test_name] = "failed not at the end of set";
            return;
        }
    }
}

void test_const_iterators() {
    string test_name = "const iterators";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<int> int_tree;
        set<int> int_tree_ref;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            int_tree.insert(random);
            int_tree_ref.emplace(random);
        }
        auto b2 = int_tree_ref.cbegin(), e2 = int_tree_ref.cend();
        for (auto b1 = int_tree.cbegin(), e1 = int_tree.cend(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
        if (b2 != e2) {
            tests[test_name] = "failed not at the end of set";
            return;
        }
    }
}

struct Tester {
    int value = 0;

    Tester(int value) {
        this->value = value;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "C" << " : " << value << Color::FG_DEFAULT << endl;
        }
    }

    Tester(const Tester & other) {
        this->value = other.value;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "C&" << " : " << value << Color::FG_DEFAULT << endl;
        }
    }

    Tester(Tester && other) {
        this->value = other.value;
        other.value = -1;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "C&&" << " : " << value << Color::FG_DEFAULT << endl;
        }
    }

    ~Tester() {
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "~" << " : " << value << Color::FG_DEFAULT << endl;
        }
        value = -1;
    }

    Tester & operator=(const Tester & other) {
        this->value = other.value;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "=&" << " : " << value << Color::FG_DEFAULT << endl;
        }
        return *this;
    }

    Tester & operator=(Tester && other) {
        this->value = other.value;
        other.value = -1;
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << "=&&" << " : " << value << Color::FG_DEFAULT << endl;
        }
        return *this;
    }

    bool operator<(const Tester & other) const {
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << other.value << "<" << value << Color::FG_DEFAULT << endl;
        }
        return value < other.value;
    }

    bool operator!=(const Tester & other) const {
        if (ENABLE_PRINT) {
            cout << Color::FG_BLUE << other.value << "!=" << value << Color::FG_DEFAULT << endl;
        }
        return value != other.value;
    }

    static bool ENABLE_PRINT;
};

bool Tester::ENABLE_PRINT = false;

void test_copy() {
    Tester::ENABLE_PRINT = false;
    string test_name = "copy";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<Tester> int_tree;
        set<Tester> int_tree_ref;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            if (Tester::ENABLE_PRINT) cout << "Me------" << endl;
            int_tree.insert(random);
            if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
            int_tree_ref.insert(random);
            if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        }
        if (Tester::ENABLE_PRINT)cout << "Me------" << endl;
        AVLTree<Tester> int_tree_copy(int_tree);
        if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
        set<Tester> int_tree_copy_ref(int_tree_ref);
        if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            if (Tester::ENABLE_PRINT)cout << "Me------" << endl;
            int_tree_copy.insert(random);
            if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
            int_tree_copy_ref.insert(random);
            if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        }
        auto b2 = int_tree_ref.begin(), e2 = int_tree_ref.end();
        for (auto b1 = int_tree.begin(), e1 = int_tree.end(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
        b2 = int_tree_copy_ref.begin(), e2 = int_tree_copy_ref.end();
        for (auto b1 = int_tree_copy.begin(), e1 = int_tree_copy.end(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
    }
}

void test_move() {
    Tester::ENABLE_PRINT = false;
    string test_name = "move";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<Tester> int_tree;
        set<Tester> int_tree_ref;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            if (Tester::ENABLE_PRINT) cout << "Me------" << endl;
            int_tree.insert(random);
            if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
            int_tree_ref.insert(random);
            if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        }
        if (Tester::ENABLE_PRINT)cout << "Me------" << endl;
        AVLTree<Tester> int_tree_copy(std::move(int_tree));
        if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
        set<Tester> int_tree_copy_ref(std::move(int_tree_ref));
        if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            if (Tester::ENABLE_PRINT)cout << "Me------" << endl;
            int_tree_copy.insert(random);
            if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
            int_tree_copy_ref.insert(random);
            if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        }
        auto b2 = int_tree_copy_ref.begin(), e2 = int_tree_copy_ref.end();
        for (auto b1 = int_tree_copy.begin(), e1 = int_tree_copy.end(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
    }
}

void test_assign_move() {
    Tester::ENABLE_PRINT = false;
    string test_name = "assign move";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<Tester> int_tree;
        set<Tester> int_tree_ref;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            if (Tester::ENABLE_PRINT) cout << "Me------" << endl;
            int_tree.insert(random);
            if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
            int_tree_ref.insert(random);
            if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        }
        if (Tester::ENABLE_PRINT)cout << "Me------" << endl;
        AVLTree<Tester> int_tree_copy(std::move(int_tree));
        if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
        set<Tester> int_tree_copy_ref(std::move(int_tree_ref));
        if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            if (Tester::ENABLE_PRINT)cout << "Me------" << endl;
            int_tree_copy.insert(random);
            if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
            int_tree_copy_ref.insert(random);
            if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        }

        auto b2 = int_tree_copy_ref.begin(), e2 = int_tree_copy_ref.end();
        for (auto b1 = int_tree_copy.begin(), e1 = int_tree_copy.end(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
    }
}

void test_assign_copy() {
    Tester::ENABLE_PRINT = false;
    string test_name = "assign copy";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < TEST_CONSTANT; ++j) {
        AVLTree<Tester> int_tree;
        set<Tester> int_tree_ref;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            if (Tester::ENABLE_PRINT) cout << "Me------" << endl;
            int_tree.insert(random);
            if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
            int_tree_ref.insert(random);
            if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        }
        if (Tester::ENABLE_PRINT)cout << "Me------" << endl;
        AVLTree<Tester> int_tree_copy;
        if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
        set<Tester> int_tree_copy_ref;
        if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        for (int i = 0; i < j; ++i) {
            int random = std::rand();
            if (Tester::ENABLE_PRINT)cout << "Me------" << endl;
            int_tree_copy.insert(random);
            if (Tester::ENABLE_PRINT)cout << "Ref------" << endl;
            int_tree_copy_ref.insert(random);
            if (Tester::ENABLE_PRINT)cout << "------" << endl << endl << endl;
        }
        auto b2 = int_tree_copy_ref.begin(), e2 = int_tree_copy_ref.end();
        for (auto b1 = int_tree_copy.begin(), e1 = int_tree_copy.end(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
        int_tree = int_tree_copy;
        int_tree_ref = int_tree_copy_ref;

        b2 = int_tree_ref.begin(), e2 = int_tree_ref.end();
        for (auto b1 = int_tree.begin(), e1 = int_tree.end(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
        b2 = int_tree_copy_ref.begin(), e2 = int_tree_copy_ref.end();
        for (auto b1 = int_tree_copy.begin(), e1 = int_tree_copy.end(); b1 != e1; ++b1, ++b2) {
            if (*b1 != *b2) {
                tests[test_name] = string_format(string("%d != %d"), *b1, *b2);
                return;
            }
        }
    }
}

int main() {
    std::srand(std::time(nullptr));
    using namespace stl;
    using namespace std;

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


