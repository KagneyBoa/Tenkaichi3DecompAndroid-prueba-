
#include "port_stagesel.h"

#include <stdlib.h>

static s32 sEnabled;
static s32 sActive;

/* Read the selected menu when the stage selector begins. */
void PortStageSel_Begin(void) {
    const char *env = getenv("BT3_STAGESEL_CLASSIC");

    sEnabled = !(env != NULL ? atoi(env) : Port_Setting("stage_menu_classic", 0));
    sActive = 0;
}

/* End the alternative menu session. */
void PortStageSel_End(void) {
    sActive = 0;
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
