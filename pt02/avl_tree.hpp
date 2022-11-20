#include <algorithm>
#include <memory>
#include <optional>
#include <iterator>
#include <iostream>

#ifdef AVL_TREE_TESTING

#include <cassert>
#include <sstream>
#include <fstream>

#endif

// TODO: fix const methods
// TODO: ad no throw
// TODO: initializer list constructor
// TODO: instance of comparator as argument to constructor
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
        using value_pointer = sus_ptr<T>;
        using const_value_pointer = sus_ptr<const T>;
        using value_reference = T &;
        using const_value_reference = const T &;

        using iterator = struct avl_iterator<value_pointer>;
        using const_iterator = avl_iterator<const_value_pointer>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using insert_result = avl_insert_result;

        using size_type = size_t;
        using difference_type = long long int;
        using value_compare = Compare;

        using descendant_ptr = std::unique_ptr<Node> self::Node::*;
      private:

        struct Node {
            explicit Node(value_type & data, Node * parent = nullptr) : data(
                    reinterpret_cast<value_type *>(data_buffer)), parent(parent), is_real(true) {
                new(static_cast<void *>(data_buffer)) value_type(data);
            }

            explicit Node(value_type && data, Node * parent = nullptr) : data(
                    reinterpret_cast<value_type *>(data_buffer)), parent(parent), is_real(true) {
                new(static_cast<void *>(data_buffer)) value_type(std::move(data));
            }

            Node() = default;

            Node(const Node & other) {
                *this = other;
                maxDepth = other.maxDepth;
            }

            Node(Node && other) {
                *this = other;
                maxDepth = other.maxDepth;
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
                data = reinterpret_cast<value_type *>(data_buffer);
                if constexpr (std::is_trivially_copy_assignable_v<value_type>) {
                    if (is_real) {
                        copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                        return *this;
                    }
                } else {
                    clear();
                }
                is_real = true;
                if constexpr (std::is_trivially_copy_constructible_v<value_type>) {
                    copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                } else {
                    new(static_cast<void *>(data_buffer)) value_type(unmove(*other.data));
                }
                return *this;
            }

            Node & operator=(Node && other) {
                if (&other == this) return *this;
                left = nullptr;
                if (other.left) {
                    // TODO: Could possibly cause issue if node have some value_pointer to root node
                    left = std::unique_ptr<Node>(std::move(other.left));
                    left->parent = this;
                }
                right = nullptr;
                if (other.right) {
                    right = std::unique_ptr<Node>(std::move(other.right));
                    right->parent = this;
                }
                if (!other.is_real) {
                    clear();
                    return *this;
                }
                data = reinterpret_cast<value_type *>(data_buffer);
                other.is_real = false;
                other.data = nullptr;
                if constexpr (std::is_trivially_move_assignable_v<value_type>) {
                    if (is_real) {
                        copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                        return *this;
                    }
                } else {
                    clear();
                }
                is_real = true;
                if constexpr (std::is_trivially_move_constructible_v<value_type>) {
                    copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                } else {
                    if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
                        new(static_cast<void *>(data_buffer)) value_type(std::move(*other.data));
                    } else {
                        new(static_cast<void *>(data_buffer)) value_type(unmove(*other.data));
                    }
                }
                return *this;
            }

            ~Node() {
                clear();
            }

            void updateMaxDepth() {
                maxDepth = 1;
                if (left) {
                    maxDepth = std::max(left->maxDepth + 1, maxDepth);
                }
                if (right) {
                    maxDepth = std::max(right->maxDepth + 1, maxDepth);
                }
            }

            difference_type sign() const {
                difference_type sign = 0;
                if (left) sign -= left->maxDepth;
                if (right) sign += right->maxDepth;
                return sign;
            }

            explicit operator value_type &() {
                return *data;
            }

            value_reference operator*() {
                return *data;
            }

            const_value_reference dataRef() const {
                return *data;
            }

            value_reference dataRef() {
                return *data;
            }

            void clear() {
                if (is_real) {
                    data->~value_type();
                    data = nullptr;
                }
                is_real = false;
            }

            friend std::ostream & operator<<(std::ostream & out, const self::Node & node) {
                return out << "\"" << node.dataRef() << ",s " << node.sign() << ",m " << node.maxDepth << "\"";
            }


            value_type * data = nullptr;
            alignas(alignof(value_type)) unsigned char data_buffer[sizeof(value_type)] = {};

            sus_ptr<Node> parent = nullptr;
            std::unique_ptr<Node> left = nullptr;
            std::unique_ptr<Node> right = nullptr;
            difference_type maxDepth = 1; // TODO: <--- this
            bool is_real = false;


            static constexpr descendant_ptr left_ptr = &Node::left;
            static constexpr descendant_ptr right_ptr = &Node::right;

            bool isDescendant() {
                return parent && parent->is_real;
            }

            bool isDescendant(descendant_ptr direction) {
                return parent && (parent->*direction).get() == this;
            }
        };

        value_compare comparator = {};

        enum class CompareResult {
            less = -1, equivalent = 0, greater = 1, none = 2
        };

        CompareResult three_way_compare(const value_type & a, const value_type & b) const {

#if __cplusplus >= 202002L
            if constexpr (std::is_convertible_v<std::invoke_result_t<decltype(comparator), value_type, value_type>, std::weak_ordering>) {
                std::weak_ordering order = comparator(a, b);
                if (order == std::weak_ordering::less) {
                    return CompareResult::less;
                }
                if (order == std::weak_ordering::greater) {
                    return CompareResult::greater;
                }
                if (order == std::weak_ordering::equivalent) {
                    return CompareResult::equivalent;
                }
            } else {
#else
#endif
            static_assert(
                    std::is_same<std::invoke_result_t<decltype(comparator), value_type, value_type>, bool>::value,
                    "Must be either less functor or three way comparator!");
            if (comparator(a, b)) {
                return CompareResult::less;
            } else if (!comparator(b, a)) {
                return CompareResult::equivalent;
            }
            return CompareResult::greater;

#if __cplusplus >= 202002L
            }
#else
#endif
        }

        template<typename TypePointer>
        struct avl_iterator
                : std::iterator<std::bidirectional_iterator_tag, value_type, difference_type, value_pointer, value_reference> {
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
                return current->dataRef();
            }

            value_pointer operator->() {
                return current->data;
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
                return compare == CompareResult::equivalent;
            }
        };

        std::unique_ptr<Node> header;
        sus_ptr<Node> root = nullptr;

        std::optional<descendant_ptr> compare(const value_type & a, const value_type & b) {
            return compare(three_way_compare(a, b));
        }

        descendant_ptr compare(CompareResult result) const {
            switch (result) {
                case CompareResult::less:
                    return Node::right_ptr;
                case CompareResult::none:
                case CompareResult::equivalent:
                    return nullptr;
                case CompareResult::greater:
                    return Node::left_ptr;
            }
            return nullptr;
        }

        avl_find_result inner_find(const value_type & element) const {
            sus_ptr<Node> current = root;
            while (current) {
                CompareResult compare_result = three_way_compare(current->dataRef(), element);
                if (compare_result == CompareResult::equivalent) {
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
            return {current, CompareResult::none};
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

        sus_ptr<Node> rotate(sus_ptr<Node> toRotate, descendant_ptr direction) {
            descendant_ptr right = direction == Node::left_ptr ? Node::right_ptr : Node::left_ptr;
            descendant_ptr left = direction;
            descendant_ptr parentDirection = toRotate->isDescendant(Node::left_ptr) ? Node::left_ptr : Node::right_ptr;
            sus_ptr<Node> parent = toRotate->parent;
#ifdef AVL_TREE_TESTING
            assert(toRotate->isDescendant(parentDirection));
#endif

            std::unique_ptr<Node> pivot = std::move(parent->*parentDirection);
            sus_ptr<Node> pivotRaw = pivot.get();
            std::unique_ptr<Node> rightSub = std::move(pivotRaw->*right);
            std::unique_ptr<Node> rightLeftSub = std::move((*rightSub).*left);
            /*
             *                     parent
             *                    /  <- parentDirection
             *              Pivot
             *              /   \
             *             /     \ <- right
             *          node     rightSub
             *                    /  <- left
             *                   /
             *                  rightLeftSub
             */

            rightSub->parent = parent;
            /*
             *                        parent
             *  parentDirection ->   /  |
             *                 Pivot    |
             *                 /        |
             *                /         ^ <- right
             *             node     rightSub
             *                       /  <- left
             *                      /
             *                     rightLeftSub
             */
            rightSub.get()->*left = std::unique_ptr<Node>(std::move(pivot));
            /*
             *                      parent
             *  parentDirection ->  /  |
             *        Both ways -> /   |
             *                    /    |
             *                   /     ^
             *                  |  rightSub
             *                  |   /
             *                  ^  /
             *                  Pivot
             *                  /
             *                 /
             *               node
             */

            pivotRaw = (rightSub.get()->*left).get();
            pivotRaw->parent = rightSub.get();
            /*
             *                      parent
             *  parentDirection ->  /  |
             *       only child -> /   |
             *                    /    |
             *                   /     ^
             *                  |  rightSub
             *                  |   /
             *                  |  / <- both ways
             *                  Pivot
             *                  /
             *                 /
             *               node
             */
            pivotRaw->*right = std::unique_ptr<Node>(std::move(rightLeftSub));
            if (pivotRaw->*right) {
                (pivotRaw->*right)->parent = pivotRaw;
            }

            /*
             *                      parent
             *  parentDirection ->  /  |
             *       only child -> /   |
             *                    /    |
             *                   /     ^
             *                  |  rightSub
             *                  |   /
             *                  |  /
             *                  Pivot
             *                  /   \  <- both ways
             *                 /     \
             *               node    rightLeftSub
             */
            pivotRaw->updateMaxDepth();
            rightSub->updateMaxDepth();

            parent->*parentDirection = std::unique_ptr<Node>(std::move(rightSub));
            if (!parent->is_real) {
#ifdef AVL_TREE_TESTING
                assert(parentDirection == Node::left_ptr);
#endif
                root = (parent->*parentDirection).get();
            }
            /*
             *                           parent
             *       parentDirection ->  /  <- both ways
             *                          /
             *                     rightSub
             *                      /
             *                     /
             *                  Pivot
             *                  /   \  <- both ways
             *                 /     \
             *               node    rightLeftSub
             */
            return (parent->*parentDirection).get();
        }

        difference_type minMax(difference_type value, difference_type min = -1, difference_type max = 1) {
            return std::min(max, std::max(min, value));
        }


        void updateUp(sus_ptr<Node> from) {
            value_reference value = *from->data;

            sus_ptr<Node> current = from;
            sus_ptr<Node> parent = current->parent;

            while (parent->is_real && parent->parent) {

                difference_type depth = current->maxDepth + 1;

                bool updated = false;

                if (parent->maxDepth < depth) {
                    updated = true;
                    parent->maxDepth = depth;
                }

                difference_type parentSign = parent->sign();
                difference_type sign = current->sign();

                bool doubleRotate = minMax(parentSign) != minMax(sign);

                if (parentSign < -1) {
                    if (doubleRotate) {
                        rotate(current, Node::left_ptr);
                        current = rotate(parent, Node::right_ptr);
                    } else {
                        current = rotate(parent, Node::right_ptr);
                    }
                } else if (parentSign > 1) {
                    if (doubleRotate) {
                        rotate(current, Node::right_ptr);
                        current = rotate(parent, Node::left_ptr);
                    } else {
                        current = rotate(parent, Node::left_ptr);
                    }
                } else {
                    current = parent;
                    if (!updated) {
                        break;
                    }
                }
                parent = current->parent;
            }
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
            return *this;
        }

        AVLTree & operator=(AVLTree && other) {
            if (&other == this) return *this;
            header = std::make_unique<Node>(std::move(*other.header));
            root = header->left.get();
            return *this;
        }


        template<typename... M>
        insert_result insert(M && ... arguments) {
            value_type element(std::forward<M>(arguments)...);
            avl_find_result result = inner_find(element);
            if (result) return {result.node, false};
            if (result.compare == CompareResult::none) {
                setRoot(std::move(element));
                return {root, true};
            }

            descendant_ptr member_ptr = compare(result.compare);
            result.node->*member_ptr = std::make_unique<Node>(std::move(element), result.node);

            sus_ptr<Node> inserted = (result.node->*member_ptr).get();
            updateUp(inserted);
            return {inserted, true};
        }

        template<typename... M>
        iterator find(M && ... arguments) const {
            avl_find_result result = inner_find(value_type(std::forward<M>(arguments)...));
            if (!result) return end();
            return iterator(result.node);
        }

        template<typename... M>
        size_type count(M && ... arguments) const {
            return (bool) inner_find(value_type(std::forward<M>(arguments)...));
        }

        //<editor-fold desc="Iterators">
        iterator end() const {
            return iterator(header.get());
        }

        iterator begin() const {
            sus_ptr<Node> current = firstNode();
            if (current) return iterator(current);
            return end();
        }

        reverse_iterator rend() const {
            return reverse_iterator(begin());
        }

        reverse_iterator rbegin() const {
            return reverse_iterator(end());
        }

        const_iterator cend() const {
            return const_iterator(header.get());
        }

        const_iterator cbegin() const {
            sus_ptr<Node> current = firstNode();
            if (current) return const_iterator(current);
            return cend();
        }

        const_reverse_iterator crend() const {
            return const_reverse_iterator(const_iterator(cbegin()));
        }

        const_reverse_iterator crbegin() const {
            return const_reverse_iterator(const_iterator(cend()));
        }
        //</editor-fold>
    };
}
