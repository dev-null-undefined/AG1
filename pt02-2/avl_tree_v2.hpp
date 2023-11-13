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
    concept NumericType = std::integral<T> || std::floating_point<T>;

    template<class T>
    constexpr std::add_const_t<T> &as_const(T &t) noexcept {
        return t;
    }

    template<typename T, typename N>
    using const_if_const = std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, std::add_const_t<N>, N>;
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

        [[nodiscard]] const K &self() const {
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
            static_cast<detail::const_if_const<std::remove_reference_t<T>,Current> &&>(t).Function(std::forward<Args>(args)...)\
            }->std::same_as<void>;\
        }) { static_cast<detail::const_if_const<std::remove_reference_t<T>,Current> &&>(self).Function(std::forward<Args>(args)...); }\
        else if constexpr (requires(T &&t) {\
            {\
            static_cast<detail::const_if_const<std::remove_reference_t<T>,Current> &&>(t).Function(std::forward<Args>(args)...)\
            };\
        }) { lambda(static_cast<detail::const_if_const<std::remove_reference_t<T>,Current> &&>(self).Function(std::forward<Args>(args)...)); }\
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
            static_cast<detail::const_if_const<std::remove_reference_t<T>,Current> &&>(t).Function(std::forward<Args>(args)...)\
            };\
        }) { static_cast<detail::const_if_const<std::remove_reference_t<T>,Current> &&>(self).Function(std::forward<Args>(args)...); }\
    }\
}

// endregion

ExecAll(updateAll, update)
ExecAll(mixinInfoAll, mixinInfo)
ExecAll(copyAll, copy)


enum class Direction {
    left, right
};

template<typename T_Node>
struct sharedThis : mixins::SureIAmThat<T_Node, sharedThis>, std::enable_shared_from_this<T_Node> {
};

template<typename T_Node>
struct ParentNode : mixins::SureIAmThat<T_Node, ParentNode> {
    using mixins::SureIAmThat<T_Node, ParentNode>::self;

    T_Node *parent = nullptr;
};

template<typename T_Node>
struct BubbleUp : mixins::SureIAmThat<T_Node, BubbleUp> {
    using mixins::SureIAmThat<T_Node, BubbleUp>::self;

    void bubbleUp() {
        static_assert(mixins::Mixin<T_Node, ParentNode>, "ParentNode mixin is required");
        T_Node *current = &self();
        while (current) {
            updateAll(*current);
            current = current->parent;
        }
    }
};

template<typename T_Node>
struct BinaryNode : mixins::SureIAmThat<T_Node, BinaryNode> {
    using mixins::SureIAmThat<T_Node, BinaryNode>::self;

private:

    static void _removeChild(T_Node *node) {
        if (!node) return;
        node->left = node->right = nullptr;
        delete node;
    }

public:

    BinaryNode() = default;

    ~BinaryNode() {
        delete left;
        delete right;
    }

    BinaryNode(const BinaryNode &) = delete;

    BinaryNode &operator=(const BinaryNode &) = delete;

    template<Direction direction>
    T_Node *&getChild() {
        static_assert(direction == Direction::left || direction == Direction::right, "Invalid direction");
        if constexpr (direction == Direction::left) {
            return left;
        } else {
            return right;
        }
    }

