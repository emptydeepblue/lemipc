# Player Registration & IPC Persistence

How a player process joins the game board, and why the board outlives any individual process.

---

## 1. The board lives in kernel-managed shared memory

When `./lemipc <team>` runs, the very first thing it does is call `add_player()` in
`game-manager.c`. That function calls `get_shmid()`, which does:

```c
key = ftok("/tmp", SHM_KEY_ID);           // deterministic key from a filesystem path
shmid = shmget(key, sizeof(t_board), IPC_CREAT | IPC_EXCL | 0666);
```

`ftok("/tmp", 'L')` hashes the inode of `/tmp` with the character `'L'` to produce a
`key_t`. **Every process on the same machine calling `ftok` with the same arguments gets
the same integer key.** That is the rendezvous point — no socket, no file path passed on
the command line, just a deterministic key both sides can derive independently.

`shmget` tells the kernel: *give me a segment of `sizeof(t_board)` bytes identified by
this key*. The kernel allocates the memory inside its own address space (not inside any
process heap or stack). It returns a small integer — the **shmid** — which is the
handle for all further operations.

`IPC_CREAT | IPC_EXCL` together mean "create only if it doesn't exist yet". If a second
process calls the same `shmget` with those flags, the call fails with `EEXIST`. That
failure is the signal that the segment is already there; the code then retries without
`IPC_EXCL` (the second branch in `get_shmid`) to attach to the existing segment.
`is_creator` tracks which path was taken.

**The segment persists in the kernel until explicitly destroyed** with
`shmctl(shmid, IPC_RMID, NULL)` or a reboot. No process needs to stay alive to keep
it there. This is the key difference from a pipe or a socket: IPC shared memory has no
owner process — the kernel owns it.

---

## 2. Mapping the segment into a process's address space

Holding a shmid is not enough to read or write the memory. Each process must call:

```c
board = shmat(shmid, NULL, 0);
```

`shmat` ("shared memory attach") asks the kernel to map the segment into the calling
process's virtual address space. The kernel picks a suitable address (because `NULL` was
passed as the hint) and returns it. From that point, `board` is a normal `t_board *` — the
process can dereference it like any pointer.

When the process is done it calls `shmdt(board)` to unmap it. That does **not** destroy
the segment; it only removes the mapping from this process. The kernel segment lives on.

The first process (`is_creator == 1`) zeroes the segment immediately after attaching:

```c
if (is_creator)
    ft_memset(board, 0, sizeof(t_board));
```

This initialises `player_count` and every `players[i].pid` to zero before anyone else
joins. Without this, the segment would contain whatever garbage the kernel page happened
to hold.

---

## 3. The semaphore: one move at a time

Because multiple processes share the same memory, concurrent writes would corrupt the
board. A **System V semaphore** (value = 1, a binary mutex) serialises all writes:

```c
key  = ftok("/tmp", SEM_KEY_ID);          // same derivation, different character 'M'
semid = semget(key, 1, IPC_CREAT | 0666);
if (is_creator)
    semctl(semid, 0, SETVAL, 1);          // initialise to 1 exactly once
```

`semget` with only `IPC_CREAT` (no `IPC_EXCL`) is idempotent: if the semaphore already
exists the call succeeds and returns the existing id. Only the creator sets the initial
value to 1 via `semctl`; subsequent processes skip that step, so the value is never reset
while the game is running.

Inside `register_player`, the lock/unlock pattern is:

```c
struct sembuf lock   = {0, -1, 0};   // decrement by 1 (block if already 0)
struct sembuf unlock = {0,  1, 0};   // increment by 1

semop(semid, &lock,   1);            // acquire
// ... mutate board ...
semop(semid, &unlock, 1);            // release
```

`semop` with `-1` blocks the calling process if the semaphore is already 0, resuming only
when another process increments it back to 1. This guarantees that at most one process
touches the players array at any moment.

---

## 4. Finding a free slot and writing the player struct

The players array inside `t_board` is a flat fixed-size array:

```c
typedef struct s_board {
    t_player  players[MAX_PLAYERS * MAX_TEAMS];  // 16 slots
    int       player_count;
    int       running;
} t_board;
```

A slot is considered free when `players[i].pid == 0`. `register_player` scans linearly:

```c
while (i < MAX_PLAYERS * MAX_TEAMS)
{
    if (board->players[i].pid == 0)
    {
        board->players[i] = make_player(team, find_spawn(board));
        board->player_count++;
        semop(semid, &unlock, 1);
        return (board->players[i]);
    }
    i++;
}
```

`pid == 0` is safe as a sentinel because `getpid()` never returns 0 for a live process (PID
0 is the idle/swapper task, never a user process). When a player dies later, clearing
`pid` back to 0 marks the slot as reusable.

`make_player` fills the struct:

```c
p.pid   = getpid();          // unique identifier — this process IS this player
p.team  = team;
p.skin  = ((team - 1) % 4) + 1;
p.alive = 1;
p.pos   = find_spawn(board); // deterministic position based on join order
```

`find_spawn` spaces players evenly across the board using the current `player_count`
as an index so no two players start on the same cell:

```c
step  = MAP_SIZE / (MAX_PLAYERS * MAX_TEAMS + 1);   // 100 / 17 ≈ 5
pos.x = (player_count * step + step) % MAP_SIZE;
pos.y = (player_count * step * 2 + step) % MAP_SIZE;
```

---

## 5. Full lifecycle summary

```
Process A (first)                   Kernel                  Process B (second)
─────────────────                   ──────                  ─────────────────────
ftok("/tmp", 'L')  ─────────────►  key = 0xABCD...
shmget(key, ..., IPC_CREAT|EXCL) ─► allocates segment       ftok("/tmp", 'L') → same key
is_creator = 1                      shmid = 7               shmget(key, ..., IPC_CREAT|EXCL) → EEXIST
                                                             shmget(key, ..., 0) → shmid = 7
shmat(7, NULL, 0)  ─────────────►  maps segment             shmat(7, NULL, 0) → maps same segment
ft_memset(board, 0, ...)
semctl(semid, 0, SETVAL, 1)         semaphore = 1

semop(-1) → lock                                             semop(-1) → BLOCKS (sem = 0)
  players[0] = make_player(...)
  player_count = 1
semop(+1) → unlock                  semaphore = 1           unblocked
                                                               players[1] = make_player(...)
                                                               player_count = 2
shmdt(board)       ─────────────►  detach, segment stays    shmdt(board)
process continues...                                         process continues...
```

The segment and semaphore remain in the kernel across all of this. Any new process can
attach to them by deriving the same key with `ftok`. They are destroyed only when
`cleanup_ipc()` calls `shmctl(shmid, IPC_RMID, NULL)` and `semctl(semid, 0, IPC_RMID)`.

---

## 6. Verifying IPC state from the shell

```bash
ipcs -m    # list shared memory segments (see key, shmid, size, nattch)
ipcs -s    # list semaphores
ipcrm -m <shmid>   # manual cleanup if cleanup_ipc() was never called
```
