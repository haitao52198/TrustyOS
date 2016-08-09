//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __ELQUINTETTYPE_H__
#define __ELQUINTETTYPE_H__

#include <elrefbase.h>

_ELASTOS_NAMESPACE_BEGIN

/** @addtogroup CARTypesRef
  *   @{
  */
typedef enum _CarQuintetFlag
{
    CarQuintetFlag_HeapAlloced      = 0x00010000,
    CarQuintetFlag_AutoRefCounted   = 0x00100000,

    CarQuintetFlag_Type_Unknown     = 0,

    CarQuintetFlag_Type_Int8        = 1,
    CarQuintetFlag_Type_Int16       = 2,
    CarQuintetFlag_Type_Int32       = 3,
    CarQuintetFlag_Type_Int64       = 4,

    CarQuintetFlag_Type_Byte        = 5,
    CarQuintetFlag_Type_UInt8       = CarQuintetFlag_Type_Byte,
    CarQuintetFlag_Type_UInt16      = 6,
    CarQuintetFlag_Type_UInt32      = 7,
    CarQuintetFlag_Type_UInt64      = 8,

    CarQuintetFlag_Type_Boolean     = CarQuintetFlag_Type_Byte,
    CarQuintetFlag_Type_Float       = 9,
    CarQuintetFlag_Type_Double      = 10,

    CarQuintetFlag_Type_Char16      = CarQuintetFlag_Type_UInt16,
    CarQuintetFlag_Type_Char32      = CarQuintetFlag_Type_UInt32,
    CarQuintetFlag_Type_String      = 11,

    CarQuintetFlag_Type_EMuid       = 12,
    CarQuintetFlag_Type_EGuid       = 13,
    CarQuintetFlag_Type_ECode       = CarQuintetFlag_Type_Int32,
    CarQuintetFlag_Type_Enum        = CarQuintetFlag_Type_Int32,
    CarQuintetFlag_Type_Struct      = 14,
    CarQuintetFlag_Type_IObject     = 15,
    CarQuintetFlag_Type_RefObject   = 16,
    CarQuintetFlag_Type_LightRefObject   = 17,

    CarQuintetFlag_TypeMask         = 0x0000ffff
} CarQuintetFlag;

typedef Int32 CarQuintetFlags;
typedef Int32 CarQuintetLocks;

typedef struct CarQuintet
{
    CarQuintetFlags mFlags;
    CarQuintetLocks mReserve;
    MemorySize      mUsed;
    MemorySize      mSize;
    PVoid           mBuf;
} CarQuintet, *PCarQuintet, *PCARQUINTET;

// Helper traits get bared type
//
template <typename T>
struct TypeTraitsItem
{
    typedef T                       BaredType;
    enum { isPointer = FALSE };
};

template <typename T>
struct TypeTraitsItem<T*>
{
    typedef T                       BaredType;
    enum { isPointer = TRUE };
};

template <typename T>
struct TypeTraitsItem<const T*>
{
    typedef T                       BaredType;
    enum { isPointer = TRUE };
};

template <typename T>
struct TypeTraits
{
    typedef typename TypeTraitsItem<T>::BaredType        BaredType;
    enum { isPointer = TypeTraitsItem<T>::isPointer };
};

#ifndef CREATE_MEMBER_DETECTOR
#define CREATE_MEMBER_DETECTOR(X)                                                   \
template<typename T> class Detect_##X {                                             \
    struct Fallback { int X; };                                                     \
    struct Derived : T, Fallback { };                                               \
                                                                                    \
    template<typename U, U> struct Check;                                           \
    template<typename U> static char func(Check<int Fallback::*, &U::X> *);         \
    template<typename U> static int func(...);                                      \
  public:                                                                           \
    typedef Detect_##X type;                                                        \
    enum { exists = sizeof(func<Derived>(0)) == sizeof(int) };                      \
};
#endif

CREATE_MEMBER_DETECTOR(AddRef)
CREATE_MEMBER_DETECTOR(Release)

#ifndef HAS_MEMBER_ADDREF_AND_RELEASE
#define HAS_MEMBER_ADDREF_AND_RELEASE(type) \
    (Detect_AddRef<type>::exists && Detect_Release<type>::exists)
