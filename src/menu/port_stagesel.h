
#ifndef PORT_STAGESEL_H
#define PORT_STAGESEL_H

#include "common.h"

/* Alternative stage selection menu. */
void PortStageSel_Begin(void);
void PortStageSel_End(void);
s32 PortStageSel_Enabled(void);
void PortStageSel_Draw(void);

/* Widescreen presentation while the alternative stage menu is active. */
void PortStageSel_SetAspectActive(s32 active);
s32 PortStageSel_AspectActive(void);

#endif
