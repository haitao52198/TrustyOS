//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================
#ifndef __HASHTABLE_INC__
#define __HASHTABLE_INC__

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MUL_LOADFACTOR(n) (((n) * 3) >> 2) //n * 0.75

template <class T>
class Enumeration
{
public:
    virtual bool HasMoreElements() = 0;

    virtual T* NextElement() = 0;
};

template <class T, class TCHAR = char>
class __HashTableElementEnumeration;

template <class T, class TCHAR = char>
class __HashTableKeyEnumeration;

template <class T, class TCHAR = char>
class HashTable
{
    friend class __HashTableElementEnumeration<T, TCHAR>;
    friend class __HashTableKeyEnumeration<T, TCHAR>;

public:
    HashTable(int nInitialCapacity = 11, float fLoadFactor = 0.75f);

    ~HashTable();

    inline int Size();

    inline bool IsEmpty();

    T* Get(TCHAR *tszName);

    T* operator[](TCHAR *tszName);

    bool Put(TCHAR *tszName, T* pValue);

    bool Put(TCHAR *tszName, T& value);

    bool Remove(TCHAR *tszName);

    bool Contains(TCHAR *tszName);

    Enumeration<T> *Elements();

    Enumeration<TCHAR*> *Keys();

    void Clear();

private:
    bool Rehash();

    unsigned int Hash(TCHAR * str);

    int tstrcmp(TCHAR *str1, TCHAR *str2);

private:
    struct HashEntry {
        int nHash;
        T value;
        HashEntry *pNext;
        TCHAR tszName[1];
    };

    struct HashEntry **m_pTable;
    int m_nCapacity;
    int m_nCount;
    int m_nThreshold;
    float m_fLoadFactor;
    int m_nModCount;
};

template <class T, class TCHAR>
class __HashTableElementEnumeration : public Enumeration<T>
{
    friend class HashTable<T, TCHAR>;

protected:
    __HashTableElementEnumeration(HashTable<T, TCHAR> *pHashtable) :
            m_pHashtable(pHashtable), m_nItems(0),
            m_nIndex(-1), m_pEntry(NULL)
    {
    }

public:
    virtual bool HasMoreElements();

    virtual T* NextElement();

private:
    HashTable<T, TCHAR> *m_pHashtable;
    int m_nItems;
    int m_nIndex;
    struct HashTable<T, TCHAR>::HashEntry *m_pEntry;
};

template <class T, class TCHAR>
class __HashTableKeyEnumeration : public Enumeration<TCHAR*>
{
    friend class HashTable<T, TCHAR>;

protected:
    __HashTableKeyEnumeration(HashTable<T, TCHAR> *pHashtable) :
            m_pHashtable(pHashtable), m_nItems(0),
            m_nIndex(-1), m_pEntry(NULL), m_wszKey(NULL)
    {
    }

public:
    virtual bool HasMoreElements();

    virtual TCHAR * NextElement();

private:
    HashTable<T, TCHAR> *m_pHashtable;
    int m_nItems;
    int m_nIndex;
    struct HashTable<T, TCHAR>::HashEntry *m_pEntry;
    TCHAR *m_wszKey;
};

template <class T, class TCHAR>
HashTable<T, TCHAR>::HashTable(int nInitialCapacity, float fLoadFactor) :
        m_pTable(NULL), m_nCapacity(0), m_nCount(0), m_nThreshold(0),
       m_fLoadFactor(0), m_nModCount(0)
{
    if (nInitialCapacity <= 0)
        nInitialCapacity = 10;

#ifndef _arm
    if (fLoadFactor <= 0)
        fLoadFactor = 0.75f;
#endif

    m_pTable = (struct HashEntry **)malloc(sizeof(struct HashEntry **)
                * nInitialCapacity);

    assert(m_pTable && "Alloc memory for Hashtable failed!");

    if (!m_pTable)
        return;

    memset(m_pTable, 0, sizeof(struct HashEntry **) * nInitialCapacity);

    m_nCapacity = nInitialCapacity;
    m_fLoadFactor = fLoadFactor;
#ifndef _arm
    m_nThreshold = (int)(nInitialCapacity * fLoadFactor);
#else
    m_nThreshold = MUL_LOADFACTOR(nInitialCapacity);
#endif
}

template <class T, class TCHAR>
HashTable<T, TCHAR>::~HashTable()
{
    if (m_pTable) {
        Clear();
        free(m_pTable);
    }
}

