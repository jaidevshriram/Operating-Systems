#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "proc_stat.h"

#define NULL 0

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;
int priority_queue[5][NPROC];
int priority_queue_top[5];
int priority_tick_count[] = {1, 2, 4, 8, 16};

int nextpid = 1;
int max_wait = 3;
int next_low_queue = 0;

extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void print_mlfq();

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:

  p->state = EMBRYO;
  p->pid = nextpid++;

#ifdef PRIORITY
  if(p->pid <=2)
    p->priority = 0;
  else
    p->priority = 60;
#else
#ifdef MLFQ
  p->priority = p->current_queue = 0;
  priority_queue[p->priority][priority_queue_top[p->priority]++] = p->pid;
  p->runtime = 0;
  p->num_run = 0;
  for(int i=0; i<5; i++)
    p->ticks[i] = 0;
  p->last_time = ticks;
#endif
#endif

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  //Include the new fields
  p->ctime = ticks;
  p->etime = 0;
  p->rtime = 0;

  // print_mlfq();

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return
  curproc->etime = ticks;
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

//System call to print information on all running/sleeping processes
int
ps(void)
{
  struct proc *p;

  acquire(&ptable.lock);
  cprintf("NAME \t pid \t state \t priority \t CTIME\n");
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == SLEEPING)
      cprintf("%s \t %d \t SLEEPING \t %d \t %d\n", p->name, p->pid, p->priority, p->ctime);
    else if(p->state == RUNNING)
      cprintf("%s \t %d \t RUNNING \t %d \t %d\n", p->name, p->pid, p->priority, p->ctime);
    else if(p->state == RUNNABLE)
      cprintf("%s \t %d \t RUNNABLE \t %d \t %d\n", p->name, p->pid, p->priority, p->ctime);
  }
  release(&ptable.lock);
  return 0;
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}


// Extends the wait sys call
// Return -1 if this process has no children.
int
waitx(int *wtime, int *rtime)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);

        //Including fields
        *wtime = p->etime - p->rtime - p->ctime;
        *rtime = p->rtime;

        // cprintf("%d is runtime\n", p->rtime);

        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//Change Priority
int
set_priority(int pid, int priority)
{
  struct proc *p;
  int not_found = 1, ret = -1;

  if(priority>100 || priority<0)
    return -1;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC] && not_found; p++) {
    if(p->pid == pid){
      ret = p->priority;
      p->priority = priority;
      not_found = 0;
    }
  }

  release(&ptable.lock);

  return ret; 
}

int getpinfo(struct proc_stat *proc_stat, int pid)
{

  int not_found = 1;
  acquire(&ptable.lock);

  struct proc *p;
  for(p = ptable.proc; p < &ptable.proc[NPROC] && not_found; p++)
  {
    if(p->pid == pid)
    {
      proc_stat->pid = pid;
      proc_stat->runtime = p->rtime;
      proc_stat->current_queue = p->current_queue;
      proc_stat->num_run = p->num_run;
      // cprintf("current queue is %d\n", p->current_queue);
      for(int i=0; i<5; i++)
      {
        // cprintf("ticks in queue %d are %d\n", i, p->ticks[i]);
        proc_stat->ticks[i] = p->ticks[i];
      }

      not_found = 0;
    }
  }

  release(&ptable.lock);

  if(not_found == 1)
  {
    cprintf("Process with pid %d doesn't exist in proc table\n");
    return -1;
  }

  return 0;
}

#ifdef MLFQ

void delete_pid(int pr, int pos)
{
  for(int i=pos; i<priority_queue_top[pr]; i++)
    priority_queue[pr][i] = priority_queue[pr][i+1];
  priority_queue_top[pr]--;
}

void print_mlfq()
{
  cprintf("------------------\n");
  for(int i=0; i<5; i++)
  {
    for(int j=0; j<priority_queue_top[i]; j++)
    {    
      cprintf("%d is in queue %d\n", priority_queue[i][j], i);
    }
  }
  cprintf("------------------\n");

}

void remove_dead_process()
{
  for(int i=0; i<5; i++)
  {
    if(priority_queue_top[i]>0)
    {
      for(int j=0; j<priority_queue_top[i]; j++)
      {
        int not_found = 1;
        struct proc *p;
        for(p = ptable.proc; p < &ptable.proc[NPROC] && not_found; p++)
        {
          if(p->pid == priority_queue[i][j])
            not_found = 0;
        }

        if(not_found == 1 && priority_queue[i][j] > 2)
        {
          delete_pid(i, j);
          // print_mlfq();
        }
      }
    }
  }
}

