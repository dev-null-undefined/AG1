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

// region mixins
namespace detail {
    template<typename... Args>
    struct next_type;

    template<typename Current, typename Second, typename... Rest>
    struct next_type<Current, Current, Second, Rest...> {
        using type = Second;
        static constexpr bool value = true;
    };

    template<typename Current, typename Second, typename... Rest>
    struct next_type<Current, Second, Rest...> {
        using type = typename next_type<Current, Rest...>::type;
        static constexpr bool value = next_type<Current, Rest...>::value;
    };

    template<typename T>
    struct next_type<T> {
        using type = T;
        static constexpr bool value = false;
    };

    template<typename ...T>
    using next_type_t = typename next_type<T...>::type;

    template<typename ...T>
    constexpr bool next_type_v = next_type<T...>::value;

    template<typename T>
    struct is_shared_ptr : std::false_type {
    };

    template<typename T>
    struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {
    };

    template<typename T>
    constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

    template<typename T>
    concept SharedPtr = is_shared_ptr_v<T>;

    template<typename T>
    concept NumericType = std::integral<T> || std::floating_point<T>;


    template<typename T>
    concept Printable = requires(T t) {
        { std::cout << t } -> std::same_as<std::ostream &>;
    };
}

template<typename T>
std::string better_to_string(const T &t);

template<>
std::string better_to_string<std::string>(const std::string &t) {
    return t;
}

template<detail::NumericType T>
std::string better_to_string(const T &t) {
    return std::to_string(t);
}

template<detail::NumericType T>
int get_sign(T t) {
    return (t > 0) - (t < 0);
}

namespace mixins {
    template<template<typename> typename ...T_mixins>
    struct Mixins : T_mixins<Mixins<T_mixins...>> ... {
        using T_this = Mixins<T_mixins...>;

        template<size_t N>
        using N_type = std::tuple_element_t<N, std::tuple<T_mixins<Mixins<T_mixins...>>...>>;

        template<class Current>
        using Next_type = detail::next_type_t<Current, T_mixins<Mixins<T_mixins...>>...>;

        template<size_t N>
        constexpr decltype(auto) get() {
            return *static_cast<N_type<N> *>(this);
        }

        constexpr decltype(auto) first() {
            return get<0>();
        }

        template<typename Current>
        constexpr decltype(auto) next() {
            return *static_cast<Next_type<Current> *>(this);
        }

        template<typename Current>
        static constexpr decltype(auto) hasNext() {
            return detail::next_type_v<Current, T_mixins<Mixins<T_mixins...>>...>;
        }
    };

    template<typename K, template<typename> typename Current>
    struct SureIAmThat {
        K &self() {
            return *static_cast<K *>(this);
        }

        const K &self() const {
            return *static_cast<const K *>(this);
        }
    };

    template<typename T, template<typename> typename ...M>
    concept Mixin = (std::is_base_of_v<M<std::decay_t<T>>, std::decay_t<T>> && ...);
}

// region execAll macro

#define ExecAll(FunName, Function) \
namespace mixins {\
    template<typename T, typename Lambda, typename...Args>\
    void FunName##Ret(T &&self, Lambda &&lambda, Args &&...args) {\
        _##FunName##Ret<T, Lambda, typename std::remove_reference_t<T>::template N_type<0>, Args...>(std::forward<T>(self),\
                                                                                                std::forward<Lambda>(\
                                                                                                        lambda),\
                                                                                                std::forward<Args>(\
                                                                                                        args)...);\
    }\
    \
    template<typename T, typename...Args>\
    void FunName(T &&self, Args &&...args) {\
        _##FunName<T, typename std::remove_reference_t<T>::template N_type<0>, Args...>(std::forward<T>(self),\
                                                                                        std::forward<Args>(args)...);\
    }\
    \
    template<typename T, typename Lambda, typename Current, typename...Args>\
    void _##FunName##Ret(T &&self, Lambda &&lambda, Args &&...args) {\
        if constexpr (std::remove_reference_t<T>::template hasNext<Current>()) {\
            _##FunName##Ret<T, Lambda, typename std::remove_reference_t<T>::template Next_type<Current>>(\
                    std::forward<T>(self), std::forward<Lambda>(lambda), std::forward<Args>(args)...);\
        }\
        if constexpr (requires(T &&t) {\
            {\
            static_cast<Current &&>(t).Function(std::forward<Args>(args)...)\
            }->std::same_as<void>;\
        }) { static_cast<Current &&>(self).Function(std::forward<Args>(args)...); }\
        else if constexpr (requires(T &&t) {\
            {\
            static_cast<Current &&>(t).Function(std::forward<Args>(args)...)\
            };\
        }) { lambda(static_cast<Current &&>(self).Function(std::forward<Args>(args)...)); }\
    }\
    \
    template<typename T, typename Current, typename...Args>\
    void _##FunName(T &&self, Args &&...args) {\
        if constexpr (std::remove_reference_t<T>::template hasNext<Current>()) {\
            _##FunName<T, typename std::remove_reference_t<T>::template Next_type<Current>>(std::forward<T>(self),\
                                                                                            std::forward<Args>(\
                                                                                                    args)...);\
        }\
        if constexpr (requires(T &&t) {\
            {\
            static_cast<Current &&>(t).Function(std::forward<Args>(args)...)\
            };\
        }) { static_cast<Current &&>(self).Function(std::forward<Args>(args)...); }\
    }\
}

