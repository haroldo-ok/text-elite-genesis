/* elite.h - Text Elite for Sega Genesis / SGDK 1.70
   Ported from txtelite.c 1.5 by Ian Bell (ian@ianbell.me)
   Original Elite by Ian Bell & David Braben.
   Port: all stdio/float replaced with Genesis VDP text + fixed-point math.
*/
#ifndef ELITE_H
#define ELITE_H

#include <genesis.h>

/* ── Basic types ─────────────────────────────────────────────────────────── */
typedef int            boolean;
typedef u8             uint8;
typedef u16            uint16;
typedef s16            int16;
typedef s32            int32;
typedef uint16         uint;
typedef int            planetnum;

#define true  1
#define false 0
#define tonnes 0

/* ── String / display constants ─────────────────────────────────────────── */
#define MAXLEN   20
#define SCREEN_W 40   /* VDP text columns (40-col mode) */
#define SCREEN_H 28   /* visible tile rows              */

/* ── Game constants ──────────────────────────────────────────────────────── */
#define galsize    256
#define AlienItems  16
#define lasttrade  AlienItems

#define numforLave   7
#define numforZaonce 129
#define numforDiso   147
#define numforRied    46

/* ── Seed structures ─────────────────────────────────────────────────────── */
typedef struct { uint8 a,b,c,d; } fastseedtype;
typedef struct { uint16 w0,w1,w2; } seedtype;

/* ── Planet system ────────────────────────────────────────────────────────── */
typedef struct {
    uint x, y;
    uint economy;
    uint govtype;
    uint techlev;
    uint population;
    uint productivity;
    uint radius;
    fastseedtype goatsoupseed;
    char name[12];
} plansys;

/* ── Trade good ───────────────────────────────────────────────────────────── */
typedef struct {
    uint   baseprice;
    int16  gradient;
    uint   basequant;
    uint   maskbyte;
    uint   units;
    char   name[20];
} tradegood;

/* ── Market ───────────────────────────────────────────────────────────────── */
typedef struct {
    uint16 quantity[lasttrade+1];
    uint16 price[lasttrade+1];
} markettype;

/* ── Screens / UI states ─────────────────────────────────────────────────── */
typedef enum {
    SCREEN_TITLE,
    SCREEN_MARKET,
    SCREEN_LOCAL,
    SCREEN_INFO,
    SCREEN_JUMP,
    SCREEN_STATUS,
    SCREEN_HELP,
    SCREEN_GALHYP
} GameScreen;

/* ── Function prototypes ─────────────────────────────────────────────────── */
void elite_init(void);
void elite_frame(void);   /* call once per vblank */

/* internal helpers exposed for main */
void drawscreen(void);

#endif /* ELITE_H */
