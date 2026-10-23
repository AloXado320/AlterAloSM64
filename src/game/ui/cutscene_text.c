// Faithful split of the former src/game/ingame_menu.c

#include <ultra64.h>

#include "actors/common1.h"
#include "audio/external.h"
#include "course_table.h"
#include "dialog_ids.h"
#include "engine/math_util.h"
#include "eu_translation.h"
#include "game/area.h"
#include "game/camera.h"
#include "game/game_init.h"
#include "game/level_update.h"
#include "game/memory.h"
#include "game/print.h"
#include "game/save_file.h"
#include "game/segment2.h"
#include "game/segment7.h"
#include "gfx_dimensions.h"
#include "levels/castle_grounds/header.h"
#include "seq_ids.h"
#include "sm64.h"
#include "types.h"
#include "text_strings.h"

#include "ui.h"
#include "dialog.h"
#include "font_string.h"
#include "cutscene_text.h"

#ifdef VERSION_EU
#undef LANGUAGE_FUNCTION
#define LANGUAGE_FUNCTION gInGameLanguage
#endif

s16 gCutsceneMsgXOffset;
s16 gCutsceneMsgYOffset;
#ifdef VERSION_CN
u8 gEndCutsceneStrEn0[] = { TEXT_EMPTY_STRING };
u8 gEndCutsceneStrEn1[] = { TEXT_EMPTY_STRING };
u8 gEndCutsceneStrEn2[] = { TEXT_EMPTY_STRING };
u8 gEndCutsceneStrEn3[] = { TEXT_EMPTY_STRING };
u8 gEndCutsceneStrEn4[] = { TEXT_EMPTY_STRING };
u8 gEndCutsceneStrEn5[] = { TEXT_EMPTY_STRING };
u8 gEndCutsceneStrEn6[] = { TEXT_EMPTY_STRING };
u8 gEndCutsceneStrEn7[] = { TEXT_EMPTY_STRING };
u8 gEndCutsceneStrEn8[] = { TEXT_EMPTY_STRING };
#else
u8 gEndCutsceneStrEn0[] = { TEXT_FILE_MARIO_EXCLAMATION };
u8 gEndCutsceneStrEn1[] = { TEXT_POWER_STARS_RESTORED };
u8 gEndCutsceneStrEn2[] = { TEXT_THANKS_TO_YOU };
u8 gEndCutsceneStrEn3[] = { TEXT_THANK_YOU_MARIO };
u8 gEndCutsceneStrEn4[] = { TEXT_SOMETHING_SPECIAL };
u8 gEndCutsceneStrEn5[] = { TEXT_LISTEN_EVERYBODY };
u8 gEndCutsceneStrEn6[] = { TEXT_LETS_HAVE_CAKE };
u8 gEndCutsceneStrEn7[] = { TEXT_FOR_MARIO };
u8 gEndCutsceneStrEn8[] = { TEXT_FILE_MARIO_QUESTION };
#endif

u8 *gEndCutsceneStringsEn[] = {
    gEndCutsceneStrEn0,
    gEndCutsceneStrEn1,
    gEndCutsceneStrEn2,
    gEndCutsceneStrEn3,
    gEndCutsceneStrEn4,
    gEndCutsceneStrEn5,
    gEndCutsceneStrEn6,
    gEndCutsceneStrEn7,
    // This [8] string is actually unused. In the cutscene handler, the developers do not
    // set the 8th one, but use the first string again at the very end, so Peach ends up
    // saying "Mario!" twice. It is likely that she was originally meant to say "Mario?" at
    // the end but the developers changed their mind, possibly because the line recorded
    // sounded more like an exclamation than a question.
    gEndCutsceneStrEn8,
    NULL
};

#ifdef VERSION_EU
u8 gEndCutsceneStrFr0[] = { TEXT_FILE_MARIO_EXCLAMATION };
u8 gEndCutsceneStrFr1[] = { TEXT_POWER_STARS_RESTORED_FR };
u8 gEndCutsceneStrFr2[] = { TEXT_THANKS_TO_YOU_FR };
u8 gEndCutsceneStrFr3[] = { TEXT_THANK_YOU_MARIO_FR };
u8 gEndCutsceneStrFr4[] = { TEXT_SOMETHING_SPECIAL_FR };
u8 gEndCutsceneStrFr5[] = { TEXT_COME_ON_EVERYBODY_FR };
u8 gEndCutsceneStrFr6[] = { TEXT_LETS_HAVE_CAKE_FR };
u8 gEndCutsceneStrFr7[] = { TEXT_FOR_MARIO_FR };
u8 gEndCutsceneStrFr8[] = { TEXT_FILE_MARIO_QUESTION };