// endregion

ExecAll(updateAll, update)
ExecAll(mixinInfoAll, mixinInfo)
ExecAll(copyAll, copy)

template<typename T>
requires (detail::SharedPtr<std::remove_reference_t<T>>)
decltype(auto) to_ptr(T something) {
    return something.get();
}

template<typename T>
decltype(auto) to_ptr(T *something) {
    return something;
}

enum class Direction {
    left, right
};

template<typename T_Node>
struct ParentNode : mixins::SureIAmThat<T_Node, ParentNode> {
    using mixins::SureIAmThat<T_Node, ParentNode>::self;

    T_Node *parent = nullptr;
};


template<typename T_Node>
struct Balancer : mixins::SureIAmThat<T_Node, Balancer> {
    using mixins::SureIAmThat<T_Node, Balancer>::self;

    void balance() {
        updateAll(self());
        int delta = self().getDelta();
        if (std::abs(delta) > 1) {
            if (delta > 0) {
                if (self().left->getSign() < 0) {
                    self().left->rotateLeft();
                }
                self().rotateRight();
            } else {
                if (self().right->getSign() > 0) {
                    self().right->rotateRight();
                }
                self().rotateLeft();
            }
        }
    }
};

template<typename T_Node>
struct BubbleUp : mixins::SureIAmThat<T_Node, BubbleUp> {
    using mixins::SureIAmThat<T_Node, BubbleUp>::self;

    void bubbleUp() {
        static_assert(mixins::Mixin<T_Node, ParentNode>, "ParentNode mixin is required");
        T_Node *current = to_ptr(&self());
        while (current) {
            updateAll(*to_ptr(current));
            if constexpr (requires { requires mixins::Mixin<T_Node, Balancer>; })
                current->balance();
            current = to_ptr(current->parent);
        }
    }
};

template<typename T_Node>
struct BinaryNode : mixins::SureIAmThat<T_Node, BinaryNode> {
    using mixins::SureIAmThat<T_Node, BinaryNode>::self;

    template<Direction direction>
    decltype(auto) getChild() {
        static_assert(direction == Direction::left || direction == Direction::right, "Invalid direction");
        if constexpr (direction == Direction::left) {
            return left;
        } else {
            return right;
        }
    }

    decltype(auto) getChild(Direction direction) {
        if (direction == Direction::left) {
            return left;
        } else {
            return right;
        }
    }

    static decltype(auto) directionToMember(Direction direction) {
        if (direction == Direction::left) {
            return &T_Node::left;
        } else {
            return &T_Node::right;
        }
    }

    template<Direction direction>
    requires mixins::Mixin<T_Node, BubbleUp>
    std::shared_ptr<T_Node> setChild(std::shared_ptr<T_Node> child) {
        if constexpr (direction == Direction::left) {
            left = child;
        } else {
            right = child;
        }
        if (child != nullptr) {
            child->parent = &self();
            child->bubbleUp();
        } else {
            self().bubbleUp();
        }
        return child;
    }

    std::shared_ptr<T_Node> setChild(Direction direction, std::shared_ptr<T_Node> child) {
        if (direction == Direction::left) {
            left = child;
        } else {
            right = child;
        }
        if (child != nullptr) {
            child->parent = &self();
            child->bubbleUp();
        } else {
            self().bubbleUp();
        }
        return child;
    }

