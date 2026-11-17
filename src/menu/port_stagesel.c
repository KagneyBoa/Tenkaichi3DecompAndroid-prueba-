

#include "port_stagesel.h"

/* Settings functions provided by port/src/plat_settings.c. */
extern int Port_Setting(const char *name, int def);
extern void Port_SettingSave(const char *name, int value);

static s32 sEnabled;

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
