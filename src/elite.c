/* elite.c – Text Elite 1.5 engine ported to Sega Genesis / SGDK 1.70
   Original C sources: Ian Bell.  Genesis adaptation: port project.

   Key changes vs. original txtelite.c:
     • No stdio / printf / gets – all output via VDP tile text helpers.
     • No <math.h>  – sqrt replaced with integer Newton–Raphson.
     • No float     – prices stored as integers (*10), shown via int2str.
     • No conio/graph headers.
     • Input: joypad instead of keyboard.
     • UI:   menu-driven screens instead of a text parser.
*/

#include "elite.h"

/* ════════════════════════════════════════════════════════════════════════════
   Palette / plane configuration
   ════════════════════════════════════════════════════════════════════════════ */
#define PAL_TEXT   PAL0
#define PAL_HI     PAL1      /* highlighted / title text */
#define PLANE_BG   BG_B
#define PLANE_FG   BG_A

/* ════════════════════════════════════════════════════════════════════════════
   Fixed-point helpers  (no float on stock GCC-m68k without soft-float lib)
   ════════════════════════════════════════════════════════════════════════════ */

/* Integer square root (returns floor(sqrt(n))) */
static u32 isqrt(u32 n)
{
    u32 x, y;
    if (n == 0) return 0;
    x = n;
    y = (x + 1) >> 1;
    while (y < x) { x = y; y = (x + n/x) >> 1; }
    return x;
}

/* ════════════════════════════════════════════════════════════════════════════
   Minimal string utilities  (no <string.h> pulled from SGDK)
   ════════════════════════════════════════════════════════════════════════════ */

static int e_strlen(const char *s)
{ int n=0; while(s[n]) n++; return n; }

static void e_strcpy(char *d, const char *s)
{ while((*d++=*s++)); }

static int e_strcmp(const char *a, const char *b)
{ while(*a && *a==*b){a++;b++;} return (unsigned char)*a-(unsigned char)*b; }

/* Convert unsigned integer to decimal string; returns pointer past last char */
static char *uint2str(u32 v, char *buf)
{
    char tmp[12]; int i=0; char *p;
    if (v == 0) { *buf++='0'; *buf=0; return buf; }
    while(v){ tmp[i++]='0'+(v%10); v/=10; }
    p=buf;
    while(i--) *p++=tmp[i];
    *p=0;
    return p;
}

/* Convert signed integer */
static char *int2str(s32 v, char *buf)
{
    if(v<0){ *buf++='-'; v=-v; }
    return uint2str((u32)v, buf);
}

/* Format as "ddd.d"  where input is tenths (e.g. 147 → "14.7") */
static void tenths2str(u32 v, char *buf)
{
    uint2str(v/10, buf);
    char *p = buf + e_strlen(buf);
    *p++='.'; *p++='0'+(v%10); *p=0;
}

/* Format cash: stored as integer tenths of CR  (e.g. 1000 = 100.0 CR) */
static void cash2str(s32 v, char *buf)
{
    if(v<0){ *buf++='-'; v=-v; }
    tenths2str((u32)v, buf);
}

/* Uppercase a char */
static char to_upper(char c)
{ if(c>='a'&&c<='z') return c-('a'-'A'); return c; }

static char to_lower(char c)
{ if(c>='A'&&c<='Z') return c+('a'-'A'); return c; }

/* Does string t start with (case-insensitive) string s ? */
static boolean stringbeg(const char *s, const char *t)
{
    int i=0, l=e_strlen(s);
    if(l==0) return false;
    while(i<l && to_upper(s[i])==to_upper(t[i])) i++;
    return (i==l);
}

/* Remove all occurrences of character c from string s in-place */
static void stripout(char *s, char c)
{
    int i=0,j=0;
    while(s[i]){ if(s[i]!=c) s[j++]=s[i]; i++; }
    s[j]=0;
}

/* ════════════════════════════════════════════════════════════════════════════
   Galaxy / market data  (verbatim from txtelite.c 1.5)
   ════════════════════════════════════════════════════════════════════════════ */

static char pairs0[]=
"ABOUSEITILETSTONLONUTHNOALLEXEGEZACEBISOUSESARMAINDIREA.ERATENBERALAVETIEDORQUANTEISRION";

static char pairs[] =
    "..LEXEGEZACEBISO"
    "USESARMAINDIREA."
    "ERATENBERALAVETI"
    "EDORQUANTEISRION";

static char govnames[][MAXLEN]={
    "Anarchy","Feudal","Multi-gov","Dictatorship",
    "Communist","Confederacy","Democracy","Corporate State"};

static char econnames[][MAXLEN]={
    "Rich Ind","Average Ind","Poor Ind","Mainly Ind",
    "Mainly Agri","Rich Agri","Average Agri","Poor Agri"};

