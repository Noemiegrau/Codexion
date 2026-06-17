*This project has been created as part of the 42 curriculum by nograu.*

# Codexion

## Description

Codexion simulates a concurrency problem rooted in the classic Dining Philosophers scenario. A group of coders share a circular workspace and fight over a scarce pool of USB dongles — the only way to run their quantum compiler.

The lifecycle of each coder is straightforward: grab two dongles and compile, then release them and debug, then refactor, then start over. The danger is time — if a coder goes `time_to_burnout` milliseconds without beginning a new compile, they burn out and bring the whole simulation down. A clean exit happens when every coder has hit the `number_of_compiles_required` target.

What makes this non-trivial: deadlock avoidance, fair access scheduling, cooldown windows on dongles, a monitor that catches burnout within 10 ms, and output that never garbles across threads.

## Instructions

### Compilation

```bash
make
```

Builds the `codexion` binary using `cc` with `-Wall -Wextra -Werror -pthread`.

### Usage

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All 8 arguments are required. The program rejects anything invalid — negative numbers, non-integers, or a scheduler value other than `fifo` or `edf`.

- `number_of_coders` — how many coders (and dongles) exist in the simulation
- `time_to_burnout` — milliseconds a coder can go without starting to compile before dying
- `time_to_compile` — milliseconds spent compiling (two dongles held the whole time)
- `time_to_debug` — milliseconds spent in the debug phase
- `time_to_refactor` — milliseconds spent refactoring before looping back to compile
- `number_of_compiles_required` — compile count per coder that triggers a clean stop
- `dongle_cooldown` — milliseconds a dongle stays locked after being released
- `scheduler` — arbitration policy: `fifo` (first come first served) or `edf` (most urgent first)

### Examples

```bash
./codexion 5 800 200 200 200 3 0 fifo
./codexion 4 600 150 100 100 5 50 edf
```

## Blocking Cases Handled

**Deadlock** — the trap here is every coder grabbing their left dongle and then waiting forever for their right one. The fix: coders always pick up the lower-indexed dongle first. This breaks the circular dependency at the root and makes deadlock structurally impossible regardless of timing.

**Starvation** — with `fifo`, every request is queued in arrival order so no coder gets skipped indefinitely. With `edf`, the coder whose deadline is closest gets priority — the one closest to burnout goes first. When two coders share the exact same deadline, the lower coder ID wins to keep the policy deterministic.

**Cooldown** — each dongle remembers when it was last released. A waiting coder only gets access once the dongle is both free and past its cooldown window (`release_time + dongle_cooldown <= current_time`). Threads waiting on a locked dongle sleep on a condition variable and get re-evaluated on every release event.

**Burnout detection** — a standalone monitor thread loops continuously, sleeping roughly 1 ms between checks. It compares each coder's `last_compile` timestamp against the current time. The moment the gap exceeds `time_to_burnout`, it fires the burnout log and raises the stop flag — all within the required 10 ms window.

**Log serialization** — every print goes through a global mutex. The lock is held for the full duration of the write, so lines from concurrent threads can never bleed into each other.

## Thread Synchronization Mechanisms

**`pthread_mutex_t`** appears in three roles: one per dongle (guards its state, release timestamp, and wait queue), one global print lock (keeps output clean), and one monitor lock (protects the shared stop flag).

**`pthread_cond_t`** — each dongle carries a condition variable. Threads that cannot acquire a dongle immediately go to sleep on it. When any dongle is released, `pthread_cond_broadcast` wakes all sleepers; each one re-evaluates its position in the queue and goes back to sleep if it is not yet its turn. `pthread_cond_timedwait` is used throughout so threads never block indefinitely — they wake up periodically to check whether the simulation has ended.

**Priority queue (min-heap)** — each dongle maintains its own custom min-heap for the wait queue. The sort key is arrival timestamp in `fifo` mode and `last_compile_start + time_to_burnout` in `edf` mode. The heap is implemented from scratch — no standard library container is used.

**Race condition prevention:**

| Scenario | Prevention |
|----------|------------|
| Two coders see a dongle as free at the same time | Dongle state is read and written under its mutex; only one thread proceeds |
| Monitor reads `last_compile` while a coder updates it | Access to `last_compile` is protected by a per-coder lock on both sides |
| A coder reads the stop flag right as the monitor sets it | The stop flag is always accessed under the monitor mutex |
| Two threads print at the same time | The global print mutex is held for the entire write operation |

**Coder/monitor communication** stays strictly one-way through shared memory: coders write their `last_compile` timestamp, the monitor reads it and writes `stop`. Once `stop` is set, the monitor broadcasts on every dongle condition variable so any thread sleeping on a dongle wakes up, sees the flag, and exits without hanging.

## Resources

- [POSIX Threads Programming — Blaise Barney (LLNL)](https://hpc-tutorials.llnl.gov/posix/)
- [The Little Book of Semaphores — Allen B. Downey](https://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf)
- [Dining philosophers problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Earliest Deadline First scheduling — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- [Binary heap — Wikipedia](https://en.wikipedia.org/wiki/Binary_heap)
- `man pthread_create`, `man pthread_mutex_init`, `man pthread_cond_wait`, `pthread_mutex_lock`, `pthread_mutex_unlock`, `usleep`, etc

### AI usage

Claude (claude.ai and Claude Code) was used throughout this project for: talking through data structure design and catching missing fields early, and building a task breakdown to track progress across the implementation. All code and documentation in the repository was written and fully understood by the author.