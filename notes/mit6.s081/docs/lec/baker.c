#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <sys/mman.h>

#include <assert.h>
#include <fcntl.h>
#include <pthread.h>

//
// Toy implementation of baker's GC algorithm for illustration in lecture.
//

#define LISTSZ 300

// Only one type of object: list elements
struct elem {
  int val;
  struct elem *next;
};

//
// API to collector
//

struct elem *readptr(struct elem **ptr);
struct elem *new();

//
// Mutator
//

// The roots of the mutator
struct elem *root_head;
struct elem *root_last;

void
print_clist() {
  printf("list: %p %p\n", root_head, root_last);
  struct elem *e = root_head;
  do {
    printf("elem: %d %p %p\n", readptr(&e)->val, readptr(&e), readptr(&e)->next);
    e = readptr(&e)->next;
  } while (readptr(&e) != root_head);
  printf("\n");
}

void
check_clist(int l) {
  int sum = 0;
  struct elem *e = readptr(&root_head);

  do {
    sum += readptr(&e)->val;
    e = readptr(&e)->next;
  } while (readptr(&e) != root_head);
  assert(sum == (l * (l-1))/2);
}


// Make a circular list
void
make_clist(void) {
  struct elem *e;

  root_head = new();
  readptr(&root_head)->val = 0;
  readptr(&root_head)->next = readptr(&root_head);
  root_last = readptr(&root_head);

  for (int i = 1; i < LISTSZ; i++) {
    e = new();
    readptr(&e)->next = readptr(&root_head);
    readptr(&e)->val = i;
    root_head = readptr(&e);
    readptr(&root_last)->next = readptr(&root_head);
    check_clist(i+1);
  }
}

void *
app_thread(void *x) {
  for (int i = 0; i < 1000; i++) {
    make_clist();
    check_clist(LISTSZ);
  }
}

//
// Collector
//

#define VM 1     // Using VM

struct obj {
  struct elem e;
  struct obj *newaddr;   // if set, obj already has been copied to to-space
  struct obj *pad;       // Hack: ensure struct obj divides page size
};
  
#define PGSIZE 4096
#define SPACESZ (3*PGSIZE)    // size of space
#define TOFROM (2*SPACESZ)    // size of to and from
#define NROOT 2               // number of roots

// the space GC allocates in and copies to
static void *to; 
static void *to_free_start;
static void *scanned;

// the space GC is collecting
static void *from;

static int collecting;         // Collecting?

// Address where from+to space is mapped for mutator
static void *mutator = NULL;
// Address where from+to space is mapped for collector
static void *collector = NULL;

static pthread_mutex_t lock;
static pthread_cond_t cond;
static int done;

#define APPADDR(a) (assert((void *)(a) >= mutator && (void *) (a) < mutator + TOFROM))
#define GCADDR(a) (assert((void *)(a) >= collector && (void *)(a) < collector + TOFROM))

// Convert from collector address to app address
void *
appaddr(void *a) {
  assert(a >= collector && a < collector + TOFROM);
  return (a - collector) + mutator;
}

// Convert from app address to gc address
void *
gcaddr(void *a) {
  assert(a >= mutator && a < mutator + TOFROM);
  return (a - mutator) + collector;
}

struct obj *
alloc() {
  struct obj *o = (struct obj *) to_free_start;
  assert(to_free_start + sizeof(struct obj) < to + SPACESZ);
  to_free_start += sizeof(struct obj);
  return o;
}

struct obj *
copy(struct obj *o) {
  struct obj *n = gcaddr(alloc());
  n->e = o->e;
  o->newaddr = n;
  n->newaddr = NULL;
  return (appaddr(n));
}

int
in_from(struct obj *o) {
  return ((void *) o >= from && ((void *) o < from+SPACESZ));
}

// Forward pointer to to-space. If pointer is in from-space, make a
// copy of the object in to-space and return pointer to copy. If
// object was already copied (there may several pointers to the same
// object), return the pointer to the to-space object.  If pointer is
// already in to-space, just return the pointer.
struct obj *
forward(struct obj *o) {
  struct obj *n = o;
  if (in_from(o)) {
    if (o->newaddr == 0) {   // not copied yet?
      n = copy(o);
      printf("forward: copy %p (%d) to %p\n", o, o->e.val, n);
    } else {
      n = o->newaddr;
      printf("forward: already copied %p (%d) to %p\n", o, o->e.val, n);
    }
  }
  return n;
}

// Scan a page of unscanned objects and forward the pointers in the
// objects.
int
scan(void *start) {
  int sz = 0;
  void *next = gcaddr(start);
  printf("scan %p unscanned %p\n", gcaddr(start), gcaddr(to_free_start));
  while (next < gcaddr(start)+PGSIZE && next < gcaddr(to_free_start)) {
    struct obj *o = (struct obj *) next;
    if(o->e.next != 0)
      printf("scan: %p %d %p\n", o, o->e.val, o->e.next);
    o->e.next = (struct elem *) forward((struct obj *) (o->e.next));
    next += sizeof(struct obj);
  }
  scanned = appaddr(next);
  printf("scan done %p\n", to_free_start);
}