static char unitnames[][5]={"t","kg","g"};

static tradegood commodities[]={
    {0x13,-0x02,0x06,0x01,0,"Food        "},
    {0x14,-0x01,0x0A,0x03,0,"Textiles    "},
    {0x41,-0x03,0x02,0x07,0,"Radioactives"},
    {0x28,-0x05,0xE2,0x1F,0,"Slaves      "},
    {0x53,-0x05,0xFB,0x0F,0,"Liquor/Wines"},
    {0xC4,+0x08,0x36,0x03,0,"Luxuries    "},
    {0xEB,+0x1D,0x08,0x78,0,"Narcotics   "},
    {0x9A,+0x0E,0x38,0x03,0,"Computers   "},
    {0x75,+0x06,0x28,0x07,0,"Machinery   "},
    {0x4E,+0x01,0x11,0x1F,0,"Alloys      "},
    {0x7C,+0x0d,0x1D,0x07,0,"Firearms    "},
    {0xB0,-0x09,0xDC,0x3F,0,"Furs        "},
    {0x20,-0x01,0x35,0x03,0,"Minerals    "},
    {0x61,-0x01,0x42,0x07,1,"Gold        "},
    {0xAB,-0x02,0x37,0x1F,1,"Platinum    "},
    {0x2D,-0x01,0xFA,0x0F,2,"Gem-Stones  "},
    {0x35,+0x0F,0xC0,0x07,0,"Alien Items "},
};

static char tradnames[lasttrade+1][MAXLEN];

/* ════════════════════════════════════════════════════════════════════════════
   Galaxy storage & player state
   ════════════════════════════════════════════════════════════════════════════ */

static plansys galaxy[galsize];

static seedtype      seed;
static fastseedtype  rnd_seed;
static boolean       nativerand;
static unsigned int  lastrand = 0;

static uint     shipshold[lasttrade+1];
static planetnum currentplanet;
static uint      galaxynum;
static s32       cash;
static uint      fuel;
static markettype localmarket;
static uint      holdspace;

static const int fuelcost = 2;   /* 0.2 CR / LY  (×10) */
static const int maxfuel  = 70;  /* 7.0 LY       (×10) */

static const uint16 base0=0x5A4A;
static const uint16 base1=0x0248;
static const uint16 base2=0xB753;

/* ════════════════════════════════════════════════════════════════════════════
   RNG
   ════════════════════════════════════════════════════════════════════════════ */
static void mysrand(unsigned int s)
{ /* srand not available; just set lastrand */ lastrand=s-1; }

static int myrand(void)
{
    int r;
    r=(int)((((((((((((lastrand<<3)-lastrand)<<3)
        +lastrand)<<1)+lastrand)<<4)
        -lastrand)<<1)-lastrand)+0xe60)
        &0x7fffffff);
    lastrand=r-1;
    return r;
}

static char randbyte(void){ return (char)(myrand()&0xFF); }

static uint mymin(uint a,uint b){ return (a<b)?a:b; }

/* ════════════════════════════════════════════════════════════════════════════
   Seed / galaxy math
   ════════════════════════════════════════════════════════════════════════════ */

static void tweakseed(seedtype *s)
{
    uint16 temp=(s->w0)+(s->w1)+(s->w2);
    s->w0=s->w1; s->w1=s->w2; s->w2=temp;
}

static uint16 rotatel(uint16 x)
{ uint16 t=x&128; return (uint16)(2*(x&127))+(t>>7); }

static uint16 twist(uint16 x)
{ return (uint16)((256*rotatel(x>>8))+rotatel(x&255)); }

static void nextgalaxy(seedtype *s)
{ s->w0=twist(s->w0); s->w1=twist(s->w1); s->w2=twist(s->w2); }

/* ════════════════════════════════════════════════════════════════════════════
   Planet name generation  (exact 6502 algorithm)
   ════════════════════════════════════════════════════════════════════════════ */

