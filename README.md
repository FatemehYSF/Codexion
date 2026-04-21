<div align="center">

# ⚡ Codexion
### *Master the Race for Resources Before the Deadline Masters You*

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Threads](https://img.shields.io/badge/Threads-POSIX-blueviolet?style=for-the-badge)
![Norminette](https://img.shields.io/badge/Norminette-passing-success?style=for-the-badge)
![Build](https://img.shields.io/badge/Build-passing-success?style=for-the-badge)

*A concurrency simulation where coders race for USB dongles using threads, mutexes, and smart scheduling*

---

</div>

## 🎯 About The Project

**Codexion** is a concurrency simulation inspired by the classic **Dining Philosophers** problem — reimagined in a coding environment.

Multiple coders sit around a circular co-working hub, each cycling through three states: **compiling**, **debugging**, and **refactoring**. To compile, a coder needs **two USB dongles** simultaneously — one on their left, one on their right. Since every dongle is shared with a neighbour, coders must compete fairly without causing deadlocks, starvation, or burnout.

Each coder runs as an independent **POSIX thread**, a dedicated **monitor thread** watches for burnout, and two scheduling policies — **FIFO** and **EDF** — are backed by a custom min-heap priority queue.

> ⚠️ A coder **burns out** if they go too long without compiling. The goal: keep everyone alive.

---

## ✨ Features

<table>
<tr>
<td width="50%">

### 🧵 Threading Model
- One `pthread` per coder
- Dedicated monitor thread
- All threads start in sync
- Clean join & resource cleanup

### ⏱️ Scheduling Policies
- **FIFO** — first come, first served
- **EDF** — earliest burnout deadline first
- Custom **min-heap** priority queue
- Guaranteed liveness under EDF

</td>
<td width="50%">

### 🔒 Synchronization
- Per-dongle `pthread_mutex_t`
- Per-dongle `pthread_cond_t`
- Global state mutex
- Log serialization mutex

### ⚙️ Simulation Controls
- Configurable burnout timer
- Configurable dongle cooldown
- Stop on burnout or compile target
- Burnout logged within **10 ms**

</td>
</tr>
</table>

---

## 🚀 Getting Started

### Compilation

```bash
# Build the binary
make

# Remove object files
make clean

# Remove object files and binary
make fclean

# Full recompile from scratch
make re
```

### Usage

```bash
./codexion number_of_coders time_to_burnout time_to_compile \
           time_to_debug time_to_refactor number_of_compiles_required \
           dongle_cooldown scheduler
```

### Examples

```bash
# 5 coders, FIFO scheduling, each must compile 10 times
./codexion 5 800 200 200 200 10 50 fifo

# Same setup but EDF — the coder closest to burnout gets priority
./codexion 5 800 200 200 200 10 50 edf

# Single coder (only one dongle, no sharing)
./codexion 1 600 200 100 100 5 0 fifo
```

### Expected Output

```
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
405 2 has taken a dongle
406 2 has taken a dongle
406 2 is compiling
606 2 is debugging
806 2 is refactoring
1505 4 burned out
```

Each line follows the format: `timestamp_in_ms  coder_id  state_message`

---

## 🛠️ Technical Specifications

| Aspect | Detail |
|--------|--------|
| **Language** | C |
| **Compiler** | cc (gcc/clang) |
| **Compilation Flags** | `-Wall -Wextra -Werror -pthread` |
| **Norm** | 42 Norminette v3 |
| **Threading** | POSIX Threads (`pthread`) |
| **Scheduling** | FIFO / EDF via custom min-heap |
| **Memory** | No leaks — full alloc/free cycle |
| **Source Files** | 11 `.c` files + 1 header |

---

## 📂 Project Structure

```
coders/
├── 📄 Makefile
├── 📄 codexion.h        — all structs, typedefs, and prototypes
├── 📄 main.c            — entry point, thread creation & join
├── 📄 args.c            — argument parsing & validation
├── 📄 init.c            — coder & dongle initialization
├── 📄 coder.c           — coder thread routine (compile → debug → refactor)
├── 📄 dongle.c          — dongle acquire/release with cooldown
├── 📄 scheduler.c       — min-heap push/pop logic
├── 📄 scheduler_cmp.c   — FIFO vs EDF comparison function
├── 📄 monitor.c         — burnout detection & simulation stop
├── 📄 log.c             — serialized logging
├── 📄 time.c            — gettimeofday wrapper (now_ms)
└── 📄 README.md
```

---

## 🔐 Blocking Cases Handled

- ✅ **Deadlock prevention** — dongles are always locked in memory-address order, breaking circular-wait (Coffman condition #4)
- ✅ **Starvation prevention** — the scheduler heap ensures every waiting coder gets their turn; no thread can jump the queue
- ✅ **Dongle cooldown** — `available_after` timestamp is checked before any coder claims a just-released dongle
- ✅ **Precise burnout detection** — monitor polls every 0.5 ms and logs burnout within the mandatory 10 ms window
- ✅ **Log serialization** — a dedicated log mutex ensures no two state messages ever interleave on the same line

---

## 🧠 Thread Synchronization Mechanisms

<table>
<tr>
<td width="50%">

### Mutexes
| Lock | Protects |
|------|----------|
| `dongle.mutex` | `in_use`, `available_after`, heap |
| `cfg->state_mutex` | compile count, last compile time, running flag |
| `cfg->log_mutex` | all stdout writes |

</td>
<td width="50%">

### Condition Variables
Each dongle has a `pthread_cond_t`.  
When a coder can't take a dongle yet, it waits on the condition.  
After a release, `pthread_cond_broadcast` wakes all waiters and lets the scheduler pick the right one.

</td>
</tr>
</table>

### Scheduler Heap (Custom Min-Heap)

```c
// Push a new request — sift up to maintain heap order
scheduler_push(dongle, coder);

// Pop the highest-priority coder — sift down after removal
scheduler_pop(dongle, cfg, &coder_id);

// Compare two requests: by arrival sequence (FIFO) or deadline (EDF)
req_better(cfg, req_a, req_b);
```

---

## 💡 Key Learnings

- ✅ POSIX thread lifecycle — create, synchronize, join, destroy
- ✅ Deadlock prevention through consistent lock ordering
- ✅ Condition variables for efficient waiting (vs. busy-waiting)
- ✅ Real-time scheduling algorithms: FIFO and EDF
- ✅ Custom data structures (min-heap) in plain C
- ✅ Precise timing with `gettimeofday` and microsecond sleeps

---

## 📖 Resources

- [POSIX Threads Programming — LLNL](https://hpc-tutorials.llnl.gov/posix/)
- [The Little Book of Semaphores — Allen B. Downey](https://greenteapress.com/wp/semaphores/)
- [Dining Philosophers Problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Earliest Deadline First Scheduling — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- [pthread man pages — linux.die.net](https://linux.die.net/man/7/pthreads)

### 🤖 AI Usage
AI (Claude) was used as a **learning and review tool**:
- Generating questions to test understanding of mutex ordering and condition variable semantics *before* writing code
- Explaining the conceptual difference between FIFO and EDF scheduling
- Reviewing draft logic in `lock_pair` and `scheduler_push` for correctness — all suggestions were verified manually and with peers
- Recommending the min-heap as the right structure for the scheduler (implementation written from scratch and independently reviewed)

---

<div align="center">

**Made with ☕ and 💻 by Fatemeh Yousefi**

*This project has been created as part of the 42 curriculum by fyousefi.*

</div>
