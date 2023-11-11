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
#include <stack>
#include <queue>
#include <random>
#include <type_traits>

#endif

#include "avl_tree_v2.hpp"

// MARK: Progtest


struct TextEditorBackend {
    void assertIndex(size_t i, size_t max) const {
        if (i > max) {
            throw std::out_of_range("Index out of range " + std::to_string(i) + " maximum is " +
                                    std::to_string(max - 1));
        }
    }

    void assertIndex(size_t i) const {
        assertIndex(i, size());
    }

    void assertStrictIndex(size_t i, size_t max) const {
        if (i >= max) {
            throw std::out_of_range("Index out of range " + std::to_string(i) + " maximum is " +
                                    std::to_string(size() - 1));
        }
    }

    void assertStrictIndex(size_t i) const {
        assertStrictIndex(i, size());
    }

    TextEditorBackend(const std::string &text) {
        for (char c: text) {
            insert(size(), c);
        }
    }

    size_t size() const {
        return tree.getSize();
    }

    size_t lines() const {
        return tree.getSize<AVLTreeNewLineCounterSize<AVLTree>>() + 1;
    }

    char at(size_t i) const {
        assertStrictIndex(i);
        auto node = tree.find(i);
        return node.getValue();
    }

    void edit(size_t i, char c) {
        assertStrictIndex(i);
        auto &node = tree.find(i);
        node.setValue(c);
    }

    void insert(size_t i, char c) {
        assertIndex(i);
        tree.insert(i, std::make_shared<AVLTree::NodeType>()).setValue(c);
    }

    void erase(size_t i) {
        assertStrictIndex(i);
        auto &node = tree.find(i);
        tree.remove(node);
    }

    size_t line_start(size_t r) const {
        assertIndex(r, lines() - 1);
        if (r == 0) {
            return 0;
        }
        auto &node = tree.find<AVLTreeNewLineCounterSize<AVLTree>>(r - 1);
        return node.getIndex();
    }

    size_t line_length(size_t r) const {
        assertIndex(r, lines() - 1);
        if (r + 1 == lines()) {
            return size() - line_start(r);
        }
        return line_start(r + 1) - line_start(r);
    }

    size_t char_to_line(size_t i) const {
        assertIndex(i);
        if (i == 0) {
            return 0;
        }
        auto &node = tree.find(i);
        return node.getIndex<AVLTreeNewLineCounterSize<AVLTree>>() - (node.getValue() == '\n');
    }

    AVLTree tree;
};
