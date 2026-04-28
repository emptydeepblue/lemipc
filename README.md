# LemIpc

A 42 school project on System V IPC. Multiple processes join a shared 5×5 board as players of up to 4 teams and fight until only one team remains. State is shared via SysV shared memory, synchronised with semaphores, and teams coordinate targets through a message queue.

## Build

```sh
make
```

Produces the `lemipc` binary. `make fclean` to clean.

## Usage

```sh
./lemipc <team_number>   # 1..4 — join the game as a player on that team
./lemipc 0               # spectator: render the board to the terminal
```

The first player to start creates the IPC segments; the last one alive cleans them up. Run several players in the background and a single spectator to watch a match:

```sh
./lemipc 0 &
./lemipc 1 & ./lemipc 1 & ./lemipc 2 & ./lemipc 2 &
wait
```

## Make targets

- `make test` — launch a preset battle of ~32 players across 4 teams.
- `make vtest` — small battle under valgrind (leaks).
- `make htest` — small battle under helgrind (race conditions).

## Layout

- [src/ipc.c](src/ipc.c) — shm/sem/msg setup and teardown
- [src/player.c](src/player.c) — registration and alive-sync
- [src/move.c](src/move.c), [src/bfs.c](src/bfs.c) — movement and pathfinding
- [src/message.c](src/message.c) — team voting on targets via msg queue
- [src/display.c](src/display.c), [src/draw.c](src/draw.c) — spectator rendering
- [inc/lemipc.h](inc/lemipc.h) — types and tunables (`MAP_SIZE`, `MAX_PLAYERS`, `MAX_TEAMS`)