static plansys makesystem(seedtype *s)
{
    plansys sys;
    uint pair1,pair2,pair3,pair4;
    uint16 longname=(s->w0)&64;

    sys.x=((s->w1)>>8);
    sys.y=((s->w0)>>8);
    sys.govtype=(((s->w1)>>3)&7);
    sys.economy=(((s->w0)>>8)&7);
    if(sys.govtype<=1) sys.economy|=2;
    sys.techlev=(((s->w1)>>8)&3)+((sys.economy)^7);
    sys.techlev+=(sys.govtype)>>1;
    if((sys.govtype)&1) sys.techlev++;
    sys.population=4*(sys.techlev)+(sys.economy)+(sys.govtype)+1;
    sys.productivity=(((sys.economy)^7)+3)*((sys.govtype)+4)*(sys.population)*8;
    sys.radius=256*(((( s->w2)>>8)&15)+11)+sys.x;

    sys.goatsoupseed.a=s->w1&0xFF;
    sys.goatsoupseed.b=s->w1>>8;
    sys.goatsoupseed.c=s->w2&0xFF;
    sys.goatsoupseed.d=s->w2>>8;

    pair1=2*(((s->w2)>>8)&31); tweakseed(s);
    pair2=2*(((s->w2)>>8)&31); tweakseed(s);
    pair3=2*(((s->w2)>>8)&31); tweakseed(s);
    pair4=2*(((s->w2)>>8)&31); tweakseed(s);

    sys.name[0]=pairs[pair1];   sys.name[1]=pairs[pair1+1];
    sys.name[2]=pairs[pair2];   sys.name[3]=pairs[pair2+1];
    sys.name[4]=pairs[pair3];   sys.name[5]=pairs[pair3+1];
    if(longname){
        sys.name[6]=pairs[pair4]; sys.name[7]=pairs[pair4+1]; sys.name[8]=0;
    } else sys.name[6]=0;
    stripout(sys.name,'.');
    return sys;
}

static void buildgalaxy(uint gn)
{
    uint sc,gc;
    seed.w0=base0; seed.w1=base1; seed.w2=base2;
    for(gc=1;gc<gn;gc++) nextgalaxy(&seed);
    for(sc=0;sc<galsize;sc++) galaxy[sc]=makesystem(&seed);
}

/* ════════════════════════════════════════════════════════════════════════════
   Navigation helpers
   ════════════════════════════════════════════════════════════════════════════ */

/* distance × 10  (light years × 10, integer) */
static uint distance(plansys a, plansys b)
{
    s32 dx=a.x-b.x, dy=a.y-b.y;
    /* Original: 4*sqrt(dx²+dy²/4)  scaled ×10 → 40*sqrt(dx²+dy²/4) */
    u32 val=(u32)(dx*dx + dy*dy/4);
    return (uint)(4*isqrt(val));
}

static void gamejump(planetnum i)
{
    currentplanet=i;
    /* localmarket will be set on actual jump call */
    /* call genmarket below */
}

static planetnum matchsys(const char *s)
{
    planetnum sc, p=currentplanet;
    uint d=9999;
    for(sc=0;sc<galsize;sc++){
        if(stringbeg(s,galaxy[sc].name)){
            uint dd=distance(galaxy[sc],galaxy[currentplanet]);
            if(dd<d){ d=dd; p=sc; }
        }
    }
    return p;
}

/* ════════════════════════════════════════════════════════════════════════════
   Market
   ════════════════════════════════════════════════════════════════════════════ */

static markettype genmarket(uint fluct, plansys p)
{
    markettype m; uint i;
    for(i=0;i<=lasttrade;i++){
        s32 q,price;
        s32 prod=(s32)(p.economy)*(commodities[i].gradient);
        s32 chg=(s32)(fluct & commodities[i].maskbyte);
        q=(s32)(commodities[i].basequant)+chg-prod;
        q&=0xFF; if(q&0x80) q=0;
        m.quantity[i]=(uint16)(q&0x3F);
        price=(s32)(commodities[i].baseprice)+chg+prod;
        price&=0xFF;
        m.price[i]=(uint16)(price*4);
    }
    m.quantity[AlienItems]=0;
    return m;
}

static uint gamebuy(uint i, uint a)
{
    uint t;
    if(cash<0) return 0;
    t=mymin(localmarket.quantity[i],a);
    if(commodities[i].units==tonnes) t=mymin(holdspace,t);
    t=mymin(t,(uint)((u32)cash/(localmarket.price[i]+1)));
    shipshold[i]+=t;
    localmarket.quantity[i]-=t;
    cash-=t*(s32)localmarket.price[i];
    if(commodities[i].units==tonnes) holdspace-=t;
    return t;
}

static uint gamesell(uint i, uint a)
{
    uint t=mymin(shipshold[i],a);
    shipshold[i]-=t;
    localmarket.quantity[i]+=t;
    if(commodities[i].units==tonnes) holdspace+=t;
    cash+=t*(s32)localmarket.price[i];
    return t;
}

static uint gamefuel(uint f)
{
    if(f+fuel>(uint)maxfuel) f=maxfuel-fuel;
    if(fuelcost>0){
        if((s32)f*fuelcost>cash) f=(uint)(cash/fuelcost);
    }
    fuel+=f; cash-=fuelcost*(s32)f;
    return f;
}

/* ════════════════════════════════════════════════════════════════════════════
   Goat Soup  – planet description generator (exact algorithm)
   ════════════════════════════════════════════════════════════════════════════ */

struct desc_choice { const char *option[5]; };

static const struct desc_choice desc_list[]={
/*81*/{"fabled","notable","well known","famous","noted"},
/*82*/{"very","mildly","most","reasonably",""},
/*83*/{"ancient","\x95","great","vast","pink"},
/*84*/{"\x9E \x9D plantations","mountains","\x9C","\x94 forests","oceans"},
/*85*/{"shyness","silliness","mating traditions","loathing of \x86","love for \x86"},
/*86*/{"food blenders","tourists","poetry","discos","\x8E"},
/*87*/{"talking tree","crab","bat","lobst","\xB2"},
/*88*/{"beset","plagued","ravaged","cursed","scourged"},
/*89*/{"\x96 civil war","\x9B \x98 \x99s","a \x9B disease","\x96 earthquakes","\x96 solar activity"},
/*8A*/{"its \x83 \x84","the \xB1 \x98 \x99","its inhabitants' \x9A \x85","\xA1","its \x8D \x8E"},
/*8B*/{"juice","brandy","water","brew","gargle blasters"},
/*8C*/{"\xB2","\xB1 \x99","\xB1 \xB2","\xB1 \x9B","\x9B \xB2"},
/*8D*/{"fabulous","exotic","hoopy","unusual","exciting"},
/*8E*/{"cuisine","night life","casinos","sit coms"," \xA1 "},
/*8F*/{"\xB0","The planet \xB0","The world \xB0","This planet","This world"},
/*90*/{"n unremarkable"," boring"," dull"," tedious"," revolting"},
/*91*/{"planet","world","place","little planet","dump"},
/*92*/{"wasp","moth","grub","ant","\xB2"},
/*93*/{"poet","arts graduate","yak","snail","slug"},
/*94*/{"tropical","dense","rain","impenetrable","exuberant"},
/*95*/{"funny","wierd","unusual","strange","peculiar"},
/*96*/{"frequent","occasional","unpredictable","dreadful","deadly"},
/*97*/{"\x82 \x81 for \x8A","\x82 \x81 for \x8A and \x8A","\x88 by \x89","\x82 \x81 for \x8A but \x88 by \x89","a\x90 \x91"},
/*98*/{"\x9B","mountain","edible","tree","spotted"},
/*99*/{"\x9F","\xA0","\x87oid","\x93","\x92"},
/*9A*/{"ancient","exceptional","eccentric","ingrained","\x95"},
/*9B*/{"killer","deadly","evil","lethal","vicious"},
/*9C*/{"parking meters","dust clouds","ice bergs","rock formations","volcanoes"},
/*9D*/{"plant","tulip","banana","corn","\xB2weed"},
/*9E*/{"\xB2","\xB1 \xB2","\xB1 \x9B","inhabitant","\xB1 \xB2"},
/*9F*/{"shrew","beast","bison","snake","wolf"},
/*A0*/{"leopard","cat","monkey","goat","fish"},
/*A1*/{"\x8C \x8B","\xB1 \x9F \xA2","its \x8D \xA0 \xA2","\xA3 \xA4","\x8C \x8B"},
/*A2*/{"meat","cutlet","steak","burgers","soup"},
/*A3*/{"ice","mud","Zero-G","vacuum","\xB1 ultra"},
/*A4*/{"hockey","cricket","karate","polo","tennis"},
};

static int gen_rnd_number(void)
{
    int a,x;
    x=(rnd_seed.a*2)&0xFF;
    a=x+rnd_seed.c;
    if(rnd_seed.a>127) a++;
    rnd_seed.a=a&0xFF; rnd_seed.c=x;
    a=a/256;
    x=rnd_seed.b;
    a=(a+x+rnd_seed.d)&0xFF;
    rnd_seed.b=a; rnd_seed.d=x;
    return a;
}

/* Goat Soup writes to a fixed buffer instead of printf */
static char gs_buf[256];
static int  gs_pos;

static void gs_char(char c){ if(gs_pos<254){ gs_buf[gs_pos++]=c; gs_buf[gs_pos]=0; } }
static void gs_str(const char *s){ while(*s) gs_char(*s++); }

