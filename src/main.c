/* main.c – Text Elite: Sega Genesis entry point
   Uses SGDK 1.70 APIs.
*/

#include <genesis.h>
#include "elite.h"

/* ── Palette ─────────────────────────────────────────────────────────────── */
/* SGDK system font: foreground pixels use colour index 15 of the active palette.
   Index 0 = background (transparent). Other indices unused by font tiles.
   Each palette only needs [0]=background and [15]=text colour.            */

/* PAL0: green – body text, labels, separators */
static const u16 pal_normal[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – background black       */
    RGB24_TO_VDPCOLOR(0x000000),  /*  1                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  2                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  3                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  4                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  5                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  6                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  7                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  8                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  9                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 10                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 11                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 12                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 13                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 14                          */
    RGB24_TO_VDPCOLOR(0x00CC44),  /* 15 – FONT COLOUR: green     */
};

/* PAL1: gold – screen titles, planet names, section headers */
static const u16 pal_hi[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – background             */
    RGB24_TO_VDPCOLOR(0x000000),  /*  1                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  2                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  3                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  4                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  5                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  6                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  7                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  8                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  9                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 10                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 11                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 12                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 13                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 14                          */
    RGB24_TO_VDPCOLOR(0xFFCC00),  /* 15 – FONT COLOUR: gold      */
};

/* PAL2: cyan – data values: prices, quantities, distances, cash, fuel */
static const u16 pal_cyan[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – background             */
    RGB24_TO_VDPCOLOR(0x000000),  /*  1                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  2                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  3                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  4                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  5                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  6                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  7                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  8                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  9                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 10                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 11                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 12                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 13                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 14                          */
    RGB24_TO_VDPCOLOR(0x00EEFF),  /* 15 – FONT COLOUR: cyan      */
};

/* PAL3: red-orange – cursor marker, status messages, warnings */
static const u16 pal_alert[16] = {
    RGB24_TO_VDPCOLOR(0x000000),  /*  0 – background             */
    RGB24_TO_VDPCOLOR(0x000000),  /*  1                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  2                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  3                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  4                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  5                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  6                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  7                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  8                          */
    RGB24_TO_VDPCOLOR(0x000000),  /*  9                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 10                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 11                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 12                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 13                          */
    RGB24_TO_VDPCOLOR(0x000000),  /* 14                          */
    RGB24_TO_VDPCOLOR(0xFF4400),  /* 15 – FONT COLOUR: red-orange*/
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
