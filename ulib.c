#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

#define MAX_THREADS 64
#define PGSIZE 4096
#define UNUSED 0
#define USED 1

struct ptr_struct {
    int state;
    void *ptr;
    void *stack;
};

struct ptr_struct threads[MAX_THREADS];

/* Andrei: x86 atomic add which returns the addition result */
static inline int
fetch_and_add(int *var, int val)
{
    __asm__ volatile("lock; xaddl %0, %1"
    : "+r" (val), "+m" (*var) // in + out
    : // No input
    : "memory"
    );
    return val;
}

/* Create the lock and set it's fields to 0 */
void
lock_init(lock_t *lock)
{
    lock->ticket = 0;
    lock->turn = 0;
}

/* Andrei: Acquire lock and spin in a while loop  */
void
lock_acquire(lock_t *lock)
{
    // Use the static inline function to spin the lock till the values become equal (on release)
    int turn = fetch_and_add(&lock->ticket, 1);
    while(lock->turn != turn) {} // spin lock
}

/* Andrei: Release lock by setting the turn to 1,
 * at which point the lock is released by breaking out of the loop */
void
lock_release(lock_t *lock)
{
    lock->turn = lock->turn + 1;
}

int
thread_create(void (*start_routine)(void*, void*), void* arg1, void* arg2)
{
  // Use malloc to create userstack for thread
  // The stack should be one page in size and page-aligned
  // Therefore the address
  void* freeptr = malloc(PGSIZE*2);
  void* stack;
  if(freeptr == 0)
    return -1;
  // Ensure address is page aligned and store it in stack variable
  if((uint)freeptr % PGSIZE == 0)
    stack = freeptr;
  else // page align the stack
    stack = freeptr + (PGSIZE - ((uint)freeptr % PGSIZE));
  // In our threads list, look for the first free slot and assign
  // The freeptr variable and stack to the first free thread
  for(int i = 0; i < MAX_THREADS; i++) {
    if(threads[i].state == UNUSED){
      threads[i].ptr = freeptr; // pointer to the start
      threads[i].stack = stack; // stack pointer
      threads[i].state = USED;
      break;
    }
  }
  // Stack address must be page aligned to give to clone
  int ret = clone(start_routine, arg1, arg2, stack);
  return ret;
}

int
thread_join()
{
  void* stack;
  // Join returns the PID of a returning process and fills in the stack
  int ret = join(&stack);
  // Find the thread that matches (state must be used) with
  // the corresponding stack and call free on it's ptr
  for(int i = 0; i < MAX_THREADS; i++) {
    if(threads[i].state == USED && threads[i].stack == stack){
      free(threads[i].ptr);
      threads[i].ptr = NULL;
      threads[i].stack = NULL;
      threads[i].state = UNUSED;
      break;
    }
  }
  return ret;
}

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

