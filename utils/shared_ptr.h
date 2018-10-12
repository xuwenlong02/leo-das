#ifndef SRC_COMPONENTS_INCLUDE_UTILS_SHARED_PTR_H_
#define SRC_COMPONENTS_INCLUDE_UTILS_SHARED_PTR_H_

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <macro.h>
#include "atomic.h"
#include <logger.h>

namespace utils {

class RefrenceObj
{
public:
    RefrenceObj():
        mReferenceCounter(0) {
    }

    virtual ~RefrenceObj() {
//        dropReference();

    }

public:
    int refenceCount() {
        return mReferenceCounter;
    }

    int  addReference() {
        return atomic_post_inc(&mReferenceCounter);;
    }

    int  dropReference() {
        if (mReferenceCounter == 0) {
            return 0;
        }
        return atomic_post_dec(&mReferenceCounter);
    }

    bool valid() {
        return (mReferenceCounter > 0);
    }
private:
    /**
     * @brief Pointer to reference counter.
     **/

    uint32_t mReferenceCounter;
};

/**
 * @brief Shared pointer.
 *
 * Pointer to an object with reference counting.
 * Object will be automatically deallocated when last shared
 * pointer is destroyed.
 *
 * @tparam ObjectType Type of wrapped object.
 **/
template <class ObjectType>
class SharedPtr {

 public:
  /**
   * @brief Constructor.
   *
   * Initialize shared pointer with wrapped object.
   * Reference counter will be initialized to 1.
   *
   * @param Object Wrapped object.
   **/
  SharedPtr(RefrenceObj* Object);

  SharedPtr();

  /**
   * @brief Copy constructor.
   *
   * Initialize shared pointer with another shared pointer.
   * Reference counter will be incremented.
   *
   * @param Other Other shared pointer.
   **/
  SharedPtr(const SharedPtr<ObjectType>& Other);

  /**
   * @brief Destructor.
   *
   * Decrement reference counter and destroy wrapped object
   * if reference counter reaches zero.
   **/
  ~SharedPtr();

  /**
   * @brief Assignment operator.
   *
   * Drop reference to currently referenced object and add
   * reference to assigned object.
   *
   * @param Other Shared pointer to an object
   *              that must be referenced.
   *
   * @return Reference to this shared pointer.
   **/
  SharedPtr<ObjectType>& operator=(const SharedPtr<ObjectType>& Other);

  bool operator==(const SharedPtr<ObjectType>& Other) const;

  bool operator<(const SharedPtr<ObjectType>& other) const;

  /**
   * @brief Member access operator.
   *
   * @return Wrapped object.
   **/
  ObjectType* operator->() const;

  ObjectType& operator*() const;
  operator bool() const;

  ObjectType* get() const;

  /**
   * @return true if mObject not NULL
   */
  bool valid() const;

 private:

  /**
   * @brief Wrapped object.
   **/
  RefrenceObj* mObject;
};

template <class ObjectType>
SharedPtr<ObjectType>::SharedPtr(utils::RefrenceObj *Object)
    : mObject(Object) {
    //DCHECK(Object != NULL);
    if (Object)
        mObject->addReference();
}

template <class ObjectType>
SharedPtr<ObjectType>::SharedPtr()
    : mObject(0) {}

template <class ObjectType>
SharedPtr<ObjectType>::SharedPtr(
        const SharedPtr<ObjectType> &Other)
    : mObject(0) {
    *this = Other;
}

template <class ObjectType>
SharedPtr<ObjectType>::~SharedPtr() {
    if (mObject) {
        int inc = mObject->dropReference();
        if (inc == 1) {
            delete mObject;
            LOG_ERR("delete %p",mObject);
            mObject = NULL;
        }
    }
}

template <class ObjectType>
SharedPtr<ObjectType>& utils::SharedPtr<ObjectType>::operator=(
        const SharedPtr<ObjectType>& Other) {
    if (mObject)
        mObject->dropReference();
    mObject = Other.mObject;

    if (mObject)
        mObject->addReference();

    return *this;
}

template <class ObjectType>
bool SharedPtr<ObjectType>::operator==(
        const SharedPtr<ObjectType>& Other) const {
    return (mObject == Other.mObject);
}

template <class ObjectType>
bool SharedPtr<ObjectType>::operator<(
        const SharedPtr<ObjectType>& other) const {
    return (mObject < other.mObject);
}

template <class ObjectType>
ObjectType* SharedPtr<ObjectType>::operator->() const {
    DCHECK(mObject);
    return (ObjectType*)mObject;
}

template <class ObjectType>
ObjectType& SharedPtr<ObjectType>::operator*() const {
    DCHECK(mObject);
    return *(ObjectType*)mObject;
}

template <class ObjectType>
SharedPtr<ObjectType>::operator bool() const {
    return valid();
}

template <class ObjectType>
ObjectType* SharedPtr<ObjectType>::get() const {
    return (ObjectType*)mObject;
}

template <class ObjectType>
bool SharedPtr<ObjectType>::valid() const {
    if (mObject)
        return mObject->valid();
    return false;
}

}  // namespace utils

#endif  // SRC_COMPONENTS_INCLUDE_UTILS_SHARED_PTR_H_

// vim: set ts=2 sw=2 et:
