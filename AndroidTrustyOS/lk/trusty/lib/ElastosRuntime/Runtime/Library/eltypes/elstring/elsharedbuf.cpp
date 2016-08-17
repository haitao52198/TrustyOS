#include <string.h>
#include <elsharedbuf.h>

_ELASTOS_NAMESPACE_BEGIN

SharedBuffer* SharedBuffer::Alloc(UInt32 size, Boolean acquire)
{
    SharedBuffer* sb = static_cast<SharedBuffer *>(malloc(sizeof(SharedBuffer) + size));
    if (sb) {
        if (acquire) {
            sb->mRefs = 1;
        }
        else {
            sb->mRefs = 0;
        }
        sb->mSize = size;

        ELA_DBGOUT(ELADBG_NORMAL,
            printf(" > ShareBuffer Alloc %p - %p, size: %d\n", sb, sb->GetData(), size));
    }
    return sb;
}

Int32 SharedBuffer::Dealloc(const SharedBuffer* released)
{
    if (released->mRefs != 0) {
        ELA_DBGOUT(ELADBG_WARNING,
            printf("Warning: RefCount of SharedBuffer %p is not zero,"
                    " you should use Release instead of Dealloc.", released));
        return -1; // XXX: invalid operation
    }

    ELA_DBGOUT(ELADBG_NORMAL,
        printf(" > ShareBuffer default Dealloc free %p - %p, size: %d\n",
                released, released->GetData(), released->mSize));

    free(const_cast<SharedBuffer*>(released));
    return 0;
}

SharedBuffer* SharedBuffer::Edit() const
{
    if (IsOnlyOwner()) {
        return const_cast<SharedBuffer*>(this);
    }
    SharedBuffer* sb = Alloc(mSize);
    if (sb) {
        memcpy(sb->GetData(), GetData(), GetSize());
        Release();
    }
    return sb;
}

SharedBuffer* SharedBuffer::EditResize(UInt32 newSize) const
{
    if (IsOnlyOwner()) {
        SharedBuffer* buf = const_cast<SharedBuffer*>(this);
        if (buf->mSize == newSize) return buf;
        buf = (SharedBuffer*)realloc(buf, sizeof(SharedBuffer) + newSize);
        if (buf != NULL) {
            buf->mSize = newSize;
            return buf;
        }
    }
    SharedBuffer* sb = Alloc(newSize);
    if (sb) {
        const UInt32 mySize = mSize;
        memcpy(sb->GetData(), GetData(), newSize < mySize ? newSize : mySize);
        Release();
    }
    return sb;
}

SharedBuffer* SharedBuffer::AttemptEdit() const
{
    if (IsOnlyOwner()) {
        return const_cast<SharedBuffer*>(this);
    }
    return 0;
}

SharedBuffer* SharedBuffer::Reset(UInt32 new_size) const
{
    // cheap-o-reset.
    SharedBuffer* sb = Alloc(new_size);
    if (sb) {
        Release();
    }
    return sb;
}

void SharedBuffer::Acquire() const
{
    atomic_inc(&mRefs);
}

Int32 SharedBuffer::Release(UInt32 flags) const
{
    Int32 curr = 0;
    if (IsOnlyOwner() || ((curr = atomic_dec(&mRefs)) == 0)) {
        mRefs = 0;
        if ((flags & eKeepStorage) == 0) {
            ELA_DBGOUT(ELADBG_NORMAL,
                printf(" > ShareBuffer default Release free %p - %p, size: %d\n",
                        this, GetData(), mSize));

            free(const_cast<SharedBuffer*>(this));
        }
    }
    return curr;
}

_ELASTOS_NAMESPACE_END