#endif

template<typename T>
struct CheckClassType
{
    enum { isClassType = TRUE };
};

#ifndef DECL_CHECK_CLASS_TYPE_IMPL
#define DECL_CHECK_CLASS_TYPE_IMPL(type, value)     \
template<>                                          \
struct CheckClassType<type>                         \
{                                                   \
    enum { isClassType = value };                   \
};
#endif

DECL_CHECK_CLASS_TYPE_IMPL(Int8,        FALSE)
DECL_CHECK_CLASS_TYPE_IMPL(Int16,       FALSE)
DECL_CHECK_CLASS_TYPE_IMPL(Int32,       FALSE)
DECL_CHECK_CLASS_TYPE_IMPL(Int64,       FALSE)
DECL_CHECK_CLASS_TYPE_IMPL(Byte,        FALSE)
DECL_CHECK_CLASS_TYPE_IMPL(UInt16,      FALSE)
DECL_CHECK_CLASS_TYPE_IMPL(UInt32,      FALSE)
DECL_CHECK_CLASS_TYPE_IMPL(UInt64,      FALSE)
DECL_CHECK_CLASS_TYPE_IMPL(Float,       FALSE)
DECL_CHECK_CLASS_TYPE_IMPL(Double,      FALSE)


// NOTE1: MS CL compiler can't support function's template specialization well,
//   so only template class works.
// NOTE2: We shall emit a COMPILE-TIME error if user let ArrayOf
//   to contain a
//   non-automation type. There's an undefined variable in Type2Flag::Flag()'s
//   default implementation, which will emit the error. Also, we must deal
//   with illegal types at runtime.
//
template <class T> struct Type2Flag
{
    static Int32 Flag() {
        if (SUPERSUBCLASS_EX(IInterface*, T)) {
            return CarQuintetFlag_Type_IObject;
        }
        else if (SUPERSUBCLASS_EX(ElRefBase*, T)) {
            return CarQuintetFlag_Type_RefObject;
        }
        else if (SUPERSUBCLASS_EX(ElLightRefBase*, T)) {
            return CarQuintetFlag_Type_LightRefObject;
        }
        else {
            return CarQuintetFlag_Type_Struct;
        }
    }
};

#define DECL_TYPE2FLAG_TMPL(type, flag)                         \
    template <> struct Type2Flag<type> { static Int32 Flag()    \
    { return (flag); } };

DECL_TYPE2FLAG_TMPL(Int8,               CarQuintetFlag_Type_Int8);
DECL_TYPE2FLAG_TMPL(Int16,              CarQuintetFlag_Type_Int16);
DECL_TYPE2FLAG_TMPL(Int32,              CarQuintetFlag_Type_Int32);
DECL_TYPE2FLAG_TMPL(Int64,              CarQuintetFlag_Type_Int64);

DECL_TYPE2FLAG_TMPL(Byte,               CarQuintetFlag_Type_Byte);
DECL_TYPE2FLAG_TMPL(UInt16,             CarQuintetFlag_Type_UInt16);
DECL_TYPE2FLAG_TMPL(UInt32,             CarQuintetFlag_Type_UInt32);
DECL_TYPE2FLAG_TMPL(UInt64,             CarQuintetFlag_Type_UInt64);

DECL_TYPE2FLAG_TMPL(Float,              CarQuintetFlag_Type_Float);
DECL_TYPE2FLAG_TMPL(Double,             CarQuintetFlag_Type_Double);

DECL_TYPE2FLAG_TMPL(String,             CarQuintetFlag_Type_String);

DECL_TYPE2FLAG_TMPL(EMuid,              CarQuintetFlag_Type_EMuid);
DECL_TYPE2FLAG_TMPL(EGuid,              CarQuintetFlag_Type_EGuid);
DECL_TYPE2FLAG_TMPL(IInterface *,       CarQuintetFlag_Type_IObject);
DECL_TYPE2FLAG_TMPL(ElRefBase *,        CarQuintetFlag_Type_RefObject);
DECL_TYPE2FLAG_TMPL(ElLightRefBase *,   CarQuintetFlag_Type_LightRefObject);

/** @} */

_ELASTOS_NAMESPACE_END

#endif // __ELQUINTETTYPE_H__

