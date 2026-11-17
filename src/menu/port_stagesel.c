
#include "port_stagesel.h"
#include "menu/menu_d.h"
#include "menu/port_screen.h"

/* Settings functions provided by port/src/plat_settings.c. */
extern int Port_Setting(const char *name, int def);
extern void Port_SettingSave(const char *name, int value);
extern void Port_GsMarker(s32 effect);

static s32 sEnabled;

static void StageSel_Rect(f32 x0, f32 y0, f32 x1, f32 y1, u32 color) {
    Ps_ScreenQuad(
        x0, y0,
        x1, y0,
        x0, y1,
        x1, y1,
        color, color, color, color
    );
}

static void StageSel_Frame(f32 x0, f32 y0, f32 x1, f32 y1, u32 color) {
    StageSel_Rect(x0, y0, x1, y0 + 1.0f, color);
    StageSel_Rect(x0, y1 - 1.0f, x1, y1, color);
    StageSel_Rect(x0, y0, x0 + 1.0f, y1, color);
    StageSel_Rect(x1 - 1.0f, y0, x1, y1, color);
}

/* Read the selected menu when the stage selector begins. */
void PortStageSel_Begin(void) {
    sEnabled = !Port_Setting("stage_menu_classic", 0);
}

/* End the alternative menu session. */
void PortStageSel_End(void) {
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

/* First visual version: preview and the 6 x 6 gallery layout. */
void PortStageSel_Draw(void) {
    s32 row;
    s32 col;
    s32 selected;

    if (!sEnabled || gCharSel == NULL || gCharSel->stage == NULL) {
        return;
    }

    /* Draw across the full screen, including widescreen areas. */
    Port_GsMarker(0x24);

    /* Preserve the original stage preview and its cross-fade. */
    if (gCharSel->bg[1] != NULL) {
        Sprite_DrawPicture(gCharSel->bg[1], 0, 0, gCharSel->bgAlpha[1]);
    }

    if (gCharSel->bg[0] != NULL) {
        Sprite_DrawPicture(gCharSel->bg[0], 0, 0, gCharSel->bgAlpha[0]);
    }

    /* Blue tint over the preview. */
    StageSel_Rect(
        0, 0, 512, 448,
        Ps_Rgba(3, 15, 43, 55)
    );

    /* Header. */
    StageSel_Rect(
        0, 0, 512, 43,
        Ps_Rgba(3, 19, 54, 220)
    );

    StageSel_Rect(
        0, 42, 512, 44,
        Ps_Rgba(85, 177, 255, 170)
    );

    /* Bottom gallery panel. */
    StageSel_Rect(
        0, 286, 512, 448,
        Ps_Rgba(3, 12, 34, 238)
    );

    StageSel_Rect(
        12, 285, 500, 287,
        Ps_Rgba(48, 137, 224, 180)
    );

    /* Current selection. Column 6 is reserved for music. */
    selected = -1;

    if (gCharSel->stage->col >= 0 &&
        gCharSel->stage->col < 6 &&
        gCharSel->stage->row >= 0 &&
        gCharSel->stage->row < 6) {
        selected = gCharSel->stage->col +
                   gCharSel->stage->row * 6;
    }

    /* Six columns and six rows. Thumbnails will be connected next. */
    for (row = 0; row < 6; row++) {
        for (col = 0; col < 6; col++) {
            f32 x = 18.0f + (f32)col * 79.0f;
            f32 y = 300.0f + (f32)row * 22.0f;
            s32 index = col + row * 6;

            if (index == selected) {
                StageSel_Rect(
                    x, y, x + 73.0f, y + 18.0f,
                    Ps_Rgba(35, 112, 205, 220)
                );

                StageSel_Frame(
                    x, y, x + 73.0f, y + 18.0f,
                    Ps_Rgba(130, 205, 255, 230)
                );
            } else {
                StageSel_Rect(
                    x, y, x + 73.0f, y + 18.0f,
                    Ps_Rgba(13, 42, 79, 210)
                );

                StageSel_Frame(
                    x, y, x + 73.0f, y + 18.0f,
                    Ps_Rgba(63, 117, 174, 150)
                );
            }
        }
    }

    Port_GsMarker(0x25);
}
