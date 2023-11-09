#include <iostream>

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

template<typename T_Node>
struct BinaryNode : mixins::SureIAmThat<T_Node, BinaryNode> {
    using mixins::SureIAmThat<T_Node, BinaryNode>::self;

    void print_value() {
        std::cout << self().value << std::endl;
    }

    void update() {
        std::cout << "Called update in BinaryNode" << std::endl;
    };

    T_Node *left = nullptr, *right = nullptr;
};

template<typename T_Node>
struct ParentNode : mixins::SureIAmThat<T_Node, ParentNode> {
    using mixins::SureIAmThat<T_Node, ParentNode>::self;

    T_Node *parent = nullptr;
};

template<typename T_Node>
struct GraphViz : mixins::SureIAmThat<T_Node, GraphViz> {
    using mixins::SureIAmThat<T_Node, GraphViz>::self;
private:

    void bst_print_dot_null(T_Node &node, int nullcount, std::ostream &output) const {
        output << "    null" << nullcount << " [shape=point];\n";
        output << "    " << node.value << " -> null" << nullcount << ";\n";
    }

    void bst_print_dot_aux(T_Node &node, std::ostream &output) const {
        static int nullcount = 0;
        if (node.parent) {
            output << "    " << node.value << " -> " << node.parent->value << "[style=dotted];\n";
        }

        if (node.left) {
            output << "    " << node.value << " -> " << node.left->value << ";\n";
            bst_print_dot_aux(*node.left, output);
        } else
            bst_print_dot_null(node, nullcount++, output);

        if (node.right) {
            output << "    " << node.value << " -> " << node.right->value << ";\n";
            bst_print_dot_aux(*node.right, output);
        } else
            bst_print_dot_null(node, nullcount++, output);
    }

public:
    void generateGraph(std::ostream &output = std::cout) {
        output << "digraph BST {\n";
        output << "    node [fontname=\"Arial\"];\n";

        if (!self().right && !self().left)
            output << "    " << self().value << ";\n";
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

        void update() {
            std::cout << "Called update in ValueNode" << std::endl;
        };

        void print_left() {
            if (self().left)
                self().left->print_value();
            else
                std::cout << "No left node" << std::endl;
        }

        void print_right() {
            if (self().right)
                self().right->print_value();
            else
                std::cout << "No right node" << std::endl;
        }

        void print_parent() const {
            std::cout << self().parent << std::endl;
        }

        T value;
    };
};

#define ExecAll(FunName, Function)                                                             \
namespace mixins {                                                                             \
    template<typename T, typename ...Args>    \
    void FunName(T && self, Args && ... args) {                                                \
        _##FunName<T,typename std::remove_reference_t<T>::template N_type<0>,Args...>(std::forward<T>(self),std::forward<Args>(args)...);\
    }                                                                                          \
                                                                                               \
    template<typename T, typename Current, typename ... Args>    \
    void _##FunName(T && self, Args && ... args) {                                                                  \
        if constexpr (std::remove_reference_t<T>::template hasNext<Current>()) {               \
            _##FunName<T, typename std::remove_reference_t<T>::template Next_type<Current>>(self);\
        }                                                                                      \
        if constexpr (requires(T && t) {{ static_cast<Current &&>(t).Function(std::forward<Args>(args)...) }; }) {        \
            static_cast<Current &&>(self).Function(std::forward<Args>(args)...);                                          \
        }                                                                                      \
    }                                                                                          \
}                                                                                              \

ExecAll(printAll, print_value)
ExecAll(updateAll, update)

auto main() -> signed {
    using NodeType = mixins::Mixins<ValueNode<int>::Inner, ParentNode, BinaryNode, GraphViz>;
    NodeType node;
    node.value = 4;
    node.print_left();
    node.print_right();
    node.print_value();
    node.print_parent();
    node.generateGraph();

    node.right = new NodeType;
    node.right->parent = &node;
    node.right->value = 5;

    node.generateGraph();

    updateAll(node);
    printAll(node);

    return 0;
}