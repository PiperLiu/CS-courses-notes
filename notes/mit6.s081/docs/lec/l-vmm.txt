6.S081 2020 Lecture 19: Virtual Machines, Dune

Read: Dune: Safe User-level Access to Privileged CPU features, Belay et al,
OSDI 2012.

Plan:
  virtual machines
  trap-and-emulate virtualization
  hardware-supported virtualization (Intel VT-x)
  Dune

*** Virtual Machines

what's a virtual machine?
  simulation of a computer, accurate enough to run an O/S

diagram: h/w, host/VMM, guest kernels, guest processes
  VMM might be stand-alone, or
  VMM might run in a host O/S, e.g. Linux

why use a VM?
  run lots of guests per physical machine
    often individual services requires modest resources
    would waste most of a dedicated server
    for cloud and enterprise computing
    need a new layer, under existing O/S kernel, to manage multiple guests
  VMs offer better isolation than processes
  one computer, multiple operating systems (OSX and Windows)
  kernel development environment (like qemu)
  tricks: checkpoint, migrate, monitor

VMs have a long history
  1960s: IBM used VMs to share big machines
  1990s: VMWare re-popularized VMs, for x86 hardware
  2000s: widely used in cloud, enterprise

why are we talking about virtual machines?
  some of the most interesting action in O/S design has shifted to VMs
    much overlap with traditional O/S kernels
    some new challenges/opportunities as well
  some problems are solved more easily by inserting a new layer under the O/S
  VMs have affected both O/S (above) and hardware (below)

how accurate must a VM be?
  usual goal:
    guest doesn't have any idea it's running in a VM
    host can support any guest O/S
  so we want:
    impossible for guest to distinguish VM from real computer
    impossible for guest to escape its VM
  must allow standard O/S to boot, run
  must contain malicious software
    cannot let guest break out of virtual machine!
  in practice, many VMs compromise
    guest O/S knows it's running on a VM,
    has modifications for efficiency

we could build a VM by writing software to simulate machine instructions
  VMM interprets each guest instruction
  maintain virtual machine state for the guest
    32 registers, satp, mode, RAM, disk, &c
  pro: it's clear this is possible
  con: slow

idea: execute guest instructions on real CPU -- fast!
  what if the guest kernel executes a privileged instruction?
    e.g. guest loads a new page table into satp
    can't give guest kernel direct access to supervisor registers &c!

idea: run the guest kernel in user mode
  similar to running the guest kernel as an xv6 process
  of course the guest kernel assumes it is in supervisior mode
  ordinary instructions work fine
    adding two registers, function call, &c
  privileged RISC-V instructions are illegal in user mode
    will cause a trap, to the VMM
  VMM trap handler emulates privileged instruction
    maybe apply the privileged operation to the virtual state
      e.g. read/write sepc
    maybe transform and apply to real hardware
      e.g. assignment to satp
  "trap-and-emulate"
  nice b/c you can build such a virtual machine entirely in software
    you could modify xv6 to be a trap-and-emulate VMM for RISC-V

what RISC-V state must a trap-and-emulate VMM "virtualize"?
  b/c guest can't be allowed to see/change the real machine state.
  all s* registers (sepc, stvec, scause, satp, &c)
  mode
  hart number

the RISC-V is very nice w.r.t. trap-and-emulate virtualization
  all privileged instructions trap if you try to execute them in user mode
  not all CPUS are as nice -- 32-bit x86, for example
    some privileged instructions don't trap; x86 ignores if run in user mode

for RISC-V trap-and-emulate, what has to happen when:

... the guest kernel executes sret (return to user)?
    [diagram: VMM, guest kernel, guest user, virtual state, real h/w state]
    CPU traps into the VMM
      it's really a trap from user mode to supervisor mode
      guest's PC saved in epc (and perhaps trapframe)
    VMM trap handler:
      virtual mode = user
      real sepc = virtual sepc
      [switch page tables for PTE_U]
      return from trap

... guest user code executes ecall to make a system call?
    CPU traps into the VMM (ecall always generates a trap)
    VMM trap handler:
      virtual sepc = real sepc
      virtual mode = supervisor
      virtual scause = "system call"
      real sepc = virtual stvec
      [switch page tables for PTE_U]
      return from trap

... the guest kernel reads scause, e.g. csrr a0, scause
    trap into VMM
    trapframe a0 = virtual scause
    real sepc += 4
    return from trap

... the guest kernel writes satp?
  VMM must ensure that guest only accesses its own memory
    and must remap guest physical addresses
  guest's page table:
    guest va -> guest pa
  vmm map for this guest
    guest pa -> host pa
  VMM's "shadow" page table
    guest va -> host pa
  VMM installs the shadow page table in the real satp

... the guest kernel modifies a PTE in the active page table?
  VMM doesn't have to do anything
  RISC-V spec says PTE modifications don't take effect until sfence.vma
  sfence.vma causes trap to VMM
    VMM generates a new shadow page table

how to simulate devices?
  e.g. disk, NIC, display
  a big challenge!
  strategy: emulate a common existing real device
    needed in order to run oblivious guest O/S
    intercept memory-mapped control register read/write
      by marking those pages invalid, so VMM gets page faults
    VMM turns page faults into operations on (different) real device
    e.g. qemu simulates uart/console for xv6
      qemu turns uart r/w into characters to your display or ssh
  strategy: special virtual device tailored for efficiency
    requires guest O/S driver -- i.e. guest knows it's in a VM
    can be more streamlined than trapping on control register r/w
    e.g. xv6's virtio_disk.c; qemu turns into r/w on file fs.img
  strategy: pass-through access to a real hardware device
    guest O/S gets direct access to device h/w, no traps
    often requires specific support in device
      modern NICs have the suppor t
    can be very efficient

