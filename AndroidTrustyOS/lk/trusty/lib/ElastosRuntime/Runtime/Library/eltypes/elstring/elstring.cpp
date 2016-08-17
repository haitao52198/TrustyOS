#include <car.h>
#include <elquintet.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

_ELASTOS_NAMESPACE_BEGIN

//=======================================================================================
//              assistance
//=======================================================================================
EXTERN_C SharedBuffer* gElEmptyStringBuf;
EXTERN_C char* gElEmptyString;

extern "C" {
    Char32 __cdecl _String_ToLowerCase(_ELASTOS Char32 ch);
    Char32 __cdecl _String_ToUpperCase(_ELASTOS Char32 ch);
}

static inline char* _getEmptyString()
{
    gElEmptyStringBuf->Acquire();
    return gElEmptyString;
}

static char* _allocFromUTF8(const char* in, Int32 numBytes)
{
    if (numBytes > 0) {
        SharedBuffer* buf = SharedBuffer::Alloc(numBytes + 1);
        if (buf) {
            char* str = (char*)buf->GetData();
            memcpy(str, in, numBytes);
            str[numBytes] = 0;
            return str;
        }
        return NULL;
    }

    return _getEmptyString();
}

//=======================================================================================
//              static members
//=======================================================================================
const Char32 String::INVALID_CHAR          = 0x110000;
const UInt32 String::MIN_CODE_POINT        = 0x000000;
const UInt32 String::MAX_CODE_POINT        = 0x10FFFF;
const UInt32 String::MIN_HIGH_SURROGATE    = 0xD800;
const UInt32 String::MAX_HIGH_SURROGATE    = 0xDBFF;
const UInt32 String::MIN_LOW_SURROGATE     = 0xDC00;
const UInt32 String::MAX_LOW_SURROGATE     = 0xDFFF;

static const Char32 kByteMask               = 0x000000BF;
static const Char32 kByteMark               = 0x00000080;

// Mask used to set appropriate bits in first byte of UTF-8 sequence,
// indexed by number of bytes in the sequence.
// 0xxxxxxx
// -> (00-7f) 7bit. Bit mask for the first byte is 0x00000000
// 110yyyyx 10xxxxxx
// -> (c0-df)(80-bf) 11bit. Bit mask is 0x000000C0
// 1110yyyy 10yxxxxx 10xxxxxx
// -> (e0-ef)(80-bf)(80-bf) 16bit. Bit mask is 0x000000E0
// 11110yyy 10yyxxxx 10xxxxxx 10xxxxxx
// -> (f0-f7)(80-bf)(80-bf)(80-bf) 21bit. Bit mask is 0x000000F0
static const Char32 kFirstByteMark[] = {
    0x00000000, 0x00000000, 0x000000C0, 0x000000E0, 0x000000F0
};

//=======================================================================================
//              ctor/dtor
//=======================================================================================

String::String()
    : mString(NULL)
    , mCharCount(0)
{
}

String::String(const String& o)
    : mString(o.mString)
    , mCharCount(o.mCharCount)
{
    if (mString != NULL) {
        SharedBuffer::GetBufferFromData(mString)->Acquire();
    }
}

String::String(const char* other)
    : mString(NULL)
    , mCharCount(0)
{
    if (other != NULL) {
        mString = _allocFromUTF8(other, strlen(other));
        if (mString == NULL) {
            mString = _getEmptyString();
        }
    }
}

String::String(const char* other, Int32 numBytes)
    : mString(NULL)
    , mCharCount(0)
{
    if (other != NULL && numBytes > 0) {
        // assert(strlen(other) > numBytes);
        mString = _allocFromUTF8(other, numBytes);
        if (mString == NULL) {
            mString = _getEmptyString();
        }
    }
}

String::String(const ArrayOf<Char32>& array, Int32 offset)
    : mCharCount(0)
{
    mString = _getEmptyString();
    Append(array, offset, array.GetLength());
}

String::String(const ArrayOf<Char32>& array, Int32 offset, Int32 length)
    : mCharCount(0)
{
    mString = _getEmptyString();
    Append(array, offset, length);
}

String::String(const ArrayOf<Byte>& array, Int32 offset)
    : mString(NULL)
    , mCharCount(0)
{
    if (offset >= 0 && offset < array.GetLength()) {
        mString = _allocFromUTF8((char*)array.GetPayload() + offset, array.GetLength() - offset);
        if (mString == NULL) {
            mString = _getEmptyString();
        }
    }
}

String::String(const ArrayOf<Byte>& array, Int32 offset, Int32 length)
    : mString(NULL)
    , mCharCount(0)
{
    if (offset >= 0 && length > 0 && offset < array.GetLength()
            && offset + length <= array.GetLength()) {
        mString = _allocFromUTF8((char*)array.GetPayload() + offset, length);
        if (mString == NULL) {
            mString = _getEmptyString();
        }
    }
}

String::~String()
{
    if (mString != NULL) {
        SharedBuffer::GetBufferFromData(mString)->Release();
    }
}

Int32 String::GetHashCode() const
{
    Int64 h = 0;

    const char* string = mString;
    if (string) {
        for ( ; *string; ++string) {
            h = 5 * h + *string;
        }
    }
    return (Int32)h;
}

void String::SetCounted(Int32 charCount) const
{
    mCharCount = (0x7FFFFFFF & charCount);
    mCharCount |= (1 << 31); // set counted flag
}

void String::ClearCounted() const
{
    mCharCount = 0;
}

Boolean String::IsCounted() const
{
    return (mCharCount & (1 << 31)) != 0;
}

//=======================================================================================
//              Getter
//=======================================================================================

