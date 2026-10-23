#ifndef CUTSCENE_TEXT_H
#define CUTSCENE_TEXT_H

#include <PR/ultratypes.h>

extern s16 gCutsceneMsgXOffset;
extern s16 gCutsceneMsgYOffset;

void reset_cutscene_msg_fade(void);
void dl_rgba16_begin_cutscene_msg_fade(void);
void dl_rgba16_stop_cutscene_msg_fade(void);
void set_cutscene_message(s16 xOffset, s16 yOffset, s16 msgIndex, s16 msgDuration);
void do_cutscene_handler(void);
void print_peach_letter_message(void);

#endif // CUTSCENE_TEXT_H
