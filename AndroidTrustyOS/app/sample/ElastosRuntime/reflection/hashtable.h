//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================
#ifndef __HASHTTABLE_H__
#define __HASHTTABLE_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <clstype.h>

_ELASTOS_NAMESPACE_USING

#define MUL_LOADFACTOR(n) (((n) * 3) >> 2) //n * 0.75

template <class T, CARDataType type = Type_UInt32>
class HashTable
{
public:
    HashTable(
        /* [in] */ Int32 initialCapacity = 11,
        /* [in] */ Float loadFactor = 0.75f);

    ~HashTable();

    inline Int32 Size();

    inline Boolean IsEmpty();

    T* Get(PVoid key);

    T* operator[](PVoid key);

    Boolean Put(PVoid key, T* value);

    Boolean Put(PVoid key, T& value);

    Boolean Remove(PVoid key);

    Boolean Contains(PVoid key);

    void Clear();

private:
    struct HashEntry
    {
        Int32 mHash;
        T mValue;
        HashEntry* mNext;
        Byte mKey[1];
    };

    Boolean Rehash();

    UInt32 Hash(PVoid key);

    Boolean keycmp(HashEntry* e, PVoid key);

    void keycpy(HashEntry* e, PVoid key);

    Int32 keylen(PVoid key);

private:
    struct HashEntry** mTable;
    Int32 mCapacity;
    Int32 mCount;
    Int32 mThreshold;
    Float mLoadFactor;
    Int32 mModCount;
};

template <class T, CARDataType type>
HashTable<T, type>::HashTable(
    /* [in] */ Int32 initialCapacity,
    /* [in] */ Float loadFactor)
    : mTable(NULL)
    , mCapacity(0)
    , mCount(0)
    , mThreshold(0)
    ,mLoadFactor(0)
    , mModCount(0)
{
    if (initialCapacity <= 0) {
        initialCapacity = 10;
    }

#ifndef _arm
    if (loadFactor <= 0) {
        loadFactor = 0.75f;
    }
#endif

    mCapacity = initialCapacity;
    mLoadFactor = loadFactor;
#ifndef _arm
    mThreshold = (Int32)(initialCapacity * loadFactor);
#else
    mThreshold = MUL_LOADFACTOR(initialCapacity);
#endif
}

template <class T, CARDataType type>
HashTable<T, type>::~HashTable()
{
    if (mTable) {
        Clear();
        free(mTable);
    }
}

