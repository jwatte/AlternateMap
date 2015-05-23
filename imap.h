#if !defined(imap_h)
#define imap_h

#include <string>
#if !defined(NDEBUG)
#include <assert.h>
#endif

template<typename T> size_t hash(T const &key);
template<> size_t hash<std::string>(std::string const &key) {
    size_t ret = 0;
    for (auto p : key) {
        ret = ((ret + 1) * 1117) ^ p;
    }
    return ret;
}
template<> size_t hash<size_t>(size_t const &key) {
    return (size_t)key * 1117;
}
size_t hash(void * const &key) {
    return (size_t)key * 1117;
}

template<typename Key, typename Value> class imap {
    public:
        typedef Key key_t;
        typedef Value value_t;
        typedef std::pair<Key, Value> key_value_t;
        struct iterator;
        struct const_iterator;

        imap();
        ~imap();
        void clear();
        Value &operator[](Key const &key);
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;
        iterator find(Key const &key);
        const_iterator find(Key const &key) const;
        size_t size() const;
        std::pair<iterator, bool> insert(typename imap::key_value_t const &kv);
        void erase(iterator const &iter);

        struct iterator {
            public:
                iterator() : map_(nullptr), bucket_(0), item_(nullptr) {
                }
                typename imap::key_value_t &operator*() const {
                    return item_->keyval_;
                }
                typename imap::key_value_t *operator->() const {
                    return &item_->keyval_;
                }
                bool operator==(iterator const &o) const {
                    return item_ == o.item_;
                }
                bool operator!=(iterator const &o) const {
                    return item_ != o.item_;
                }
                iterator operator++() {
                    if (!map_) {
                        throw std::runtime_error("Attempt to increment the end() iterator.");
                    }
                    item *ret = item_->next_;
                    while (!ret) {
                        ++bucket_;
                        if (bucket_ == map_->bucketCount_) {
                            map_ = nullptr;
                            bucket_ = 0;
                            break;
                        }
                        ret = map_->buckets_[bucket_];
                    }
                    item_ = ret;
                    return *this;
                }
                iterator operator++(int) {
                    auto tmp(*this);
                    this->operator++();
                    return tmp;
                }

            protected:
                friend class imap;
                iterator(imap &i) {
                    map_ = &i;
                    bucket_ = 0;
                    item_ = map_->buckets_[0];
                    while (!item_) {
                        ++bucket_;
                        if (bucket_ == map_->bucketCount_) {
                            map_ = nullptr;
                            bucket_ = 0;
                            break;
                        }
                        item_ = map_->buckets_[bucket_];
                    }
                }

                imap *map_;
                size_t bucket_;
                typename imap::item *item_;
        };

        struct const_iterator : protected iterator {
            public:
                const_iterator() {}
                typename imap::key_value_t const &operator*() const {
                    return const_cast<const_iterator *>(this)->iterator::operator*();
                }
                typename imap::key_value_t const *operator->() const {
                    return const_cast<const_iterator *>(this)->iterator::operator->();
                }
                bool operator==(const_iterator const &o) const {
                    return const_cast<const_iterator *>(this)->iterator::operator==(o);
                }
                bool operator!=(const_iterator const &o) const {
                    return const_cast<const_iterator *>(this)->iterator::operator!=(o);
                }
                const_iterator operator++() {
                    return reinterpret_cast<const_iterator &>(const_cast<const_iterator *>(this)->iterator::operator++());
                }
                const_iterator operator++(int) {
                    return reinterpret_cast<const_iterator &>(const_cast<const_iterator *>(this)->iterator::operator++(1));
                }
        protected:
                friend class imap;
                const_iterator(imap &i) : iterator(i) {}
        };

    private:
        friend struct iterator;
        friend struct const_iterator;
        imap(imap const &o) = delete;
        imap &operator=(imap const &o) = delete;

        enum {
            //  InitialBucketCount = 16,
            InitialBucketCount = 4,
            LoadFactor = 4,
            //  BlockSize = 32
            BlockSize = 4
        };

        struct item {
            item(key_value_t const &kvt) : keyval_(kvt), hashval_(0), next_(0) {}
            key_value_t keyval_;
            size_t hashval_;
            item *next_;
        };
        struct item_block {
            item_block *next_;
            char items_[imap::BlockSize][sizeof(item)];
        };

        item_block *blocks_;
        item *freeList_;

        item **buckets_;
        size_t bucketCount_;
        size_t count_;

        item *newitem(key_value_t const &kvt);
        void delitem(item *i);
};



template<typename Key, typename Value> imap<Key, Value>::imap() :
blocks_(nullptr),
freeList_(nullptr),
buckets_(new imap<Key, Value>::item *[InitialBucketCount]),
bucketCount_(InitialBucketCount),
count_(0)
{
    memset(buckets_, 0, sizeof(buckets_[0]) * bucketCount_);
}

template<typename Key, typename Value> imap<Key, Value>::~imap()
{
    for (item_block *ib = blocks_; ib != NULL;)
    {
        item_block *next = ib->next_;
        delete ib;
        ib = next;
    }
    delete[] buckets_;
}

template<typename Key, typename Value> void imap<Key, Value>::clear()
{
    freeList_ = nullptr;
    item_block *ib = blocks_;
    while (ib) {
        item_block *d = ib;
        ib = ib->next_;
        delete d;
    }
    blocks_ = nullptr;
    if (bucketCount_ != InitialBucketCount) {
        delete[] buckets_;
        buckets_ = new item *[InitialBucketCount];
        bucketCount_ = InitialBucketCount;
    }
    memset(buckets_, 0, sizeof(buckets_[0]) * InitialBucketCount);
    count_ = 0;
}

