
#include "port_stagesel.h"
#include "menu/menu_d.h"
#include "menu/port_charsel.h"
#include "menu/port_screen.h"
#include "battle/col_c.h"
#include "menu/tbo_art.h"
#include <time.h>

extern void Font_PrintAsciiAt(s32 x, s32 y, char *str);
extern void Font_FlushAll(void);
extern FontStyle gFontStyle;

extern volatile int gUiSongX, gUiSongY, gUiSongW, gUiSongH;
#define SYS_LOADING_NO_PROGRESS_EXTERN
#define SYS_SPRITE_NO_DRAWPICTURE_EXTERN
#include "sys/sprite.h"
#undef SYS_SPRITE_NO_DRAWPICTURE_EXTERN
#undef SYS_LOADING_NO_PROGRESS_EXTERN

/* Settings functions provided by port/src/plat_settings.c. */
extern int Port_Setting(const char *name, int def);
extern void Port_SettingSave(const char *name, int value);
extern void Port_GsMarker(s32 effect);
extern int Port_AspectMilli(void);

static s32 sEnabled;
static s32 sAspectActive;
static s32 sVisiblePage = 0;
static f32 sCanvasW = 512.0f;
static u32 sEggRandomState;
static s32 sCornerRightArt = TBO_STAGE_CORNER_RIGHT;
static u32 StageSel_Random(void) {
    if (sEggRandomState == 0) {
        sEggRandomState = (u32)time(NULL) ^ (u32)clock() ^ 0xA53C91E7u;
    }

    sEggRandomState ^= sEggRandomState << 13;
    sEggRandomState ^= sEggRandomState >> 17;
    sEggRandomState ^= sEggRandomState << 5;

    return sEggRandomState;
}

/* Convert the responsive design coordinates to the game's 512-wide buffer. */
static f32 StageSel_X(f32 x) {
    return x * 512.0f / sCanvasW;
}

static void StageSel_Rect(f32 x0, f32 y0, f32 x1, f32 y1, u32 color) {
    Ps_ScreenQuad(
        StageSel_X(x0), y0,
        StageSel_X(x1), y0,
        StageSel_X(x0), y1,
        StageSel_X(x1), y1,
        color, color, color, color
    );
}

