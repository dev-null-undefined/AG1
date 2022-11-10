#include <algorithm>
#include <memory>
#include <optional>
#include <iterator>

// TODO: fix const methods
// TODO: ad no throw
#define AVL_TREE_TESTING 1
namespace stl {

    template<typename K>
    using sus_ptr = K *;

    template<typename T>
    void copy_array(const T & source, T & destination, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            destination[i] = source[i];
        }
    }

    template<class T>
    T & unmove(T && t) {
        return t;
    }

#if __cplusplus >= 202002L
    template<typename T, typename Compare=std::compare_three_way> // TODO: default back to less if not possible to use three way
#else

    template<typename T, typename Compare=std::less<T>>
#endif
    class AVLTree {

        using self = AVLTree<T, Compare>;
        template<typename TypePointer>
        struct avl_iterator;

        struct avl_insert_result;


        struct Node;

      public:
        using value_type = T;
        using pointer = sus_ptr<T>;
        using const_pointer = sus_ptr<const T>;
        using reference = T &;

        using iterator = struct avl_iterator<pointer>;
        using const_iterator = avl_iterator<const_pointer>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using insert_result = avl_insert_result;

        using size_type = size_t;
        using difference_type = std::ptrdiff_t;
        using value_compare = Compare;

        using descendant_ptr = std::unique_ptr<Node> self::Node::*;
      private:

        struct Node {
            explicit Node(value_type & data, Node * parent = nullptr) : data(
                    reinterpret_cast<value_type *>(data_buffer)),
                                                                        parent(parent), is_real(true) {
                new(static_cast<void *>(data_buffer)) value_type(data);
            }

            explicit Node(value_type && data, Node * parent = nullptr) : data(
                    reinterpret_cast<value_type *>(data_buffer)),
                                                                         parent(parent), is_real(true) {
                new(static_cast<void *>(data_buffer)) value_type(std::move(data));
            }

            Node() = default;

            Node(const Node & other) {
                *this = other;
            }

            Node(Node && other) {
                *this = other;
            }

            Node & operator=(const Node & other) {
                if (&other == this) return *this;
                left = nullptr;
                if (other.left) {
                    left = std::make_unique<Node>(unmove(*other.left));
                    left->parent = this;
                }
                right = nullptr;
                if (other.right) {
                    right = std::make_unique<Node>(unmove(*other.right));
                    right->parent = this;
                }
                if (!other.is_real) {
                    clear();
                    return *this;
                }
                is_real = true;
                if constexpr (std::is_trivially_copy_assignable_v<value_type>) {
                    copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                } else {
                    new(static_cast<void *>(data_buffer)) value_type(unmove(*other.data));
                }
                data = reinterpret_cast<value_type *>(data_buffer);
                return *this;
            }

            Node & operator=(Node && other) {
                if (&other == this) return *this;
                left = nullptr;
                if (other.left) {
                    left = std::make_unique<Node>(std::move(*other.left));
                    left->parent = this;
                }
                right = nullptr;
                if (other.right) {
                    right = std::make_unique<Node>(std::move(*other.right));
                    right->parent = this;
                }
                if (!other.is_real) {
                    clear();
                    return *this;
                }
                other.is_real = false;
                is_real = true;
                if constexpr (std::is_trivially_move_assignable_v<value_type>) {
                    copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                } else {
                    new(static_cast<void *>(data_buffer)) value_type(std::move(*other.data));
                }
                data = reinterpret_cast<value_type *>(data_buffer);
                return *this;
            }

            ~Node() {
                clear();
            }

            explicit operator value_type &() {
                return *data;
            }

            void clear() {
                if (is_real) {
                    data->~value_type();
                    data = nullptr;
                }
                is_real = false;
            }

            value_type * data = nullptr;
            alignas(alignof(value_type)) unsigned char data_buffer[sizeof(value_type)] = {};

            sus_ptr<Node> parent = nullptr;
            std::unique_ptr<Node> left = nullptr;
            std::unique_ptr<Node> right = nullptr;
            long long int depth = 0; // TODO: <--- this
            bool is_real = false;


            static constexpr descendant_ptr left_ptr = &Node::left;
            static constexpr descendant_ptr right_ptr = &Node::right;

            bool isDescendant() {
                return parent && parent->is_real;
            }

            bool isDescendant(const descendant_ptr & direction) {
                return parent && (parent->*direction).get() == this;
            }
        };

        value_compare comparator = {};

        enum CompareResult {
            less = -1, equivalent = 0, greater = 1, none = 2
        };

        struct Comparator {
            value_compare & comparator_inner;

            CompareResult operator()(const value_type & a, const value_type & b) {

#if __cplusplus >= 202002L
                if constexpr (std::is_convertible_v<std::invoke_result_t<decltype(comparator_inner), value_type, value_type>, std::weak_ordering>) {
                    std::weak_ordering order = comparator_inner(a, b);
                    if (order == std::weak_ordering::less) {
                        return less;
                    }
                    if (order == std::weak_ordering::greater) {
                        return greater;
                    }
                    if (order == std::weak_ordering::equivalent) {
                        return equivalent;
                    }
                } else {
#else
#endif
                static_assert(
                        std::is_same<std::invoke_result_t<decltype(comparator_inner), value_type, value_type>, bool>::value,
                        "Must be either less functor or three way comparator!");
                if (comparator_inner(a, b)) {
                    return less;
                } else if (!comparator_inner(b, a)) {
                    return equivalent;
                }
                return greater;

#if __cplusplus >= 202002L
                }
#else
#endif
            }
        };

        Comparator three_way_compare{comparator};

        template<typename TypePointer>
        struct avl_iterator
                : std::iterator<std::bidirectional_iterator_tag, value_type, difference_type, pointer, reference> {
            sus_ptr<Node> current;

            explicit avl_iterator(const sus_ptr<Node> current) : current(current) {}

            void move(bool forward) {
                auto left = Node::left_ptr;
                auto right = Node::right_ptr;
                if (forward) swap(left, right);

                if (current->*right) {
                    current = (current->*right).get();
                    while (current->*left) current = (current->*left).get();
                } else {
                    while (current->isDescendant(right)) current = current->parent;
                    current = current->parent;
                }
            }

            avl_iterator & operator--() {
                move(true);
                return *this;
            }

            avl_iterator operator--(int) {
                avl_iterator copy = *this;
                --*this;
                return copy;
            }

            avl_iterator & operator++() {
                move(false);
                return *this;
            }

            avl_iterator operator++(int) {
                avl_iterator copy = *this;
                ++*this;
                return copy;
            }


            bool operator==(const avl_iterator & other) const {
                return current == other.current;
            }

            bool operator!=(const avl_iterator & other) const {
                return !(*this == other);
            }

            value_type & operator*() {
                return *current->data;
            }
        };

        struct avl_insert_result {
            iterator iter;
            bool status;

            avl_insert_result(const iterator & iter, bool status) : iter(iter), status(status) {}

            avl_insert_result(const sus_ptr<Node> & iter, bool status) : iter({iter}), status(status) {}

            explicit avl_insert_result(const std::pair<iterator, bool> & result) : iter(result.first),
                                                                                   status(result.second) {}

            explicit avl_insert_result(const std::pair<sus_ptr<Node>, bool> & result) : iter(result.first),
                                                                                        status(result.second) {}


            explicit operator std::pair<iterator, bool>() {
                return {iter, status};
            }

            explicit operator bool() {
                return status;
            }
        };

        struct avl_find_result {
            sus_ptr<Node> node;
            CompareResult compare;

            explicit operator bool() {
                return compare == equivalent;
            }
        };

        std::unique_ptr<Node> header;
        sus_ptr<Node> root = nullptr;

        std::optional<descendant_ptr> compare(const value_type & a, const value_type & b) {
            return compare(three_way_compare(a, b));
        }

        descendant_ptr compare(CompareResult result) {
            switch (result) {
                case less:
                    return Node::right_ptr;
                case none:
                case equivalent:
                    return nullptr;
                case greater:
                    return Node::left_ptr;
            }
            return nullptr;
        }

        avl_find_result inner_find(const value_type & element) {
            sus_ptr<Node> current = root;
            while (current) {
                CompareResult compare_result = three_way_compare((current)->operator value_type &(), element);
                if (!compare_result) {
                    return {current, compare_result};
                } else {
                    descendant_ptr member_ptr = compare(compare_result);
                    if (!(current->*member_ptr)) {
                        return {current, compare_result};
                    } else {
                        current = (current->*member_ptr).get();
                    }
                }
            }
            return {current, none};
        }

        sus_ptr<Node> setRoot(value_type && data) {
            header->left = std::make_unique<Node>(std::move(data));
            root = header->left.get();
            root->parent = header.get();
            return root;
        }

        sus_ptr<Node> firstNode() const {
            sus_ptr<Node> current = root;
            while (current && current->left) {
                current = current->left.get();
            }
            return current;
        }

      public:

        AVLTree() {
            header = std::make_unique<Node>();
        }

        AVLTree(const AVLTree & other) {
            *this = other;
        }

        AVLTree(AVLTree && other) {
            *this = other;
        }

        AVLTree & operator=(const AVLTree & other) {
            if (&other == this) return *this;
            header = std::make_unique<Node>(unmove(*other.header));
            root = header->left.get();
        }

        AVLTree & operator=(AVLTree && other) {
            if (&other == this) return *this;
            header = std::make_unique<Node>(std::move(*other.header));
            root = header->left.get();
        }


        template<typename... M>
        insert_result insert(M && ... arguments) {
            value_type element(std::forward<M>(arguments)...);
            avl_find_result result = inner_find(element);
            if (result) return {result.node, false};
            if (result.compare == none) {
                setRoot(std::move(element));
                return {root, true};
            }

            descendant_ptr member_ptr = compare(result.compare);
            return {(result.node->*member_ptr = std::make_unique<Node>(std::move(element), result.node)).get(), true};
        }

        template<typename... M>
        iterator find(M && ... arguments) {
            avl_find_result result = inner_find(value_type(std::forward<M>(arguments)...));
            if (!result) return end();
            return result.node;
        }

        template<typename... M>
        size_type count(M && ... arguments) {
            return (bool) inner_find(value_type(std::forward<M>(arguments)...));
        }

        //<editor-fold desc="Iterators">
        iterator end() {
            return iterator(header.get());
        }

        iterator begin() {
            sus_ptr<Node> current = firstNode();
            if (current) return iterator(current);
            return end();
        }

        reverse_iterator rend() {
            return reverse_iterator(begin());
        }

        reverse_iterator rbegin() {
            return reverse_iterator(end());
        }

        const_iterator cend() {
            return const_iterator(header.get());
        }

        const_iterator cbegin() {
            sus_ptr<Node> current = firstNode();
            if (current) return const_iterator(current);
            return cend();
        }

        const_reverse_iterator crend() {
            return const_reverse_iterator(const_iterator(cbegin()));
        }

        const_reverse_iterator crbegin() {
            return const_reverse_iterator(const_iterator(cend()));
        }
        //</editor-fold>
    };
}
