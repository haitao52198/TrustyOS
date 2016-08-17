
#include "ucase.h"
#include "utrie2.h"

struct UCaseProps {
    const int32_t *indexes;
    const uint16_t *exceptions;
    const Char16 *unfold;

    UTrie2 trie;
    uint8_t formatVersion[4];
};

#include "ucase_props_data.c"
#define GET_EXCEPTIONS(csp, props) ((csp)->exceptions + ((props) >> UCASE_EXC_SHIFT))

#define PROPS_HAS_EXCEPTION(props) ((props)&UCASE_EXCEPTION)

static const uint8_t flagsOffset[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

#define HAS_SLOT(flags, idx) ((flags) & (1 << (idx)))
#define SLOT_OFFSET(flags, idx) flagsOffset[(flags) & ((1 << (idx)) - 1)]

#define GET_SLOT_VALUE(excWord, idx, pExc16, value) \
    if (((excWord)&UCASE_EXC_DOUBLE_SLOTS) == 0) { \
        (pExc16) += SLOT_OFFSET(excWord, idx); \
        (value) = *pExc16; \
    } else { \
        (pExc16) += 2 * SLOT_OFFSET(excWord, idx); \
        (value) = *pExc16++; \
        (value) = ((value) << 16) | *pExc16; \
    }

Char32 ucase_tolower(
    /* [in] */ const UCaseProps *csp,
    /* [in] */ Char32 c)
{
    uint16_t props = UTRIE2_GET16(&csp->trie, c);
    if (!PROPS_HAS_EXCEPTION(props)) {
        if (UCASE_GET_TYPE(props)>=UCASE_UPPER) {
            c += UCASE_GET_DELTA(props);
        }
    }
    else {
        const uint16_t* pe = GET_EXCEPTIONS(csp, props);
        uint16_t excWord = *pe++;
        if (HAS_SLOT(excWord, UCASE_EXC_LOWER)) {
            GET_SLOT_VALUE(excWord, UCASE_EXC_LOWER, pe, c);
        }
    }
    return c;
}

Char32 ucase_toupper(
    /* [in] */ const UCaseProps *csp,
    /* [in] */ Char32 c)
{
    uint16_t props = UTRIE2_GET16(&csp->trie, c);
    if (!PROPS_HAS_EXCEPTION(props)) {
        if (UCASE_GET_TYPE(props) == UCASE_LOWER) {
            c += UCASE_GET_DELTA(props);
        }
    }
    else {
        const uint16_t* pe = GET_EXCEPTIONS(csp, props);
        uint16_t excWord = *pe++;
        if (HAS_SLOT(excWord, UCASE_EXC_UPPER)) {
            GET_SLOT_VALUE(excWord, UCASE_EXC_UPPER, pe, c);
        }
    }
    return c;
}

int32_t ucase_getType(
    /* [in] */ const UCaseProps *csp,
    /* [in] */ Char32 c)
{
    uint16_t props = UTRIE2_GET16(&csp->trie, c);
    return UCASE_GET_TYPE(props);
}

#define GET_CASE_PROPS() &ucase_props_singleton
Boolean u_isULowercase(
    /* [in] */ Char32 c)
{
    return (Boolean)(UCASE_LOWER == ucase_getType(GET_CASE_PROPS(), c));
}

/* Transforms the Unicode character to its lower case equivalent.*/
Char32 u_tolower(
    /* [in] */ Char32 c)
{
    return ucase_tolower(GET_CASE_PROPS(), c);
}

/* Transforms the Unicode character to its upper case equivalent.*/
Char32 u_toupper(
    /* [in] */ Char32 c)
{
    return ucase_toupper(GET_CASE_PROPS(), c);
}
