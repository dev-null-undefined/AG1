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
            Node() = default;

            Node(const Node & other) {
                *this = other;
                maxDepth = other.maxDepth;
            }

            Node(Node && other) {
                *this = other;
                maxDepth = other.maxDepth;
            }

            template<typename... M>
            void construct(M && ... args) {
                clear();
                data = reinterpret_cast<value_type *>(data_buffer);
                is_real = true;
                new(data) value_type(std::forward<M>(args)...);
            }


            void moveData(Node & other) {
                if (!other.is_real) {
                    clear();
                    return;
                }
                other.is_real = false;
                other.data = nullptr;
                if constexpr (std::is_trivially_move_assignable_v<value_type>) {
                    if (is_real) {
                        copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                        return;
                    }
                } else {
                    clear();
                }
                data = reinterpret_cast<value_type *>(data_buffer);
                is_real = true;
                if constexpr (std::is_trivially_move_constructible_v<value_type>) {
                    copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                } else {
                    if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
                        new(static_cast<void *>(data_buffer)) value_type(std::move(*other.data));
                    } else {
                        new(static_cast<void *>(data_buffer)) value_type(*other.data);
                    }
                }
            }


            void copyData(const Node & other) {
                if (!other.is_real) {
                    clear();
                    return;
                }
                data = reinterpret_cast<value_type *>(data_buffer);
                if constexpr (std::is_trivially_copy_assignable_v<value_type>) {
                    if (is_real) {
                        copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                        return;
                    }
                } else {
                    clear();
                }
                data = reinterpret_cast<value_type *>(data_buffer);
                is_real = true;
                if constexpr (std::is_trivially_copy_constructible_v<value_type>) {
                    copy_array(other.data_buffer, data_buffer, sizeof(value_type));
                } else {
                    new(static_cast<void *>(data_buffer)) value_type(*other.data);
                }
            }

            Node & operator=(const Node & other) {
                if (&other == this) return *this;
                left = nullptr;
                if (other.left) {
                    left = std::make_unique<Node>(*other.left);
                    left->parent = this;
                }
                right = nullptr;
                if (other.right) {
                    right = std::make_unique<Node>(*other.right);
                    right->parent = this;
                }
                copyData(other);
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
                moveData(other);
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

            size_type leftCount() {
                if (left) return left->count;
                return 0;
            }

            size_type rightCount() {
                if (right) return right->count;
                return 0;
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
            difference_type maxDepth = 1;
            size_type count = 0;
            bool is_real = false;


            static constexpr descendant_ptr left_ptr = &Node::left;
            static constexpr descendant_ptr right_ptr = &Node::right;

            bool isDescendant() {
                return parent && parent->is_real;
            }

            bool isDescendant(descendant_ptr direction) {
                return parent && (parent->*direction).get() == this;
            }

            descendant_ptr parentDirection() const {
#if AVL_TREE_TESTING
                assert(parent);
#endif
                return parent->left.get() == this ? left_ptr : right_ptr;
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

        sus_ptr<Node> setRoot(std::unique_ptr<Node> && data) {
            header->left = std::move(data);
            root = header->left.get();
            root->parent = header.get();
            root->count = 1;
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
            descendant_ptr parentDirection = toRotate->parentDirection();
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
             *                 /   \    |
             *                /     \   ^
             *             node     rightSub
             *                       /  <- left
             *                      /
             *                     rightLeftSub
             */
            pivotRaw->*right = nullptr;
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

            pivotRaw->count -= 1;
            if (rightSub.get()->*right) pivotRaw->count -= (rightSub.get()->*right)->count;
            rightSub->count += 1;
            if (pivotRaw->*left) rightSub->count += (pivotRaw->*left)->count;

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

        void updateUp(sus_ptr<Node> from) {
            value_reference value = *from->data;

            sus_ptr<Node> current = from;
            sus_ptr<Node> parent = current->parent;

            while (parent->is_real && parent->parent) {
                difference_type before = parent->maxDepth;
                parent->updateMaxDepth();

                difference_type parentSign = parent->sign();
                difference_type sign = current->sign();

                if (parentSign < -1) {
                    if (sign > 0) {
                        rotate(current, Node::left_ptr);
                        current = rotate(parent, Node::right_ptr);
                    } else {
                        current = rotate(parent, Node::right_ptr);
                    }
                } else if (parentSign > 1) {
                    if (sign < 0) {
                        rotate(current, Node::right_ptr);
                        current = rotate(parent, Node::left_ptr);
                    } else {
                        current = rotate(parent, Node::left_ptr);
                    }
                } else {
                    current = parent;
                    if (before == parent->maxDepth) {
                        break;
                    }
                }
                parent = current->parent;
            }
        }

        void deleteNode(sus_ptr<Node> target) {
            sus_ptr<Node> parent = target->parent;
            if (target->left && target->right) {
                iterator it{target};
                if (target->left->maxDepth > target->right->maxDepth) {
                    ++it;
                } else {
                    --it;
                }
                target->copyData(*it.current);
                deleteNode(it.current);
            } else {
                sus_ptr<Node> ptr = target;
                while (ptr->is_real) {
                    ptr->count--;
                    ptr = ptr->parent;
                }
                std::unique_ptr<Node> child = std::move(target->left ? target->left : target->right);
                descendant_ptr direction = target->parentDirection();

                if (child) {
                    child->parent = parent;
                }
                parent->*direction = std::move(child);
                if (!parent->is_real) {
                    root = parent->left.get();
                } else {
                    if (parent->*direction) {
                        updateUp((parent->*direction).get());
                    } else {
//                        updateUp(parent);
//                        if (parent->left)updateUp(parent->left.get());
//                        if (parent->right)updateUp(parent->right.get());
                    }
                }
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
            *this = std::move(other);
        }

        AVLTree & operator=(const AVLTree & other) {
            if (&other == this) return *this;
            header = std::make_unique<Node>(*other.header);
            root = header->left.get();
            return *this;
        }

        AVLTree & operator=(AVLTree && other) {
            if (&other == this) return *this;
            header = std::move(other.header);
            root = header->left.get();
            other.root = nullptr;
            return *this;
        }


        template<typename... M>
        insert_result insert(M && ... arguments) {
            std::unique_ptr<Node> node = std::make_unique<Node>();
            node->construct(std::forward<M>(arguments)...);

            avl_find_result result = inner_find(node->dataRef());
            if (result) return {result.node, false};
            if (result.compare == CompareResult::none) {
                setRoot(std::move(node));
                return {root, true};
            }

            descendant_ptr member_ptr = compare(result.compare);
            result.node->*member_ptr = std::move(node);
            (result.node->*member_ptr)->parent = result.node;

            sus_ptr<Node> inserted = (result.node->*member_ptr).get();

            sus_ptr<Node> ptr = inserted;
            while (ptr->is_real) {
                ptr->count++;
                ptr = ptr->parent;
            }

            updateUp(inserted);
            return {inserted, true};
        }

        template<typename... M>
        bool remove(M && ... arguments) {
            value_type element(std::forward<M>(arguments)...);
            avl_find_result result = inner_find(element);
            if (result) {
                deleteNode(result.node);
                return true;
            }
            return false;
        }

        iterator operator[](size_type index) {
            index++;
            if (index > root->count) return end();
            sus_ptr<Node> current = root;
            while (current->leftCount() + 1 != index) {
                if (current->count > index) {
                    current = current->left.get();
                } else {
                    index -= current->leftCount() + 1;
                    current = current->right.get();
                }
            }
            return iterator(current);
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
        iterator begin() {
            sus_ptr<Node> current = firstNode();
            if (current) return iterator(current);
            return end();
        }

        iterator end() {
            return iterator(header.get());
        }

        const_iterator begin() const {
            sus_ptr<Node> current = firstNode();
            if (current) return const_iterator(current);
            return end();
        }

        const_iterator end() const {
            return const_iterator(header.get());
        }

        reverse_iterator rbegin() {
            return reverse_iterator(end());
        }

        reverse_iterator rend() {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }

        const_iterator cbegin() const {
            return begin();
        }

        const_iterator cend() const {
            return end();
        }

        const_reverse_iterator crbegin() const {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator crend() const {
            return const_reverse_iterator(begin());
        }
        //</editor-fold>
    };
}
