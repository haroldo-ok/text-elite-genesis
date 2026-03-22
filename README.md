# Text Elite Genesis

Sega Genesis / Mega Drive Port of Text Elite, using Claude to automate the inictial steps.
Based on Text Elite 1.5.

## Built with SGDK 1.70

Original Text Elite by **Ian Bell** (www.ianbellelite.com), from his 6502
Elite sources, co-authored with David Braben.

Genesis port: adapted from `txtelite.c` v1.5 (2015).

---

## What it is

A faithful port of the **Elite trading universe engine** to the Sega Genesis.
All 8 galaxies, 256 planets each, the authentic market algorithm, Goat Soup
planet descriptions, fuel/cash economy – everything from the original source.

The text-parser interface has been replaced with a **joypad-driven menu UI**
displayed through the VDP tile plane in 40-column text mode.

---

## Directory layout

```
txtelite_md/
  Makefile          ← wraps SGDK makefile.gen
  src/
    main.c          ← SGDK entry point, palette setup, main loop
    elite.c         ← entire game engine (Elite universe + UI logic)
  inc/
    elite.h         ← shared types and prototypes
  res/              ← (empty – no bitmaps needed, pure text game)
  README.md         ← this file
```

---

## Build requirements

| Tool | Version |
|------|---------|
| SGDK | **1.70** (set `GDK=/path/to/sgdk`) |
| GCC  | m68k-elf cross-compiler (bundled with SGDK) |
| Make | GNU make 4.x |

### Quick build

```bash
export GDK=/opt/sgdk          # adjust to your SGDK install path
cd txtelite_md
make
```

Output: `out/rom.bin`

### Docker build (recommended on non-Linux)

```bash
docker run --rm -v "$(pwd)":/src -w /src \
    ghcr.io/stephane-d/sgdk:1.70 \
    make GDK=/sgdk
```

### Clean

```bash
make clean
```

---

## Running

Load `out/rom.bin` in any Genesis emulator that supports 6-button pads
(BlastEm, Gens/GS, Genesis Plus GX).  A real console with a flash cart works
too.

---

## Controls (6-button pad recommended)

```
Title screen
  START           – start game (Lave, 100 CR, full tank)

Market screen  (default)
  UP / DOWN       – move cursor between commodities
  LEFT / RIGHT    – change quantity to buy/sell (1–63)
  A               – BUY  selected commodity
  B               – SELL selected commodity
  START           – buy fuel (fill tank)
  C               – STATUS screen (planet info + cargo)
  Y               – LOCAL MAP (jump screen)
  X               – HELP screen

Local Map screen
  UP / DOWN       – select destination planet
  A               – JUMP to selected system (uses fuel)
  Y               – INFO on selected system (Goat Soup desc)
  B / C           – back to Market
  START           – GALACTIC HYPERSPACE (next galaxy)

Info / Status / Help
  B or START      – back
```

---

## Porting notes

### What changed from `txtelite.c`

| Original | Genesis port |
|----------|-------------|
| `printf` / `gets` | VDP_drawText / joypad |
| `<math.h>` `sqrt()` | Integer Newton–Raphson `isqrt()` |
| `double` / `float` | All arithmetic is 32-bit integer |
| `conio.h` / `graph.h` | Removed (MSVC DOS headers) |
| `exit(0)` | Infinite loop (console has no OS) |
| Text parser (`buy food 5`) | Menu cursor + A/B buttons |
| `srand` / `rand` | Deterministic LCG from original source |

### Fixed-point prices

Prices are stored as integers equal to the real price × 10 (tenths of CR),
exactly as in the original 6502 code.  Display routines convert to "ddd.d"
strings without any floating-point operations.

### Distance calculation

Original: `4 * sqrt(dx² + dy²/4)` in floating point.
Port: `4 * isqrt(dx² + dy²/4)` using integer square root, result in tenths
of light-years (divide by 10 to get LY).

### Goat Soup

The recursive planet-description generator works unchanged; it writes into a
static char buffer (`gs_buf`) instead of calling `printf`.

### Memory

The `galaxy[256]` array of `plansys` structs is ~14 KB – well within the
Genesis 64 KB work RAM.

---

## Known limitations / future work

- No combat or missions (same as the original txtelite)
- 3-button pad users: Y and X are unavailable; use START for fuel, B to go
  back; Local Map is reachable by pressing UP past the market header (not
  ideal – a future revision will add a proper menu bar)
- Galactic hyperspace always lands at Lave (planet 7) in the new galaxy,
  matching original Elite behaviour
- Goat Soup descriptions longer than ~3 lines wrap but may be clipped at row 26

---

## Licence

The Elite universe algorithms are copyright Ian Bell & David Braben.
Ian Bell has made the source freely available; see www.ianbellelite.com.
This Genesis port is provided for personal / educational use.