void
end_collecting() {
  printf("collection done\n");
  memset(from, 0, SPACESZ);
  collecting = 0;
}

void
flip() {
  char *tmp = to;

  printf("flip spaces\n");

  assert(!collecting);
  to = from;
  to_free_start = from;
  from = tmp;

  collecting = 1;
  scanned = to;

#ifdef VM
  if (mprotect(to, SPACESZ, PROT_NONE) < 0) {
    fprintf(stderr, "Couldn't unmap to space; %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
#endif

  // move root_head and root_last to to-space
  root_head = (struct elem *) forward((struct obj *)root_head);
  root_last = (struct elem *) forward((struct obj *)root_last);

  pthread_cond_broadcast(&cond);
}

// Without VM: check if pointer should be forwarded.  With VM, do
// nothing; the segfault handler will fix things up and when it
// returns the pointer will be pointing to object in to-space.
struct elem *
__attribute__ ((noinline))
readptr(struct elem **p) {
#ifndef VM
  struct obj *o = forward((struct obj *) (*p));
  *p = (struct elem *) o;
#endif
  return *p;
}

struct elem *
new(void) {
  struct elem *n;
  
  pthread_mutex_lock(&lock);
  if (collecting && scanned < to_free_start) {
    scan(scanned);
    if (scanned >= to_free_start) {
      end_collecting();
    }
  }
  if (to_free_start + sizeof(struct obj) >= to + SPACESZ) {
    flip();
  }
  n = (struct elem *) alloc();
  pthread_mutex_unlock(&lock);
  
  return n;
}

void *
collector_thread(void *x) {
  printf("start collector\n");
  while (!done) {
    pthread_mutex_lock(&lock);
    if (collecting && scanned < to_free_start) {
      printf("collector: scan\n");
      scan(scanned);
      if (scanned >= to_free_start) {
        end_collecting();
      }
    }
    while (!collecting) {
      pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);
  }
  printf("done collector\n");
}

// Rounds v down to an alignment a
#define align_down(v, a) ((v) & ~((typeof(v))(a) - 1))

static void
handle_sigsegv(int sig, siginfo_t *si, void *ctx)
{
  uintptr_t fault_addr = (uintptr_t)si->si_addr;
  double *page_base = (double *)align_down(fault_addr, PGSIZE);

  printf("fault at adddr %p (page %p)\n", fault_addr, page_base);

  pthread_mutex_lock(&lock);
  scan(page_base);
  pthread_mutex_unlock(&lock);
  
  if (mprotect(page_base, PGSIZE, PROT_READ|PROT_WRITE) < 0) {
    fprintf(stderr, "Couldn't mprotect to-space page; %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

//
// Initialization
//

static void
setup_spaces(void)
{
  struct sigaction act;
  int shm;

  // make a shm obj for to-space and from-space
  if ((shm = shm_open("baker", O_CREAT|O_RDWR|O_TRUNC, S_IRWXU)) < 0) {
    fprintf(stderr, "Couldn't shm_open: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (shm_unlink("baker") != 0) {
    fprintf(stderr, "shm_unlink failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  if (ftruncate(shm, TOFROM) != 0) {
    fprintf(stderr, "ftruncate failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // map it once for mutator
  mutator = mmap(NULL, TOFROM, PROT_READ|PROT_WRITE, MAP_SHARED, shm, 0);
  if (mutator == MAP_FAILED) {
    fprintf(stderr, "Couldn't mmap() from space; %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // map it once for collector
  collector = mmap(NULL, TOFROM, PROT_READ|PROT_WRITE, MAP_SHARED, shm, 0);
  if (collector == MAP_FAILED) {
    fprintf(stderr, "Couldn't mmap() from space; %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  from = mutator;
  to = mutator + SPACESZ;
  to_free_start = to;

  // Register a signal handler to capture SIGSEGV.
  act.sa_sigaction = handle_sigsegv;
  act.sa_flags = SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  if (sigaction(SIGSEGV, &act, NULL) == -1) {
    fprintf(stderr, "Couldn't set up SIGSEGV handler: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}


int
main(int argc, char *argv[])
{
  pthread_t the_app;
  pthread_t the_collector;
  
  printf("sizeof %d %d\n", sizeof(struct elem), sizeof(struct obj));
  assert(PGSIZE % sizeof(struct obj) == 0);
  setup_spaces();

  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cond, NULL);

  if (pthread_create(&the_app, NULL, app_thread,  NULL) != 0) {
    fprintf(stderr, "Couldn't start app thread: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

#ifdef VM
  if (pthread_create(&the_collector, NULL, collector_thread,  NULL) != 0) {
    fprintf(stderr, "Couldn't start app thread: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
#endif

  if (pthread_join(the_app, NULL) != 0) {
    fprintf(stderr, "Couldn't join app thread: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  done = 1;
  
  return 0;
}
