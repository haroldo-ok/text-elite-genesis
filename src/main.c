/* main.c – Text Elite: Sega Genesis entry point
   Uses SGDK 1.70 APIs.
*/

#include <genesis.h>
#include "elite.h"

/* ── Palette ─────────────────────────────────────────────────────────────── */
/* PAL0: normal text – dark background, light green text (classic terminal) */
static const u16 pal_normal[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – transparent / black bg */
    RGB24_TO_VDPCOLOR(0x00CC44),  /*  1 – green  (text fg)       */
    RGB24_TO_VDPCOLOR(0x004400),  /*  2 – dark green             */
    RGB24_TO_VDPCOLOR(0x00FF88),  /*  3 – bright green           */
    RGB24_TO_VDPCOLOR(0xFFFFFF),  /*  4 – white                  */
    RGB24_TO_VDPCOLOR(0xCCCCCC),  /*  5 – light grey             */
    RGB24_TO_VDPCOLOR(0x888888),  /*  6 – mid grey               */
    RGB24_TO_VDPCOLOR(0x444444),  /*  7 – dark grey              */
    RGB24_TO_VDPCOLOR(0xFFFF00),  /*  8 – yellow                 */
    RGB24_TO_VDPCOLOR(0xFF8800),  /*  9 – orange                 */
    RGB24_TO_VDPCOLOR(0xFF0000),  /* 10 – red                    */
    RGB24_TO_VDPCOLOR(0x0088FF),  /* 11 – blue                   */
    RGB24_TO_VDPCOLOR(0x00FFFF),  /* 12 – cyan                   */
    RGB24_TO_VDPCOLOR(0xFF00FF),  /* 13 – magenta                */
    RGB24_TO_VDPCOLOR(0x884400),  /* 14 – brown                  */
    RGB24_TO_VDPCOLOR(0x002200),  /* 15 – very dark green        */
};

/* PAL1: highlighted text – amber / gold on dark */
static const u16 pal_hi[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – transparent            */
    RGB24_TO_VDPCOLOR(0xFFCC00),  /*  1 – gold  (hi text)        */
    RGB24_TO_VDPCOLOR(0xFF8800),  /*  2 – orange                 */
    RGB24_TO_VDPCOLOR(0xFFFF88),  /*  3 – pale yellow            */
    RGB24_TO_VDPCOLOR(0xFFFFFF),  /*  4 – white                  */
    RGB24_TO_VDPCOLOR(0xCCCCCC),  /*  5 */
    RGB24_TO_VDPCOLOR(0x888888),  /*  6 */
    RGB24_TO_VDPCOLOR(0x444444),  /*  7 */
    RGB24_TO_VDPCOLOR(0x00FF00),  /*  8 */
    RGB24_TO_VDPCOLOR(0x00CC44),  /*  9 */
    RGB24_TO_VDPCOLOR(0xFF0000),  /* 10 */
    RGB24_TO_VDPCOLOR(0x0088FF),  /* 11 */
    RGB24_TO_VDPCOLOR(0x00FFFF),  /* 12 */
    RGB24_TO_VDPCOLOR(0xFF00FF),  /* 13 */
    RGB24_TO_VDPCOLOR(0x884400),  /* 14 */
    RGB24_TO_VDPCOLOR(0x002200),  /* 15 */
};

int main(void)
{
    /* ── Hardware init ───────────────────────────────────────────────────── */
    JOY_init();
    VDP_setScreenWidth320();    /* 40-column text mode             */

    /* Use BG_A as the text plane so VDP_drawText writes there */
    VDP_setTextPlane(BG_A);

    /* Load palettes */
    PAL_setPalette(PAL0, pal_normal, CPU);
    PAL_setPalette(PAL1, pal_hi,     CPU);

    /* Default text colour (PAL0 pen 1 = green) */
    VDP_setTextPalette(PAL0);

    /* Background colour – pick a very dark green from PAL0[0] */
    VDP_setBackgroundColor(0);  /* colour index 0 of PAL0 = black */

    /* ── Game init ───────────────────────────────────────────────────────── */
    elite_init();
    drawscreen();               /* draw title immediately           */

    /* ── Main loop ───────────────────────────────────────────────────────── */
    while (1)
    {
        SYS_doVBlankProcess();  /* wait for VBlank, process DMA etc */
        elite_frame();          /* handle input, redraw if needed   */
    }

    return 0;   /* unreachable */
}