    size_t childCount() {
        return (left != nullptr) + (right != nullptr);
    }

    std::shared_ptr<T_Node> getAnyChild() {
        if (left)
            return left;
        return right;
    }

    std::shared_ptr<T_Node> left = nullptr, right = nullptr;
};

template<mixins::Mixin<BinaryNode, ParentNode> T_Node>
Direction getChildDirection(const T_Node *parent, const T_Node *child) {
    if (to_ptr(parent->left) == child)
        return Direction::left;
    return Direction::right;
}

template<typename T_Node>
struct Debug : mixins::SureIAmThat<T_Node, Debug> {
    using mixins::SureIAmThat<T_Node, Debug>::self;

public:
    std::string getDebug() {
        std::string buffer = "\"";
        mixinInfoAllRet(self(), [&buffer](const auto &out) noexcept {
            buffer += (buffer.size() > 1 ? ", " : "") + better_to_string(out);
        });
        return buffer + "\"";
    }
};

#ifndef __PROGTEST__

#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
#endif

template<typename T_Node>
struct GraphViz : mixins::SureIAmThat<T_Node, GraphViz>, Debug<T_Node> {
    using mixins::SureIAmThat<T_Node, GraphViz>::self;
private:

    void bst_print_dot_null(T_Node &node, int nullcount, std::ostream &output, Direction dir) const {
        output << "    null" << nullcount << " [shape=point];\n";
        output << "    " << node.getDebug() << " -> null" << nullcount
               << (dir == Direction::left ? "[color=blue]" : "[color=red]") << ";\n";
    }

    void bst_print_dot_aux(T_Node &node, std::ostream &output) const {
        static int nullcount = 0;
        if (node.parent) {
            output << "    " << node.getDebug() << " -> " << node.parent->getDebug() << "[style=dotted];\n";
        }

        if (node.left) {
            output << "    " << node.getDebug() << " -> " << node.left->getDebug() << "[color=blue];\n";
            bst_print_dot_aux(*node.left, output);
        } else
            bst_print_dot_null(node, nullcount++, output, Direction::left);

        if (node.right) {
            output << "    " << node.getDebug() << " -> " << node.right->getDebug() << "[color=red];\n";
            bst_print_dot_aux(*node.right, output);
        } else
            bst_print_dot_null(node, nullcount++, output, Direction::right);
    }

public:
    void generateGraph(std::ostream &output = std::cout) {
        output << "digraph BST {\n";
        output << "    node [fontname=\"Arial\"];\n";

        if (!self().right && !self().left)
            output << "    " << self().getDebug() << ";\n";
        else
            bst_print_dot_aux(self(), output);

        output << "}\n" << std::flush;
    }

#ifndef __PROGTEST__

    void generateGraph(const std::string &filename) {

        auto tmpDir = fs::temp_directory_path();
        auto tmpFile = tmpDir / (filename + ".dot");
        std::ofstream output(tmpFile);
        generateGraph(output);
        output.close();
        system(("dot -T png < " + tmpFile.string() + " > " + filename + ".png").c_str());
        fs::remove(tmpFile);
    }

#endif

};

template<typename T>
struct ValueNode {
    template<typename T_Node>
    struct Inner : mixins::SureIAmThat<T_Node, Inner> {
        using mixins::SureIAmThat<T_Node, Inner>::self;

    private:
        T _value;

    public:
        auto mixinInfo() {
            return _value == '\n' ? "\\n" : std::string(1, _value);
        }

        const T &getValue() const {
            return _value;
        }

        T_Node &setValue(T value) {
            _value = std::move(value);
            if constexpr (requires { requires mixins::Mixin<T_Node, BubbleUp>; }) {
                self().bubbleUp();
            }
            return self();
        }

        void copy(const T_Node &other) {
            setValue(other.getValue());
        }
    };
};

template<typename T_Node>
struct SizeCounter : mixins::SureIAmThat<T_Node, SizeCounter> {
    using mixins::SureIAmThat<T_Node, SizeCounter>::self;

    void update() {
        size = getSize<&T_Node::left>() + getSize<&T_Node::right>() + 1;
    }

    template<auto direction>
    size_t getSize() {
        if (self().*direction)
            return (self().*direction)->SizeCounter<T_Node>::size;
        return 0;
    }