static void goat_soup(const char *src, plansys *psy)
{
    for(;;){
        int c=(unsigned char)*src++;
        if(!c) break;
        if(c<0x80){ gs_char((char)c); }
        else if(c<=0xA4){
            int rnd=gen_rnd_number();
            goat_soup(desc_list[c-0x81].option[(rnd>=0x33)+(rnd>=0x66)+(rnd>=0x99)+(rnd>=0xCC)],psy);
        } else {
            switch(c){
            case 0xB0:{ /* planet name */
                int i=1;
                gs_char(psy->name[0]);
                while(psy->name[i]) gs_char(to_lower(psy->name[i++]));
            } break;
            case 0xB1:{ /* <name>ian */
                int i=1;
                gs_char(psy->name[0]);
                while(psy->name[i]){
                    if((psy->name[i+1]!=0)||((psy->name[i]!='E')&&(psy->name[i]!='I')))
                        gs_char(to_lower(psy->name[i]));
                    i++;
                }
                gs_str("ian");
            } break;
            case 0xB2:{ /* random name */
                int i, len=gen_rnd_number()&3;
                for(i=0;i<=len;i++){
                    int x=gen_rnd_number()&0x3e;
                    if(i==0) gs_char(pairs0[x]);
                    else     gs_char(to_lower(pairs0[x]));
                    gs_char(to_lower(pairs0[x+1]));
                }
            } break;
            default: break;
            }
        }
    }
}

static void make_description(plansys *psy, char *out)
{
    gs_buf[0]=0; gs_pos=0;
    rnd_seed=psy->goatsoupseed;
    goat_soup("\x8F is \x97.", psy);
    e_strcpy(out, gs_buf);
}

/* ════════════════════════════════════════════════════════════════════════════
   VDP text helpers
   The genesis.h SYS_disableInts / VDP_drawText API is SGDK 1.x standard.
   We use PAL0 for normal text, PAL1 for highlighted.
   ════════════════════════════════════════════════════════════════════════════ */

#define COL_NORMAL  0   /* palette index for normal text tiles  */
#define COL_HIGH    1   /* palette index for highlighted text   */

static void clrscreen(void)
{
    VDP_clearPlane(PLANE_BG, 1);
    VDP_clearPlane(PLANE_FG, 1);
}

/* Draw a string on the foreground plane */
static void draw(int col, int row, const char *s)
{
    VDP_drawText(s, col, row);
}

/* Draw highlighted text */
/* SGDK text uses PAL0 by default.  We mark high text by prefixing a
   colour-attribute change via VDP_setTextPalette.  Because this is
   global we switch before/after. */
static void draw_hi(int col, int row, const char *s)
{
    VDP_setTextPalette(PAL1);
    VDP_drawText(s, col, row);
    VDP_setTextPalette(PAL0);
}

/* Draw a right-aligned string within a field of width w */
static void draw_r(int right_col, int row, const char *s, int w)
{
    int len=e_strlen(s);
    int col=right_col-len;
    if(col<0) col=0;
    (void)w;
    draw(col, row, s);
}

/* ════════════════════════════════════════════════════════════════════════════
   UI state
   ════════════════════════════════════════════════════════════════════════════ */

static GameScreen  cur_screen;
static int         cursor;        /* selected item on current screen */
static int         trade_amount;  /* amount to buy/sell              */
static int         amount_editing;/* 1 while editing amount          */
static char        msg[64];       /* status message line             */

/* For jump screen: list of reachable planets */
#define MAX_LOCAL 24
static planetnum local_list[MAX_LOCAL];
static int       local_count;

static void build_local_list(void)
{
    int i; local_count=0;
    for(i=0;i<galsize && local_count<MAX_LOCAL;i++){
        uint d=distance(galaxy[i],galaxy[currentplanet]);
        if(d<=maxfuel && i!=currentplanet)
            local_list[local_count++]=i;
    }
}

/* ════════════════════════════════════════════════════════════════════════════
   Screen drawing routines
   ════════════════════════════════════════════════════════════════════════════ */

/* Header bar – always drawn */
static void draw_header(void)
{
    char buf[64];
    /* Planet name centred */
    int nlen=e_strlen(galaxy[currentplanet].name);
    draw(20-nlen/2, 0, galaxy[currentplanet].name);

    /* Cash */
    e_strcpy(buf,"CR:");
    cash2str(cash, buf+3);
    draw(0,0,buf);

    /* Fuel */
    e_strcpy(buf,"Fuel:");
    tenths2str(fuel, buf+5);
    draw(27,0,buf);

    /* Galaxy */
    buf[0]='G'; buf[1]='x'; buf[2]='0'+galaxynum; buf[3]=0;
    draw(37,0,buf);

    /* Separator */
    {
        char line[41];
        int j; for(j=0;j<40;j++) line[j]='-'; line[40]=0;
        draw(0,1,line);
    }
}

/* ── TITLE ─────────────────────────────────────────────────────────────── */
static void draw_title(void)
{
    draw_hi(11, 4, "TEXT  ELITE  1.5");
    draw(8,  6, "Genesis Port  (Ian Bell / D.Braben)");
    draw(10, 8, "B/A = select    START = ok");
    draw(6, 10, "You begin at Lave with 100.0 CR");
    draw(4, 12, "Press START to begin your trading career");
}