static void StageSel_Frame(f32 x0, f32 y0, f32 x1, f32 y1, u32 color) {
    StageSel_Rect(x0, y0, x1, y0 + 1.0f, color);
    StageSel_Rect(x0, y1 - 1.0f, x1, y1, color);
    StageSel_Rect(x0, y0, x0 + 1.0f, y1, color);
    StageSel_Rect(x1 - 1.0f, y0, x1, y1, color);
}
/* Draw one stage thumbnail, preserving its original texture proportions. */
static void StageSel_DrawThumbnail(s32 id, f32 x, f32 y, f32 w, f32 h) {
    s32 icon;
    s32 texW;
    s32 texH;
    s32 drawW;
    s32 drawH;
    s32 drawX;
    s32 drawY;
    u64 tex0;
    MTexRes *res;
    LoadSprite sprites[2];

    if (id == 0x3E) {
        icon = 37;
    } else if (id == 0x3F) {
        icon = 38;
    } else if (id >= 0x24) {
        icon = 1 + (id - 0x24) % 36;
    } else {
        icon = id + 1;
    }

    res = (MTexRes *)MPACK_AT(gCharSel->stagePack, icon);

    tex0 = ((SpriteTex *)res->tex)->tex0;

    texW = 1 << ((tex0 >> 26) & 0xF);
    texH = 1 << ((tex0 >> 30) & 0xF);

    drawH = (s32)h;
    drawW = drawH * texW / texH;

    if (drawW > (s32)w) {
        drawW = (s32)w;
        drawH = drawW * texH / texW;
    }

    drawX = (s32)(x + (w - drawW) * 0.5f);
    drawY = (s32)(y + (h - drawH) * 0.5f);

    sprites[0].flags = LOAD_SPR_DRAW;
    sprites[0].x0 = 0;
    sprites[0].y0 = 0;
    sprites[0].x1 = (s32)StageSel_X(drawW);
    sprites[0].y1 = drawH;
    sprites[0].u0 = 0;
    sprites[0].v0 = 0;
    sprites[0].u1 = texW;
    sprites[0].v1 = texH;
    sprites[0].tex = 0;
    sprites[0].r = 255;
    sprites[0].g = 255;
    sprites[0].b = 255;
    sprites[0].a = 255;

    sprites[1].flags = LOAD_SPR_END;
    sprites[1].x0 = 0;
    sprites[1].y0 = 0;
    sprites[1].x1 = 0;
    sprites[1].y1 = 0;
    sprites[1].u0 = 0;
    sprites[1].v0 = 0;
    sprites[1].u1 = 0;
    sprites[1].v1 = 0;
    sprites[1].tex = 0;
    sprites[1].r = 0;
    sprites[1].g = 0;
    sprites[1].b = 0;
    sprites[1].a = 0;

    Sprite_DrawList(
        (SpriteRes *)res,
        (s32)StageSel_X(drawX),
        drawY,
        sprites
    );
}
/* Read the selected menu when the stage selector begins. */
void PortStageSel_Begin(void) {
    u32 roll;

    sEnabled = !Port_Setting("stage_menu_classic", 0);
    sAspectActive = 0;
    sVisiblePage = 0;

    roll = StageSel_Random() % 100;

    if (roll < 5) {
        sCornerRightArt = TBO_STAGE_CORNER_RIGHT_EGG_K;
    } else if (roll < 10) {
        sCornerRightArt = TBO_STAGE_CORNER_RIGHT_EGG_S;
    } else {
        sCornerRightArt = TBO_STAGE_CORNER_RIGHT;
    }
}

/* End the alternative menu session. */
void PortStageSel_End(void) {
    sAspectActive = 0;
}

/* Is the alternative menu selected? */
s32 PortStageSel_Enabled(void) {
    return sEnabled;
}

/* Change and save the user's choice. */
void PortStageSel_SetEnabled(s32 enabled) {
    sEnabled = enabled != 0;
    Port_SettingSave("stage_menu_classic", !sEnabled);
}

/* Tell the port when this menu needs its own aspect ratio. */
void PortStageSel_SetAspectActive(s32 active) {
    sAspectActive = sEnabled && (active != 0);
}

s32 PortStageSel_AspectActive(void) {
    return sAspectActive;
}