    std::string mixinInfo() {
        return "s: " + std::to_string(size);
    }

    size_t size = 0;
};

template<typename T, T filter>
struct FilteredSizeCounter {
    template<typename T_Node>
    struct Inner : mixins::SureIAmThat<T_Node, Inner> {
        using mixins::SureIAmThat<T_Node, Inner>::self;

        void update() {
            size = getSize<&T_Node::left>() + getSize<&T_Node::right>() + (self().getValue() == filter);
        }

        template<auto direction>
        size_t getSize() {
            if (self().*direction)
                return (self().*direction)->Inner<T_Node>::size;
            return 0;
        }

        std::string mixinInfo() {
            return "f[" + std::to_string(filter) + "]: " + std::to_string(size);
        }

        size_t size = 0;
    };
};

template<typename T_Node>
struct MaxDepth : mixins::SureIAmThat<T_Node, MaxDepth> {
    using mixins::SureIAmThat<T_Node, MaxDepth>::self;

    void update() {
        maxDepth = std::max(getDepth<&T_Node::left>(), getDepth<&T_Node::right>()) + 1;
    }

    template<auto direction>
    size_t getDepth() {
        if (self().*direction)
            return (self().*direction)->maxDepth;
        return 0;
    }

    auto mixinInfo() {
        return "d:" + std::to_string(maxDepth);
    }

    int getDelta() {
        return getDepth<&T_Node::left>() - getDepth<&T_Node::right>();
    }

    int getSign() {
        return get_sign(getDelta());
    }

    size_t maxDepth = 0;
};

template<typename T_Node>
struct Indexable : mixins::SureIAmThat<T_Node, Indexable> {
    using mixins::SureIAmThat<T_Node, Indexable>::self;

private:
    template<auto size_counter>
    const T_Node &_find(size_t index) const {
        const T_Node *current = &self();
        while (true) {
            if (index == _currentIndex<size_counter>(current) && _isCurrent<size_counter>(current))
                return *current;

            if (index < _currentIndex<size_counter>(current)) {
                current = to_ptr(current->left);
            } else if (index >= _currentIndex<size_counter>(current)) {
                index -= _currentIndex<size_counter>(current);
                current = to_ptr(current->right);
            }
        }
    }

    template<auto size_counter>
    T_Node &_find(size_t index) {
        T_Node *current = &self();
        while (true) {
            if (index == _currentIndex<size_counter>(current) && _isCurrent<size_counter>(current))
                return *current;

            if (index < _currentIndex<size_counter>(current)) {
                current = to_ptr(current->left);
            } else if (index >= _currentIndex<size_counter>(current)) {
                index -= _currentIndex<size_counter>(current);
                current = to_ptr(current->right);
            }
        }
    }

    template<auto size_counter>
    size_t _currentIndex(const T_Node *current) const {
        size_t index = current->*size_counter;
        if (current->right)
            index -= to_ptr(current->right)->*size_counter;
        return index;
    }

    template<auto size_counter>
    bool _isCurrent(const T_Node *current) const {
        size_t subCount = 0;
        if (current->right)
            subCount += to_ptr(current->right)->*size_counter;
        if (current->left)
            subCount += to_ptr(current->left)->*size_counter;
        return subCount != current->*size_counter;
    }

public:

    template<auto size_counter>
    const T_Node &find(size_t index) const {
        if (index >= self().*size_counter)
            throw std::out_of_range("Index out of range " + std::to_string(index) + " maximum is " +
                                    std::to_string(self().*size_counter));
        return _find<size_counter>(index + 1);
    }

    template<auto size_counter>
    T_Node &find(size_t index) {
        if (index >= self().*size_counter)
            throw std::out_of_range("Index out of range " + std::to_string(index) + " maximum is " +
                                    std::to_string(self().*size_counter));
        return _find<size_counter>(index + 1);
    }
};