u8 *gEndCutsceneStringsFr[] = {
    gEndCutsceneStrFr0,
    gEndCutsceneStrFr1,
    gEndCutsceneStrFr2,
    gEndCutsceneStrFr3,
    gEndCutsceneStrFr4,
    gEndCutsceneStrFr5,
    gEndCutsceneStrFr6,
    gEndCutsceneStrFr7,
    gEndCutsceneStrFr8,
    NULL
};

u8 gEndCutsceneStrDe0[] = { TEXT_FILE_MARIO_EXCLAMATION };
u8 gEndCutsceneStrDe1[] = { TEXT_POWER_STARS_RESTORED_DE };
u8 gEndCutsceneStrDe2[] = { TEXT_THANKS_TO_YOU_DE };
u8 gEndCutsceneStrDe3[] = { TEXT_THANK_YOU_MARIO_DE };
u8 gEndCutsceneStrDe4[] = { TEXT_SOMETHING_SPECIAL_DE };
u8 gEndCutsceneStrDe5[] = { TEXT_COME_ON_EVERYBODY_DE };
u8 gEndCutsceneStrDe6[] = { TEXT_LETS_HAVE_CAKE_DE };
u8 gEndCutsceneStrDe7[] = { TEXT_FOR_MARIO_DE };
u8 gEndCutsceneStrDe8[] = { TEXT_FILE_MARIO_QUESTION };

u8 *gEndCutsceneStringsDe[] = {
    gEndCutsceneStrDe0,
    gEndCutsceneStrDe1,
    gEndCutsceneStrDe2,
    gEndCutsceneStrDe3,
    gEndCutsceneStrDe4,
    gEndCutsceneStrDe5,
    gEndCutsceneStrDe6,
    gEndCutsceneStrDe7,
    gEndCutsceneStrDe8,
    NULL
};
#endif
u16 gCutsceneMsgFade = 0;
s16 gCutsceneMsgIndex = -1;
s16 gCutsceneMsgDuration = -1;
s16 gCutsceneMsgTimer = 0;
void reset_cutscene_msg_fade(void) {
    gCutsceneMsgFade = 0;
}

void dl_rgba16_begin_cutscene_msg_fade(void) {
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gCutsceneMsgFade);
}

void dl_rgba16_stop_cutscene_msg_fade(void) {
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    if (gCutsceneMsgFade < 250) {
        gCutsceneMsgFade += 25;
    } else {
        gCutsceneMsgFade = 255;
    }
}
void set_cutscene_message(s16 xOffset, s16 yOffset, s16 msgIndex, s16 msgDuration) {
    // is message done printing?
    if (gCutsceneMsgIndex == -1) {
        gCutsceneMsgIndex = msgIndex;
        gCutsceneMsgDuration = msgDuration;
        gCutsceneMsgTimer = 0;
        gCutsceneMsgXOffset = xOffset;
        gCutsceneMsgYOffset = yOffset;
        gCutsceneMsgFade = 0;
    }
}

void do_cutscene_handler(void) {
    s16 x;

    // is a cutscene playing? do not perform this handler's actions if so.
    if (gCutsceneMsgIndex == -1) {
        return;
    }

    create_dl_ortho_matrix();

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gCutsceneMsgFade);

#ifdef VERSION_EU
    switch (eu_get_language()) {
        case LANGUAGE_ENGLISH:
            x = get_str_x_pos_from_center(gCutsceneMsgXOffset, gEndCutsceneStringsEn[gCutsceneMsgIndex], 10.0f);
            print_generic_string(x, 240 - gCutsceneMsgYOffset, gEndCutsceneStringsEn[gCutsceneMsgIndex]);
            break;
        case LANGUAGE_FRENCH:
            x = get_str_x_pos_from_center(gCutsceneMsgXOffset, gEndCutsceneStringsFr[gCutsceneMsgIndex], 10.0f);
            print_generic_string(x, 240 - gCutsceneMsgYOffset, gEndCutsceneStringsFr[gCutsceneMsgIndex]);
            break;
        case LANGUAGE_GERMAN:
            x = get_str_x_pos_from_center(gCutsceneMsgXOffset, gEndCutsceneStringsDe[gCutsceneMsgIndex], 10.0f);
            print_generic_string(x, 240 - gCutsceneMsgYOffset, gEndCutsceneStringsDe[gCutsceneMsgIndex]);
            break;
    }
