*This project has been created as part of the 42 curriculum by ayoufkir*

## Description

**Codexion** is a concurrency simulation written in C, modeled on the classic dining philosophers problem. A configurable number of "coders" sit in a circular co-working hub, each with a shared USB dongle to their left and to their right (one dongle per pair of neighbors, so `N` coders means `N` dongles total).

To `compile`, a coder must hold both of their neighboring dongles at the same time. After compiling, they release both dongles and move on to `debugging` and `refactoring` (phases that don't require shared resources) before looping back to try to compile again.

Every coder has a personal countdown: if too much time passes since the start of their last compile without starting a new one, they `burn out`, and the simulation stops. The program also supports two dongle-arbitration policies:

- **FIFO**: first come, first served.
- **EDF**: Earliest Deadline First, prioritizing whichever coder is closest to burning out.

along with a mandatory cooldown period before a released dongle can be taken again.

## Project structure
    .
    ├── Makefile
    ├── README.md
    ├── includes
    │   └── codexion.h
    └── srcs
        ├── dongles.c
        ├── heap.c
        ├── init.c
        ├── main.c
        ├── monitor.c
        ├── parse.c
        ├── routine.c
        └── utils.c

## Instructions

### Compilation
    make

This builds the `codexion` binary at the project root using cc with **-Wall -Wextra -Werror -pthread**, following the required Makefile rules:

- `make` / `make all`: builds the project
- `make clean`: removes object files
- `make fclean`: removes object files and the binary
- `make re` : rebuilds from scratch

### Running
    ./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler

### Arguments

| Argument | Description |
|------|--------|
| number_of_coders | Number of coders (and dongles) in the simulation |
| time_to_burnout(ms) | Max time since the start of a coder's last compile before they burn out |
| time_to_compile (ms) | Duration of the compile phase |
| time_to_debug (ms) | Duration of the debug phase |
| time_to_refactor (ms) | Duration of the refactor phase |
| number_of_compiles_required | Simulation stops successfully once every coder reaches this many compiles |
| dongle_cooldown (ms) | Time a dongle must sit idle after release before it can be taken again |
| scheduler | Dongle arbitration policy: fifo or edf |

All arguments are mandatory. Invalid input (negative numbers, non-integers, or an unrecognized scheduler) is rejected.

Example usage:

    ./codexion 5 500 200 200 200 10 0 fifo

Example output:

    0 1 has taken a dongle
    0 1 has taken a dongle
    0 1 is compiling
    0 3 has taken a dongle
    0 3 has taken a dongle
    0 3 is compiling
    204 1 is debugging
    204 3 is debugging
    204 2 has taken a dongle
    204 2 has taken a dongle
    204 2 is compiling
    204 4 has taken a dongle
    204 4 has taken a dongle
    204 4 is compiling
    404 1 is refactoring
    404 4 is debugging
    404 3 is refactoring
    404 5 has taken a dongle
    404 5 has taken a dongle
    404 5 is compiling
    404 2 is debugging
    500 1 burned out

## Blocking cases handled

**Deadlock prevention (Coffman's conditions):** a coder never partially acquires its dongles. Checking eligibility (both dongles free and past cooldown), checking priority against rivals, and committing the acquisition all happen as a single atomic step while holding one shared lock. A coder either wins both dongles in that step or wins neither, it never holds one dongle while blocked waiting for the other, which breaks the hold-and-wait condition and makes circular-wait deadlock structurally impossible.

**Starvation prevention:** dongle access is arbitrated by priority rather than by raw scheduling luck. Under **fifo**, priority is the order in which a coder joined the waiting queue(`request_order`). Under **edf**, priority is the coder's current burnout deadline (`deadline` which is the current time + `time_to_burnout`), with arrival order used as a deterministic tie-breaker when two deadlines coincide, ensuring the coder closest to burning out is never passed over in favor of one with more slack, given feasible parameters.

**Cooldown handling:** each dongle tracks a `free_at` timestamp, set on release to `now + dongle_cooldown`. A dongle is only eligible to be taken again once real elapsed time has passed that timestamp, enforced on every acquisition attempt, not just checked once.

**Precise burnout detection:** a dedicated monitor thread continuously compares each coder's deadline against the current elapsed time and logs burned out as soon as a deadline is missed, independently of what any coder thread is doing. Polling at a short, fixed interval keeps detection comfortably inside the required 10 ms reporting window.

**Log serialization**: every log line is printed while holding a shared mutex, so two state-change messages can never interleave mid-line.

## Thread synchronization mechanisms

The design uses two layers of locking, plus one condition variable:

- **`queue.mutex` (shared, "outer" lock)**: protects the priority queue (a hand-rolled binary heap), the global request counter, each coder's `deadline`/`done` flags, and the simulation's `stop` flag. This is the lock that makes dongle acquisition atomic: a coder holds it for the entire "check eligibility → check priority → commit" sequence, so no other thread can observe or act on an inconsistent in-between state.

- **`queue.cond` (condition variable):** coders that fail to acquire their dongles wait on this condition variable instead of busy-spinning. Because cooldown expiry is a time-based condition (not something another thread necessarily signals), coders use `pthread_cond_timedwait` with a short timeout rather than a plain `pthread_cond_wait`, so they periodically wake up on their own to re-check eligibility even if no one broadcasts.

- **Per-dongle `pthread_mutex_t` (nested, "inner" lock):** each dongle has its own mutex protecting its `available` and `free_at` fields specifically. It is always acquired after `queue.mutex` is already held, never before. A strict, consistent lock ordering that guarantees this nested locking can never itself introduce a deadlock.

- **`print_mutex` (independent lock):** guards all console output and the
`print_stopped` flag. Every log line (state transitions from coder threads
and the burnout announcement from the monitor thread) is printed only
while holding this lock, which both serializes output (so two messages can
never interleave mid-line) and guarantees the `burned out` line is always
the last line printed: once `print_stopped` is set, any log call still in
flight checks the flag under the same lock and silently skips printing.

**Preventing race conditions:** every read or write of shared state (dongle fields, heap contents, deadlines, the stop flag) happens under the appropriate lock, with no exceptions, including loop conditions that read shared state on every iteration, not just the statements inside the loop body. This was verified using ThreadSanitizer (-fsanitize=thread) during development, which caught and helped resolve several unlocked-access races before the final version.

**Thread-safe coder/monitor communication:** the monitor thread and coder threads never communicate directly, they coordinate entirely through shared state protected by `queue.mutex`. The monitor reads each coder's `deadline` under the lock; coders write their new deadline under the same lock immediately after winning dongle acquisition, before ever releasing it. This guarantees the monitor never observes a stale or partially-updated deadline.

## Resources

### Educational resources

- [Understanding heap sort and priority queues](https://youtu.be/HqPJF2L5h9U?si=ZhA5ux2NOtRcFg0g)

- [Process vs Thread](https://youtu.be/4rLW7zg21gI?si=PVIJnQyhMFfZRvyE)
### Tools

- [Excalidraw](https://excalidraw.com/)

### Use of AI

AI was used in this project to:

- Identify where possible leaks could happen as well thread races
- Understand the key concepts of the projects such as threads and  

All algorithms, implementation decisions, program architecture, and final code were designed, implemented, tested, and validated by me