template<typename T_Node>
struct GetIndex : mixins::SureIAmThat<T_Node, GetIndex> {
    using mixins::SureIAmThat<T_Node, GetIndex>::self;

private:
    // TODO: remove repeating code
    template<auto size_counter>
    size_t _currentIndex(const T_Node *current) const {
        size_t index = current->*size_counter;
        if (current->right)
            index -= to_ptr(current->right)->*size_counter;
        return index;
    }

public:
    template<auto size_counter>
    size_t getIndex() const {
        const T_Node *current = &self();
        size_t index = _currentIndex<size_counter>(current);
        while (current->parent) {
            if (getChildDirection(current->parent, current) == Direction::right) {
                index += _currentIndex<size_counter>(current->parent);
            }
            current = current->parent;
        }
        return index;
    }
};


template<typename T_Node>
struct Insert : mixins::SureIAmThat<T_Node, Insert> {
    using mixins::SureIAmThat<T_Node, Insert>::self;

    T_Node &insert(std::shared_ptr<T_Node> node) {
        Direction direction = Direction::right;
        T_Node *toInsert = to_ptr(&self());
        while (toInsert->getChild(direction)) {
            toInsert = to_ptr(toInsert->getChild(direction));
            direction = Direction::left;
        }
        auto aux = std::make_shared<T_Node>();
        copyAll(*aux, self());
        toInsert->setChild(direction, aux);
        copyAll(self(), *node);
        return self();
    }
};


template<typename T_Node>
struct PushBack : mixins::SureIAmThat<T_Node, PushBack> {
    using mixins::SureIAmThat<T_Node, PushBack>::self;

    T_Node &pushBack(std::shared_ptr<T_Node> node) {
        T_Node *toInsert = to_ptr(&self());
        while (toInsert->template getChild<Direction::right>()) {
            toInsert = to_ptr(toInsert->template getChild<Direction::right>());
        }
        auto aux = std::make_shared<T_Node>();
        copyAll(*aux, *node);
        toInsert->template setChild<Direction::right>(aux);
        return *aux;
    }
};

template<typename T_Node>
struct Delete : mixins::SureIAmThat<T_Node, Delete> {
    using mixins::SureIAmThat<T_Node, Delete>::self;

    void remove(std::shared_ptr<T_Node> &root) {
        if (self().childCount() == 0) {
            if (self().parent) {
                self().parent->setChild(getChildDirection(self().parent, &self()), nullptr);
            } else {
                root = nullptr;
            }
        } else if (self().childCount() == 1) {
            if (self().parent) {
                self().parent->setChild(getChildDirection(self().parent, &self()), self().getAnyChild());
            } else {
                root = self().getAnyChild();
            }
        } else if (self().childCount() == 2) {
            T_Node *toReplace = to_ptr(self().right);
            while (toReplace->left)
                toReplace = to_ptr(toReplace->left);
            copyAll(self(), *toReplace);
            toReplace->remove(root);
        }
    }
};

template<typename T_Node>
struct SharedThis : mixins::SureIAmThat<T_Node, SharedThis>, std::enable_shared_from_this<T_Node> {
};

template<typename T_Node>
struct Rotator : mixins::SureIAmThat<T_Node, Rotator> {
    using mixins::SureIAmThat<T_Node, Rotator>::self;

    void rotate(Direction direction) {
        Direction revDirection = direction == Direction::left ? Direction::right : Direction::left;
        auto pivot = self().shared_from_this();
        auto parent = self().parent;
        auto child = self().getChild(revDirection)->shared_from_this();
        auto grandChild = child->getChild(direction);

        if (parent) {
            Direction parentDir = getChildDirection(parent, pivot.get());
            (*parent).*(BinaryNode<T_Node>::directionToMember(parentDir)) = child;
        }
        child->parent = parent;

        pivot->parent = to_ptr(child);

        (*pivot).*(BinaryNode<T_Node>::directionToMember(revDirection)) = grandChild;
        if (grandChild)
            grandChild->parent = to_ptr(pivot);

        (*child).*(BinaryNode<T_Node>::directionToMember(direction)) = pivot;

        updateAll(*pivot);
        updateAll(*child);
        if (parent)
            updateAll(*parent);
    }

    void rotateLeft() {
        rotate(Direction::left);
    }

    void rotateRight() {
        rotate(Direction::right);
    }
};


template<typename T_Node>
using NewLineCounter = FilteredSizeCounter<char, '\n'>::Inner<T_Node>;

template<typename T_Node>
using CharValueNode = ValueNode<char>::Inner<T_Node>;