template <class T, CARDataType type>
Boolean HashTable<T, type>::keycmp(
    /* [in] */ HashEntry* e,
    /* [in] */ PVoid key)
{
    Int32 ret = 0;
    switch (type) {
        case Type_UInt32:
            ret = memcmp(e->mKey, key, sizeof(Int32));
            break;
        case Type_UInt64:
            ret = memcmp(e->mKey, key, sizeof(UInt64));
            break;
        case Type_String:
            ret = strcmp((char *)e->mKey, (char*)key);
            break;
        case Type_EMuid:
            ret = memcmp(e->mKey, key, sizeof(EMuid));
            break;
        default:
            ret = 1;
            break;
    }

    if (!ret) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

template <class T, CARDataType type>
void HashTable<T, type>::keycpy(
    /* [in] */ HashEntry* e,
    /* [in] */ PVoid key)
{
    switch (type) {
        case Type_UInt32:
            *(UInt32 *)e->mKey = *(UInt32 *)key;
            break;
        case Type_UInt64:
            *(UInt64 *)e->mKey = *(UInt64 *)key;
            break;
        case Type_String:
            strcpy((char *)e->mKey, (char*)key);
            break;
        case Type_EMuid:
            memcpy(e->mKey, key, sizeof(EMuid));
            break;
        default:
            break;
    }
    return;
}

template <class T, CARDataType type>
Int32 HashTable<T, type>::keylen(
    /* [in] */ PVoid key)
{
    Int32 len = 0;
    switch (type) {
        case Type_UInt32:
            len = sizeof(Int32);
            break;
        case Type_UInt64:
            len = sizeof(UInt64);
            break;
        case Type_String:
            len  = strlen((char*)key) + 1;
            break;
        case Type_EMuid:
            len = sizeof(EMuid);
            break;
        default:
            break;
    }
    return len;
}

template <class T, CARDataType type>
Int32 HashTable<T, type>::Size()
{
    return mCount;
}

template <class T, CARDataType type>
Boolean HashTable<T, type>::IsEmpty()
{
    return mCount == 0;
}

template <class T, CARDataType type>
T* HashTable<T, type>::Get(
    /* [in] */ PVoid key)
{
    assert(key  && "NULL or empty key name!");

    if (!key || !mTable) {
        return NULL;
    }

    Int32 hash = Hash(key);
    Int32 index = (hash & 0x7FFFFFFF) % mCapacity;
    for (struct HashEntry *e = mTable[index]; e != NULL ; e = e->mNext) {
        if ((e->mHash == hash) && keycmp(e, key)) {
            return &e->mValue;
        }
    }

    return NULL;
}

template <class T, CARDataType type>
T* HashTable<T, type>::operator[](
    /* [in] */ PVoid key)
{
    return Get(key);
}

template <class T, CARDataType type>
Boolean HashTable<T, type>::Rehash()
{
    Int32 oldCapacity = mCapacity;
    struct HashEntry** oldTable = mTable;

    Int32 newCapacity = oldCapacity * 2 + 1;
    struct HashEntry** newTable = (struct HashEntry **)malloc(
            sizeof(struct HashEntry **) * newCapacity);
    if (!newTable) {
        return FALSE;
    }

    memset(newTable, 0, sizeof(struct HashEntry **) * newCapacity);

    mModCount++;
#ifndef _arm
    mThreshold = (Int32)(newCapacity * mLoadFactor);
#else
    mThreshold = MUL_LOADFACTOR(newCapacity);
#endif
    mCapacity = newCapacity;
    mTable = newTable;

    for (Int32 i = oldCapacity ; i--> 0 ;) {
        for (struct HashEntry* p = oldTable[i]; p != NULL ;) {
            struct HashEntry* e = p;
            p = p->mNext;

            Int32 index = (e->mHash & 0x7FFFFFFF) % newCapacity;
            e->mNext = newTable[index];
            newTable[index] = e;
        }
    }

    free(oldTable);

    return TRUE;
}

template <class T, CARDataType type>
Boolean HashTable<T, type>::Put(
    /* [in] */ PVoid key,
    /* [in] */ T* value)
{
    assert(key && "NULL or empty key name!");
    assert(value && "Can not put NULL value to Hashtable!");

    if (!key || !value) {
        return FALSE;
    }

    if (!mTable) {
        mTable = (struct HashEntry **)malloc(sizeof(struct HashEntry **) * mCapacity);

        if (!mTable) {
            return FALSE;
        }

        memset(mTable, 0, sizeof(struct HashEntry **) * mCapacity);
    }

    // Makes sure the key is not already in the hashtable.
    Int32 hash = Hash(key);
    Int32 index = (hash & 0x7FFFFFFF) % mCapacity;
    struct HashEntry *e;
    for (e = mTable[index] ; e != NULL ; e = e->mNext) {
        if ((e->mHash == hash) && keycmp(e, key)) {
            memcpy(&(e->mValue), value, sizeof(T));
            return TRUE;
        }
    }

    mModCount++;
    if (mCount >= mThreshold) {
        // Rehash the table if the threshold is exceeded
        if (!Rehash()) {
            return FALSE;
        }

        index = (hash & 0x7FFFFFFF) % mCapacity;
    }

    // Creates the new entry.
    Int32 size = keylen(key);

    e = (struct HashEntry *)malloc(sizeof(struct HashEntry) + size);
    if (!e) {
        return FALSE;
    }

    e->mHash = hash;
    e->mNext = mTable[index];
    memcpy(&(e->mValue), value, sizeof(T));
    keycpy(e, key);

    mTable[index] = e;
    mCount++;

    return TRUE;
}

template <class T, CARDataType type>
Boolean HashTable<T, type>::Put(
    /* [in] */ PVoid key,
    /* [in] */ T& value)
{
    return Put(key, &value);
}

template <class T, CARDataType type>
Boolean HashTable<T, type>::Remove(
    /* [in] */ PVoid key)
{
    assert(key && "NULL or empty key name!");

    if (!key || !mTable) {
        return FALSE;
    }

    Int32 hash = Hash(key);
    Int32 index = (hash & 0x7FFFFFFF) % mCapacity;
    for (struct HashEntry* e = mTable[index], *prev = NULL ; e != NULL ;
        prev = e, e = e->mNext) {
        if ((e->mHash == hash) && keycmp(e, key)) {
            mModCount++;
            if (prev != NULL) {
                prev->mNext = e->mNext;
            }
            else {
                mTable[index] = e->mNext;
            }

            mCount--;

            free(e);
            return TRUE;
        }
    }

    return FALSE;
}

template <class T, CARDataType type>
Boolean HashTable<T, type>::Contains(
    /* [in] */ PVoid key)
{
    assert(key  && "NULL or empty key name!");

    if (!key || !mTable) {
        return FALSE;
    }

    Int32 hash = Hash(key);
    Int32 index = (hash & 0x7FFFFFFF) % mCapacity;
    for (struct HashEntry* e = mTable[index] ; e != NULL ; e = e->mNext) {
        if ((e->mHash == hash) && keycmp(e, key)) {
            return TRUE;
        }
    }

    return FALSE;
}

template <class T, CARDataType type>
void HashTable<T, type>::Clear()
{
    if (!mTable) {
        return;
    }

    mModCount++;

    for (Int32 index = mCapacity;--index >= 0;) {
        for (struct HashEntry* e = mTable[index]; e != NULL;) {
            struct HashEntry* p = e;
            e = e->mNext;
            free(p);
        }

        mTable[index] = NULL;
    }

    mCount = 0;
}

template <class T, CARDataType type>
UInt32 HashTable<T, type>::Hash(
    /* [in] */ PVoid key)
{
    UInt32 value = 0;
    char ch = '\0', *str = (char *)key;
    unsigned long* lvalue = (unsigned long *)key;
    Int32 len = sizeof(EMuid) / sizeof(unsigned long);
    Int32 i = 0;
    switch (type) {
        case Type_UInt32:
            value = value ^ ((value << 5) + (value >> 3) + *lvalue);
            break;
        case Type_UInt64:
            value = value ^ ((value << 5) + (value >> 3) + lvalue[0]);
            value = value ^ ((value << 5) + (value >> 3) + lvalue[1]);
            break;
        case Type_String:
            if (str != NULL) {
                value += 30 * (*str);
                while ((ch = *str++) != 0) {
                    value = value ^ ((value << 5) + (value >> 3)
                            + (unsigned long)ch);
                }
            }
            break;
        case Type_EMuid:
            for (i = 0; i < len; i++) {
                value = value ^ ((value << 5) + (value >> 3) + lvalue[i]);
            }
            break;
        default:
            break;
    }

    return value;
}

#endif // __HASHTTABLE_H__
