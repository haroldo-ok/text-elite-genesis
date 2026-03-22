/* main.c – Text Elite: Sega Genesis entry point
   Uses SGDK 1.70 APIs.
*/

#include <genesis.h>
#include "elite.h"

/* ── Palette ─────────────────────────────────────────────────────────────── */
/* PAL0: body text – classic green-on-black terminal look */
static const u16 pal_normal[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – background black       */
    RGB24_TO_VDPCOLOR(0x00AA44),  /*  1 – body green             */
    RGB24_TO_VDPCOLOR(0x006622),  /*  2 – dim green              */
    RGB24_TO_VDPCOLOR(0x00FF66),  /*  3 – bright green           */
    RGB24_TO_VDPCOLOR(0xCCCCCC),  /*  4 – light grey             */
    RGB24_TO_VDPCOLOR(0x888888),  /*  5 – mid grey               */
    RGB24_TO_VDPCOLOR(0x444444),  /*  6 – dark grey              */
    RGB24_TO_VDPCOLOR(0x222222),  /*  7 – near-black             */
    RGB24_TO_VDPCOLOR(0x004422),  /*  8 – dark bg tint           */
    RGB24_TO_VDPCOLOR(0x004422),  /*  9                          */
    RGB24_TO_VDPCOLOR(0x004422),  /* 10                          */
    RGB24_TO_VDPCOLOR(0x004422),  /* 11                          */
    RGB24_TO_VDPCOLOR(0x004422),  /* 12                          */
    RGB24_TO_VDPCOLOR(0x004422),  /* 13                          */
    RGB24_TO_VDPCOLOR(0x004422),  /* 14                          */
    RGB24_TO_VDPCOLOR(0x004422),  /* 15                          */
};

/* PAL1: gold – screen titles and headers */
static const u16 pal_hi[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – background             */
    RGB24_TO_VDPCOLOR(0xFFCC00),  /*  1 – gold                   */
    RGB24_TO_VDPCOLOR(0xFF8800),  /*  2 – orange                 */
    RGB24_TO_VDPCOLOR(0xFFFF88),  /*  3 – pale yellow            */
    RGB24_TO_VDPCOLOR(0xFFFFFF),  /*  4 – white                  */
    RGB24_TO_VDPCOLOR(0xCCCCCC),  /*  5                          */
    RGB24_TO_VDPCOLOR(0x888888),  /*  6                          */
    RGB24_TO_VDPCOLOR(0x444444),  /*  7                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  8                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  9                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 10                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 11                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 12                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 13                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 14                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 15                          */
};

/* PAL2: cyan – data values, prices, quantities, distances */
static const u16 pal_cyan[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – background             */
    RGB24_TO_VDPCOLOR(0x00EEFF),  /*  1 – bright cyan            */
    RGB24_TO_VDPCOLOR(0x0099BB),  /*  2 – dim cyan               */
    RGB24_TO_VDPCOLOR(0xAAFFFF),  /*  3 – pale cyan              */
    RGB24_TO_VDPCOLOR(0xFFFFFF),  /*  4                          */
    RGB24_TO_VDPCOLOR(0xCCCCCC),  /*  5                          */
    RGB24_TO_VDPCOLOR(0x888888),  /*  6                          */
    RGB24_TO_VDPCOLOR(0x444444),  /*  7                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  8                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  9                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 10                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 11                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 12                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 13                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 14                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 15                          */
};

/* PAL3: red/orange – messages, warnings, cursor marker */
static const u16 pal_alert[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – background             */
    RGB24_TO_VDPCOLOR(0xFF4400),  /*  1 – red-orange             */
    RGB24_TO_VDPCOLOR(0xFF8844),  /*  2 – light orange           */
    RGB24_TO_VDPCOLOR(0xFF2200),  /*  3 – deep red               */
    RGB24_TO_VDPCOLOR(0xFFFFFF),  /*  4                          */
    RGB24_TO_VDPCOLOR(0xCCCCCC),  /*  5                          */
    RGB24_TO_VDPCOLOR(0x888888),  /*  6                          */
    RGB24_TO_VDPCOLOR(0x444444),  /*  7                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  8                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  9                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 10                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 11                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 12                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 13                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 14                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 15                          */
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
    PAL_setPalette(PAL2, pal_cyan,   CPU);
    PAL_setPalette(PAL3, pal_alert,  CPU);

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