/* ── STATUS ────────────────────────────────────────────────────────────── */
static void draw_status(void)
{
    char buf[48];
    plansys *p=&galaxy[currentplanet];
    int row=2;

    draw_hi(0, row, "STATUS"); row+=2;
    draw(0,row,"Planet : "); draw(9,row,p->name); row++;
    draw(0,row,"Economy: "); draw(9,row,econnames[p->economy]); row++;
    draw(0,row,"Govt   : "); draw(9,row,govnames[p->govtype]); row++;

    e_strcpy(buf,"Tech   : ");
    uint2str(p->techlev+1, buf+9);
    draw(0,row,buf); row++;

    e_strcpy(buf,"Pop    : ");
    uint2str(p->population>>3, buf+9);
    e_strcpy(buf+e_strlen(buf)," Billion");
    draw(0,row,buf); row++;

    e_strcpy(buf,"Turnov : ");
    uint2str(p->productivity, buf+9);
    draw(0,row,buf); row++;

    /* Description */
    char desc[256];
    make_description(p, desc);
    /* word-wrap at col 38 */
    int c=0, r=row+1;
    draw(0,row,"Desc:");
    int dlen=e_strlen(desc), di=0;
    while(di<dlen && r<27){
        int end=di+38; if(end>dlen) end=dlen;
        /* find last space before end */
        int br=end;
        while(br>di && desc[br]!=' ') br--;
        if(br==di) br=end;
        char tmp[40]; int ti=0;
        while(di<br) tmp[ti++]=desc[di++];
        tmp[ti]=0;
        if(desc[di]==' ') di++;
        draw(0, r++, tmp);
    }
    (void)c;

    /* Cargo */
    row=r+1;
    if(row<26){
        e_strcpy(buf,"Cargo bay: ");
        uint t=0; for(int i=0;i<=lasttrade;i++) if(!commodities[i].units) t+=shipshold[i];
        uint2str(holdspace, buf+11);
        e_strcpy(buf+e_strlen(buf),"t free");
        draw(0,row,buf);
    }
}

/* ── MARKET ────────────────────────────────────────────────────────────── */
#define MKT_ROWS 17
static void draw_market(void)
{
    /* Columns: Name(12) Price(7) Avail(6) Hold(5) */
    draw_hi(0,2,"MARKET");
    draw(0,3,"Commodity    Price  Avail  Hold");
    draw(0,4,"------------ ------ ------ ----");

    char buf[8];
    for(int i=0;i<=lasttrade;i++){
        int row=5+i;
        /* Highlight selected row */
        if(i==cursor){
            /* draw a '>' marker */
            draw(0, row, ">");
        } else {
            draw(0, row, " ");
        }
        draw(1, row, commodities[i].name);      /* 12 chars */

        /* price in tenths */
        tenths2str(localmarket.price[i], buf);
        draw_r(20, row, buf, 6);

        /* availability */
        uint2str(localmarket.quantity[i], buf);
        e_strcpy(buf+e_strlen(buf), unitnames[commodities[i].units]);
        draw_r(27, row, buf, 6);

        /* in hold */
        uint2str(shipshold[i], buf);
        draw_r(33, row, buf, 4);
    }

    /* Amount selector */
    draw(0, 23, "Amount: ");
    char abuf[8]; uint2str((uint)trade_amount, abuf);
    draw(8, 23, abuf);
    draw(10,23,"   ");

    draw(0, 24, "UP/DN=item  LR=qty  A=buy  B=sell  C=status");
    /* msg */
    if(msg[0]) draw(0,25,msg);
}

/* ── LOCAL MAP ──────────────────────────────────────────────────────────── */
#define LOCAL_ROWS 20
static void draw_local(void)
{
    draw_hi(0,2,"LOCAL SYSTEMS  (fuel range)");
    draw(0,3,"  Planet         TL  Economy        Dist");
    draw(0,4,"  -------------- --- -------------- ----");

    for(int i=0;i<local_count && i<LOCAL_ROWS;i++){
        int row=5+i;
        plansys *p=&galaxy[local_list[i]];
        char buf[8];

        if(i==cursor) draw(0,row,">"); else draw(0,row," ");
        draw(1, row, p->name);
        uint2str(p->techlev+1, buf);
        draw(17, row, buf);
        draw(20, row, econnames[p->economy]);
        uint d=distance(*p, galaxy[currentplanet]);
        tenths2str(d, buf);
        draw(35, row, buf);
    }
    draw(0,26,"A=jump here   B/START=back");
    if(msg[0]) draw(0,27,msg);
}

