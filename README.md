*This project has been created as part of the 42 curriculum by nograu.*

# Codexion

> Master the race for resources before the deadline masters you

## Description

Codexion is a concurrency simulation inspired by the classic Dining Philosophers problem. Multiple coders sit in a circular co-working hub and compete for a limited number of USB dongles in order to compile their quantum code.

Each coder follows a fixed cycle:
1. **Compile** — requires holding 2 dongles simultaneously for `time_to_compile` ms
2. **Debug** — releases both dongles, spends `time_to_debug` ms debugging
3. **Refactor** — spends `time_to_refactor` ms refactoring, then loops back

If a coder does not start compiling within `time_to_burnout` ms since their last compile (or since the simulation started), they **burn out** and the simulation stops.

The simulation also stops when all coders have compiled at least `number_of_compiles_required` times.

Key challenges addressed:
- Deadlock prevention (circular resource dependency)
- Starvation prevention (fair FIFO and EDF scheduling)
- Precise burnout detection (< 10 ms accuracy)
- Dongle cooldown enforcement
- Serialized logging

## Instructions

### Compilation

```bash
make
```

This produces the `codexion` binary. Requires `cc` with `-Wall -Wextra -Werror -pthread`.

### Usage

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Type | Description |
|----------|------|-------------|
| `number_of_coders` | int > 0 | Number of coders (= number of dongles) |
| `time_to_burnout` | int > 0 | Max ms a coder can go without starting to compile |
| `time_to_compile` | int > 0 | ms spent compiling (holding 2 dongles) |
| `time_to_debug` | int > 0 | ms spent debugging |
| `time_to_refactor` | int > 0 | ms spent refactoring |
| `number_of_compiles_required` | int > 0 | Target compile count per coder to end cleanly |
| `dongle_cooldown` | int >= 0 | ms a dongle is unavailable after being released |
| `scheduler` | string | `fifo` (arrival order) or `edf` (earliest deadline first) |

### Examples

```bash
# 5 coders, 800ms burnout, 200ms compile, 200ms debug, 200ms refactor
# stop after 3 compiles each, 0 cooldown, FIFO scheduling
./codexion 5 800 200 200 200 3 0 fifo

# 4 coders, 600ms burnout, 150ms compile, 100ms debug, 100ms refactor
# stop after 5 compiles each, 50ms cooldown, EDF scheduling
./codexion 4 600 150 100 100 5 50 edf
```

### Log format

```
timestamp_in_ms  X has taken a dongle
timestamp_in_ms  X is compiling
timestamp_in_ms  X is debugging
timestamp_in_ms  X is refactoring
timestamp_in_ms  X burned out
```

Timestamps are in milliseconds relative to the simulation start.

### Makefile rules

| Rule | Effect |
|------|--------|
| `make` / `make all` | Compile the project |
| `make clean` | Remove object files |
| `make fclean` | Remove object files and binary |
| `make re` | Full recompile |

## Blocking Cases Handled

### Deadlock prevention (Coffman's conditions)

The classic deadlock scenario occurs when every coder holds their left dongle and waits indefinitely for their right dongle. This project breaks the **circular wait** condition by enforcing a **global dongle acquisition order**: a coder always picks up the lower-indexed dongle first, regardless of which side it is physically on. This guarantees the resource graph is acyclic and deadlock cannot occur.

The four Coffman conditions and how they are addressed:
- **Mutual exclusion**: necessary — each dongle can only be held by one coder at a time.
- **Hold and wait**: broken by the global ordering — a coder never holds one dongle while blocking on another in a circular fashion.
- **No preemption**: retained (dongles are only released voluntarily), but deadlock is prevented by ordering.
- **Circular wait**: broken by the global index ordering.

### Starvation prevention

- **FIFO mode**: requests are served strictly in arrival order per dongle, so no coder waits forever.
- **EDF mode**: the coder with the earliest deadline (`last_compile_start + time_to_burnout`) is served first, which prioritises the most urgent coder and prevents starvation under feasible parameters.
- Tie-breaking in EDF uses coder ID to ensure a fully deterministic policy.

### Cooldown handling

