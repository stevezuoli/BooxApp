#ifndef IMAGE_ITEMS_MANAGER_H_
#define IMAGE_ITEMS_MANAGER_H_

#include <QHash>

#include "onyx/sys/sys_utils.h"

#include "image_item.h"
#include "image_render_policy.h"

using namespace vbf;

namespace image
{

static const unsigned int DEFAULT_MEM_LIMITATION = 15 * 1024 * 1024;

template <typename KeyType, typename ValueType>
class ItemsManager
{
public:
    typedef QHash<KeyType, shared_ptr<ValueType> > CacheMap;

public:
    ItemsManager();
    virtual ~ItemsManager();

    inline void setMemoryLimit(const unsigned int bytes) { mem_limit_bytes_ = bytes; }
    inline size_t itemCount() const { return images_.size(); }
    inline CacheMap & images() { return images_; }

    bool makeEnoughMemory(const unsigned int dm,
                          const KeyType & key,
                          vbf::RenderPolicy *render_policy);

    shared_ptr<ValueType> getImage(const KeyType & key);
    void addImage(const KeyType & key, shared_ptr<ValueType> value);
    bool removeImage(const KeyType & key);
    void invalidateAll();
    void removeInvalid();

    void clear();

private:
    /// Remove an image
    bool removeImageData(const KeyType & key,
                         vbf::RenderPolicy *render_policy);

    /// Recalculate length of all images
    /// the total length won't be updated unless calling this function
    void recalcTotalLength();

private:
    // the images cache
    CacheMap images_;
    unsigned int mem_limit_bytes_;
    int used_mem_bytes_;
};

template <typename KeyType, typename ValueType>
ItemsManager<KeyType,ValueType>::ItemsManager()
  : images_()
  , mem_limit_bytes_(DEFAULT_MEM_LIMITATION)
  , used_mem_bytes_(0)
{
}

template <typename KeyType, typename ValueType>
ItemsManager<KeyType,ValueType>::~ItemsManager()
{
    clear();
}

template <typename KeyType, typename ValueType>
shared_ptr<ValueType> ItemsManager<KeyType,ValueType>::getImage(const KeyType & key)
{
    typename CacheMap::iterator iter = images_.find(key);
    if (iter != images_.end())
    {
        return iter.value();
    }
    return shared_ptr<ValueType>();
}

template <typename KeyType, typename ValueType>
void ItemsManager<KeyType,ValueType>::addImage(const KeyType & key,
                                               shared_ptr<ValueType> value)
{
    images_[key] = value;
}

template <typename KeyType, typename ValueType>
bool ItemsManager<KeyType,ValueType>::removeImage(const KeyType & key)
{
    // remove all of the images which has key
    typename CacheMap::iterator iter = images_.find(key);
    if (iter != images_.end())
    {
        images_.erase(iter);
        return true;
    }
    return false;
}

template <typename KeyType, typename ValueType>
void ItemsManager<KeyType,ValueType>::clear()
{
    images_.clear();
    used_mem_bytes_ = 0;
}


template <typename KeyType, typename ValueType>
bool ItemsManager<KeyType,ValueType>::makeEnoughMemory(const unsigned int dm,
                                                       const KeyType & key,
                                                       vbf::RenderPolicy *render_policy)
{
    if (mem_limit_bytes_ > 0)
    {  // Memory limit is enabled
        if (dm >= mem_limit_bytes_)
        {
            return false;
        }

        // TODO(hjiang): Isn't this too expensive?
        recalcTotalLength();
        if (dm + used_mem_bytes_ > mem_limit_bytes_)
        {
            unsigned int left_size = mem_limit_bytes_ >> 1;
            while (dm + used_mem_bytes_ > left_size)
            {
                if (!removeImageData(key, render_policy))
                {
                    return false;
                }
            }
        }
    }
    else
    {
        while (sys::systemFreeMemory() < DEFAULT_MEM_LIMITATION)
        {
            if (!removeImageData(key, render_policy))
            {
                return false;
            }
        }
    }
    return true;
}

template <typename KeyType, typename ValueType>
void ItemsManager<KeyType,ValueType>::recalcTotalLength()
{
    // TODO(hjiang): I don't think this works. To know the actual
    // memory used by the hash map itself, you need to call
    // QHash::capacity(). Each image might also have externally
    // allocated space. I don't think one can easily calculate the
    // size of memory consumed by the images.
    used_mem_bytes_ = 0;
    for (typename CacheMap::iterator iter = images_.begin();
         iter != images_.end();
         ++iter)
    {
        used_mem_bytes_ += iter.value()->length();
    }
}

template <typename KeyType, typename ValueType>
bool ItemsManager<KeyType,ValueType>::removeImageData(const KeyType & key,
                                                      vbf::RenderPolicy *render_policy)
{
    // remove the out-of-date page based on the remove strategy
    typename CacheMap::iterator iter = images_.begin();
    typename CacheMap::iterator remove_iter = iter;

    while(iter != images_.end())
    {
        int ret = compare(*(iter.value()),
                          *(remove_iter.value()),
                          render_policy);
        if (ret < 0)
        {
            remove_iter = iter;
        }
        iter++;
    }

    if (remove_iter.value()->locked() || remove_iter.value()->image() == 0)
    {
        return false;
    }

    shared_ptr<ValueType> dst_imge = getImage(key);
    assert(dst_imge.get());
    if (dst_imge == remove_iter.value() ||
        comparePriority(*remove_iter.value(),
                        *dst_imge,
                        render_policy) > 0)
    {
        // if the priority of removing page is higher than destination
        // page, return
        return false;
    }

    printf("Remove Image:%d, %s\n\n", remove_iter.value()->index(),
           remove_iter.value()->name().toUtf8().constData());

    // delete data of the page
    // sub the total length at first
    used_mem_bytes_ -= remove_iter.value()->length();

    // remove the bitmap and other records, reset the status at the time
    remove_iter.value()->clearPage();
    return true;
}

template <typename KeyType, typename ValueType>
void ItemsManager<KeyType,ValueType>::invalidateAll()
{
    for (typename CacheMap::iterator iter = images_.begin();
         iter != images_.end();
         ++iter)
    {
        iter.value()->invalidate();
        iter.value()->setIndex(-1);
    }
}

template <typename KeyType, typename ValueType>
void ItemsManager<KeyType,ValueType>::removeInvalid() {
    for (typename CacheMap::iterator iter = images_.begin();
         iter != images_.end();
         /* */)
    {
        if (!iter.value()->valid())
        {
            iter = images_.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

};
#endif