/* ── INFO ───────────────────────────────────────────────────────────────── */
static void draw_info(void)
{
    if(local_count==0){ draw(0,12,"No reachable systems"); return; }
    plansys *p=&galaxy[local_list[cursor % local_count]];
    int row=2;
    draw_hi(0,row,"SYSTEM INFO"); row+=2;
    draw(0,row,p->name); row++;
    draw(0,row,econnames[p->economy]); row++;
    draw(0,row,govnames[p->govtype]); row++;
    char buf[16];
    e_strcpy(buf,"Tech: "); uint2str(p->techlev+1, buf+6);
    draw(0,row,buf); row++;
    e_strcpy(buf,"Pop : "); uint2str(p->population>>3, buf+6);
    e_strcpy(buf+e_strlen(buf),"B");
    draw(0,row,buf); row+=2;

    char desc[256]; make_description(p, desc);
    int dlen=e_strlen(desc), di=0;
    while(di<dlen && row<26){
        int end=di+38; if(end>dlen) end=dlen;
        int br=end;
        while(br>di && desc[br]!=' ') br--;
        if(br==di) br=end;
        char tmp[40]; int ti=0;
        while(di<br) tmp[ti++]=desc[di++];
        tmp[ti]=0;
        if(di<dlen && desc[di]==' ') di++;
        draw(0,row++,tmp);
    }
    draw(0,27,"A=jump  B=back");
}

/* ── HELP ───────────────────────────────────────────────────────────────── */
static void draw_help(void)
{
    draw_hi(0,2,"CONTROLS");
    int r=4;
    draw(0,r++,"Market screen:");
    draw(2,r++,"UP/DOWN  - select commodity");
    draw(2,r++,"LEFT/RIGHT - change buy amount");
    draw(2,r++,"A - buy  |  B - sell");
    draw(2,r++,"C - status screen");
    r++;
    draw(0,r++,"Local screen  (UP from market):");
    draw(2,r++,"UP/DOWN - select destination");
    draw(2,r++,"A - jump to system");
    draw(2,r++,"START - galactic hyperspace");
    r++;
    draw(0,r++,"X - this help screen");
    draw(0,r++,"Y - system info");
    draw(0,27,"B/START = back");
}

/* ════════════════════════════════════════════════════════════════════════════
   Master draw dispatcher
   ════════════════════════════════════════════════════════════════════════════ */

void drawscreen(void)
{
    clrscreen();
    switch(cur_screen){
    case SCREEN_TITLE:  draw_title();  return;
    case SCREEN_HELP:   draw_help();   break;
    case SCREEN_STATUS: draw_status(); break;
    case SCREEN_MARKET: draw_market(); break;
    case SCREEN_LOCAL:  draw_local();  break;
    case SCREEN_INFO:   draw_info();   break;
    default: break;
    }
    draw_header();
}

/* ════════════════════════════════════════════════════════════════════════════
   Input handling
   ════════════════════════════════════════════════════════════════════════════ */

static u16 prev_joy = 0;

/* Returns bitmask of newly-pressed buttons this frame */
static u16 joy_pressed(void)
{
    u16 joy = JOY_readJoypad(JOY_1);
    u16 pressed = joy & ~prev_joy;
    prev_joy = joy;
    return pressed;
}

static void set_msg(const char *s){ e_strcpy(msg, s); }
static void clr_msg(void){ msg[0]=0; }

static void do_jump_to(planetnum dest)
{
    if(dest==currentplanet){ set_msg("Already here!"); return; }
    uint d=distance(galaxy[dest],galaxy[currentplanet]);
    if(d>fuel){ set_msg("Not enough fuel!"); return; }
    fuel-=d;
    currentplanet=dest;
    localmarket=genmarket((uint)randbyte(), galaxy[dest]);
    build_local_list();
    cursor=0;
    set_msg("Jumped!");
}

static void handle_title(u16 p)
{
    if(p & BUTTON_START){
        cur_screen=SCREEN_MARKET;
        clr_msg();
        drawscreen();
    }
}