using NodeType = mixins::Mixins<
        CharValueNode,
        ParentNode,
        GetIndex,
        BubbleUp,
        BinaryNode,
        GraphViz,
        MaxDepth,
        NewLineCounter,
        SizeCounter,
        Indexable,
        Insert,
        Delete,
        SharedThis,
        Rotator,
        PushBack>;

template<typename T_Node>
constexpr auto NewLineCounterSize = &NewLineCounter<T_Node>::size;

template<typename T_Node>
constexpr auto NormalSize = &SizeCounter<T_Node>::size;
//endregion

struct TextEditorBackend {

    void assertIndex(size_t i, size_t max) const {
        if (!root && i != 0) {
            throw std::out_of_range("Index out of range " + std::to_string(i) + " maximum is " +
                                    std::to_string(0));
        }

        if (i > max) {
            throw std::out_of_range("Index out of range " + std::to_string(i) + " maximum is " +
                                    std::to_string(max - 1));
        }
    }

    void assertIndex(size_t i) const {
        assertIndex(i, size());
    }

    TextEditorBackend(const std::string &text) {
        for (char c: text) {
            insert(size(), c);
        }
    }

    size_t size() const {
        if (!root) {
            return 0;
        }
        return root->SizeCounter<NodeType>::size;
    }

    size_t lines() const {
        if (!root) {
            return 1;
        }
        return root->NewLineCounter<NodeType>::size + 1;
    }

    char at(size_t i) const {
        assertIndex(i);
        auto node = root->find<NormalSize<NodeType>>(i);
        return node.getValue();
    }

    void edit(size_t i, char c) {
        assertIndex(i);
        auto &node = root->find<NormalSize<NodeType>>(i);
        node.setValue(c);
    }

    void insert(size_t i, char c) {
        assertIndex(i);
        if (!root) {
            root = std::make_shared<NodeType>();
            root->setValue(c);
            return;
        }
        if (i == size()) {
            root->pushBack(std::make_shared<NodeType>()).setValue(c);
            return;
        }
        auto &node = root->find<NormalSize<NodeType>>(i);
        node.insert(std::make_shared<NodeType>()).setValue(c);
    }

    void erase(size_t i) {
        assertIndex(i);
        auto node = root->find<NormalSize<NodeType>>(i);
        node.remove(root);
    }

    size_t line_start(size_t r) const {
        assertIndex(r, lines());
        if (r == 0) {
            return 0;
        }
        auto node = root->find<NewLineCounterSize<NodeType>>(r - 1);
        return node.getIndex<NormalSize<NodeType>>();
    }

    size_t line_length(size_t r) const {
        assertIndex(r, lines());
        if (r + 1 == lines()) {
            return size() - line_start(r);
        }
        return line_start(r + 1) - line_start(r);
    }

    size_t char_to_line(size_t i) const {
        assertIndex(i);
        auto node = root->find<NormalSize<NodeType>>(i);
        return node.getIndex<NewLineCounterSize<NodeType>>() - (node.getValue() == '\n');
    }

    std::shared_ptr<NodeType> root = nullptr;
};

#ifndef __PROGTEST__

////////////////// Dark magic, ignore ////////////////////////

template<typename T>
auto quote(const T &t) { return t; }

std::string quote(const std::string &s) {
    std::string ret = "\"";
    for (char c: s) if (c != '\n') ret += c; else ret += "\\n";
    return ret + "\"";
}

#define STR_(a) #a
#define STR(a) STR_(a)

#define CHECK_(a, b, a_str, b_str) do { \
    auto _a = (a); \
    decltype(a) _b = (b); \
    if (_a != _b) { \
      std::cout << "Line " << __LINE__ << ": Assertion " \
        << a_str << " == " << b_str << " failed!" \
        << " (lhs: " << quote(_a) << ")" << std::endl; \
      fail++; \
    } else ok++; \
  } while (0)

#define CHECK(a, b) CHECK_(a, b, #a, #b)

#define CHECK_ALL(expr, ...) do { \
    std::array _arr = { __VA_ARGS__ }; \
    for (size_t _i = 0; _i < _arr.size(); _i++) \
      CHECK_((expr)(_i), _arr[_i], STR(expr) "(" << _i << ")", _arr[_i]); \
  } while (0)

