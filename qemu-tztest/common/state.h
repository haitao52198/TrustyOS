#ifndef _STATE_H
#define _STATE_H

extern char *sec_state_str[];
extern uint32_t secure_state;
extern const uint32_t exception_level;

#define EL0 0
#define EL1 1
#define EL2 2
#define EL3 3

#define SECURE 0
#define NONSECURE 1

#endif