//This functions moves a starved process to a higher queue
void upgrade_process()
{
  int rerun = 1;
  while(rerun) {
    
    rerun = 0;
    
    for(int i=1; i<5; i++)
    {
      if(priority_queue_top[i]>0)
      {
        for(int j=0; j<priority_queue_top[i]; j++)
        {
          struct proc *p;
          for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
          {
            if(p->pid == priority_queue[i][j] && p->pid >= 3 && (ticks - p->last_time) > max_wait)
            {
              delete_pid(i,j);
              j--;
              priority_queue[i-1][priority_queue_top[i-1]++] = p->pid;
              p->priority = i-1;
              p->current_queue = i-1;
              break;
            }
          }
        }
      }
    }

  }
}

//This process moves a process to a lower queue
void downgrade_process()
{
  int rerun = 1;
  while(rerun) {
    
    rerun = 0;
    
    for(int i=0; i<4; i++)
    {
      if(priority_queue_top[i]>0)
      {
        for(int j=0; j<priority_queue_top[i]; j++)
        {
          struct proc *p;
          for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
          {
            if(p->pid == priority_queue[i][j] && p->pid >= 3 && p->ticks[i] >= priority_tick_count[i])
            {
              delete_pid(i,j);
              j--;
              priority_queue[i+1][priority_queue_top[i+1]++] = p->pid;
              p->priority = i+1;
              p->current_queue = i+1;
              break;
            }
          }
        }
      }
    }
  
  }
}

void update_queue()
{
  remove_dead_process();
  upgrade_process();
}

#endif

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);

  #ifdef MLFQ
  
    update_queue();
    // print_mlfq();

    struct proc *chosen = NULL;

    for(int i=0; i<5; i++)
    {
      // cprintf("%d\n", i);
      if(priority_queue_top[i]==0)
        continue;
    
      for(int j=0; j<priority_queue_top[i]; j++)
      {
        for(p = ptable.proc; p < &ptable.proc[NPROC] && !chosen; p++)
        {
          if(priority_queue[i][j] == p->pid && p->state == RUNNABLE)
          {
            // cprintf("%d is chosen\n", p->pid);
            chosen = p;
            break;
          }
        }

        if(chosen != NULL)
          break;
      }

      if(chosen!=NULL)
      {
        p = chosen;
        break;
      }
    }

    if(next_low_queue >= priority_queue_top[4])
      next_low_queue = 0;

    while(next_low_queue < priority_queue_top[4] && chosen == NULL)
    {
      next_low_queue = (next_low_queue+1)%priority_queue_top[4];

      for(p = ptable.proc; p < &ptable.proc[NPROC] && !chosen; p++)
      {
        if(priority_queue[4][next_low_queue] == p->pid && p->state == RUNNABLE)
        {
          chosen = p;
          break;
        }
      }
    }

    if(chosen == NULL)
    {
      release(&ptable.lock);
      continue;
    }
  
  #else
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      
      if(p->state != RUNNABLE)
        continue;

      #ifdef RR
      #else
      #ifdef FCFS
        // --------------------- START FCFS -----------------------
        // cprintf("FCFS\n");
        struct proc *lowest_p = p;

        for(struct proc *j = ptable.proc; j < &ptable.proc[NPROC]; j++){
          if(j->state != RUNNABLE || j->pid<=2)
            continue;
          if(j->ctime < lowest_p->ctime)
            lowest_p = j;
        }

        p = lowest_p;
        // ---------------------- END FCFS -------------------------

      #else
      #ifdef PRIORITY
        // --------------------- START PRIORITY -----------------------
        // cprintf("PRIORITY\n");
        struct proc *highest_p;
        highest_p = p;

        for(struct proc *j = ptable.proc; j < &ptable.proc[NPROC]; j++){
          if(j->state != RUNNABLE)
            continue;
          if(highest_p->priority > j->priority)
            highest_p = j;
        }

        p = highest_p;
        // ---------------------- END PRIORITY -------------------------
      #else
      #endif
      #endif
      #endif


      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.

      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      p->num_run++;

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
  
    }
  #endif

  #ifdef MLFQ
      
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.

      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      p->num_run++;

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
      
      downgrade_process();
  #endif

    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

#ifdef MLFQ
  for(int j=0; j<priority_queue_top[p->current_queue]; j++)
  {
    if(priority_queue[p->current_queue][j] == p->pid)
    {
      delete_pid(p->current_queue, j);
      break;
    }
  }
#endif

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
    {
      p->state = RUNNABLE;
#ifdef MLFQ
      priority_queue[p->current_queue][priority_queue_top[p->current_queue]++] = p->pid;
#endif
    }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