Int32 String::GetLength() const
{
    if (IsNullOrEmpty()) return 0;
    if (IsCounted()) return (0x7FFFFFFF & mCharCount);

    Int32 charCount = 0;
    Int32 byteLength;
    const char* p = mString;
    const char *pEnd = mString + GetByteLength() + 1;
    while (p && *p && p < pEnd) {
        byteLength = UTF8SequenceLength(*p);
        if (!byteLength || p + byteLength >= pEnd) break;
        p += byteLength;
        ++charCount;
    }

    SetCounted(charCount);
    return charCount;
}

Char32 String::GetChar(Int32 index) const
{
    if (IsNullOrEmpty() || index < 0) return INVALID_CHAR;

    Int32 byteLength, i = 0;
    const char* p = mString;
    const char *pEnd = mString + GetByteLength() + 1;
    while (p && *p && p < pEnd) {
        byteLength = UTF8SequenceLength(*p);
        if (!byteLength || p + byteLength >= pEnd) break;
        if ((i++) == index) {
            return GetCharInternal(p, &byteLength);
        }
        p += byteLength;
    }

    // index out of bounds
    return INVALID_CHAR;
}

AutoPtr<ArrayOf<Char16> > String::GetChar16s(Int32 start) const
{
    Int32 charCount = GetLength();
    return GetChar16s(start, charCount);
}

AutoPtr<ArrayOf<Char16> > String::GetChar16s(Int32 start, Int32 end) const
{
    Int32 charCount = GetLength();
    end = MIN(charCount, end);

    AutoPtr<ArrayOf<Char16> > array;
    if (start >= end || start < 0) {
        array = ArrayOf<Char16>::Alloc(0);
        return array;
    }

    array = ArrayOf<Char16>::Alloc(end - start);
    Int32 byteLength, i = 0, j = 0;
    const char* p = mString;
    const char *pEnd = mString + GetByteLength() + 1;
    Char16 ch;
    while (p && *p && p < pEnd) {
        if (i < start) {
            byteLength = UTF8SequenceLength(*p);
            if (!byteLength || p + byteLength >= pEnd) break;
        }
        else if (i < end) {
            ch = GetCharInternal(p, &byteLength);
            if (!byteLength || p + byteLength >= pEnd) break;
            assert(j < end - start);
            (*array)[j++] = ch;
        }
        else {
            break;
        }

        p += byteLength;
        if (++i >= end) break;
    }

    if (!IsCounted() && p + byteLength >= pEnd) {
        SetCounted(--i);
    }
    return array;
}

AutoPtr<ArrayOf<Char32> > String::GetChars(Int32 start) const
{
    Int32 charCount = GetLength();
    return GetChars(start, charCount);
}

AutoPtr<ArrayOf<Char32> > String::GetChars(Int32 start, Int32 end) const
{
    Int32 charCount = GetLength();
    end = MIN(charCount, end);

    AutoPtr<ArrayOf<Char32> > array;
    if (start >= end || start < 0) {
        array = ArrayOf<Char32>::Alloc(0);
        return array;
    }

    array = ArrayOf<Char32>::Alloc(end - start);
    Int32 byteLength, i = 0, j = 0;
    const char* p = mString;
    const char *pEnd = mString + GetByteLength() + 1;
    Char32 ch;
    while (p && *p && p < pEnd) {
        if (i < start) {
            byteLength = UTF8SequenceLength(*p);
            if (!byteLength || p + byteLength >= pEnd) break;
        }
        else if (i < end) {
            ch = GetCharInternal(p, &byteLength);
            if (!byteLength || p + byteLength >= pEnd) break;
            assert(j < end - start);
            (*array)[j++] = ch;
        }
        else {
            break;
        }

        p += byteLength;
        if (++i >= end) break;
    }

    if (!IsCounted() && p + byteLength >= pEnd) {
        SetCounted(--i);
    }
    return array;
}

Char32 String::GetCharInternal(const char* cur, Int32* numBytes)
{
    assert(numBytes);
    if (IsASCII(*cur)) {
        *numBytes = 1;
        return *cur;
    }

    const char first_char = *cur++;
    Char32 result = first_char;
    Char32 mask, to_ignore_mask;
    Int32 num_to_read = 0;
    for (num_to_read = 1, mask = 0x40, to_ignore_mask = 0xFFFFFF80;
         (first_char & mask);
         num_to_read++, to_ignore_mask |= mask, mask >>= 1) {
        // 0x3F == 00111111
        result = (result << 6) + (*cur++ & 0x3F);
    }
    to_ignore_mask |= mask;
    result &= ~(to_ignore_mask << (6 * (num_to_read - 1)));
    *numBytes = num_to_read;
    return result;
}

AutoPtr<ArrayOf<Byte> > String::GetBytes(Int32 start) const
{
    Int32 byteCount = GetByteLength();
    return GetBytes(start, byteCount);
}

AutoPtr<ArrayOf<Byte> > String::GetBytes(Int32 start, Int32 end) const
{
    Int32 byteCount = GetByteLength();
    end = MIN(byteCount, end);

    AutoPtr<ArrayOf<Byte> > array;
    if (start >= end || start < 0) {
        array = ArrayOf<Byte>::Alloc(0);
        return array;
    }

    const char* p = mString + start;
    Int32 length = end - start;
    array = ArrayOf<Byte>::Alloc(end - start);
    for (Int32 i = 0; i < length; ++i) {
        (*array)[i] = (Byte)(*(p + i));
    }
    return array;
}

Int32 String::ToByteIndex(Int32 charIndex) const
{
    return CharIndexToByteIndex(mString, charIndex);
}

Int32 String::CharIndexToByteIndex(const char* string, Int32 charIndex)
{
    if (string == NULL || string[0] == '\0' || charIndex < 0) return -1;

    Int32 charlen = 0, i = 0;
    Int32 byteLength = strlen(string);
    const char* p = string;
    const char* pEnd = p + byteLength + 1;

    // find where to start
    while (p && *p && p < pEnd) {
        if (i == charIndex) {
            return p - string;
        }

        charlen = UTF8SequenceLength(*p);
        if (!charlen || p + charlen >= pEnd) break;

        p += charlen;
        ++i;
    }

    return -1;
}