template <class T, class TCHAR>
int HashTable<T, TCHAR>::tstrcmp(TCHAR *str1, TCHAR *str2)
{
    int ret = 0;
    do {
        ret = *str1 - *str2;
        str1++;
    } while (ret == 0 && *str2++ != 0);
    return ret;
}

template <class T, class TCHAR>
int HashTable<T, TCHAR>::Size()
{
    return m_nCount;
}

template <class T, class TCHAR>
bool HashTable<T, TCHAR>::IsEmpty()
{
    return m_nCount == 0;
}

template <class T, class TCHAR>
T* HashTable<T, TCHAR>::Get(TCHAR *tszName)
{
    assert(tszName && *tszName && "NULL or empty key name!");
    assert(m_pTable && "Uninitialized Hashtable!");

    if (!tszName || !*tszName || !m_pTable) {
        return NULL;
    }

    int nHash = Hash(tszName);
    int nIndex = (nHash & 0x7FFFFFFF) % m_nCapacity;
    for (struct HashEntry *e = m_pTable[nIndex]; e != NULL ; e = e->pNext) {
        if ((e->nHash == nHash) && tstrcmp(e->tszName, tszName) == 0)
            return &e->value;
    }

    return NULL;
}

template <class T, class TCHAR>
T* HashTable<T, TCHAR>::operator[](TCHAR *tszName)
{
    return Get(tszName);
}

template <class T, class TCHAR>
bool HashTable<T, TCHAR>::Rehash()
{
    int nOldCapacity = m_nCapacity;
    struct HashEntry **pOldTable = m_pTable;

    int nNewCapacity = nOldCapacity * 2 + 1;
    struct HashEntry **pNewTable = (struct HashEntry **)malloc(
            sizeof(struct HashEntry **) * nNewCapacity);
    if (!pNewTable)
        return false;

    memset(pNewTable, 0, sizeof(struct HashEntry **) * nNewCapacity);

    m_nModCount++;
#ifndef _arm
    m_nThreshold = (int)(nNewCapacity * m_fLoadFactor);
#else
    m_nThreshold = MUL_LOADFACTOR(nNewCapacity);
#endif
    m_nCapacity = nNewCapacity;
    m_pTable = pNewTable;

    for (int i = nOldCapacity ; i-- > 0 ;) {
        for (struct HashEntry *p = pOldTable[i]; p != NULL ; ) {
            struct HashEntry *e = p;
            p = p->pNext;

            int nIndex = (e->nHash & 0x7FFFFFFF) % nNewCapacity;
            e->pNext = pNewTable[nIndex];
            pNewTable[nIndex] = e;
        }
    }

    free(pOldTable);

    return true;
}

template <class T, class TCHAR>
bool HashTable<T, TCHAR>::Put(TCHAR *tszName, T* pValue)
{
    assert(tszName && *tszName && "NULL or empty key name!");
    assert(pValue && "Can not put NULL value to Hashtable!");
    assert(m_pTable && "Uninitialized Hashtable!");

    if (!tszName || *tszName == 0 || !pValue || !m_pTable)
        return false;

    // Makes sure the key is not already in the hashtable.
    int nHash = Hash(tszName);
    int nIndex = (nHash & 0x7FFFFFFF) % m_nCapacity;
    struct HashEntry *e;
    for (e = m_pTable[nIndex] ; e != NULL ; e = e->pNext) {
        if ((e->nHash == nHash) && tstrcmp(e->tszName, tszName) == 0) {
            memcpy(&(e->value), pValue, sizeof(T));
            return true;
        }
    }

    m_nModCount++;
    if (m_nCount >= m_nThreshold) {
        // Rehash the table if the threshold is exceeded
        if (!Rehash())
            return false;

        nIndex = (nHash & 0x7FFFFFFF) % m_nCapacity;
    }

    // Creates the new entry.
    int size = 0;
    while (tszName[size]) size++;

    e = (struct HashEntry *)malloc(
            sizeof(struct HashEntry) + sizeof(TCHAR) * size);
    if (!e)
        return false;

    e->nHash = nHash;
    e->pNext = m_pTable[nIndex];
    memcpy(&(e->value), pValue, sizeof(T));
//    wcscpy(e->tszName, tszName);
    TCHAR *p = tszName, *q =e->tszName;
    while ((*q++ = *p++) != 0)    // copy Source to Dest
        ;

    m_pTable[nIndex] = e;
    m_nCount++;

    return true;
}

template <class T, class TCHAR>
bool HashTable<T, TCHAR>::Put(TCHAR *tszName, T& value)
{
    return Put(tszName, &value);
}

