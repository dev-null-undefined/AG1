#include <iostream>
#include <memory>

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

template<typename T_Node>
struct ParentNode : mixins::SureIAmThat<T_Node, ParentNode> {
    using mixins::SureIAmThat<T_Node, ParentNode>::self;

    T_Node *parent = nullptr;
};

enum class Direction {
    left, right
};

template<typename T_Node>
struct BubbleUp : mixins::SureIAmThat<T_Node, BubbleUp>, ParentNode<T_Node> {
    using mixins::SureIAmThat<T_Node, BubbleUp>::self;

    void bubbleUp() {
        T_Node *current = to_ptr(&self());
        updateAll(*current);
        while (current->parent) {
            updateAll(*to_ptr(current->parent));
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

    template<Direction direction>
    requires mixins::Mixin<T_Node, BubbleUp>
    std::shared_ptr<T_Node> setChild(std::shared_ptr<T_Node> child) {
        if constexpr (direction == Direction::left) {
            left = child;
        } else {
            right = child;
        }
        child->parent = &self();
        child->bubbleUp();
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
};

template<typename T>
struct ValueNode {
    template<typename T_Node>
    struct Inner : mixins::SureIAmThat<T_Node, Inner> {
        using mixins::SureIAmThat<T_Node, Inner>::self;

    private:
        T _value;

    public:
        T mixinInfo() {
            return _value;
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

template<mixins::Mixin<BinaryNode, ParentNode> T_Node>
Direction getChildDirection(T_Node *parent, T_Node *child) {
    if (to_ptr(parent->left) == child)
        return Direction::left;
    return Direction::right;
}

template<typename T_Node>
struct Delete : mixins::SureIAmThat<T_Node, Delete> {
    using mixins::SureIAmThat<T_Node, Delete>::self;

    void remove() {
        if (self().childCount() == 0) {
            self().parent->setChild(getChildDirection(self().parent, &self()), nullptr);
        } else if (self().childCount() == 1) {
            self().parent->setChild(getChildDirection(self().parent, &self()), self().getAnyChild());
        } else if (self().childCount() == 2) {
            T_Node *toReplace = to_ptr(self().right);
            while (toReplace->left)
                toReplace = to_ptr(toReplace->left);
            copyAll(self(), *toReplace);
            toReplace->remove();
        }
    }
};

#include <fstream>

auto main() -> signed {
    using NodeType = mixins::Mixins<
            ValueNode<char>::Inner,
            BubbleUp,
            BinaryNode,
            GraphViz,
            MaxDepth,
            FilteredSizeCounter<char, '\n'>::Inner,
            SizeCounter,
            Indexable,
            Insert,
            Delete>;

    NodeType node;
    node.setValue(4);
    node.generateGraph();
//    node.find<&FilteredSizeCounter<char, '\n'>::Inner<NodeType>::size>(0);
    auto t = node.find<&SizeCounter<NodeType>::size>(0);
    std::cout << t.getValue() << std::endl;

    std::shared_ptr<NodeType> node2 = std::make_shared<NodeType>();
    node.setChild<Direction::right>(node2);
    node.right->setValue(99);

    for (int i = 1; i < 8; ++i) {
        node.right->insert(std::make_shared<NodeType>()).setValue(i);
        if (i == 3 || i == 7) {
            std::ofstream fs("file" + std::to_string(i) + "-d.dot");
            node.generateGraph(fs);
            fs.close();
            system(("dot -T png < file" + std::to_string(i) + "-d.dot > image" + std::to_string(i) + "-d.png").c_str());

            node.right->remove();
        }
        std::ofstream fs("file" + std::to_string(i) + ".dot");
        node.generateGraph(fs);
        fs.close();
        system(("dot -T png < file" + std::to_string(i) + ".dot > image" + std::to_string(i) + ".png").c_str());
    }

    for (int i = 0; i < 7; ++i) {
        std::cout << int(node.find<&SizeCounter<NodeType>::size>(i).getValue()) << std::endl;
    }
    updateAll(node);
    return 0;
}