String String::Substring(Int32 start) const
{
    return Substring(start, GetLength());
}

String String::Substring(Int32 start, Int32 end) const
{
    if (!mString) return String(NULL);
    if (start < 0 || end <= 0) return String("");
    if (IsEmpty()) return String("");

    Int32 byteCount = GetByteLength();
    Int32 charCount = GetLength();
    if (end > charCount) {
        end = charCount;
    }

    if (start >= end) {
        return String("");
    }

    if (start == 0 && end == charCount) {
        return String(mString);
    }

    // NOTE last character not copied!
    // Fast range check.
    Int32 count = 0;
    const char *p = mString;
    const char *pEnd = p + byteCount + 1;
    const char *p1 = p, *p2 = pEnd;
    const char *p3 = p;

    if (start == byteCount) {
        p1 += byteCount;
    }
    else {
        Int32 charlen;
        while (p && *p && p < pEnd) {
            charlen = UTF8SequenceLength(*p);
            if (!charlen || p + charlen >= pEnd) break;
            else p += charlen;

            if (count == start) {
                p1 = p3;
            }
            if (count == end) {
                p2 = p3;
                break;
            }

            count++;
            p3 = p;
        }
    }

    if ((Int32)(p2 - mString) > byteCount) {
        p2 = mString + byteCount;
    }

    return String(p1, p2 - p1);
}

String String::Replace(Char32 oldChar, Char32 newChar) const
{
    assert(IsValidChar(oldChar) && IsValidChar(newChar));

    AutoPtr<ArrayOf<Char32> > charArray = GetChars();
    Boolean copied = FALSE;
    for (Int32 i = 0; i < charArray->GetLength(); ++i) {
        if ((*charArray)[i] == oldChar) {
            (*charArray)[i] = newChar;
            copied = TRUE;
        }
    }

    return copied? String(*charArray) : *this;
}

//=======================================================================================
//              Update
//=======================================================================================
void String::WriteUTFBytesToBuffer(Byte* dstP, Char32 srcChar, Int32 bytes)
{
    dstP += bytes;
    switch (bytes)
    {   /* note: everything falls through. */
        case 4: *--dstP = (Byte)((srcChar | kByteMark) & kByteMask); srcChar >>= 6;
        case 3: *--dstP = (Byte)((srcChar | kByteMark) & kByteMask); srcChar >>= 6;
        case 2: *--dstP = (Byte)((srcChar | kByteMark) & kByteMask); srcChar >>= 6;
        case 1: *--dstP = (Byte)(srcChar | kFirstByteMark[bytes]);
    }
}

AutoPtr<ArrayOf<Byte> > String::ToByteArray(Char32 ch)
{
    AutoPtr<ArrayOf<Byte> > array;
    if (!IsValidChar(ch)) {
        array = ArrayOf<Byte>::Alloc(0);
        return array;
    }

    Int32 bytes = GetByteCount(ch);
    array = ArrayOf<Byte>::Alloc(bytes);
    WriteUTFBytesToBuffer((Byte*)(array->GetPayload()), ch, bytes);
    return array;
}

ECode String::SetTo(const String& other)
{
    if (other.mString != NULL) {
        SharedBuffer::GetBufferFromData(other.mString)->Acquire();
    }
    if (mString != NULL) {
        SharedBuffer::GetBufferFromData(mString)->Release();
    }
    mString = other.mString;
    mCharCount = other.mCharCount;
    return NOERROR;
}

ECode String::SetTo(const char* other)
{
    if (other == NULL) {
        if (mString != NULL) {
            SharedBuffer::GetBufferFromData(mString)->Release();
            mString = NULL;
            SetCounted(0);
        }
        return NOERROR;
    }

    const char *newString = _allocFromUTF8(other, strlen(other));
    if (mString != NULL) {
        SharedBuffer::GetBufferFromData(mString)->Release();
    }
    mString = newString;
    ClearCounted();
    if (mString) return NOERROR;

    mString = _getEmptyString();
    return E_OUT_OF_MEMORY;
}

ECode String::SetTo(const char* other, Int32 numBytes)
{
    const char *newString = _allocFromUTF8(other, numBytes);
    if (mString != NULL) {
        SharedBuffer::GetBufferFromData(mString)->Release();
    }
    mString = newString;
    ClearCounted();
    if (mString) return NOERROR;

    mString = _getEmptyString();
    return E_OUT_OF_MEMORY;
}

ECode String::Append(Char32 ch)
{
    if (!IsValidChar(ch)) return E_INVALID_ARGUMENT;

    AutoPtr<ArrayOf<Byte> > buf = ToByteArray(ch);
    if (buf && buf->GetLength() > 0)
        return Append((const char*)buf->GetPayload(), buf->GetLength());
    return NOERROR;
}

ECode String::Append(const String& other)
{
    const Int32 numBytes = other.GetByteLength();
    if (GetByteLength() == 0) {
        SetTo(other);
        return NOERROR;
    }
    else if (numBytes == 0) {
        return NOERROR;
    }

    return RealAppend(other.string(), numBytes);
}

ECode String::Append(const String& other, Int32 charOffset)
{
    if (other.IsNullOrEmpty()) return NOERROR;

    Int32 byteOffset = CharIndexToByteIndex(other.mString, charOffset);
    if (byteOffset < 0) return E_INVALID_ARGUMENT;
    Int32 byteLength = other.GetByteLength();
    byteOffset = MIN(byteLength, byteOffset);
    return Append(other.mString + byteOffset, byteLength - byteOffset);
}