static void handle_market(u16 p)
{
    if(p & BUTTON_UP)   { if(cursor>0) cursor--; clr_msg(); drawscreen(); }
    if(p & BUTTON_DOWN) { if(cursor<lasttrade) cursor++; clr_msg(); drawscreen(); }
    if(p & BUTTON_LEFT) {
        if(trade_amount>1) trade_amount--;
        drawscreen();
    }
    if(p & BUTTON_RIGHT){
        if(trade_amount<63) trade_amount++;
        drawscreen();
    }
    if(p & BUTTON_A){ /* buy */
        uint got=gamebuy((uint)cursor,(uint)trade_amount);
        char buf[32]; e_strcpy(buf,"Bought ");
        uint2str(got, buf+7);
        e_strcpy(buf+e_strlen(buf)," "); e_strcpy(buf+e_strlen(buf), commodities[cursor].name);
        set_msg(buf); drawscreen();
    }
    if(p & BUTTON_B){ /* sell */
        uint sold=gamesell((uint)cursor,(uint)trade_amount);
        char buf[32]; e_strcpy(buf,"Sold ");
        uint2str(sold, buf+5);
        e_strcpy(buf+e_strlen(buf)," "); e_strcpy(buf+e_strlen(buf), commodities[cursor].name);
        set_msg(buf); drawscreen();
    }
    if(p & BUTTON_C){ cur_screen=SCREEN_STATUS; cursor=0; clr_msg(); drawscreen(); }
    if(p & BUTTON_X){ cur_screen=SCREEN_HELP;   cursor=0; clr_msg(); drawscreen(); }
    if(p & BUTTON_Y){ cur_screen=SCREEN_LOCAL;  cursor=0; clr_msg(); drawscreen(); }
    if(p & BUTTON_START){
        /* Buy fuel up to max */
        uint f=gamefuel((uint)(maxfuel-fuel));
        char buf[24]; e_strcpy(buf,"Fuel +"); tenths2str(f, buf+6);
        set_msg(buf); drawscreen();
    }
}

static void handle_local(u16 p)
{
    if(p & BUTTON_UP)   { if(cursor>0) cursor--; clr_msg(); drawscreen(); }
    if(p & BUTTON_DOWN) { if(cursor<local_count-1) cursor++; clr_msg(); drawscreen(); }
    if(p & BUTTON_A){
        if(local_count>0){
            do_jump_to(local_list[cursor]);
            cur_screen=SCREEN_MARKET; cursor=0;
        }
        drawscreen();
    }
    if(p & BUTTON_Y){ cur_screen=SCREEN_INFO; drawscreen(); }
    if(p & BUTTON_B || p & BUTTON_C){ cur_screen=SCREEN_MARKET; cursor=0; clr_msg(); drawscreen(); }
    if(p & BUTTON_START){
        /* Galactic hyperspace */
        galaxynum++;
        if(galaxynum==9) galaxynum=1;
        buildgalaxy(galaxynum);
        localmarket=genmarket(0, galaxy[currentplanet]);
        build_local_list();
        cursor=0;
        set_msg("Galactic jump!");
        cur_screen=SCREEN_MARKET;
        drawscreen();
    }
}

static void handle_info(u16 p)
{
    if(p & BUTTON_A){
        if(local_count>0){
            do_jump_to(local_list[cursor % local_count]);
            cur_screen=SCREEN_MARKET; cursor=0;
        }
        drawscreen();
    }
    if(p & BUTTON_B || p & BUTTON_START){ cur_screen=SCREEN_LOCAL; drawscreen(); }
}

static void handle_status(u16 p)
{
    if(p & BUTTON_B || p & BUTTON_START){ cur_screen=SCREEN_MARKET; cursor=0; clr_msg(); drawscreen(); }
}

static void handle_help(u16 p)
{
    if(p & BUTTON_B || p & BUTTON_START){ cur_screen=SCREEN_MARKET; drawscreen(); }
}

/* ════════════════════════════════════════════════════════════════════════════
   Public API
   ════════════════════════════════════════════════════════════════════════════ */

void elite_init(void)
{
    uint i;

    /* Copy trade names */
    for(i=0;i<=lasttrade;i++) e_strcpy(tradnames[i], commodities[i].name);

    mysrand(12345);
    nativerand=0;

    galaxynum=1; buildgalaxy(galaxynum);
    currentplanet=numforLave;
    localmarket=genmarket(0x00, galaxy[numforLave]);

    for(i=0;i<=lasttrade;i++) shipshold[i]=0;
    holdspace=20;     /* 20t cargo bay */
    cash=1000;        /* 100.0 CR  (stored ×10) */
    fuel=(uint)maxfuel;

    cursor=0;
    trade_amount=1;
    amount_editing=0;
    msg[0]=0;
    cur_screen=SCREEN_TITLE;
    prev_joy=0;

    build_local_list();
}

void elite_frame(void)
{
    u16 p = joy_pressed();
    if(!p) return;

    switch(cur_screen){
    case SCREEN_TITLE:  handle_title(p);  break;
    case SCREEN_MARKET: handle_market(p); break;
    case SCREEN_LOCAL:  handle_local(p);  break;
    case SCREEN_INFO:   handle_info(p);   break;
    case SCREEN_STATUS: handle_status(p); break;
    case SCREEN_HELP:   handle_help(p);   break;
    default: break;
    }
}