template<typename Key, typename Value> Value &imap<Key, Value>::operator[](Key const &key)
{
    size_t h = hash(key);
    item *bucket = buckets_[h & (bucketCount_ - 1)];
    while (bucket) {
        if (bucket->hashval_ == h && bucket->keyval_.first == key) {
            return bucket->keyval_.second;
        }
        bucket = bucket->next_;
    }
    bucket = newitem(std::pair<Key, Value>(key, value_t()));
    return bucket->keyval_.second;
}

template<typename Key, typename Value> typename imap<Key, Value>::iterator imap<Key, Value>::begin()
{
    return iterator(*this);
}

template<typename Key, typename Value> typename imap<Key, Value>::const_iterator imap<Key, Value>::begin() const
{
    return reinterpret_cast<const_iterator &>(const_cast<imap *>(this)->begin());
}

template<typename Key, typename Value> typename imap<Key, Value>::iterator imap<Key, Value>::end()
{
    return iterator();
}

template<typename Key, typename Value> typename imap<Key, Value>::const_iterator imap<Key, Value>::end() const
{
    return const_iterator();
}

template<typename Key, typename Value> typename imap<Key, Value>::iterator imap<Key, Value>::find(Key const &key)
{
    size_t h = hash(key);
    item *i = buckets_[h & (bucketCount_ - 1)];
    while (i) {
        if (i->hashval_ == h && i->keyval_.first == key) {
            iterator ret(*this);
            ret.bucket_ = h & (bucketCount_ - 1);
            ret.item_ = i;
            return ret;
        }
        i = i->next_;
    }
    return iterator();
}

template<typename Key, typename Value> typename imap<Key, Value>::const_iterator imap<Key, Value>::find(Key const &key) const
{
    return reinterpret_cast<const_iterator &>(const_cast<imap *>(this)->find(key));
}

template<typename Key, typename Value> size_t imap<Key, Value>::size() const
{
    return count_;
}

template<typename Key, typename Value> std::pair<typename imap<Key, Value>::iterator, bool> imap<Key, Value>::insert(typename imap::key_value_t const &kv)
{
    Key const &key = kv.first;
    size_t h = hash(key);
    item *i = buckets_[h & (bucketCount_ - 1)];
    while (i) {
        if (i->hashval_ == h && i->keyval_.first == key) {
            iterator ret(*this);
            ret.bucket_ = h & (bucketCount_ - 1);
            ret.item_ = i;
            assert(i->next_ != i);
            return std::pair<iterator, bool>(ret, false);
        }
        i = i->next_;
    }
    i = newitem(kv);
    iterator ret(*this);
    ret.bucket_ = h & (bucketCount_ - 1);
    ret.item_ = i;
    assert(i->next_ != i);
    return std::pair<iterator, bool>(ret, true);
}

template<typename Key, typename Value> void imap<Key, Value>::erase(typename imap<Key, Value>::iterator const &iter)
{
    if (!iter.map_) {
        return; //  erasing end()
    }
    assert(iter.map_ == this);
    item **ip = &buckets_[iter.bucket_ & (bucketCount_ - 1)];
    while (*ip) {
        assert((*ip)->next_ != *ip);
        if (*ip == iter.item_) {
            *ip = iter.item_->next_;
            --count_;
            delitem(iter.item_);
            if (count_ == 0) {
                clear();
            }
            return;
        }
        ip = &(*ip)->next_;
    }
    throw std::runtime_error("Erasing an invalid iterator");
}

template<typename Key, typename Value> typename imap<Key, Value>::item *imap<Key, Value>::newitem(typename imap<Key, Value>::key_value_t const &kvt)
{
    if (!freeList_) {
        item_block *nb = new item_block;
        nb->next_ = blocks_;
        blocks_ = nb;
        for (int i = 1; i != BlockSize; ++i) {
            ((item *)nb->items_[i-1])->next_ = (item *)nb->items_[i];
        }
        ((item *)nb->items_[BlockSize - 1])->next_ = freeList_;
        freeList_ = (item *)nb->items_[0];
#if !defined(NDEBUG)
        item *fli = freeList_;
        int nfli = 0;
        while (fli) {
            assert(nfli < BlockSize);
            ++nfli;
            fli = fli->next_;
        }
        assert(nfli == BlockSize);
#endif
    }
    void *p = freeList_;
    freeList_ = freeList_->next_;
    item *ret = new (p) item(kvt);
    size_t h = hash(kvt.first);
    ret->hashval_ = h;
    ret->next_ = buckets_[h & (bucketCount_ - 1)];
    buckets_[h & (bucketCount_ - 1)] = ret;
    ++count_;
    if (count_ > LoadFactor * bucketCount_) {
        item **nub = new item *[bucketCount_ * 2];
        memset(nub, 0, sizeof(nub[0]) * bucketCount_ * 2);
        size_t mask = bucketCount_ * 2 - 1;
        for (size_t j = 0; j != bucketCount_; ++j) {
            item *i = buckets_[j];
            while (i) {
                item *n = i;
                i = i->next_;
                size_t nix = n->hashval_ & mask;
                n->next_ = nub[nix];
                nub[nix] = n;
            }
        }
        delete[] buckets_;
        buckets_ = nub;
        bucketCount_ = bucketCount_ * 2;
#if !defined(NDEBUG)
        size_t ncnt = 0;
        for (size_t bix = 0; bix != bucketCount_; ++bix) {
            item *bit = buckets_[bix];
            while (bit) {
                assert(ncnt < count_);
                ncnt++;
                bit = bit->next_;
            }
        }
        assert(ncnt == count_);
#endif
    }
    return ret;
}

template<typename Key, typename Value> void imap<Key, Value>::delitem(typename imap<Key, Value>::item *i)
{
    i->~item();
    i->next_ = freeList_;
    freeList_ = i;
}

#endif  //  imap_h
