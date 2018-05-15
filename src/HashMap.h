#ifndef AISDI_MAPS_HASHMAP_H
#define AISDI_MAPS_HASHMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <list>
#include <array>
#include <algorithm>
#include <stdexcept>

namespace aisdi {

    template<typename KeyType, typename ValueType>
    class HashMap {
        static const int MAP_SIZE = 11;

    public:
        using key_type = KeyType;
        using mapped_type = ValueType;
        using value_type = std::pair<const key_type, mapped_type>;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;
        using bucketIterator = typename std::array<std::list<value_type>, MAP_SIZE>::iterator;
        using valueTypeIterator = typename std::list<value_type>::iterator;

        class ConstIterator;

        class Iterator;

        using iterator = Iterator;
        using const_iterator = ConstIterator;

        HashMap() : size(0) {}

        HashMap(std::initializer_list<value_type> list) : HashMap() {
            std::for_each(list.begin(), list.end(),
                          [this](const value_type &v) { (*this)[v.first] = v.second; });
        }

        HashMap(const HashMap &other) : HashMap() {
            fill(other.begin(), other.end());
        }

        HashMap(HashMap &&other) {
            this->buckets = std::move(other.buckets);
            this->size = other.size;
        }


        HashMap &operator=(const HashMap &other) {
            if (this == &other) {
                return *this;
            }
            this->size = 0;
            fill(other.begin(), other.end());
            return *this;
        }

        HashMap &operator=(HashMap &&other) {
            if (this == &other) {
                return *this;
            }
            this->size = other.size;
            this->buckets = std::move(other.buckets);
            return *this;
        }

        bool isEmpty() const {
            return this->size == 0;
        }

        mapped_type &operator[](const key_type &key) {
            const auto bucket = findBucket(key);
            auto found = findInBucket(bucket, key);
            if (found == bucket->end()) {
                bucket->emplace_back(std::make_pair(key, mapped_type{}));
                ++(this->size);
                return bucket->back().second;
            }
            return (*found).second;
        }

        const mapped_type &valueOf(const key_type &key) const {
            return findOrThrow(key).second;
        }

        mapped_type &valueOf(const key_type &key) {
            return findOrThrow(key).second;
        }

        const_iterator find(const key_type &key) const {
            const auto &bucket = findBucket(key);
            auto found = findInBucket(bucket, key);
            return const_iterator(buckets, bucket, found);
        }

        iterator find(const key_type &key) {
            const auto &bucket = findBucket(key);
            auto found = findInBucket(bucket, key);
            return iterator(buckets, bucket, found);
        }

        void remove(const key_type &key) {
            const auto &bucket = findBucket(key);
            auto found = findInBucket(bucket, key);
            if (found == bucket->end()) {
                throw std::out_of_range("Map does not contain key: " + key);
            }
            bucket->erase(found);
            --(this->size);
        }

        void remove(const const_iterator &it) {
            if (it == end()) {
                throw std::out_of_range("Iterator out of range");
            }

            it.currentBucket->erase(it.iter);
            --(this->size);
        }

        size_type getSize() const {
            return this->size;
        }

        bool operator==(const HashMap &other) const {
            if (this->size != other.size) {
                return false;
            }

            return this->buckets == other.buckets;
        }

        bool operator!=(const HashMap &other) const {
            return !(*this == other);
        }

        iterator begin() {
            return iterator(buckets, buckets.begin(), buckets.begin()->begin());
        }

        iterator end() {
            return iterator(buckets, buckets.end() - 1, buckets.rbegin()->end());
        }

        const_iterator cbegin() const {
            return const_iterator(buckets, buckets.begin(), buckets.begin()->begin());
        }

        const_iterator cend() const {
            return const_iterator(buckets, buckets.end() - 1, buckets.rbegin()->end());
        }

        const_iterator begin() const {
            return const_iterator(buckets, buckets.begin(), buckets.begin()->begin());
        }

        const_iterator end() const {
            return const_iterator(buckets, buckets.end() - 1, buckets.rbegin()->end());
        }

    private:
        mutable std::array<std::list<value_type>, MAP_SIZE> buckets;
        size_type size;

