
#include "port_stagesel.h"
#include "menu/menu_d.h"
#include "menu/port_screen.h"
#include "battle/col_c.h"

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
static f32 sCanvasW = 512.0f;

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
    sEnabled = !Port_Setting("stage_menu_classic", 0);
    sAspectActive = 0;
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
    f32 margin;
    f32 gap;
    f32 cellW;

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
    cellW = (sCanvasW - margin * 2.0f - gap * 5.0f) / 6.0f;

    Port_GsMarker(0x24);

    /* Original stage preview and its cross-fade. */
    if (gCharSel->bg[1] != NULL) {
        Sprite_DrawPicture(gCharSel->bg[1], 0, 0, gCharSel->bgAlpha[1]);
    }

    if (gCharSel->bg[0] != NULL) {
        Sprite_DrawPicture(gCharSel->bg[0], 0, 0, gCharSel->bgAlpha[0]);
    }

    /* Blue tint over the preview. */
    StageSel_Rect(
        0, 0, sCanvasW, 448,
        Ps_Rgba(3, 15, 43, 55)
    );

    /* Header. */
    StageSel_Rect(
        0, 0, sCanvasW, 43,
        Ps_Rgba(3, 19, 54, 220)
    );

    StageSel_Rect(
        0, 42, sCanvasW, 44,
        Ps_Rgba(85, 177, 255, 170)
    );

    /* Bottom gallery panel. */
    StageSel_Rect(
        0, 286, sCanvasW, 448,
        Ps_Rgba(3, 12, 34, 238)
    );

    StageSel_Rect(
        margin - 6.0f, 285, sCanvasW - margin + 6.0f, 287,
        Ps_Rgba(48, 137, 224, 180)
    );

    /* Current selection. */
    selected = -1;

    if (gCharSel->stage->col >= 0 &&
        gCharSel->stage->col < 6 &&
        gCharSel->stage->row >= 0 &&
        gCharSel->stage->row < 6) {
        selected = gCharSel->stage->col +
                   gCharSel->stage->row * 6;
    }

    /* Six columns and six rows, with responsive cell widths. */
    /* Six columns and six rows, with stage thumbnails. */
for (row = 0; row < 6; row++) {
    for (col = 0; col < 6; col++) {
        f32 x = margin + (f32)col * (cellW + gap);
        f32 y = 300.0f + (f32)row * 22.0f;
        s32 index = col + row * 6;
        s32 id = gCharSel->stageIds[index];

        if (index == selected) {
            StageSel_Rect(
                x, y, x + cellW, y + 18.0f,
                Ps_Rgba(35, 112, 205, 220)
            );

            StageSel_Frame(
                x, y, x + cellW, y + 18.0f,
                Ps_Rgba(130, 205, 255, 230)
            );
        } else {
            StageSel_Rect(
                x, y, x + cellW, y + 18.0f,
                Ps_Rgba(13, 42, 79, 210)
            );

            StageSel_Frame(
                x, y, x + cellW, y + 18.0f,
                Ps_Rgba(63, 117, 174, 150)
            );
        }

        StageSel_DrawThumbnail(
            id,
            x + 1.0f,
            y + 1.0f,
            cellW - 2.0f,
            16.0f
        );
    }
}
    /*
     * Music selector: centered below the stage name.
     * Column 6 is the existing music entry.
     */
    {
        s32 musicSelected = (gCharSel->stage->col == 6);
        FontStyle saved = gFontStyle;

        /* Background and selection highlight. */
        StageSel_Rect(
            145.0f, 254.0f, 367.0f, 284.0f,
            musicSelected
                ? Ps_Rgba(35, 112, 205, 245)
                : Ps_Rgba(13, 42, 79, 225)
        );

        StageSel_Frame(
            145.0f, 254.0f, 367.0f, 284.0f,
            musicSelected
                ? Ps_Rgba(130, 205, 255, 255)
                : Ps_Rgba(63, 117, 174, 190)
        );

        /* Music title. */
        gFontStyle.align = FONT_ALIGN_CENTER;
        gFontStyle.color = 0xFFFFFFFF;
        gFontStyle.shadowMode = FONT_SHADOW_DROP;
        gFontStyle.shadowColor = 0xC0000000;
        gFontStyle.shadowDx = 1;
        gFontStyle.shadowDy = 1;

        Font_PrintAsciiAt(256, 255, "MUSICA");

        gFontStyle = saved;

        /* Move the existing song-name overlay into this panel. */
        gUiSongX = 145;
        gUiSongY = 269;
        gUiSongW = 222;
        gUiSongH = 14;

        Font_FlushAll();
    }
    Port_GsMarker(0x25);
        }
