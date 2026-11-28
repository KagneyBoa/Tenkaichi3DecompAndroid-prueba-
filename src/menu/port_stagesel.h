
#ifndef PORT_STAGESEL_H
#define PORT_STAGESEL_H

#include "common.h"

/*
 * Alternative stage selection menu, shared by the one-on-one select (src/menu/menu_c_e.c) and the team select
 * (src/menu/menu_e_b.c). The drawing and the pad navigation know nothing of either screen: each one fills a
 * PortStageSelView from its own work area every frame and calls the same PortStageSel_Draw / PortStageSel_Move.
 */

/* What the menu needs to draw itself. Filled by the screen that owns the stage choice (nothing here is kept). */
typedef struct PortStageSelView {
    s32 *ids;        /* stage ids of the list, in the game's order (CharSel.stageIds / TeamSel.stageIds) */
    s32 count;       /* entries of the list */
    s32 rows;        /* rows of the game's six-column stage grid */
    u32 *stagePack;  /* section 46 of the screen's pack: the texture lists of the stage thumbnails */
    s32 col, row;    /* the stage cursor in the game's six-column grid (col == musicCol: on the music entry) */
    s32 musicCol;    /* the column of that grid that is the music entry (6) */
    void *bg[2];     /* the stage pictures drawn behind (MTexRes *): new, old */
    s32 bgAlpha[2];  /* their opacity, 0..0x80 */
} PortStageSelView;

void PortStageSel_Begin(void);
void PortStageSel_End(void);
s32 PortStageSel_Enabled(void);
void PortStageSel_Draw(const PortStageSelView *view);

/*
 * Pad navigation of the four-column visual grid over the game's six-column list (flat index = col + row * 6,
 * the same number in both). dx / dy: one step. The screen's own chips are refreshed through refreshChips
 * (CharSel_SetStageChips / TeamSel_SetStageChips) after a move that needs it.
 * Returns: 0 no movement, 1 moved to another stage, 2 entered the music entry, 3 left the music entry.
 */
s32 PortStageSel_Move(s32 *ids, s32 count, s32 *col, s32 *row, s32 musicCol, s32 dx, s32 dy,
                      void (*refreshChips)(void));

/* Widescreen presentation while the alternative stage menu is active. */
void PortStageSel_SetAspectActive(s32 active);
s32 PortStageSel_AspectActive(void);

#endif