#else
    // get the x coordinate of where the cutscene string starts.
    x = get_str_x_pos_from_center(gCutsceneMsgXOffset, gEndCutsceneStringsEn[gCutsceneMsgIndex], 10.0f);
    print_generic_string(x, 240 - gCutsceneMsgYOffset, gEndCutsceneStringsEn[gCutsceneMsgIndex]);
#endif

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    // if the timing variable is less than 5, increment
    // the fade until we are at full opacity.
    if (gCutsceneMsgTimer < 5) {
        gCutsceneMsgFade += 50;
    }

    // if the cutscene frame length + the fade-in counter is
    // less than the timer, it means we have exceeded the
    // time that the message is supposed to remain on
    // screen. if (message_duration = 50) and (msg_timer = 55)
    // then after the first 5 frames, the message will remain
    // on screen for another 50 frames until it starts fading.
    if (gCutsceneMsgDuration + 5 < gCutsceneMsgTimer) {
        gCutsceneMsgFade -= 50;
    }

    // like the first check, it takes 5 frames to fade out, so
    // perform a + 10 to account for the earlier check (10-5=5).
    if (gCutsceneMsgDuration + 10 < gCutsceneMsgTimer) {
        gCutsceneMsgIndex = -1;
        gCutsceneMsgFade = 0;
        gCutsceneMsgTimer = 0;
        return;
    }

    gCutsceneMsgTimer++;
}
#ifdef VERSION_JP
    #define PEACH_MESSAGE_TIMER 170
#else
    #define PEACH_MESSAGE_TIMER 250
#endif

#if defined(VERSION_JP) || defined(VERSION_SH)
    #define STR_X 53
    #define STR_Y 136
#else
    #define STR_X 38
    #define STR_Y 142
#endif

// "Dear Mario" message handler
void print_peach_letter_message(void) {
    void **dialogTable;
    struct DialogEntry *dialog;
    u8 *str;

#ifdef VERSION_EU
    gInGameLanguage = eu_get_language();
    switch (gInGameLanguage) {
        case LANGUAGE_ENGLISH:
            dialogTable = segmented_to_virtual(dialog_table_eu_en);
            break;
        case LANGUAGE_FRENCH:
            dialogTable = segmented_to_virtual(dialog_table_eu_fr);
            break;
        case LANGUAGE_GERMAN:
            dialogTable = segmented_to_virtual(dialog_table_eu_de);
            break;
    }
#else
    dialogTable = segmented_to_virtual(seg2_dialog_table);
#endif
    dialog = segmented_to_virtual(dialogTable[gDialogID]);
    str = segmented_to_virtual(dialog->str);

    create_dl_translation_matrix(MENU_MTX_PUSH, 97.0f, 118.0f, 0);

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gCutsceneMsgFade);
    gSPDisplayList(gDisplayListHead++, castle_grounds_seg7_dl_0700EA58);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 20, 20, 20, gCutsceneMsgFade);

    print_generic_string(STR_X, STR_Y, str);
#if defined(VERSION_JP)
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
#endif
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
#ifndef VERSION_JP
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    gDPSetEnvColor(gDisplayListHead++, 200, 80, 120, gCutsceneMsgFade);
    gSPDisplayList(gDisplayListHead++, castle_grounds_seg7_us_dl_0700F2E8);
#endif

    // at the start/end of message, reset the fade.
    if (gCutsceneMsgTimer == 0) {
        gCutsceneMsgFade = 0;
    }

    // we're less than 20 increments, so increase the fade.
    if (gCutsceneMsgTimer < 20) {
        gCutsceneMsgFade += 10;
    }

    // we're after PEACH_MESSAGE_TIMER increments, so decrease the fade.
    if (gCutsceneMsgTimer > PEACH_MESSAGE_TIMER) {
        gCutsceneMsgFade -= 10;
    }

    // 20 increments after the start of the decrease, we're
    // back where we are, so reset everything at the end.
    if (gCutsceneMsgTimer > (PEACH_MESSAGE_TIMER + 20)) {
        gCutsceneMsgIndex = -1;
        gCutsceneMsgFade = 0; //! uselessly reset since the next execution will just set it to 0 again.
        gDialogID = DIALOG_NONE;
        gCutsceneMsgTimer = 0;
        return; // return to avoid incrementing the timer
    }

    gCutsceneMsgTimer++;
}
