*This project has been created as part of the 42 curriculum by fyousefi.*

# Codexion

*Master the race for resources before the deadline masters you*

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Threads](https://img.shields.io/badge/Threads-POSIX-blueviolet?style=for-the-badge)
![Norminette](https://img.shields.io/badge/Norminette-passing-success?style=for-the-badge)
![Build](https://img.shields.io/badge/Build-passing-success?style=for-the-badge)

---

## Description

Codexion is a concurrency simulation inspired by the classic **Dining Philosophers** problem, set in a coding environment.

Several coders sit around a circular co-working hub. Each coder cycles through three states: **compiling**, **debugging**, and **refactoring**. To compile, a coder needs **two USB dongles at the same time** — one on their left side and one on their right side. Because every dongle is shared with a neighbour, coders have to compete for them fairly.

The main challenge is to keep every coder alive. A coder **burns out** if they go too long without compiling. When a coder burns out, the simulation stops.

Each coder runs as its own **POSIX thread**. A separate **monitor thread** watches for burnouts. Two scheduling policies are supported — **FIFO** and **EDF** — both backed by a custom min-heap priority queue.

**Goal: keep all coders compiling until everyone reaches the required number of compiles — without any burnouts.**

---

## Instructions

### Compilation

```bash
# Build the binary
make

# Remove object files
make clean

# Remove object files and the binary
make fclean

# Full recompile from scratch
make re
```

### How to run

```bash
./codexion number_of_coders time_to_burnout time_to_compile \
           time_to_debug time_to_refactor number_of_compiles_required \
           dongle_cooldown scheduler
```

All arguments are **mandatory**. Invalid inputs (negative numbers, non-integers, unknown scheduler name) are rejected.

### Arguments explained

| Argument | Type | Description |
|----------|------|-------------|
| `number_of_coders` | integer | How many coders (and dongles) there are |
| `time_to_burnout` | ms | How long a coder can go without compiling before they burn out |
| `time_to_compile` | ms | How long compiling takes (coder holds both dongles during this time) |
| `time_to_debug` | ms | How long debugging takes |
| `time_to_refactor` | ms | How long refactoring takes |
| `number_of_compiles_required` | integer | Stop the simulation once every coder has compiled this many times |
| `dongle_cooldown` | ms | How long a dongle is unavailable after being released |
| `scheduler` | `fifo` or `edf` | How to decide which coder gets a dongle when multiple are waiting |

`fifo` = First In, First Out — the coder who asked first gets the dongle first.  
`edf` = Earliest Deadline First — the coder closest to burning out gets priority.  
EDF deadline = `last_compile_start + time_to_burnout`.

### Usage examples

```bash
# 5 coders, FIFO scheduling, each must compile 10 times
./codexion 5 800 200 200 200 10 50 fifo

# Same setup but with EDF — the coder closest to burnout gets priority
./codexion 5 800 200 200 200 10 50 edf

# Single coder (only one dongle, no sharing needed)
./codexion 1 600 200 100 100 5 0 fifo

# 4 coders, tight burnout timer, no cooldown, FIFO
./codexion 4 410 200 200 200 10 0 fifo

# 4 coders, tight burnout timer, no cooldown, EDF
./codexion 4 410 200 200 200 10 0 edf

# 3 coders with longer times
./codexion 3 1000 300 200 200 5 100 fifo
```

### Output format

Each line in the output follows this format:

```
timestamp_in_ms  X  state_message
```

Where `timestamp_in_ms` is the time since the simulation started (in milliseconds) and `X` is the coder's number.

Possible state messages:
```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

### Example output

```
0 1 has taken a dongle
2 1 has taken a dongle
2 1 is compiling
202 1 is debugging
402 1 is refactoring
405 2 has taken a dongle
406 2 has taken a dongle
406 2 is compiling
606 2 is debugging
806 2 is refactoring
900 3 has taken a dongle
902 3 has taken a dongle
902 3 is compiling
1102 3 is debugging
1302 3 is refactoring
1505 4 burned out
```

Each "compiling" line is always preceded by two "has taken a dongle" lines. The "burned out" message appears within **10 ms** of the actual burnout time.

---

## Blocking cases handled

**Deadlock prevention** — Dongles are always locked in memory-address order. This breaks the circular-wait condition (Coffman condition #4), which is required for a deadlock to happen. No two coders can ever be stuck waiting for each other in a cycle.

**Starvation prevention** — The scheduler heap ensures that every waiting coder eventually gets their turn. With FIFO, requests are served strictly in arrival order. With EDF, the coder with the closest burnout deadline is served first. Neither policy allows a coder to be skipped indefinitely.

**Dongle cooldown** — After a dongle is released, an `available_after` timestamp is stored. Any coder that tries to take it before the cooldown has passed is blocked until it expires. This prevents a coder from immediately re-grabbing a dongle they just released.

**Precise burnout detection** — The monitor thread polls every 0.5 ms. This ensures that a burnout is detected and logged within the required **10 ms window** after the actual burnout time.

**Log serialization** — A dedicated log mutex (`cfg->log_mutex`) protects all writes to stdout. This guarantees that no two state messages ever appear mixed up on the same line.

---

## Thread synchronization mechanisms

### Mutexes

| Lock | What it protects |
|------|-----------------|
| `dongle.mutex` | `in_use` flag, `available_after` timestamp, and the scheduler heap |
| `cfg->state_mutex` | Compile count per coder, last compile time, and the `running` flag |
| `cfg->log_mutex` | All writes to stdout |

### Condition variables

Each dongle has its own `pthread_cond_t`. When a coder cannot take a dongle yet (either because it is in use or still in cooldown), it calls `pthread_cond_wait` and sleeps. When a dongle is released, `pthread_cond_broadcast` wakes up all waiting coders. The scheduler then picks the one with the highest priority (based on FIFO arrival order or EDF deadline).

This approach avoids busy-waiting — coders sleep instead of spinning in a loop.

### Scheduler heap (custom min-heap)

```c
// Add a new request — sifts up to maintain heap order
scheduler_push(dongle, coder);

// Remove and return the highest-priority coder — sifts down after removal
scheduler_pop(dongle, cfg, &coder_id);

// Compare two requests: by arrival order (FIFO) or burnout deadline (EDF)
req_better(cfg, req_a, req_b);
```

The heap is protected by the dongle's mutex. This means push and pop are always atomic with respect to other threads accessing the same dongle.

### How coders and the monitor communicate

The monitor thread reads each coder's `last_compile_time` and `compile_count` under `cfg->state_mutex`. Coder threads update these values under the same mutex before logging state changes. The `running` flag (also protected by `cfg->state_mutex`) is how the monitor signals all threads to stop — either after a burnout or after the compile target is reached.

---

## Resources

- [POSIX Threads Programming — LLNL](https://hpc-tutorials.llnl.gov/posix/)
- [The Little Book of Semaphores — Allen B. Downey](https://greenteapress.com/wp/semaphores/)
- [Dining Philosophers Problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Earliest Deadline First Scheduling — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- [pthread man pages — linux.die.net](https://linux.die.net/man/7/pthreads)

### AI usage

AI (Claude) was used as a **learning and review tool** during this project:

- Generating questions to test my understanding of mutex ordering and condition variable semantics *before* writing code
- Explaining the conceptual difference between FIFO and EDF scheduling
- Reviewing draft logic in `lock_pair` and `scheduler_push` for correctness — all suggestions were verified manually and with peers
- Recommending the min-heap as the right data structure for the scheduler (implementation was written from scratch and independently reviewed)

All AI-generated suggestions were checked, questioned, and tested before being used.

---

*Made with ☕ and 💻 by Fatemeh Yousefi*
