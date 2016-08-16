#ifndef __UCASE_H__
#define __UCASE_H__

#include <elatypes.h>

using namespace Elastos;

extern "C" {

struct UCaseProps;
typedef struct UCaseProps UCaseProps;


/* single-code point functions */

Char32 u_toupper(Char32 c);

Char32 u_tolower(Char32 c);

Boolean u_isULowercase(Char32 c);

}
/* indexes into indexes[] */
enum {
    UCASE_IX_INDEX_TOP,
    UCASE_IX_LENGTH,
    UCASE_IX_TRIE_SIZE,
    UCASE_IX_EXC_LENGTH,
    UCASE_IX_UNFOLD_LENGTH,

    UCASE_IX_MAX_FULL_LENGTH=15,
    UCASE_IX_TOP=16
};
#define UCASE_TYPE_MASK     3
enum {
    UCASE_NONE,
    UCASE_LOWER,
    UCASE_UPPER,
    UCASE_TITLE
};

#define UCASE_GET_TYPE(props) ((props)&UCASE_TYPE_MASK)
#define UCASE_EXCEPTION     8
#define UCASE_DELTA_SHIFT   6
#define UCASE_GET_DELTA(props) ((int16_t)(props)>>UCASE_DELTA_SHIFT)
#define UCASE_EXC_SHIFT     4

enum {
    UCASE_EXC_LOWER,
    UCASE_EXC_FOLD,
    UCASE_EXC_UPPER,
    UCASE_EXC_TITLE,
    UCASE_EXC_4,            /* reserved */
    UCASE_EXC_5,            /* reserved */
    UCASE_EXC_CLOSURE,
    UCASE_EXC_FULL_MAPPINGS,
    UCASE_EXC_ALL_SLOTS     /* one past the last slot */
};

#define UCASE_EXC_DOUBLE_SLOTS      0x100

#endif