ECode String::Append(const String& other, Int32 offset, Int32 numOfChars)
{
    if (other.IsNullOrEmpty()) return NOERROR;
    if (numOfChars <= 0) return NOERROR;

    Int32 charlen = 0, i = 0, j = 0;
    Int32 byteLength = strlen(other.mString);
    const char* p = other.mString;
    const char* pEnd = p + byteLength + 1;
    Int32 startByte = -1, endByte = -1;

    // find where to start
    while (p && *p && p < pEnd) {
        if (j == numOfChars) {
            endByte = p - other.mString;
            break;
        }

        if (i == offset) startByte = p - other.mString;
        if (i >= offset) ++j;

        charlen = UTF8SequenceLength(*p);
        if (!charlen || p + charlen >= pEnd) {
            startByte = -1;
            endByte = -1;
            break;
        }

        p += charlen;
        ++i;
    }

    if (startByte < 0) return E_INVALID_ARGUMENT;
    if (endByte < 0) endByte = byteLength;
    return Append(other.mString + startByte, endByte - startByte);
}

ECode String::Append(const char* other)
{
    return Append(other, strlen(other));
}

ECode String::Append(const char* other, Int32 numOfBytes)
{
    if (!other) return E_INVALID_ARGUMENT;
    if (numOfBytes <= 0) return NOERROR;

    if (GetByteLength() == 0) {
        return SetTo(other, numOfBytes);
    }

    numOfBytes = MIN(strlen(other), (size_t)numOfBytes);
    return RealAppend(other, numOfBytes);
}

ECode String::AppendFormat(const char* fmt, ...)
{
    ClearCounted();

    va_list ap;
    va_start(ap, fmt);

    Int32 result = NOERROR;
    Int32 n = vsnprintf(NULL, 0, fmt, ap);
    if (n != 0) {
        Int32 oldLength = GetByteLength();
        char* buf = LockBuffer(oldLength + n);
        if (buf) {
            vsnprintf(buf + oldLength, n + 1, fmt, ap);
            UnlockBuffer(oldLength + n);
        }
        else {
            result = E_OUT_OF_MEMORY;
        }
    }

    va_end(ap);
    return result;
}

ECode String::RealAppend(const char* other, Int32 numOfBytes)
{
    if (other == NULL || numOfBytes <= 0) {
        return NOERROR;
    }

    ClearCounted();

    Int32 byteCount;
    SharedBuffer* buf;
    if (mString != NULL) {
        byteCount = GetByteLength();
        buf = SharedBuffer::GetBufferFromData(mString)
                ->EditResize(byteCount + numOfBytes + 1);
    }
    else {
        byteCount = 0;
        buf = SharedBuffer::Alloc(numOfBytes + 1);
    }

    if (buf) {
        char* str = (char*)buf->GetData();
        mString = str;
        str += byteCount;
        memcpy(str, other, numOfBytes);
        str[numOfBytes] = '\0';
        return NOERROR;
    }
    return E_OUT_OF_MEMORY;
}

ECode String::Append(const ArrayOf<Char32>& array, Int32 offset)
{
    return Append(array, offset, array.GetLength());
}

ECode String::Append(const ArrayOf<Char32>& array, Int32 offset, Int32 length)
{
    offset = MAX(0, offset);
    if (offset >= (Int32)array.GetLength()) return E_INVALID_ARGUMENT;
    length = MIN(array.GetLength() - offset, length);
    if (length <= 0) return NOERROR;

    Int32 isNullStr = IsNullOrEmpty();
    Int32 appendedCharCount = 0;
    Int32 totalByteLength = 0;

    AutoPtr<ArrayOf<Int32> > byteCounts = ArrayOf<Int32>::Alloc(length);
    if (byteCounts == NULL) return E_OUT_OF_MEMORY;
    for (Int32 i = 0; i < length; ++i) {
        (*byteCounts)[i] = GetByteCount(array[offset + i]);
        if ((*byteCounts)[i] > 0) {
            ++appendedCharCount;
            totalByteLength += (*byteCounts)[i];
        }
    }

    if (totalByteLength == 0) return NOERROR;

    AutoPtr<ArrayOf<Byte> > byteArray = ArrayOf<Byte>::Alloc(totalByteLength);
    if (byteArray == NULL) return E_OUT_OF_MEMORY;

    Byte* p = (Byte*)(byteArray->GetPayload());
    for (Int32 i = 0; i < length; ++i) {
        if ((*byteCounts)[i] > 0) {
            WriteUTFBytesToBuffer(p, array[offset + i], (*byteCounts)[i]);
            p += (*byteCounts)[i];
        }
    }

    ECode ec = RealAppend((char*)(byteArray->GetPayload()), totalByteLength);
    if (ec != NOERROR) return ec;

    if (isNullStr) {
        SetCounted(appendedCharCount);
    }
    else {
        ClearCounted();
    }

    return NOERROR;
}

//---- Lock/Unlock ----

char* String::LockBuffer(Int32 numBytes)
{
    assert(numBytes >= 0);
    SharedBuffer* buf;
    if (mString != NULL) {
        buf = SharedBuffer::GetBufferFromData(mString)
                ->EditResize(numBytes + 1);
    }
    else {
        buf = SharedBuffer::Alloc(numBytes + 1);
    }

    if (buf) {
        char* str = (char*)buf->GetData();
        mString = str;
        return str;
    }
    return NULL;
}

void String::UnlockBuffer()
{
    if (IsNull()) return;

    UnlockBuffer(strlen(mString));
}

ECode String::UnlockBuffer(Int32 numBytes)
{
    assert(numBytes >= 0);
    if (IsNull()) return NOERROR;

    if (numBytes != GetLength()) {
        SharedBuffer* buf = SharedBuffer::GetBufferFromData(mString)
                    ->EditResize(numBytes + 1);
        if (!buf) {
            return E_OUT_OF_MEMORY;
        }

        char* str = (char*)buf->GetData();
        str[numBytes] = 0;
        mString = str;
    }

    return NOERROR;
}

