typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;

/* Andrei add definition for type lock_t used for thread locks */
typedef struct {
    volatile int ticket;
    volatile int turn;
} lock_t;
