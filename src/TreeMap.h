#ifndef AISDI_MAPS_TREEMAP_H
#define AISDI_MAPS_TREEMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <stdexcept>

namespace aisdi {

    template<typename KeyType, typename ValueType>
    class TreeMap {
    public:
        using key_type = KeyType;
        using mapped_type = ValueType;
        using value_type = std::pair<const key_type, mapped_type>;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;

        class ConstIterator;

        class Iterator;

        using iterator = Iterator;
        using const_iterator = ConstIterator;

        using node = struct TreeNode {
            value_type val;
            TreeNode *parent;
            TreeNode *leftChild;
            TreeNode *rightChild;
            int height;

            TreeNode() : val(std::make_pair(key_type(), mapped_type())), parent(nullptr), leftChild(nullptr),
                         rightChild(nullptr), height(0) {}

            explicit TreeNode(value_type value, TreeNode *parent = nullptr) : val(value), parent(parent),
                                                                              leftChild(nullptr),
                                                                              rightChild(nullptr), height(0) {}

            key_type key() {
                return val.first;
            }

            mapped_type &value() {
                return val.second;
            }
        };
        using node_pointer = node *;

        TreeMap() : root(nullptr), size(0) {}

        TreeMap(std::initializer_list<value_type> list) : TreeMap() {
            std::for_each(list.begin(), list.end(),
                          [this](const value_type &v) { this->operator[](v.first) = v.second; });
        }

        TreeMap(const TreeMap &other) : TreeMap() {
            fill(other);
        }

        TreeMap(TreeMap &&other) {
            this->root = other.root;
            this->size = other.size;
            other.root = nullptr;
        }

        ~TreeMap() {
            clear();
        }

        TreeMap &operator=(const TreeMap &other) {
            if (this == &other) {
                return *this;
            }
            clear();
            fill(other);
            return *this;
        }

        TreeMap &operator=(TreeMap &&other) {
            if (this == &other) {
                return *this;
            }
            clear();
            this->root = other.root;
            this->size = other.size;
            other.root = nullptr;
            return *this;
        }

        bool isEmpty() const {
            return getSize() == 0;
        }

        mapped_type &operator[](const key_type &key) {
            auto *node = &root;
            node_pointer parent = nullptr;

            if (root == nullptr) {
                root = new TreeNode(std::make_pair(key, mapped_type()));
                ++size;
                return root->value();
            }

            while (*node != nullptr && (*node)->key() != key) {
                parent = *node;
                if ((*node)->key() > key) {
                    node = &(*node)->leftChild;
                } else {
                    node = &(*node)->rightChild;
                }
            }

            if (*node != nullptr) {
                return (*node)->value();
            }
            *node = new TreeNode(std::make_pair(key, mapped_type()), parent);
            auto ret = *node;
            ++size;

            return ret->value();
        }

        const mapped_type &valueOf(const key_type &key) const {
            return (*find(key)).second;
        }

        mapped_type &valueOf(const key_type &key) {
            return (*find(key)).second;
        }

        const_iterator find(const key_type &key) const {
            return const_iterator(*this, findNode(key));
        }

        iterator find(const key_type &key) {
            return iterator(*this, findNode(key));
        }

        void remove(const key_type &key) {
            remove(find(key));
        }

        void remove(const const_iterator &it) {
            if (it == end()) {
                throw std::out_of_range("Iterator out of range");
            }

            auto nodeToDelete = it.currentNode;
            node_pointer *nodeToDeleteParentPtr = (nodeToDelete->parent == nullptr) ? &root :
                                                  (nodeToDelete->parent->leftChild == nodeToDelete) ?
                                                  &nodeToDelete->parent->leftChild :
                                                  &nodeToDelete->parent->rightChild;

            if (nodeToDelete->leftChild == nullptr && nodeToDelete->rightChild == nullptr) {
                *nodeToDeleteParentPtr = nullptr;
            } else {
                auto branch = nodeToDelete->rightChild == nullptr ? nodeToDelete->leftChild : nodeToDelete->rightChild;
                branch->parent = nodeToDelete->parent;
                *nodeToDeleteParentPtr = branch;
            }
            delete nodeToDelete;
            --size;
        }

        size_type getSize() const {
            return size;
        }