#define CHECK_EX(expr, ex) do { \
    try { \
      (expr); \
      fail++; \
      std::cout << "Line " << __LINE__ << ": Expected " STR(expr) \
        " to throw " #ex " but no exception was raised." << std::endl; \
    } catch (const ex&) { ok++; \
    } catch (...) { \
      fail++; \
      std::cout << "Line " << __LINE__ << ": Expected " STR(expr) \
        " to throw " #ex " but got different exception." << std::endl; \
    } \
  } while (0)

////////////////// End of dark magic ////////////////////////


std::string text(const TextEditorBackend &t) {
    std::string ret;
    for (size_t i = 0; i < t.size(); i++) ret.push_back(t.at(i));
    return ret;
}

void test1(int &ok, int &fail) {
    TextEditorBackend s("123\n456\n789");
    CHECK(s.size(), 11);
    CHECK(text(s), "123\n456\n789");
    CHECK(s.lines(), 3);
    CHECK_ALL(s.char_to_line, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2);
    CHECK_ALL(s.line_start, 0, 4, 8);
    CHECK_ALL(s.line_length, 4, 4, 3);
}

void test2(int &ok, int &fail) {
    TextEditorBackend t("123\n456\n789\n");
    CHECK(t.size(), 12);
    CHECK(text(t), "123\n456\n789\n");
    CHECK(t.lines(), 4);
    CHECK_ALL(t.char_to_line, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2);
    CHECK_ALL(t.line_start, 0, 4, 8, 12);
    CHECK_ALL(t.line_length, 4, 4, 4, 0);
}

void test3(int &ok, int &fail) {
    TextEditorBackend t("asdfasdfasdf");

    CHECK(t.size(), 12);
    CHECK(text(t), "asdfasdfasdf");
    CHECK(t.lines(), 1);
    CHECK_ALL(t.char_to_line, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    CHECK(t.line_start(0), 0);
    CHECK(t.line_length(0), 12);

    t.insert(0, '\n');
    CHECK(t.size(), 13);
    CHECK(text(t), "\nasdfasdfasdf");
    CHECK(t.lines(), 2);
    CHECK_ALL(t.line_start, 0, 1);

    t.insert(4, '\n');
    CHECK(t.size(), 14);
    CHECK(text(t), "\nasd\nfasdfasdf");
    CHECK(t.lines(), 3);
    CHECK_ALL(t.line_start, 0, 1, 5);

    t.insert(t.size(), '\n');
    CHECK(t.size(), 15);
    CHECK(text(t), "\nasd\nfasdfasdf\n");
    CHECK(t.lines(), 4);
    CHECK_ALL(t.line_start, 0, 1, 5, 15);

    t.edit(t.size() - 1, 'H');
    CHECK(t.size(), 15);
    CHECK(text(t), "\nasd\nfasdfasdfH");
    CHECK(t.lines(), 3);
    CHECK_ALL(t.line_start, 0, 1, 5);

    t.erase(8);
    CHECK(t.size(), 14);
    CHECK(text(t), "\nasd\nfasfasdfH");
    CHECK(t.lines(), 3);
    CHECK_ALL(t.line_start, 0, 1, 5);

    t.erase(4);
    CHECK(t.size(), 13);
    CHECK(text(t), "\nasdfasfasdfH");
    CHECK(t.lines(), 2);
    CHECK_ALL(t.line_start, 0, 1);
}

void test_ex(int &ok, int &fail) {
    TextEditorBackend t("123\n456\n789\n");
    CHECK_EX(t.at(12), std::out_of_range);

    CHECK_EX(t.insert(13, 'a'), std::out_of_range);
    CHECK_EX(t.edit(12, 'x'), std::out_of_range);
    CHECK_EX(t.erase(12), std::out_of_range);

    CHECK_EX(t.line_start(4), std::out_of_range);
    CHECK_EX(t.line_start(40), std::out_of_range);
    CHECK_EX(t.line_length(4), std::out_of_range);
    CHECK_EX(t.line_length(6), std::out_of_range);
    CHECK_EX(t.char_to_line(12), std::out_of_range);
    CHECK_EX(t.char_to_line(25), std::out_of_range);
}

int main() {
    try {
        int ok = 0, fail = 0;
        if (!fail) test1(ok, fail);
        if (!fail) test2(ok, fail);
        if (!fail) test3(ok, fail);
        if (!fail) test_ex(ok, fail);

        if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
        else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
    } catch (...) {
        std::cout << "Unexpected exception thrown." << std::endl;
    }
}

#endif