template <class T, class TCHAR>
bool HashTable<T, TCHAR>::Remove(TCHAR *tszName)
{
    assert(tszName && *tszName && "NULL or empty key name!");
    assert(m_pTable && "Uninitialized Hashtable!");

    if (!tszName || *tszName == 0 || !m_pTable)
        return false;

    int nHash = Hash(tszName);
    int nIndex = (nHash & 0x7FFFFFFF) % m_nCapacity;
    for (struct HashEntry *e = m_pTable[nIndex], *prev = NULL ; e != NULL ;
            prev = e, e = e->pNext) {
        if ((e->nHash == nHash) && tstrcmp(e->tszName, tszName) == 0) {
            m_nModCount++;
            if (prev != NULL) {
                prev->pNext = e->pNext;
            } else {
                m_pTable[nIndex] = e->pNext;
            }

            m_nCount--;

            free(e);
            return true;
        }
    }

    return false;
}

template <class T, class TCHAR>
bool HashTable<T, TCHAR>::Contains(TCHAR *tszName)
{
    assert(tszName && *tszName && "NULL or empty key name!");
    assert(m_pTable && "Uninitialized Hashtable!");

    if (!tszName || *tszName == 0 || !m_pTable)
        return false;

    int nHash = Hash(tszName);
    int nIndex = (nHash & 0x7FFFFFFF) % m_nCapacity;
    for (struct HashEntry *e = m_pTable[nIndex] ; e != NULL ; e = e->pNext) {
        if ((e->nHash == nHash) && tstrcmp(e->tszName, tszName) == 0)
            return true;
    }

    return false;
}

template <class T, class TCHAR>
void HashTable<T, TCHAR>::Clear()
{
    if (!m_pTable)
        return;

    m_nModCount++;

    for (int nIndex = m_nCapacity; --nIndex >= 0; ) {
        for (struct HashEntry *e = m_pTable[nIndex]; e != NULL; ) {
            struct HashEntry *p = e;
            e = e->pNext;
            free(p);
        }

        m_pTable[nIndex] = NULL;
    }

    m_nCount = 0;
}

template <class T, class TCHAR>
Enumeration<TCHAR*> *HashTable<T, TCHAR>::Keys()
{
    return new __HashTableKeyEnumeration<T, TCHAR>(this);
}

template <class T, class TCHAR>
Enumeration<T> *HashTable<T, TCHAR>::Elements()
{
    return new __HashTableElementEnumeration<T, TCHAR>(this);
}

template <class T, class TCHAR>
unsigned int HashTable<T, TCHAR>::Hash(TCHAR * str)
{
    unsigned int value = 0L;
    TCHAR ch;

    if (str != NULL) {
        value += 30 * (*str);
        while ((ch = *str++) != 0) {
            value = value ^ ((value << 5) + (value >> 3) + (unsigned long)ch);
        }
    }

    return value;
}


template <class T, class TCHAR>
bool __HashTableElementEnumeration<T, TCHAR>::HasMoreElements()
{
    return m_nItems < m_pHashtable->m_nCount;
}

template <class T, class TCHAR>
T* __HashTableElementEnumeration<T, TCHAR>::NextElement()
{
    if (m_pEntry)
        m_pEntry = m_pEntry->pNext;

    if (!m_pEntry) {
        for (++m_nIndex; m_nIndex < m_pHashtable->m_nCapacity &&
                (m_pEntry = m_pHashtable->m_pTable[m_nIndex]) == NULL;
                m_nIndex++);

        if (!m_pEntry) {
            assert(0 && "End of element enumeration!");
            return NULL;
        }

    }

    m_nItems++;
    return &m_pEntry->value;
}

template <class T, class TCHAR>
bool __HashTableKeyEnumeration<T, TCHAR>::HasMoreElements()
{
    return m_nItems < m_pHashtable->m_nCount;
}

template <class T, class TCHAR>
TCHAR * __HashTableKeyEnumeration<T, TCHAR>::NextElement()
{
    if (m_pEntry)
        m_pEntry = m_pEntry->pNext;

    if (!m_pEntry) {
       for (++m_nIndex; m_nIndex < m_pHashtable->m_nCapacity &&
                (m_pEntry = m_pHashtable->m_pTable[m_nIndex]) == NULL;
                m_nIndex++);

        if (!m_pEntry) {
            assert(0 && "End of key enumeration!");
            m_wszKey = NULL;
            return m_wszKey;
        }
    }

    m_nItems++;
    m_wszKey = m_pEntry->tszName;
    return m_wszKey;
}

#endif // __HASHTABLE_INC__
