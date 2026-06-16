*This project has been created as part of the 42 curriculum by nograu.*

# Codexion

## Description

Codexion is a concurrency simulation inspired by the classic Dining Philosophers problem. Multiple coders sit in a circular co-working hub and compete for a limited number of USB dongles to compile their quantum code.

Each coder follows a fixed cycle: compile (holding 2 dongles for `time_to_compile` ms) → debug → refactor → repeat. A coder who does not start compiling within `time_to_burnout` ms since their last compile burns out and the simulation stops. It also stops cleanly when all coders have compiled at least `number_of_compiles_required` times.

Concurrency challenges addressed: deadlock prevention, starvation prevention (FIFO and EDF scheduling), precise burnout detection (< 10 ms), dongle cooldown enforcement, and serialized logging.

## Instructions

### Compilation

```bash
make
```

Produces the `codexion` binary. Requires `cc` with `-Wall -Wextra -Werror -pthread`.

### Usage

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All 8 arguments are mandatory. `scheduler` must be exactly `fifo` or `edf`. All numeric arguments must be positive integers (`dongle_cooldown` may be 0). Invalid inputs are rejected.

### Examples

```bash
./codexion 5 800 200 200 200 3 0 fifo
./codexion 4 600 150 100 100 5 50 edf
```

### Log format

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

Timestamps are in milliseconds relative to the simulation start.

## Blocking Cases Handled

**Deadlock prevention** — a coder always acquires the lower-indexed dongle first, regardless of physical position. This breaks circular wait (one of Coffman's four conditions) and guarantees the resource graph stays acyclic.

**Starvation prevention** — in FIFO mode, requests are served in strict arrival order. In EDF mode, the coder with the earliest deadline (`last_compile_start + time_to_burnout`) is served first. EDF ties are broken by coder ID for deterministic behaviour.

**Cooldown handling** — each dongle tracks its `release_time`. A dongle is only granted when it is free and `current_time >= release_time + dongle_cooldown`. Waiters sleep on a condition variable and are re-evaluated on each release.

**Precise burnout detection** — a dedicated monitor thread polls each coder's `last_compile` timestamp in a tight loop (~1 ms sleep). When `current_time - last_compile > time_to_burnout`, it prints the burnout message and sets the stop flag immediately, guaranteeing the log appears within 10 ms.

**Log serialization** — a global `print_mutex` is held for the entire duration of each write, preventing any two messages from interleaving on the same output line.

## Thread Synchronization Mechanisms

**`pthread_mutex_t`** is used in three places: a per-dongle mutex protects dongle state (`in_use`, `release_time`, wait queue); a global print mutex serializes all output; a monitor mutex protects the shared `stop` flag.

**`pthread_cond_t`** — each dongle has a condition variable on which waiting coders sleep. On release, `pthread_cond_broadcast` wakes all waiters; each re-checks the scheduler queue and goes back to sleep if not next. `pthread_cond_timedwait` is used (instead of `pthread_cond_wait`) so coder threads can time out and check the stop flag without hanging.

**Priority queue (min-heap)** — a custom min-heap manages each dongle's wait queue. The key is arrival sequence number in FIFO mode, and `last_compile_start + time_to_burnout` in EDF mode. No standard library priority queue is used.

**Race condition examples and prevention:**

| Scenario | Prevention |
|----------|------------|
| Two coders read a dongle as "free" simultaneously | Both read under the dongle mutex; only one proceeds |
| Monitor reads `last_compile` while a coder writes it | Both access under a per-coder lock |
| Coder checks stop flag after monitor sets it | Stop flag read/written under the monitor mutex |
| Log lines from two threads overlap | Global print mutex held for the entire write |

**Coder/monitor communication** is one-way via shared state: coders write `last_compile`; the monitor reads it and writes `stop`. When `stop` is set, the monitor calls `pthread_cond_broadcast` on all dongle condition variables to unblock waiting threads so they exit cleanly.

## Resources

- [POSIX Threads Programming — Blaise Barney (LLNL)](https://hpc-tutorials.llnl.gov/posix/)
- [The Little Book of Semaphores — Allen B. Downey](https://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf)
- [Dining philosophers problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Earliest Deadline First scheduling — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- [Binary heap — Wikipedia](https://en.wikipedia.org/wiki/Binary_heap)
- `man pthread_create`, `man pthread_mutex_init`, `man pthread_cond_wait`, `man pthread_cond_timedwait`, `man gettimeofday`

### AI usage

Claude (claude.ai and Claude Code) was used for: discussing data structure design and identifying missing fields, generating an ordered task breakdown, and drafting the initial structure of this README. All code was written, reviewed, and understood by the author.