        void fill(const_iterator begin, const_iterator end) {
            std::for_each(begin, end, [this](const value_type &value) { (*this)[value.first] = value.second; });
        }

        bucketIterator findBucket(const KeyType &key) const {
            return (buckets.begin() + (std::hash<key_type>{}(key) % MAP_SIZE));
        }

        value_type &findOrThrow(const key_type &key) const {
            const auto bucket = findBucket(key);
            auto bucket_it = std::find_if(
                    bucket->begin(),
                    bucket->end(), [&key](const value_type &v) { return v.first == key; });
            if (bucket_it == bucket->end()) {
                throw std::out_of_range("Map does not contain key: " + key);
            }
            return *bucket_it;
        }

        valueTypeIterator findInBucket(const bucketIterator &bucket, const key_type &key) const {
            return std::find_if(bucket->begin(), bucket->end(),
                                [&key](const value_type &v) { return v.first == key; });
        }
    };

    template<typename KeyType, typename ValueType>
    class HashMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename HashMap::const_reference;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename HashMap::value_type;
        using pointer = const typename HashMap::value_type *;
        using bucketIterator = typename HashMap::bucketIterator;
        using valueTypeIterator = typename HashMap::valueTypeIterator;

        friend class HashMap;

        explicit ConstIterator(std::array<std::list<value_type>, MAP_SIZE> &buckets,
                               const bucketIterator &currentBucket,
                               const valueTypeIterator &iter) : buckets(buckets),
                                                                currentBucket(currentBucket),
                                                                iter(iter) {
            if (iter == currentBucket->end()) {
                next();
            }
        }

        ConstIterator(const ConstIterator &other) : buckets(other.buckets), currentBucket(other.currentBucket),
                                                    iter(other.iter) {}

        ConstIterator &operator++() {
            if (isEnd()) {
                throw std::out_of_range("Index out of range");
            }
            ++iter;
            next();
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator ret = *this;
            ++*this;
            return ret;
        }

        ConstIterator &operator--() {
            if (iter == currentBucket->begin()) {
                while (currentBucket->empty() && currentBucket != buckets.begin()) {
                    --currentBucket;
                }
                if (currentBucket->empty()) {
                    throw std::out_of_range("Iterator out of range");
                }
                iter = --(currentBucket->end());
            } else {
                --iter;
            }
            return *this;
        }

        ConstIterator operator--(int) {
            ConstIterator ret = *this;
            --*this;
            return ret;
        }

        reference operator*() const {
            if (isEnd()) {
                throw std::out_of_range("Iterator out of range");
            }
            return *iter;
        }

        pointer operator->() const {
            if (isEnd()) {
                throw std::out_of_range("Iterator out of range");
            }
            return &this->operator*();
        }

        bool operator==(const ConstIterator &other) const {
            return currentBucket == other.currentBucket && iter == other.iter;
        }

        bool operator!=(const ConstIterator &other) const {
            return !(*this == other);
        }

    private:
        void next() {
            while (iter == currentBucket->end() && currentBucket != buckets.end() - 1) {
                ++currentBucket;
                iter = currentBucket->begin();
            }
        }

        bool isEnd() const {
            return iter == buckets.rbegin()->end();
        }

        std::array<std::list<value_type>, MAP_SIZE> &buckets;
        bucketIterator currentBucket;
        valueTypeIterator iter;
    };

    template<typename KeyType, typename ValueType>
    class HashMap<KeyType, ValueType>::Iterator : public HashMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename HashMap::reference;
        using pointer = typename HashMap::value_type *;
        using bucketIterator = typename HashMap::bucketIterator;
        using valueTypeIterator = typename HashMap::valueTypeIterator;

        explicit Iterator(std::array<std::list<value_type>, MAP_SIZE> &buckets,
                          const bucketIterator &currentBucket,
                          const valueTypeIterator &iter) : ConstIterator(buckets, currentBucket, iter) {}

        explicit Iterator(const ConstIterator &other)
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

#endif /* AISDI_MAPS_HASHMAP_H */