        bool operator==(const TreeMap &other) const {
            if (size != other.size) {
                return false;
            }

            for (auto &val : other) {
                if (this->valueOf(val.first) != val.second) {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const TreeMap &other) const {
            return !(*this == other);
        }

        iterator begin() {
            return iterator(*this, minElement());
        }

        iterator end() {
            return iterator(*this, nullptr);
        }

        const_iterator cbegin() const {
            return const_iterator(*this, minElement());
        }

        const_iterator cend() const {
            return const_iterator(*this, nullptr);
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator end() const {
            return cend();
        }

    private:
        node_pointer root;
        size_type size;

        node_pointer minElement() const {
            node_pointer element = root;
            while (element != nullptr && element->leftChild != nullptr) {
                element = element->leftChild;
            }
            return element;
        }

        node_pointer maxElement() const {
            node_pointer element = root;
            while (element != nullptr && element->rightChild != nullptr) {
                element = element->rightChild;
            }
            return element;
        }

        void clear() {
            auto it = begin();
            while (it != end()) {
                auto node = it.currentNode;
                ++it;
                delete (node);
            }
            root = nullptr;
            size = 0;
        }

        void fill(const TreeMap &other) {
            std::for_each(other.begin(), other.end(),
                          [this](const value_type &v) { this->operator[](v.first) = v.second; });
        }

        node_pointer findNode(const KeyType &key) const {
            node_pointer currentNode = root;
            while (currentNode != nullptr && currentNode->key() != key) {
                if (currentNode->key() > key) {
                    currentNode = currentNode->leftChild;
                } else {
                    currentNode = currentNode->rightChild;
                }
            }
            return currentNode;
        }


    };

    template<typename KeyType, typename ValueType>
    class TreeMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename TreeMap::const_reference;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename TreeMap::value_type;
        using pointer = const typename TreeMap::value_type *;

        friend class TreeMap;

        explicit ConstIterator(const TreeMap &parent, node_pointer currentNode) : parent(parent),
                                                                                  currentNode(currentNode) {}

        ConstIterator(const ConstIterator &other) : parent(other.parent), currentNode(other.currentNode) {}

        ConstIterator &operator++() {
            if (currentNode == nullptr) {
                throw std::out_of_range("Iterator out of range");
            }

            if (currentNode->rightChild != nullptr) {
                currentNode = currentNode->rightChild;
                while (currentNode->leftChild != nullptr) {
                    currentNode = currentNode->leftChild;
                }
            } else {
                while (currentNode->parent != nullptr && currentNode->parent->rightChild == currentNode) {
                    currentNode = currentNode->parent;
                }
                currentNode = currentNode->parent;
            }
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator ret = *this;
            ++*this;
            return ret;
        }

        ConstIterator &operator--() {
            if (parent.isEmpty()) {
                throw std::out_of_range("Iterator out of range");
            }

            if (currentNode == nullptr) {
                currentNode = parent.maxElement();
                return *this;
            }

            node_pointer initialValue = currentNode;
            if (currentNode->leftChild != nullptr) {
                currentNode = currentNode->leftChild;
                while (currentNode->rightChild != nullptr) {
                    currentNode = currentNode->rightChild;
                }
            } else {
                while (currentNode->parent != nullptr && currentNode->parent->leftChild == currentNode) {
                    currentNode = currentNode->parent;
                }
                if (currentNode->parent == nullptr) {
                    currentNode = initialValue;
                    throw std::out_of_range("Iterator out of range");
                }
                currentNode = currentNode->parent;
            }
            return *this;
        }

        ConstIterator operator--(int) {
            ConstIterator ret = *this;
            --*this;
            return ret;
        }

        reference operator*() const {
            if (currentNode == nullptr) {
                throw std::out_of_range("Iterator out of range");
            }
            return currentNode->val;
        }

        pointer operator->() const {
            return &this->operator*();
        }

        bool operator==(const ConstIterator &other) const {
            return this->currentNode == other.currentNode;
        }

        bool operator!=(const ConstIterator &other) const {
            return !(*this == other);
        }

    private:
        const TreeMap &parent;
        node_pointer currentNode;
    };

    template<typename KeyType, typename ValueType>
    class TreeMap<KeyType, ValueType>::Iterator : public TreeMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename TreeMap::reference;
        using pointer = typename TreeMap::value_type *;

        explicit Iterator(const TreeMap &parent, node_pointer currentNode) : ConstIterator(parent, currentNode) {}

        Iterator(const ConstIterator &other)
                : ConstIterator(other) {}

        Iterator &operator++() {
            ConstIterator::operator++();
            return *this;
        }

        Iterator operator++(int) {
            auto result = *this;
            ConstIterator::operator++();
            return result;
        }

        Iterator &operator--() {
            ConstIterator::operator--();
            return *this;
        }

        Iterator operator--(int) {
            auto result = *this;
            ConstIterator::operator--();
            return result;
        }

        pointer operator->() const {
            return &this->operator*();
        }

        reference operator*() const {
            // ugly cast, yet reduces code duplication.
            return const_cast<reference>(ConstIterator::operator*());
        }
    };

}

#endif /* AISDI_MAPS_MAP_H */
