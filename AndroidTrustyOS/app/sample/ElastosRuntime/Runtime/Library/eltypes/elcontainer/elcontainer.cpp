
#include <elcontainer.h>

_ELASTOS_NAMESPACE_BEGIN

//-----------------------------------------------------
// DoubleLinkNode
//-----------------------------------------------------
DoubleLinkNode* DoubleLinkNode::InsertPrev(
    /* [in] */ DoubleLinkNode* prevNode)
{
    assert(prevNode);

    prevNode->mPrev = mPrev;
    prevNode->mNext = this;
    mPrev->mNext = prevNode;
    mPrev = prevNode;

    return prevNode;
}

DoubleLinkNode* DoubleLinkNode::InsertNext(
    /* [in] */ DoubleLinkNode* nextNode)
{
    assert(nextNode);

    nextNode->mPrev = this;
    nextNode->mNext = mNext;
    mNext->mPrev = nextNode;
    mNext = nextNode;

    return nextNode;
}

DoubleLinkNode* DoubleLinkNode::Detach()
{
    mPrev->mNext = mNext;
    mNext->mPrev = mPrev;
    mPrev = mNext = NULL;

    return this;
}

//-----------------------------------------------------
// SingleLinkNode
//-----------------------------------------------------
SingleLinkNode* SingleLinkNode::InsertNext(
    /* [in] */ SingleLinkNode* nextNode)
{
    assert(nextNode);

    nextNode->mNext = mNext;
    mNext = nextNode;

    return nextNode;
}


SingleLinkNode* SingleLinkNode::Detach(
    /* [in] */ SingleLinkNode* prevNode)
{
    prevNode->mNext = mNext;
    mNext = NULL;

    return this;
}

//-----------------------------------------------------
// SimpleContainer
//-----------------------------------------------------
SimpleContainer::SimpleContainer()
{
    mHead.Initialize();
    mCurrent = &mHead;
}

ECode SimpleContainer::Current(
    /* [out] */ DLinkNode** node)
{
    assert(NULL != node);

    if (NULL == mCurrent || &mHead == mCurrent) {
        return E_INVALID_OPERATION;
    }
    *node = mCurrent;
    return NOERROR;
}

ECode SimpleContainer::MoveNext()
{
    if (NULL == mCurrent) {
        return E_INVALID_OPERATION;
    }
    if (mCurrent == mHead.Prev()) {
        mCurrent = NULL;
        return S_FALSE;
    }
    mCurrent = mCurrent->Next();
    return NOERROR;
}

ECode SimpleContainer::Reset()
{
    mCurrent = &mHead;
    return NOERROR;
}

ECode SimpleContainer::Add(
    /* [in] */ DLinkNode* node)
{
    mHead.InsertPrev(node);
    return NOERROR;
}

ECode SimpleContainer::Remove(
    /* [in] */ DLinkNode* node)
{
    if (mCurrent == node) {
        mCurrent = mCurrent->Prev();
    }
    node->Detach();
    return NOERROR;
}

//-----------------------------------------------------
// ObjectContainer
//-----------------------------------------------------
ECode ObjectContainer::Dispose()
{
    ObjectNode* node = (ObjectNode *)mHead.Next();
    while (&mHead != (DLinkNode *)node) {
        node->Detach();
        delete node;
        node = (ObjectNode *)mHead.Next();
    }
    mCurrent = NULL;
    return NOERROR;
}

ECode ObjectContainer::Current(
    /* [out] */ PInterface* obj)
{
    *obj = NULL;
    ObjectNode* node;
    ECode ec = SimpleContainer::Current((DLinkNode**)&node);
    if (SUCCEEDED(ec)) *obj = node->mObject;
    return ec;
}

ECode ObjectContainer::Add(
    /* [in] */ PInterface pObj)
{
    ObjectNode* node = new ObjectNode(pObj);
    if (NULL == node) return E_OUT_OF_MEMORY;

    return SimpleContainer::Add(node);
}

ECode ObjectContainer::Remove(PInterface pObj)
{
    ObjectNode* node;
    ForEachDLinkNode(ObjectNode *, node, &mHead) {
        if (node->mObject == pObj) {
            SimpleContainer::Remove(node);
            delete node;
            return NOERROR;
        }
    }
    return S_FALSE;
}

_ELASTOS_NAMESPACE_END
