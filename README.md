# lemipc

A multi-process battle on a shared 5×5 board: each process is a player on one of up to 4 teams, fighting until only one team remains. State is shared via System V IPC — shared memory, semaphores, and a message queue.

**Specialization:** Unix/Kernel
**XP:** 9,450 · **People:** 1 · **Status:** 🟦 Completed
---

## 🚀 Features

- Multi-process game over System V IPC (shm + sem + msg)
- Up to 4 teams, up to 8 players per team (configurable in [inc/lemipc.h](inc/lemipc.h))
- Spectator mode that renders the board to the terminal
- BFS-based pathfinding toward the nearest enemy
- Team coordination: targets voted via the message queue
- Bootstrap / teardown: first player creates IPCs, last alive cleans them up

## 🛠️ Build & Run

**Prerequisites:** GCC, GNU Make, Linux (System V IPC support)

```bash
make                    # Build ./lemipc

./lemipc <team>         # team ∈ 1..4 — join as a player
./lemipc 0              # spectator: render the board

# A small match: 1 spectator + 4 players across 2 teams
./lemipc 0 &
./lemipc 1 & ./lemipc 1 & ./lemipc 2 & ./lemipc 2 &
wait
```

## 🧪 Testing

```bash
make test       # Preset battle: ~32 players across 4 teams
make vtest      # Smaller battle under valgrind (leak check)
make htest      # Smaller battle under helgrind (race / lock check)
```

## 📁 Project Structure

```
lemipc/
├── src/
│   ├── lemipc.c        # Entry point, top-level orchestration
│   ├── ipc.c           # shm/sem/msg setup and teardown
│   ├── player.c        # Registration, alive-count sync
│   ├── move.c          # Movement step / collision resolution
│   ├── bfs.c           # BFS pathfinding to nearest enemy
│   ├── message.c       # Team voting on targets via msg queue
│   ├── display.c       # Spectator rendering
│   └── draw.c          # Frame buffer / output
├── inc/lemipc.h        # Types + tunables (MAP_SIZE, MAX_PLAYERS, MAX_TEAMS)
├── libft/              # 42 libft
└── Makefile
```

## 🔧 Implementation

**IPC layout:**

| Mechanism      | Purpose                                                  |
|----------------|----------------------------------------------------------|
| Shared memory  | Game board + per-player state (team, position, alive)    |
| Semaphores     | Mutual exclusion on the board, alive-count synchronization |
| Message queue  | Team-internal voting on the next target                  |

**Tunables (in `inc/lemipc.h`):**
- `MAP_SIZE` = 5 (5×5 board)
- `MAX_PLAYERS` = 8 per team
- `MAX_TEAMS` = 4

**Lifecycle:**
1. First process to start creates the SysV segments and seeds shared state
2. Each player registers a slot, then loops: lock board → look at neighbors → vote/move → unlock
3. Players die on collision with an enemy; the last process alive removes the IPC segments

## 📚 Learning Objectives

- System V IPC: `shmget`/`shmat`, `semget`/`semop`, `msgget`/`msgsnd`/`msgrcv`
- Multi-process synchronization without threads
- BFS on a small grid; coordinating moves under contention
- Teardown discipline — leaking IPC segments is a real failure mode
- Helgrind/valgrind on multi-process programs

## 🔗 References

- `man 7 sysvipc`, `man 2 shmget`, `man 2 semop`, `man 2 msgsnd`
- *Advanced Programming in the Unix Environment*, ch. 15 (IPC)
- `ipcs(1)` / `ipcrm(1)` — list and clean up SysV segments

---

*42 School — RNCP7 · Unix/Kernel*