//---- Trim ----

String String::TrimStart() const
{
    if (!mString) return String(NULL);

    const char* str = mString;
    while (isspace(*str) && *str) {
        ++str;
    }

    Int32 byteCount = GetByteLength();
    if (byteCount == (str - mString)) return String("");
    return String(str, byteCount - (str - mString));
}

String String::TrimEnd() const
{
    if (!mString) return String(NULL);

    const char* end = mString + GetByteLength() - 1;
    while (isspace(*end) && end >= mString) {
        --end;
    }
    if (end < mString) return String("");
    return String(mString, end - mString + 1);
}

String String::Trim() const
{
    if (!mString) return String(NULL);

    const char* start = mString;
    while (isspace(*start) && *start) {
        ++start;
    }

    const char* end = mString + GetByteLength() - 1;
    while (isspace(*end) && end >= mString) {
        --end;
    }
    if(end < start) return String("");
    return String(start, end - start + 1);
}

//---- Case ----
Char32 String::ToLowerCase(Char32 codePoint)
{
    // Optimized case for ASCII
    if ('A' <= codePoint && codePoint <= 'Z') {
        return (codePoint + ('a' - 'A'));
    }
    if (codePoint < 192) {
        return codePoint;
    }
    return _String_ToLowerCase(codePoint);
}

Char32 String::ToUpperCase(Char32 codePoint)
{
    // Optimized case for ASCII
    if ('a' <= codePoint && codePoint <= 'z') {
        return (codePoint - ('a' - 'A'));
    }
    if (codePoint < 181) {
        return codePoint;
    }
    return _String_ToUpperCase(codePoint);
}

String String::ToLowerCase() const
{
    return ToLowerCase(0, GetLength());
}

String String::ToLowerCase(Int32 offset, Int32 numOfChars) const
{
    offset = MAX(offset, 0);
    AutoPtr<ArrayOf<Char32> > chars = GetChars();
    Int32 length = chars->GetLength();
    if (length == 0) return String("");

    AutoPtr<ArrayOf<Char32> > newChars = ArrayOf<Char32>::Alloc(length);
    for (Int32 i = 0; i < length; ++i) {
        if ((Int32)i >= offset && (Int32)i < offset + numOfChars) {
            (*newChars)[i] = ToLowerCase((*chars)[i]);
        }
        else {
            (*newChars)[i] = (*chars)[i];
        }
    }

    return String(*newChars);
}

String String::ToUpperCase() const
{
    return ToUpperCase(0, GetLength());
}

String String::ToUpperCase(Int32 offset, Int32 numOfChars) const
{
    offset = MAX(0, offset);
    AutoPtr<ArrayOf<Char32> > chars = GetChars();
    Int32 length = chars->GetLength();
    if (length == 0) return String("");

    AutoPtr<ArrayOf<Char32> > newChars = ArrayOf<Char32>::Alloc(length);
    for (Int32 i = 0; i < length; ++i) {
        if ((Int32)i >= offset && (Int32)i < offset + numOfChars) {
            (*newChars)[i] = ToUpperCase((*chars)[i]);
        }
        else {
            (*newChars)[i] = (*chars)[i];
        }
    }

    return String(*newChars);
}

//---- Compare ----

Int32 String::Compare(const char* other) const
{
    if (mString == other) return 0;
    if (!mString) return -2;
    if (!other) return 2;

    if (IsEmpty() && strcmp(other, "") == 0) {
        return 0;
    }

    return strcmp(mString, other);
}

Int32 String::CompareIgnoreCase(const char* other) const
{
    if (mString == other) return 0;
    if (!mString) return -2;
    if (!other) return 2;

    if (IsEmpty() && strcmp(other, "") == 0) {
        return 0;
    }

    Int32 strLength = strlen(mString);
    Int32 subLength = strlen(other);

    const char* p1 = mString;
    const char* p1End = mString + strLength;
    const char* p2 = other;
    const char* p2End = other + subLength;
    Int32 len1, len2;
    Char32 ch1, ch2;

    while (p1 < p1End && p2 < p2End) {
        if (IsASCII(*p1)) {
            if (*p1 != *p2 && ToUpperCase(*p1) != ToUpperCase(*p2))
                return *p1 - *p2;
            ++p1; ++p2;
        }
        else {
            ch1 = GetCharInternal(p1, &len1);
            ch2 = GetCharInternal(p2, &len2);
            if (ch1 != ch2 && ToUpperCase(ch1) != ToUpperCase(ch2))
                return ch1 - ch2;
            p1 += len1; p2 += len2;
        }
    }

    return *p1 - *p2;
}

//---- Contains/StartWith/EndWith ----

Boolean String::Contains(const char* subString) const
{
    if (!mString || !subString) return FALSE;
    if (subString[0] == '\0') return TRUE;
    return (NULL != strstr(mString, subString));
}

Boolean String::Contains(const String& subString) const
{
    return Contains(subString.mString);
}

Boolean String::StartWith(const char* subString) const
{
    if (!mString || !subString) return FALSE;
    if (subString[0] == '\0') return TRUE;
    return !strncmp(mString, subString, strlen(subString));
}

Boolean String::StartWith(const String& subString) const
{
    return StartWith(subString.mString);
}

Boolean String::EndWith(const char* subString) const
{
    if (!mString || !subString) return FALSE;
    if (subString[0] == '\0') return TRUE;

    const char *endString = mString + strlen(mString) - strlen(subString);
    if (endString < mString) return FALSE;
    return !strncmp(endString, subString, strlen(subString));
}

Boolean String::EndWith(const String& subString) const
{
    return EndWith(subString.mString);
}