trap-and-emulate works well -- but it can be slow!
  lots of traps into the VMM

*** Hardware-supported x86 virtualization

VT-x/VMX/SVM: hardware supported virtualization
  Intel (and AMD) CPUs have hardware virtualization support
    CPU directly supports virtual vs real state
    so guest can execute privileged instructions without trapping
  faster than trap-and-emulate
  easier to implement the software for a VMM
  widely used to implement virtual machines

VT-x root and non-root mode
  [diagram: two sets of registers, plus EPT]
  Host (VMM) runs in "root mode" -- nothing special
  Guest runs in "non-root mode"
    has full set of "virtual" control registers
      x86 equivalents of satp, stvec, user vs supervisor etc
    guest supervisor mode can do everything an O/S kernel expects
      switch to (guest) user mode, take system calls, traps, &c
    non-root mode *looks* just like an ordinary computer
    but VT-x non-root mode limits guest in some critical ways
      mostly via page table, as we'll see in a moment
  New instructions to change between root/non-root mode
    VMLAUNCH/VMRESUME: root -> non-root
    VMCALL: non-root -> root
  Device interrupts (e.g. timer, disk) cause exit from non-root mode
    VMM handles devices by default
  VM control structure (VMCS)
    Virtual state saved in VMCS when non-root mode exits
    Virtual state restored from VMCS in VMLAUNCH/VMRESUME

VT-x page tables
  problem:
    we want to let the guest kernel control the page table,
    and we want to restrict the guest to just its allowed physical memory,
    but we don't want the expense of trapping into the VMM when
      the guest sets %cr3 (== satp) or modifies a PTE.
  EPT: VT-x has *two* layers of address translation in non-root mode
    the VMM sets up the EPT page table
    contains just the "guest physical addresses" the guest can use
    maps to the real (host) physical addresses the VMM has allocated for the guest
    EPT = Extended Page Table
  MMU hardware consults %cr3 to get a guest pa,
    then uses EPT to convert guest pa to host pa
  EPT is not visible to the guest
  so:
    guest can freely read/write %cr3, change PTEs, read D bits, &c
    VMM can still provide isolation via EPT
  typical setup:
    VMM allocates some RAM for guest to use
    VMM maps guest physical addresses 0..size to RAM, in the EPT
    guest uses %cr3 to configure guest process address spaces

*** Dune

the big idea:
  use VT-x to run Linux user processes (rather than to run a guest kernel)
  then user code has fast direct access to page tables, exceptions, &c
  to allow user code to efficiently:
    sandbox untrusted code
    modify page table and take page faults

the scheme
  [diagram]
  Dune is a "loadable kernel module" for Linux
  an ordinary process can switch into "Dune mode"
  a Dune-mode process is still a process
    has memory, can make Linux system calls (via VMCALL)
  the isolation machinery is a little different
    VT-x non-root supervisor mode
    memory protection via EPT page table
    timer interrupts go to Linux, not process, so Linux controls scheduling
  Dune gives a process additional functionality
    read and write its own page table, including PTE D (dirty) bit
      faster than Linux mprotect() system call
    handle its own page faults
      faster than having Linux turn faults into upcalls
    switch into (guest) user mode
    intercept guest user system calls and page faults

Example: sandboxed execution (paper section 5.1)
  suppose your web browser wants to run a 3rd-party plug-in
    e.g. a video decoder or ad blocker
    the plug-in might be malicious or buggy
  browser needs a "sandbox"
    execute the plug-in, but limit syscalls / memory accesses
  assume browser runs as a Dune process:
    [diagram: browser in guest supervisor mode, plug-in guest user mode]
    create page table with PTE_U mappings for memory plug-in can use
      and non-PTE_U mappings for rest of browser's memory
    set %cr3
    sret into untrusted code, in guest user mode
    plug-in can read/write image memory via page table
    plug-in can execute system call instruction
      but its system calls trap into the browser (not the underlying kernel)
      and the browser can decide whether to allow each one

Example: garbage collection (GC)
  (modified Boehm concurrent mark-and-sweep collector)
  GC follows pointers to find all live (reachable) objects
    starting at registers
  But this GC is concurrent -- so program may modify an object after GC has traced it
  GC needs a way to know which objects were modified,
    so it can re-visit modified objects
  How does Dune help?
    Use PTE dirty bit (PTE_D) to detect written pages
    Clear all dirty bits when GC is done
    Dune allows direct access to PTEs
      much faster than making Linux system calls to get at PTEs

Fast user-level access to VM could help many programs
  Appel and Li paper

How might Dune hurt performance?
  Table 2
    sys call overhead higher due to VT-x entry/exit
    faults to kernel slower, for same reason
    TLB misses slower b/c of EPT
  But they claim most apps aren't much affected
    b/c they don't spend much time in short syscalls &c
    Figure 3 shows Dune within 5% for most apss in SPEC2000 benchmark
      exceptions take lots of TLB misses

Of course it's not enough to merely not slow down apps much.

How much can clever use of Dune speed up real apps?
  Table 6 -- GC
  compare "Dune dirty" line to "Normal" line
  overall benefit depends on how fast the program allocates
  huge win on three allocation-intensive micro-benchmarks
  not a win for applications that don't allocate much -- XML parser
    EPT overhead does slow it down
    but many real apps allocate more than this

Dune summary
  The key idea: use VT-x to give processes access to privileged hardware features
  The bottom line: much faster in some situations than Linux system calls
