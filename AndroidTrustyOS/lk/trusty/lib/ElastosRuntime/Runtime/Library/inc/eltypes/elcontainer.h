//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __ELASTOS_CONTAINER_H__
#define __ELASTOS_CONTAINER_H__

//
// In this file, there are two simple container implementations, they are
// ObjectContainer and CallbackContextContainer, both of them inherited from
// base class SimpleContainer. They provide six methods: Current,
// MoveNext, Reset, Add, Remove and Dispose.
// In fact, Add, Remove and Dispose are right methods of container, but
// the Current, MoveNext and Reset are more like methods of enumerator,
// we take them together for terse reason.
//
// Using them in multi-threading program is not safe, the synchronizing
// mechanism should be decided by outer codes.
// We don't record reference count of interface in ObjectContainer, the
// counting rules should be decided by outer codes too. A more precise
// definition and implementation of ObjectContainer is suplied with CAR
// aspect class AObjectContainer in elastos.dll.
//
#include <elastos.h>

#ifndef ForEachDLinkNode

#define ForEachDLinkNode(t, p, h) \
        for (p = (t)((h)->mNext); p != (t)h; p = (t)(p->mNext))

#define FOR_EACH_DLINKNODE(t, p, h) \
        ForEachDLinkNode(t, p, h)

#define ForEachDLinkNodeReversely(t, p, h) \
        for (p = (t)((h)->mPrev); p != (t)h; p = (t)(p->mPrev))

#define FOR_EACH_DLINKNODE_REVERSELY(t, p, h) \
        ForEachDLinkNodeReversely(t, p, h)

#endif // ForEachDLinkNode

#ifndef ForEachSLinkNode

#define ForEachSLinkNode(t, p, h) \
        for (p = (t)((h)->mNext); p != (t)h; p = (t)(p->mNext))

#define FOR_EACH_SLINKNODE(t, p, h) \
        ForEachSLinkNode(t, p, h)

#endif // ForEachSLinkNode

_ELASTOS_NAMESPACE_BEGIN

//
//  Double Link Node -- For double link circular list with head-node.
//
class DoubleLinkNode
{
public:
    DoubleLinkNode()
    {
        mPrev = mNext = NULL;
    }

    DoubleLinkNode(DoubleLinkNode *prevNode, DoubleLinkNode *nextNode)
    {
        mPrev = prevNode;
        mNext = nextNode;
    }

    //
    // Non-head-node operations
    //
    // Return the previous node.
    DoubleLinkNode *Prev() const    { return mPrev; }

    // Return the next node.
    DoubleLinkNode *Next() const    { return mNext; }

    // Insert a new node before me. Return the new previous node.
    DoubleLinkNode *InsertPrev(DoubleLinkNode *prevNode);

    // Insert a new node after me.  Return the new next node.
    DoubleLinkNode *InsertNext(DoubleLinkNode *nextNode);

    // Remove myself from list. Return myself.
    DoubleLinkNode *Detach();

    //
    // Head-node operations
    //
    void Initialize()               { mPrev = mNext = this; }

    Boolean IsEmpty() const          { return this == mNext; }

    // Return the first node of list.
    DoubleLinkNode *First() const   { return Next(); }

    // Return the last node of list.
    DoubleLinkNode *Last()  const   { return Prev(); }

    // Insert a new node as the first of list. Return the new first node.
    DoubleLinkNode *InsertFirst(DoubleLinkNode *firstNode)
    {
        return InsertNext(firstNode);
    }

    // Insert a new node as the last of list.  Return the new last node.
    DoubleLinkNode *InsertLast(DoubleLinkNode *lastNode)
    {
        return InsertPrev(lastNode);
    }

public:
    DoubleLinkNode      *mPrev;
    DoubleLinkNode      *mNext;
};

typedef DoubleLinkNode DLinkNode;

//
//  Single Link Node -- For single link circular list with head-node.
//
class SingleLinkNode
{
public:
    SingleLinkNode()
    {
        mNext = NULL;
    }

    //
    // Non-head-node operations
    //
    // Return the next node.
    SingleLinkNode *Next() const    { return mNext; }

    // Insert a new node after me.  Return the new next node.
    SingleLinkNode *InsertNext(SingleLinkNode *nextNode);

    // Remove myself from list. Return myself.
    SingleLinkNode *Detach(SingleLinkNode *prevNode);

    //
    // Head-node operations
    //
    void Initialize()               { mNext = this; }

    Boolean IsEmpty() const          { return this == mNext; }

    // Return the first node of list.
    SingleLinkNode *First() const   { return Next(); }

    // Insert a new node as the first of list. Return the new first node.
    SingleLinkNode *InsertFirst(SingleLinkNode *firstNode)
    {
        return InsertNext(firstNode);
    }

public:
    SingleLinkNode      *mNext;
};

typedef SingleLinkNode SLinkNode;

/** @addtogroup BaseTypesRef
  *   @{
  */
class SimpleContainer
{
public:
    SimpleContainer();

    _ELASTOS ECode Current(DLinkNode **);
    _ELASTOS ECode MoveNext();
    _ELASTOS ECode Reset();
    _ELASTOS ECode Add(DLinkNode *);
    _ELASTOS ECode Remove(DLinkNode *);

public:
    DLinkNode  mHead;
    DLinkNode  *mCurrent;
};
/** @} */

/** @addtogroup BaseTypesRef
  *   @{
  */
struct ObjectNode : public DLinkNode
{
    ObjectNode(PInterface obj) { mObject = obj; }

    PInterface mObject;
};

class ObjectContainer : public SimpleContainer
{
public:
    _ELASTOS ECode Current(PInterface *obj);
    _ELASTOS ECode Add(PInterface obj);
    _ELASTOS ECode Remove(PInterface obj);
    _ELASTOS ECode Dispose();

    virtual ~ObjectContainer() { Dispose(); }
};
/** @} */

_ELASTOS_NAMESPACE_END

#endif // __ELASTOS_CONTAINER_H__