Boolean String::ContainsIgnoreCase(const char* subString) const
{
    return IndexOfIgnoreCase(subString, 0) != -1;
}

Boolean String::ContainsIgnoreCase(const String& subString) const
{
    return ContainsIgnoreCase(subString.mString);
}

Boolean String::StartWithIgnoreCase(const char* subString) const
{
    return InsensitiveStartWith(mString, subString);
}
Boolean String::StartWithIgnoreCase(const String& subString) const
{
    return StartWithIgnoreCase(subString.mString);
}

Boolean String::EndWithIgnoreCase(const char* subString) const
{
    if (!mString || !subString) return FALSE;
    if (mString[0] == '\0' && subString[0] == '\0') return TRUE;

    const char *endString = mString + strlen(mString) - strlen(subString);
    if (endString < mString) return FALSE;

    return InsensitiveStartWith(endString, subString);
}

Boolean String::EndWithIgnoreCase(const String& subString) const
{
    return EndWithIgnoreCase(subString.mString);
}

Boolean String::InsensitiveStartWith(const char* string, const char* subString)
{
    if (!string || !subString) return FALSE;
    if (subString[0] == '\0') return TRUE;

    Int32 strLength = strlen(string);
    Int32 subLength = strlen(subString);
    if (strLength == 0 || subLength == 0 || strLength < subLength) return FALSE;

    const char* p1 = string;
    const char* p1End = string + strLength;
    const char* p2 = subString;
    const char* p2End = subString + subLength;
    Int32 len1, len2;
    Char32 ch1, ch2;

    while (p1 < p1End && p2 < p2End) {
        if (IsASCII(*p1)) {
            if (*p1 != *p2 && ToUpperCase(*p1) != ToUpperCase(*p2))
                return FALSE;
            ++p1; ++p2;
        }
        else {
            ch1 = GetCharInternal(p1, &len1);
            ch2 = GetCharInternal(p2, &len2);
            if (ch1 != ch2 && ToUpperCase(ch1) != ToUpperCase(ch2))
                return FALSE;
            p1 += len1; p2 += len2;
        }
    }

    return TRUE;
}

//---- IndexOf ----

Int32 String::ByteIndexOf(Char32 target) const
{
    if (!IsValidChar(target)) return -1;
    if (IsNullOrEmpty()) return -1;

    Int32 byteLength, i = 0;
    const char* p = mString;
    const char *pEnd = mString + GetByteLength() + 1;

    Char32 ch;
    while (p && *p && p < pEnd) {
        ch = GetCharInternal(p, &byteLength);
        if (!byteLength || p + byteLength >= pEnd) break;
        if (ch == target) return p - mString;
        p += byteLength;
    }

    if (p + byteLength >= pEnd && !IsCounted()) {
        SetCounted(--i);
    }
    return -1;
}

Int32 String::IndexOf(Char32 target, Int32 start) const
{
    if (!IsValidChar(target)) return -1;

    start = MAX(0, start);
    Int32 byteCount = GetByteLength();
    if (start >= byteCount) return -1;

    Int32 byteLength, i = 0;
    const char* p = mString;
    const char *pEnd = mString + byteCount + 1;
    Char32 ch;

    while (p && *p && p < pEnd) {
        if (i < start) {
            byteLength = UTF8SequenceLength(*p);
            if (!byteLength || p + byteLength >= pEnd) break;
        }
        else {
            ch = GetCharInternal(p, &byteLength);
            if (!byteLength || p + byteLength >= pEnd) break;
            if (ch == target) return i;
        }

        p += byteLength;
        ++i;
    }

    return -1;
}

Int32 String::IndexOfIgnoreCase(Char32 target, Int32 start) const
{
    if (!IsValidChar(target)) return -1;

    start = MAX(0, start);
    Int32 byteCount = GetByteLength();
    if (start >= byteCount) return -1;

    Int32 byteLength, i = 0;
    const char* p = mString;
    const char *pEnd = mString + byteCount + 1;
    const Char32 upperTarget = ToUpperCase(target);
    Char32 ch;

    while (p && *p && p < pEnd) {
        if (i < start) {
            byteLength = UTF8SequenceLength(*p);
            if (!byteLength || p + byteLength >= pEnd) break;
        }
        else {
            if (IsASCII(*p)) {
                byteLength = 1;
                if (p + byteLength >= pEnd) break;
                if (*p == target || ToUpperCase(*p) == upperTarget)
                    return i;
            }
            else {
                ch = GetCharInternal(p, &byteLength);
                if (!byteLength || p + byteLength >= pEnd) break;
                if (ch == target || ToUpperCase(ch) == upperTarget)
                    return i;
            }
        }

        p += byteLength;
        ++i;
    }

    return -1;
}

Int32 String::LastIndexOf(Char32 target) const
{
    return LastIndexOf(target, GetLength());
}

Int32 String::LastIndexOf(Char32 target, Int32 start) const
{
    if (IsNull()) return -1;
    if (!IsValidChar(target)) return -1;

    start = MAX(0, start);
    AutoPtr<ArrayOf<Char32> > chars = GetChars(0, start + 1);
    for (Int32 i = chars->GetLength() - 1; i >= 0; --i) {
        if ((*chars)[i] == target) {
            return i;
        }
    }

    return -1;
}

Int32 String::LastIndexOfIgnoreCase(Char32 target) const
{
    return LastIndexOfIgnoreCase(target, GetLength());
}

Int32 String::LastIndexOfIgnoreCase(Char32 target, Int32 start) const
{
    if (IsNull()) return -1;
    if (!IsValidChar(target)) return -1;

    start = MAX(0, start);
    const Char32 upperTarget = ToUpperCase(target);
    AutoPtr<ArrayOf<Char32> > chars = GetChars(0, start + 1);
    for (Int32 i = chars->GetLength() - 1; i >= 0; --i) {
        if ((*chars)[i] == target || ToUpperCase((*chars)[i]) == upperTarget) {
            return i;
        }
    }

    return -1;
}