    T_Node *&getChild(Direction direction) {
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
    T_Node *setChild(T_Node *child) {
        if constexpr (direction == Direction::left) {
            _removeChild(left);
            left = child;
        } else {
            _removeChild(right);
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

    T_Node *setChild(Direction direction, T_Node *child) {
        if (direction == Direction::left) {
            return setChild<Direction::left>(std::move(child));
        } else {
            return setChild<Direction::right>(std::move(child));
        }
    }

    size_t childCount() const {
        return (left != nullptr) + (right != nullptr);
    }

    const T_Node *const &getAnyChild() const {
        if (left)
            return left;
        return right;
    }

    T_Node *&getAnyChild() {
        return const_cast<T_Node *&>(detail::as_const(*this).getAnyChild());
    }

    T_Node *left = nullptr, *right = nullptr;
};

template<typename T_Node>
struct Equals : mixins::SureIAmThat<T_Node, Equals> {
    using mixins::SureIAmThat<T_Node, Equals>::self;

    bool equals(const T_Node *other) const {
        if constexpr (requires { requires mixins::Mixin<T_Node, sharedThis>; }) {
            return other->shared_from_this() == self().shared_from_this();
        }
        return other == this;
    }
};

template<mixins::Mixin<BinaryNode, ParentNode> T_Node>
Direction getChildDirection(const T_Node *child) {
    if constexpr (requires { requires mixins::Mixin<T_Node, Equals>; }) {
        if (child->parent->left && child->parent->left->equals(child))
            return Direction::left;
    } else {
        if (child->parent->left == (child))
            return Direction::left;
    }
    return Direction::right;
}

template<typename T_Node>
struct Debug : mixins::SureIAmThat<T_Node, Debug> {
    using mixins::SureIAmThat<T_Node, Debug>::self;

public:
    std::string getDebug() const {
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

    static void bst_print_dot_null(const T_Node &node, int nullcount, std::ostream &output, Direction dir) {
        output << "    null" << nullcount << " [shape=point];\n";
        output << "    " << node.getDebug() << " -> null" << nullcount
               << (dir == Direction::left ? "[color=blue]" : "[color=red]") << ";\n";
    }

    static void bst_print_dot_aux(const T_Node &node, std::ostream &output) {
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
    void generateGraph(std::ostream &output = std::cout) const {
        output << "digraph BST {\n";
        output << "    node [fontname=\"Arial\"];\n";

        if (!self().right && !self().left)
            output << "    " << self().getDebug() << ";\n";
        else
            bst_print_dot_aux(self(), output);

        output << "}\n" << std::flush;
    }

#ifndef __PROGTEST__

    void generateGraph(const std::string &filename) const {
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
        auto mixinInfo() const {
            return std::to_string(_value) + "," + std::to_string(self().getIndex());
        }

        const T &getValue() const {
            return _value;
        }

        T &getValue() {
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


template<typename T>
struct Comparable {
    template<typename T_Node>
    struct Inner : mixins::SureIAmThat<T_Node, Inner> {
        using mixins::SureIAmThat<T_Node, Inner>::self;
    public:

        auto operator<=>(const T_Node &other) const {
            static_assert(mixins::Mixin<T_Node, ValueNode<T>::template Inner>, "ValueNode mixin is required");
            return self().getValue() <=> other.getValue();
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

    std::string mixinInfo() const {
        return "s: " + std::to_string(size);
    }

    size_t size = 0;
};

template<typename T_Node>
constexpr auto NormalSize = &SizeCounter<T_Node>::size;

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

        std::string mixinInfo() const {
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

    auto mixinInfo() const {
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
            if (index == _currentIndex<size_counter>(current)) {
                if (_isCurrent<size_counter>(current))
                    return *current;
                if (index == _size<size_counter>(current->left)) {
                    current = current->left;
                    continue;
                } else if (index == _size<size_counter>(current->left)) {
                    current = current->right;
                    continue;
                }
            }

            if (index < _currentIndex<size_counter>(current)) {
                current = current->left;
            } else /* if (index > _currentIndex<size_counter>(current)) */ {
                index -= _currentIndex<size_counter>(current);
                current = current->right;
            }
        }
    }

    template<auto size_counter>
    T_Node &_find(size_t index) {
        return const_cast<T_Node &>(detail::as_const(*this).template _find<size_counter>(index));
    }

    template<auto size_counter>
    size_t _size(const T_Node *current) const {
        if (!current) return 0;
        return current->*size_counter;
    }

    template<auto size_counter>
    size_t _currentIndex(const T_Node *current) const {
        if (!current) return 0;
        size_t index = current->*size_counter;
        if (current->right)
            index -= (*current->right).*size_counter;
        return index;
    }

    template<auto size_counter>
    bool _isCurrent(const T_Node *current) const {
        size_t subCount = 0;
        if (current->right)
            subCount += (*current->right).*size_counter;
        if (current->left)
            subCount += (*current->left).*size_counter;
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
        return const_cast<T_Node &>(detail::as_const(*this).template find<size_counter>(index));
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
            index -= (*current->right).*size_counter;
        return index;
    }

public:
    template<auto size_counter = NormalSize<T_Node>>
    [[nodiscard]] size_t getIndex() const {
        const T_Node *current = &self();
        size_t index = _currentIndex<size_counter>(current);
        while (current->parent) {
            if (getChildDirection(current) == Direction::right) {
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

    T_Node &insert(const T_Node &node) {
        Direction direction = Direction::right;
        T_Node *toInsert = &self();
        while (toInsert->getChild(direction)) {
            toInsert = toInsert->getChild(direction);
            direction = Direction::left;
        }
        auto aux = new T_Node();
        copyAll(*aux, self());
        toInsert->setChild(direction, std::move(aux));
        copyAll(self(), node);
        return *toInsert->getChild(direction);
    }
};


template<typename T_Node>
struct PushBack : mixins::SureIAmThat<T_Node, PushBack> {
    using mixins::SureIAmThat<T_Node, PushBack>::self;

    T_Node &pushBack(const T_Node &node) {
        T_Node *toInsert = &self();
        while (toInsert->template getChild<Direction::right>()) {
            toInsert = toInsert->template getChild<Direction::right>();
        }
        auto aux = new T_Node();
        copyAll(*aux, node);
        toInsert->template setChild<Direction::right>(std::move(aux));
        return *toInsert->right;
    }
};

template<typename T_Node>
struct InsertAt : mixins::SureIAmThat<T_Node, InsertAt>, PushBack<T_Node>, Insert<T_Node> {
    using mixins::SureIAmThat<T_Node, InsertAt>::self;

    template<auto size_counter>
    T_Node &insert(size_t index, const T_Node &node) {
        if (index == self().*size_counter)
            return self().pushBack(std::move(node));
        auto &toInsert = self().template find<size_counter>(index);
        return toInsert.Insert<T_Node>::insert(std::move(node));
    }
};


template<typename T_Node>
struct TreeMixer {
    static constexpr auto default_size_counter = NormalSize<T_Node>;

    template<typename T_Tree>
    struct Inner : mixins::SureIAmThat<T_Tree, Inner> {
        using mixins::SureIAmThat<T_Tree, Inner>::self;
        using NodeType = T_Node;
        T_Node *root = nullptr;

        Inner() = default;

        ~Inner() {
            delete root;
        }

        Inner(const Inner &) = delete;

        Inner &operator=(const Inner &) = delete;
    };


    template<typename T_Tree>
    struct Rotator : mixins::SureIAmThat<T_Tree, Rotator> {
        using mixins::SureIAmThat<T_Tree, Rotator>::self;

        void rotate(Direction direction, T_Node *&node) {
            Direction revDirection = direction == Direction::left ? Direction::right : Direction::left;
            auto pivot = node;
            auto parent = node->parent;
            auto child = node->getChild(revDirection);
            auto grandChild = child->getChild(direction);

            if (parent) {
                Direction parentDir = getChildDirection(pivot);
                (*parent).*(BinaryNode<T_Node>::directionToMember(parentDir)) = child;
            } else {
                self().root = child;
            }
            child->parent = parent;

            pivot->parent = child;

            (*pivot).*(BinaryNode<T_Node>::directionToMember(revDirection)) = grandChild;
            if (grandChild)
                grandChild->parent = pivot;

            (*child).*(BinaryNode<T_Node>::directionToMember(direction)) = pivot;

            updateAll(*pivot);
            updateAll(*child);
            if (parent)
                updateAll(*parent);
        }

        void rotateLeft(T_Node *&node) {
            rotate(Direction::left, node);
        }

        void rotateRight(T_Node *&node) {
            rotate(Direction::right, node);
        }

        template<Direction direction>
        void rotate(T_Node *&node) {
            if (direction == Direction::left) {
                rotateLeft(node);
            } else {
                rotateRight(node);
            }
        }
    };

    template<typename T_Tree>
    struct Balancer : mixins::SureIAmThat<T_Tree, Balancer> {
        using mixins::SureIAmThat<T_Tree, Balancer>::self;

        void balance(T_Node *&node) {
            updateAll(*node);
            int delta = node->getDelta();
            if (std::abs(delta) > 1) {
                if (delta > 0) {
                    if (node->left->getSign() < 0) {
                        self().rotateLeft(node->left);
                    }
                    self().rotateRight(node);
                } else {
                    if (node->right->getSign() > 0) {
                        self().rotateRight(node->right);
                    }
                    self().rotateLeft(node);
                }
            }
        }

        void balanceUp(T_Node *node) {
            while (node) {
                balance(node);
                node = node->parent;
            }
        }
    };

    template<typename T_Tree>
    struct Indexable : mixins::SureIAmThat<T_Tree, Indexable> {
        using mixins::SureIAmThat<T_Tree, Indexable>::self;

    public:
        template<auto size_counter = default_size_counter>
        const T_Node &find(size_t index) const {
            return self().root->template find<size_counter>(index);
        }

        template<auto size_counter = default_size_counter>
        T_Node &find(size_t index) {
            return const_cast<T_Node &>(detail::as_const(*this).template find<size_counter>(index));
        }
    };

    template<typename T_Tree>
    struct InsertAt : mixins::SureIAmThat<T_Tree, InsertAt> {
        using mixins::SureIAmThat<T_Tree, InsertAt>::self;

        template<auto size_counter = default_size_counter>
        T_Node &insert(size_t index, const std::decay_t<decltype(std::declval<T_Node>().getValue())> &data) {
            T_Node newNode;
            newNode.setValue(data);

            if (!self().root) {
                self().root = new T_Node();
                copyAll(*self().root, newNode);
                return *self().root;
            }
            T_Node &inserted = self().root->template insert<size_counter>(index, std::move(newNode));
            if constexpr (requires { requires mixins::Mixin<T_Tree, Balancer>; }) {
                self().balanceUp(&inserted);
            }
            return inserted;
        }
    };

    template<typename T_Tree>
    struct InsertSorted : mixins::SureIAmThat<T_Tree, InsertSorted> {
        using mixins::SureIAmThat<T_Tree, InsertSorted>::self;

    private:
        T_Node *_insert(const std::decay_t<decltype(std::declval<T_Node>().getValue())> &value) {
            if (!self().root) {
                self().root = new T_Node();
                return &self().root->setValue(value);
            }

            auto *newNode = new T_Node();
            newNode->setValue(value);

            T_Node *current = self().root;
            while (true) {
                if (*current < *newNode) {
                    if (current->right) {
                        current = current->right;
                    } else {
                        return current->template setChild<Direction::right>(newNode);
                    }
                } else if (*newNode < *current) {
                    if (current->left) {
                        current = current->left;
                    } else {
                        return current->template setChild<Direction::left>(newNode);
                    }
                } else {
                    delete newNode;
                    return nullptr;
                }
            }
        }

    public:
        bool insert(const std::decay_t<decltype(std::declval<T_Node>().getValue())> &value) {
            T_Node *inserted = _insert(value);
            if (inserted) {
                if constexpr (requires { requires mixins::Mixin<T_Tree, Balancer>; }) {
                    self().balanceUp(inserted);
                }
                return true;
            }
            return false;
        }
    };


    template<typename T_Tree>
    struct FindSorted : mixins::SureIAmThat<T_Tree, FindSorted> {
        using mixins::SureIAmThat<T_Tree, FindSorted>::self;

        T_Node *find(const std::decay_t<decltype(std::declval<T_Node>().getValue())> &value) {
            return const_cast<T_Node *>(detail::as_const(*this).find(value));
        }

        const T_Node *find(const std::decay_t<decltype(std::declval<T_Node>().getValue())> &value) const {
            if (!self().root) {
                return nullptr;
            }

            T_Node newNode;
            newNode.setValue(value);

            const T_Node *current = self().root;

            while (true) {
                if (*current < newNode) {
                    if (current->right) {
                        current = current->right;
                    } else {
                        return nullptr;
                    }
                } else if (newNode < *current) {
                    if (current->left) {
                        current = current->left;
                    } else {
                        return nullptr;
                    }
                } else {
                    return current;
                }
            }
        }
    };

    template<typename T_Tree>
    struct Delete : mixins::SureIAmThat<T_Tree, Delete> {
        using mixins::SureIAmThat<T_Tree, Delete>::self;

        void remove(T_Node &node) {
            T_Node *parent = node.parent;
            if (node.childCount() == 0) {
                if (node.parent) {
                    node.parent->setChild(getChildDirection(&node), nullptr);
                } else {
                    delete self().root;
                    self().root = nullptr;
                }
            } else if (node.childCount() == 1) {
                if (node.parent) {
                    node.parent->setChild(getChildDirection(&node), std::move(node.getAnyChild()));
                } else {
                    auto save = self().root;
                    self().root = std::move(node.getAnyChild());
                    self().root->parent = nullptr;
                    save->left = save->right = nullptr;
                    delete save;
                }
            } else if (node.childCount() == 2) {
                T_Node *toReplace = node.right;
                while (toReplace->left)
                    toReplace = toReplace->left;
                copyAll(node, *toReplace);
                remove(*toReplace);
                return;
            }

            if constexpr (requires { requires mixins::Mixin<T_Tree, Balancer>; }) {
                if (parent) {
                    self().balanceUp(parent);
                }
            }
        }
    };


    template<typename T_Tree>
    struct Size : mixins::SureIAmThat<T_Tree, Size> {
        using mixins::SureIAmThat<T_Tree, Size>::self;

        template<auto size_counter = default_size_counter>
        [[nodiscard]] size_t getSize() const {
            if (self().root)
                return (*self().root).*size_counter;
            return 0;
        }
    };

};


template<typename T_Node>
using NewLineCounter = FilteredSizeCounter<char, '\n'>::Inner<T_Node>;


template<typename ValueType = char>
using AugmentedNode = mixins::Mixins<
        ValueNode<ValueType>::template Inner,
        ParentNode,
        GetIndex,
        BubbleUp,
        BinaryNode,
        GraphViz,
        MaxDepth,
        NewLineCounter,
        SizeCounter,
        Indexable,
        InsertAt,
        Equals>;


template<typename ValueType = char>
using AVLNode = mixins::Mixins<
        ValueNode<ValueType>::template Inner,
        ParentNode,
        BubbleUp,
        BinaryNode,
        GraphViz,
        SizeCounter,
        MaxDepth,
        Equals,
        Comparable<ValueType>::template Inner>;

template<typename T_Node>
constexpr auto NewLineCounterSize = &NewLineCounter<T_Node>::size;

template<typename Value = char>
using AugmentedAVLTree = mixins::Mixins<
        TreeMixer<AugmentedNode<Value>>::template Inner,
        TreeMixer<AugmentedNode<Value>>::template Indexable,
        TreeMixer<AugmentedNode<Value>>::template InsertAt,
        TreeMixer<AugmentedNode<Value>>::template Delete,
        TreeMixer<AugmentedNode<Value>>::template Size,
        TreeMixer<AugmentedNode<Value>>::template Rotator,
        TreeMixer<AugmentedNode<Value>>::template Balancer
>;

template<typename Value = char>
using AVLTree = mixins::Mixins<
        TreeMixer<AVLNode<Value>>::template Inner,
        TreeMixer<AVLNode<Value>>::template Delete,
        TreeMixer<AVLNode<Value>>::template Rotator,
        TreeMixer<AVLNode<Value>>::template Balancer,
        TreeMixer<AVLNode<Value>>::template InsertSorted,
        TreeMixer<AVLNode<Value>>::template FindSorted,
        TreeMixer<AVLNode<Value>>::template Size
>;

template<typename T_Tree>
constexpr auto AVLTreeNewLineCounterSize = &NewLineCounter<typename T_Tree::NodeType>::size;

//endregion