/* Responsive stage selection layout. */
void PortStageSel_Draw(void) {
    s32 row;
    s32 col;
    s32 selected;
    s32 pageStart;
    f32 margin;
    f32 gap;
    f32 cellW;
    f32 gridX;
    f32 gridWidth;

    if (!sEnabled || gCharSel == NULL || gCharSel->stage == NULL) {
        return;
    }

    /*
     * The virtual canvas grows horizontally with the selected aspect ratio.
     * Individual shapes are compensated by StageSel_X so they keep
     * their proportions when presented on a widescreen display.
     */
    sCanvasW = 512.0f * (f32)Port_AspectMilli() / 1333.0f;

    margin = 18.0f;
gap = 10.0f;

/* Compact centered grid. */
gridWidth = sCanvasW * 0.60f;
gridX = (sCanvasW - gridWidth) * 0.5f;
cellW = (gridWidth - gap * 3.0f) / 4.0f;

    Port_GsMarker(0x24);



    /* Blue tint over the preview. */
    StageSel_Rect(
        0, 0, sCanvasW, 448,
        Ps_Rgba(3, 15, 43, 255)
    );
        /* Full-screen artwork behind the stage preview and selector. */
    Ps_SpriteScreen(
        TBO_STAGE_MENU_BG,
        0, 0, 512, 448,
        0.0f, 0.0f, 1.0f, 1.0f,
        PS_PLAIN, 0x80
    );
    /* Draw the selected stage picture using the original crossfade. */
for (row = 0; row < 2; row++) {
    if (gCharSel->bg[row] != NULL &&
        gCharSel->bgAlpha[row] > 0) {
        Sprite_DrawPicture(
            gCharSel->bg[row],
            0,
            0,
            gCharSel->bgAlpha[row]
        );
    }
}
    /* Blend the stage preview into the decorative background. */
    Ps_SpriteScreen(
        TBO_STAGE_MENU_EDGE,
        0, 0, 512, 448,
        0.0f, 0.0f, 1.0f, 1.0f,
        PS_PLAIN, 0x80
    );
    /* Header. */
    
    /* Draw the fighters behind the stage selection interface. */
PortCharSel_DrawStage();

    
    /* Current selection. */
    selected = -1;

    if (gCharSel->stage->col >= 0 &&
    gCharSel->stage->col < 6 &&
    gCharSel->stage->row >= 0 &&
    gCharSel->stage->row < gCharSel->stageRows) {
    selected = gCharSel->stage->col +
               gCharSel->stage->row * 6;

    /* Each page displays twelve stages. */
    sVisiblePage = selected / 12;
}

    pageStart = sVisiblePage * 12;

    /* Four columns and three visible rows. */
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 4; col++) {
            f32 x = gridX + (f32)col * (cellW + gap);
f32 y = 294.0f + (f32)row * 50.0f;
            s32 index = pageStart + col + row * 4;
s32 id;

if (index >= gCharSel->stageCount) {
    continue;
}

id = gCharSel->stageIds[index];

            if (index == selected) {
                StageSel_Rect(
                    x, y, x + cellW, y + 44.0f,
                    Ps_Rgba(35, 112, 205, 220)
                );

                StageSel_Frame(
                    x, y, x + cellW, y + 44.0f,
                    Ps_Rgba(130, 205, 255, 230)
                );
            } else {
                StageSel_Rect(
                    x, y, x + cellW, y + 44.0f,
                    Ps_Rgba(13, 42, 79, 210)
                );

                StageSel_Frame(
                    x, y, x + cellW, y + 44.0f,
                    Ps_Rgba(63, 117, 174, 150)
                );
            }

            StageSel_DrawThumbnail(
    id,
    x + 2.0f,
    y + 2.0f,
    cellW - 4.0f,
    40.0f
);
        }
    }

        
    
    /*
     * Music selector panel.
     */
    {
        s32 musicSelected = (gCharSel->stage->col == 6);
        f32 musicX = (sCanvasW - 222.0f) * 0.5f;
                        gUiSongX = (s32)(musicX * 512.0f / sCanvasW);
        gUiSongY = 262;
gUiSongW = (s32)(222.0f * 512.0f / sCanvasW);
gUiSongH = 14;

        StageSel_Rect(
            musicX, 254.0f, musicX + 222.0f, 284.0f,
            musicSelected
                ? Ps_Rgba(35, 112, 205, 245)
                : Ps_Rgba(13, 42, 79, 225)
        );

        StageSel_Frame(
            musicX, 254.0f, musicX + 222.0f, 284.0f,
            musicSelected
                ? Ps_Rgba(130, 205, 255, 255)
                : Ps_Rgba(63, 117, 174, 190)
        );
    }


            /* Top decorations: preserve proportions on every aspect ratio. */
Ps_SpriteScreen(
    TBO_STAGE_CORNER_LEFT,
    StageSel_X(0.0f), 0.0f,
    StageSel_X(225.0f), 159.1f,
    0.0f, 0.0f, 1.0f, 1.0f,
    PS_PLAIN, 0x80
);

Ps_SpriteScreen(
    TBO_STAGE_CORNER_RIGHT,
    StageSel_X(sCanvasW - 158.4f), 0.0f,
    StageSel_X(sCanvasW), 160.3f,
    0.0f, 0.0f, 1.0f, 1.0f,
    PS_PLAIN, 0x80
);

    Port_GsMarker(0x25);
}