Int32 String::IndexOf(const char* subString, Int32 start) const
{
    return SensitiveIndexOf(mString, subString, start);
}

Int32 String::IndexOf(const String& subString, Int32 start) const
{
    return IndexOf(subString.string(), start);
}

Int32 String::IndexOfIgnoreCase(const char* subString, Int32 start) const
{
    return InsensitiveIndexOf(mString, subString, start);
}

Int32 String::IndexOfIgnoreCase(const String& subString, Int32 start) const
{
    return IndexOfIgnoreCase(subString.mString, start);
}

Int32 String::LastIndexOf(const char* subString) const
{
    // Use GetLength() instead of GetLength() - 1 so lastIndexOf("") returns GetLength()
    String sub(subString);
    return LastIndexOf(subString, GetLength());
}

Int32 String::LastIndexOf(const char* subString, Int32 start) const
{
    String sub(subString);
    return LastIndexOf(sub, start);
}

Int32 String::LastIndexOf(const String& subString) const
{
    // Use GetLength() instead of GetLength() - 1 so lastIndexOf("") returns GetLength()
    return LastIndexOf(subString, GetLength());
}

Int32 String::LastIndexOf(const String& subString, Int32 start) const
{
    if (IsNull() || subString.IsNull()) return -1;

    AutoPtr<ArrayOf<Char32> > chars = GetChars();
    AutoPtr<ArrayOf<Char32> > subChars = subString.GetChars();

    Int32 count = chars->GetLength();
    Int32 subCount = subChars->GetLength();
    if (subCount <= count && start >= 0) {
        if (subCount > 0) {
            if (start > (Int32)(count - subCount)) {
                start = count - subCount;
            }

            if (start >= (Int32)count) {
                start = count - 1;
            }

            // count and subCount are both >= 1
            Char32 firstChar = (*subChars)[0];
            Int32 end = subCount;
            Int32 o1, o2, index;
            while (TRUE) {
                index = -1;
                for (Int32 i = start; i >= 0; --i) {
                    if ((*chars)[i] == firstChar) {
                        index = i;
                        break;
                    }
                }

                if (index == -1) {
                    return -1;
                }

                o1 = index;
                o2 = 0;
                while (++o2 < end && (*chars)[++o1] == (*subChars)[o2]) {
                    // Intentionally empty
                }
                if (o2 == end) {
                    return index;
                }
                start = index - 1;
            }
        }
        return start < (Int32)count ? start : count;
    }
    return -1;
}

Int32 String::LastIndexOfIgnoreCase(const char* subString) const
{
    // Use GetLength() instead of GetLength() - 1 so lastIndexOf("") returns GetLength()
    String sub(subString);
    return LastIndexOfIgnoreCase(sub, GetLength());
}

Int32 String::LastIndexOfIgnoreCase(const char* subString, Int32 start) const
{
    String sub(subString);
    return LastIndexOfIgnoreCase(sub, start);
}

Int32 String::LastIndexOfIgnoreCase(const String& subString) const
{
    // Use GetLength() instead of GetLength() - 1 so lastIndexOf("") returns GetLength()
    return LastIndexOfIgnoreCase(subString.mString, GetLength());
}

Int32 String::LastIndexOfIgnoreCase(const String& subString, Int32 start) const
{
    if (IsNull() || subString.IsNull()) return -1;

    AutoPtr<ArrayOf<Char32> > chars = GetChars();
    AutoPtr<ArrayOf<Char32> > subChars = subString.GetChars();

    Int32 count = chars->GetLength();
    Int32 subCount = subChars->GetLength();
    if (subCount <= count && start >= 0) {
        if (subCount > 0) {
            if (start > (Int32)(count - subCount)) {
                start = count - subCount;
            }

            if (start >= (Int32)count) {
                start = count - 1;
            }

            // count and subCount are both >= 1
            Char32 firstChar = (*subChars)[0];
            const Char32 upperFirstChar = ToUpperCase(firstChar);
            Int32 end = subCount;
            Int32 o1, o2, index;
            while (TRUE) {
                index = -1;
                for (Int32 i = start; i >= 0; --i) {
                    if ((*chars)[i] == firstChar || ToUpperCase((*chars)[i]) == upperFirstChar) {
                        index = i;
                        break;
                    }
                }

                if (index == -1) {
                    return -1;
                }

                o2 = 1;
                o1 = index + 1;
                while (o2 < end && ((*chars)[o1] == (*subChars)[o2]
                    || ToUpperCase((*chars)[o1]) == ToUpperCase((*subChars)[o2]))) {
                    // Intentionally empty
                    ++o1;
                    ++o2;
                }
                if (o2 == end) {
                    return index;
                }
                start = index - 1;
            }
        }
        return start < (Int32)count ? start : count;
    }
    return -1;
}

Boolean String::RegionMatches(Int32 thisStart, const String& otherStr, Int32 start, Int32 length) const
{
    if (!mString || otherStr.IsNull()) return FALSE;
    if (otherStr.IsEmpty() && IsEmpty()) return TRUE;

    Int32 otherStrLen = otherStr.GetLength();
    if (start < 0 || start + length > otherStrLen) {
        return FALSE;
    }

    Int32 strLen = GetLength();
    if (thisStart < 0 || thisStart + length > strLen) {
        return FALSE;
    }

    if (length == 0) {
        return TRUE;
    }

    const char* str1 = mString + ToByteIndex(thisStart);
    const char* str2 = otherStr.mString + ToByteIndex(start);
    return !strncmp(str1, str2, length);
}

