# Evaluation readiness — mandatory part

Audit of the codebase against the lemipc subject (`docs/en.subject.pdf`) and FAQ
(`docs/faq.en.txt`). Status: **not ready** — one definite rule violation and one
architectural concern.

---

## 1. Diagonal surround missing (definite mandatory failure)

Subject:

> For a player to be killed, they must be touched by at least 2 players from
> the same team, that is, one adjacent tile to the tile the target stands on
> **(diagonal works)**.

Current implementation in [src/move.c:156-177](../src/move.c#L156-L177)
(`is_pos_surrounded`) only checks the 4 cardinal neighbours:

```c
adj[0] = (t_position){pos.x, pos.y - 1};
adj[1] = (t_position){pos.x, pos.y + 1};
adj[2] = (t_position){pos.x - 1, pos.y};
adj[3] = (t_position){pos.x + 1, pos.y};
```

**Fix:** extend `adj` to all 8 surrounding tiles (skip the centre cell, skip
out-of-bounds). One-line change to the array, plus loop bound from 4 → 8.

---

## 2. SHM exposes individual player identity (architectural — likely concern)

Subject:

> On the map, a player can see if a tile is empty or occupied by another
> player. If so, the player's team number will be displayed. **You cannot
> differentiate players from the same team.**

Current `t_board` ([inc/lemipc.h:73-78](../inc/lemipc.h#L73-L78)) stores a flat
array of full `t_player` structs (`pid`, `alive`, individual positions). Any
process attached to SHM can read two same-team players as distinct entries,
which violates the spirit of the rule.

A strict reading expects the board in SHM to look like:

```c
typedef struct s_board {
    int  map[MAP_SIZE][MAP_SIZE];   // 0 = empty, 1..MAX_TEAMS = team number
    int  alive_count[MAX_TEAMS + 1];
    int  running;
} t_board;
```

With that layout, each process keeps its own `pid`/`team`/`pos` in local memory
and only writes `map[x][y]` in SHM. Game logic (surround check, "is this tile
occupied", BFS blockers) reads only team numbers from the grid.

**Impact:** medium-large refactor. Touches `player.c` (registration, spawn),
`move.c` (`team_at_pos`, `handle_collision`, `update_board_pos`,
`eliminate_self`, `is_cell_free`), `bfs.c` (`is_blocked`), `display.c`
(`draw_grid`, score counting). The team-centroid / nearest-enemy helpers in
`move.c` need rethinking — without per-player coordinates in SHM you can't
compute a centroid; either drop that AI heuristic or scan the map for tiles of
your own team.

**Verdict:** debatable. Some evaluators accept the current form; a defender
focused on the subject wording will reject it. Worth fixing if you want a
clean defence.

---

## 3. Minor concerns to be ready to defend

- **Abnormal exit (SIGINT/SIGTERM):** no signal handler. Killing a player with
  Ctrl-C leaves their slot occupied and IPCs orphaned (`ipcs` will show
  leftovers). Subject doesn't strictly require it, but it mentions
  `man ipcs(1)` and a defender may probe this. Adding a `signal()` handler
  that calls `cleanup_game` (after releasing the slot under sem lock) would
  shore this up.
- **Race on `board->running` in `cleanup_game`:** read without holding the
  semaphore ([src/ipc.c:132](../src/ipc.c#L132)). Benign in practice (word-size
  read on x86), but `make htest` (helgrind) will flag it. Either acquire the
  sem briefly or document that the read is intentional.
- **Board size 5×5 with 32 player slots** is very cramped. Spawn search and
  BFS hold up, but on a small map the AI sometimes oscillates. Consider
  bumping `MAP_SIZE` (FAQ: "size is up to you") for a smoother defence demo.

---

## 4. What is solid (no action needed)

- Single executable, no `fork`, team passed as argv (FAQ-compliant).
- First player creates SHM/SEM/MSGQ via `ftok` + `IPC_CREAT | IPC_EXCL`;
  others attach. Last player on board removes them with `IPC_RMID`. Verified
  end-to-end: `ipcs` is empty after a 2v2 match.
- All board mutations sit under a binary System V semaphore.
- Team coordination uses MSGQ (`mtype = team`), with vote/agree-on-target
  pattern matching the FAQ.
- BFS-based movement towards the voted target, with fallbacks to any free
  adjacent tile.
- Two display modes: each player logs its own moves, plus `./lemipc 0` runs a
  read-only spectator (`SHM_RDONLY`) — both forms allowed by subject.
- No memory leaks observed; `make vtest` and `make htest` targets exist for
  defence.

---

## Suggested fix order

1. Diagonal surround (item 1) — cheap, mandatory.
2. Decide on SHM layout (item 2). If refactoring: do it before bonuses, since
   AI heuristics in `move.c` depend on the current layout.
3. Signal handler for clean exit (item 3, first bullet).
4. Optional: bump `MAP_SIZE` to 10–15 for nicer demo.
