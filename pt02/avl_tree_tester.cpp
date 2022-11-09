#include "./avl_tree.cpp"
#include <set>
#include <map>
#include <iostream>
#include <vector>
#include <variant>
#include <ctime>

using namespace stl;
using namespace std;
std::map<std::string, std::optional<std::string>> tests;

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
    for (int j = 0; j < 1000; ++j) {
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
    using namespace stl;
    using namespace std;
    string test_name = "iterators";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < 1000; ++j) {
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
    using namespace stl;
    using namespace std;
    string test_name = "cosnt reverse iterators";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < 1000; ++j) {
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
    using namespace stl;
    using namespace std;
    string test_name = "const iterators";
    cout << "Testing now: " << test_name << endl;
    tests[test_name] = std::optional<std::string>();
    for (int j = 0; j < 1000; ++j) {
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

int main() {
    std::srand(std::time(nullptr));
    using namespace stl;
    using namespace std;

    test_iterators();
    test_reverse_iterators();
    test_const_iterators();
    test_const_reverse_iterators();

    for (auto & [key, error] : tests) {
        if (error) {
            cout << "Failed: " << key << ", Error:" << error.value() << endl;
        } else {
            cout << "Pog: " << key << endl;
        }
    }

    return 0;
}