Boolean String::RegionMatchesIgnoreCase(Int32 thisStart, const String& otherStr, Int32 start, Int32 length) const
{
    if (!mString || otherStr.IsNull()) return FALSE;
    if (otherStr.IsEmpty() && IsEmpty()) return TRUE;

    Int32 otherStrLen = otherStr.GetLength();
    if (start < 0 || start + length > otherStrLen) {
        return FALSE;
    }

    Int32 strLen = GetLength();
    if (thisStart < 0 || thisStart + length > strLen) {
        return FALSE;
    }

    if (length == 0) {
        return TRUE;
    }

    String subStr1 = Substring(thisStart, thisStart + length);
    String subStr2 = otherStr.Substring(start, start + length);
    return !subStr1.CompareIgnoreCase(subStr2);
}

Int32 String::SensitiveIndexOf(const char* string, const char* subString, Int32 startChar)
{
    if (!string || !subString) return -1;
    if (subString[0] == '\0') return 0;

    startChar = MAX(0, startChar);
    Int32 byteLength = strlen(string);
    Int32 subByteLength = strlen(subString);
    if (startChar >= byteLength || subByteLength > byteLength) return -1;

    Int32 byteStart = CharIndexToByteIndex(string, startChar);
    if (byteStart < 0 || subByteLength + byteStart > byteLength) return -1;

    Int32 i = 0, j = 0, o1 = 0, o2 = 0;
    Int32 charStart = startChar, charlen1 = 0;
    const char* p = string;
    const char* pEnd = p + byteLength + 1;
    Boolean found = FALSE;
    Int32 result = -1;

    while (subByteLength + byteStart <= byteLength) {
        i = byteStart;
        found = FALSE;
        for (; i < byteLength;) {
            p = string + i;

            if (IsASCII(*p)) {
                charlen1 = 1;
                if (p + charlen1 >= pEnd) return result;
                found = (*p == subString[0]);
                j = 1;
            }
            else {
                charlen1 = UTF8SequenceLength(*p);
                if (!charlen1 || p + charlen1 >= pEnd) return result;

                found = TRUE;
                for (j = 0; j < charlen1; ++j) {
                    if (*(p + j) != subString[j]) {
                        found = FALSE;
                        break;
                    }
                }
            }

            if (found) {
                if (j == subByteLength) {
                    return charStart;
                }
                break;
            }
            else {
                i += charlen1;
                charStart++;
            }
        }

        // handles subByteLength > count || start >= count
        if (!found || subByteLength + i > byteLength) return result;

        o1 = i; o2 = 0;

        while (++o2 < subByteLength && string[++o1] == subString[o2]) {
            // Intentionally empty
        }

        if (o2 == subByteLength) {
            return charStart;
        }

        if (found) {
            i += charlen1;
            ++charStart;
        }
        byteStart = i;
    }

    return result;
}

Int32 String::InsensitiveIndexOf(const char* string, const char* subString, Int32 startChar)
{
    if (!string || !subString) return -1;
    if (subString[0] == '\0') return 0;

    startChar = MAX(0, startChar);
    Int32 byteLength = strlen(string);
    Int32 subByteLength = strlen(subString);
    if (subByteLength > byteLength) return -1;

    Int32 byteStart = CharIndexToByteIndex(string, startChar);
    if (byteStart < 0 || subByteLength + byteStart > byteLength) return -1;

    Int32 i = 0, j = 0, o1 = 0, o2 = 0;
    Int32 charStart = startChar, charlen1 = 0, charlen2 = 0, remainCharLen = 0;
    const char* p = string;
    const char* pEnd = p + byteLength + 1;
    Char32 ch1, ch2;
    Boolean found = FALSE;
    Int32 result = -1;

    while (subByteLength + byteStart <= byteLength) {
        i = byteStart;
        found = FALSE;
        for (; i < byteLength;) {
            p = string + i;

            if (IsASCII(*p)) {
                charlen1 = 1;
                if (p + charlen1 >= pEnd) return result;
                found = (*p == subString[0] || ToUpperCase(*p) == ToUpperCase(subString[0]));
                j = 1;
            }
            else {
                ch1 = GetCharInternal(p, &charlen1);
                if (!charlen1 || p + charlen1 >= pEnd) return result;
                ch2 = GetCharInternal(subString, &charlen2);
                if (!charlen2 || charlen2 > subByteLength) return result;

                found = ch1 == ch2 || ToUpperCase(ch1) == ToUpperCase(ch2);
                j = charlen1;
            }

            if (found) {
                if (j == subByteLength) {
                    return charStart;
                }
                break;
            }
            else {
                i += charlen1;
                charStart++;
            }
        }

        // handles subByteLength > count || start >= count
        if (!found || subByteLength + i > byteLength) return result;

        o1 = i; o2 = 0;
        for (o1 = i, o2 = 0; o2 < subByteLength;) {
            p = string + o1;

            if (IsASCII(*p)) {
                remainCharLen = 1;
                if (p + remainCharLen >= pEnd) break;

                if (*p != subString[o2] && ToUpperCase(*p) != ToUpperCase(subString[o2]))
                    break;
                o1 += remainCharLen; o2 += remainCharLen;
            }
            else {
                ch1 = GetCharInternal(p, &charlen1);
                if (!charlen1 || p + charlen1 >= pEnd) return result;
                ch2 = GetCharInternal(subString + o2, &charlen2);
                if (!charlen2 || o2 + charlen2 > subByteLength) return result;

                if (ch1 != ch2 && ToUpperCase(ch1) != ToUpperCase(ch2))
                    break;
                o1 += charlen1; o2 += charlen1;
            }
        }

        if (o2 == subByteLength) {
            return charStart;
        }

        if (found) {
            i += charlen1;
            charStart++;
        }
        byteStart = i;
    }

    return result;
}

_ELASTOS_NAMESPACE_END