Each dongle tracks its `release_time`. When a coder requests a dongle, the scheduler checks both that the dongle is free **and** that `current_time >= release_time + dongle_cooldown` before granting access. Waiters sleep on a condition variable and are re-evaluated when a release occurs.

### Precise burnout detection

A dedicated **monitor thread** runs independently and polls each coder's `last_compile` timestamp in a tight loop (sleeping ~1 ms between checks). When `current_time - last_compile > time_to_burnout`, the monitor immediately prints the burnout message and sets the global stop flag. This guarantees the log appears within 10 ms of the actual deadline.

### Log serialization

A global `print_mutex` is held for the entire duration of each `printf` call. This ensures that messages from different threads never interleave on the same output line.

## Thread Synchronization Mechanisms

### `pthread_mutex_t`

- **Per-dongle mutex** (`t_dongle_data.mutex`): protects the dongle's state (`in_use`, `release_time`, wait queue). Every read or write to dongle state is done under this lock.
- **Global print mutex**: serializes all output. Any thread that wants to print acquires this mutex, calls `printf`/`write`, then releases it immediately.
- **Monitor mutex**: protects the global `stop` flag and burnout state shared between the monitor thread and coder threads.

### `pthread_cond_t`

- **Per-dongle condition variable** (`t_dongle_data.available`): used by waiting coders. When a dongle is released, the releasing thread calls `pthread_cond_broadcast` to wake all waiters. Each woken thread re-checks the scheduler queue to see if it is next in line, and goes back to sleep if not.
- `pthread_cond_timedwait` is used instead of plain `pthread_cond_wait` so that the burnout check in the coder thread can time out and verify the stop flag without hanging indefinitely.

### Priority queue (min-heap)

A custom min-heap is implemented for each dongle's wait queue. In FIFO mode, the key is the request's arrival sequence number. In EDF mode, the key is `last_compile_start + time_to_burnout`. The heap provides O(log n) enqueue and dequeue, ensuring fair and efficient arbitration without using any standard library priority queue.

### Race condition prevention — examples

| Scenario | Prevention |
|----------|-----------|
| Two coders simultaneously read a dongle as "free" | Both read under the dongle mutex; only one will proceed, the other re-enters the wait queue |
| Monitor reads `last_compile` while a coder writes it | `last_compile` is updated under a per-coder lock or written atomically; monitor reads under the same lock |
| Coder checks stop flag after monitor sets it | Stop flag is read/written under the monitor mutex; coder checks it before each blocking operation |
| Log lines from two threads overlap | Global print mutex held for the entire write |

### Thread-safe communication between coders and the monitor

Coders never communicate directly with each other (as specified). Communication with the monitor is one-way via shared state:
- Coders write `last_compile` (timestamp of their last compile start).
- The monitor reads all `last_compile` values and writes the global `stop` flag.
- When the monitor sets `stop = 1`, it also calls `pthread_cond_broadcast` on all dongle condition variables to unblock any waiting coder threads so they can exit cleanly.

## Resources

### Concurrency and POSIX threads
- [POSIX Threads Programming — Blaise Barney (Lawrence Livermore)](https://hpc-tutorials.llnl.gov/posix/)
- [The Little Book of Semaphores — Allen B. Downey](https://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf)
- `man pthread_create`, `man pthread_mutex_init`, `man pthread_cond_wait`, `man pthread_cond_timedwait`
- `man gettimeofday`, `man usleep`

### Scheduling algorithms
- [Earliest Deadline First scheduling — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- [Priority queue / Binary heap — Wikipedia](https://en.wikipedia.org/wiki/Binary_heap)

### Dining Philosophers (foundational problem)
- [Dining philosophers problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- E. W. Dijkstra — original formulation (1971)

### AI usage

Claude Code (claude-sonnet-4-6) was used during this project for:
- **Structural design assistance**: discussing data structure layout (heap, monitor state, dongle struct fields) and identifying missing fields before implementation.
- **README drafting**: generating the initial structure of this README based on the subject requirements; all sections were reviewed and adjusted to reflect the actual implementation.
- **Plan/checklist generation**: producing an ordered task breakdown to track progress across the implementation.

All code was written, reviewed, and understood by the author. No function was copy-pasted without full comprehension of its logic.
