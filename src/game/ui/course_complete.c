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
#include "font_string.h"
#include "font_hud.h"
#include "course_complete.h"

#ifdef VERSION_EU
#undef LANGUAGE_FUNCTION
#define LANGUAGE_FUNCTION gInGameLanguage
// gTextCourse is defined in pause_menu.c and declared in ui.h.
#define textCourse gTextCourse
#endif

s8 gCourseCompleteCoinsEqual = FALSE;
s32 gCourseCompleteScreenTimer = 0;
s32 gCourseCompleteCoins = 0;
s8 gHudFlash = 0;
#if defined(VERSION_JP)
    #define TXT_HISCORE_X 112
    #define TXT_HISCORE_Y 48
    #define TXT_CONGRATS_X 60
    #define TXT_CONGRATS_Y 67
#elif defined(VERSION_US)
    #define TXT_HISCORE_X 109
    #define TXT_HISCORE_Y 36
    #define TXT_CONGRATS_X 70
    #define TXT_CONGRATS_Y 67
#elif defined(VERSION_EU)
    #define TXT_HISCORE_X get_str_x_pos_from_center_scale(160, LANGUAGE_ARRAY(textHiScore), 12.0f)
    #define TXT_HISCORE_Y 36
    #define TXT_CONGRATS_X get_str_x_pos_from_center_scale(160, LANGUAGE_ARRAY(textCongratulations), 12.0f)
    #define TXT_CONGRATS_Y 67
#elif defined(VERSION_SH)
    #define TXT_HISCORE_X 118
    #define TXT_HISCORE_Y 48
    #define TXT_CONGRATS_X 70
    #define TXT_CONGRATS_Y 67
#elif defined(VERSION_CN)
    #define TXT_HISCORE_X 109
    #define TXT_HISCORE_Y 26
    #define TXT_CONGRATS_X 100
    #define TXT_CONGRATS_Y 62
#endif

#define HUD_PRINT_HISCORE         0
#define HUD_PRINT_CONGRATULATIONS 1

void print_hud_course_complete_string(s8 str) {
#ifdef VERSION_EU
    u8 textHiScore[][15] = {
        { TEXT_HUD_HI_SCORE },
        { TEXT_HUD_HI_SCORE_FR },
        { TEXT_HUD_HI_SCORE_DE }
    };
    u8 textCongratulations[][16] = {
        { TEXT_HUD_CONGRATULATIONS },
        { TEXT_HUD_CONGRATULATIONS_FR },
        { TEXT_HUD_CONGRATULATIONS_DE }
    };
#else
    u8 textHiScore[] = { TEXT_HUD_HI_SCORE };
    u8 textCongratulations[] = { TEXT_HUD_CONGRATULATIONS };
#endif

    u8 color = sins(gMenuTextColorTransTimer) * 50.0f + 200.0f;

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, color, color, color, 255);

    if (str == HUD_PRINT_HISCORE) {
        print_hud_lut_string(HUD_LUT_GLOBAL, TXT_HISCORE_X, TXT_HISCORE_Y, LANGUAGE_ARRAY(textHiScore));
    } else { // HUD_PRINT_CONGRATULATIONS
        print_hud_lut_string(HUD_LUT_GLOBAL, TXT_CONGRATS_X, TXT_CONGRATS_Y, LANGUAGE_ARRAY(textCongratulations));
    }

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
}

void print_hud_course_complete_coins(s16 x, s16 y) {
    u8 courseCompleteCoinsStr[4];
    u8 hudTextSymCoin[] = { GLYPH_COIN, GLYPH_SPACE };
    u8 hudTextSymX[] = { GLYPH_MULTIPLY, GLYPH_SPACE };

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);

    print_hud_lut_string(HUD_LUT_GLOBAL, x, y, hudTextSymCoin);
    print_hud_lut_string(HUD_LUT_GLOBAL, x + 16, y, hudTextSymX);

    int_to_str(gCourseCompleteCoins, courseCompleteCoinsStr);
    print_hud_lut_string(HUD_LUT_GLOBAL, x + 32, y, courseCompleteCoinsStr);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    if (gCourseCompleteCoins >= gHudDisplay.coins) {
        gCourseCompleteCoinsEqual = TRUE;
        gCourseCompleteCoins = gHudDisplay.coins;

        if (gGotFileCoinHiScore) {
            print_hud_course_complete_string(HUD_PRINT_HISCORE);
        }
    } else {
        if ((gCourseCompleteScreenTimer & 1) || gHudDisplay.coins > 70) {
            gCourseCompleteCoins++;
            play_sound(SOUND_MENU_YOSHI_GAIN_LIVES, gGlobalSoundSource);

            if (gCourseCompleteCoins == 50 || gCourseCompleteCoins == 100 || gCourseCompleteCoins == 150) {
                play_sound(SOUND_GENERAL_COLLECT_1UP, gGlobalSoundSource);
                gMarioState->numLives++;
            }
        }

        if (gCourseCompleteCoins == gHudDisplay.coins && gGotFileCoinHiScore) {
            play_sound(SOUND_MENU_MARIO_CASTLE_WARP2, gGlobalSoundSource);
        }
    }
}

void play_star_fanfare_and_flash_hud(s32 arg, u8 starFlag) {
    if (gCourseCompleteCoins == gHudDisplay.coins && !(gCurrCourseStarFlags & starFlag) && gHudFlash == 0) {
        play_star_fanfare();
        gHudFlash = arg;
    }
}

#ifdef VERSION_EU
    #define TXT_NAME_X1 centerX
    #define TXT_NAME_X2 centerX - 1
#else
    #define TXT_NAME_X1 71
    #define TXT_NAME_X2 TXT_NAME_X1 - 2
#endif

#if defined(VERSION_JP) || defined(VERSION_SH)
    #define CRS_NUM_X2 95
    #define CRS_NUM_X3 CRS_NUM_X2 - 2
    #define TXT_CLEAR_X1 205
    #define TXT_CLEAR_X2 TXT_CLEAR_X1 - 2
#else
    #define CRS_NUM_X2 104
    #define CRS_NUM_X3 CRS_NUM_X2 - 2
    #define TXT_CLEAR_X1 get_string_width(name) + 81
    #define TXT_CLEAR_X2 TXT_CLEAR_X1 - 2
#endif

void render_course_complete_lvl_info_and_hud_str(void) {
#if defined(VERSION_JP)
    u8 textSymStar[] = { GLYPH_STAR, GLYPH_SPACE };
    u8 textCourse[] = { TEXT_COURSE };
    u8 textCatch[] = { TEXT_CATCH };
    u8 textClear[] = { TEXT_CLEAR };
#elif defined(VERSION_EU)
    UNUSED u8 textCatch[] = { TEXT_CATCH }; // unused in EU
    u8 textSymStar[] = { GLYPH_STAR, GLYPH_SPACE };
#else
    u8 textCourse[] = { TEXT_COURSE };
    UNUSED u8 textCatch[] = { TEXT_CATCH }; // unused in US and iQue
    u8 textClear[] = { TEXT_CLEAR };
    u8 textSymStar[] = { GLYPH_STAR, GLYPH_SPACE };
#endif

    void **actNameTbl;
    void **courseNameTbl;
    u8 *name;

    u8 strCourseNum[4];

#ifdef VERSION_EU
    s16 centerX;
    switch (gInGameLanguage) {
        case LANGUAGE_ENGLISH:
            actNameTbl = segmented_to_virtual(act_name_table_eu_en);
            courseNameTbl = segmented_to_virtual(course_name_table_eu_en);
            break;
        case LANGUAGE_FRENCH:
            actNameTbl = segmented_to_virtual(act_name_table_eu_fr);
            courseNameTbl = segmented_to_virtual(course_name_table_eu_fr);
            break;
        case LANGUAGE_GERMAN:
            actNameTbl = segmented_to_virtual(act_name_table_eu_de);
            courseNameTbl = segmented_to_virtual(course_name_table_eu_de);
            break;
    }
#else
    actNameTbl = segmented_to_virtual(seg2_act_name_table);
    courseNameTbl = segmented_to_virtual(seg2_course_name_table);
#endif

    if (gLastCompletedCourseNum <= COURSE_STAGES_MAX) { // Main courses
        print_hud_course_complete_coins(118, 103);
        play_star_fanfare_and_flash_hud(1, 1 << (gLastCompletedStarNum - 1));

        if (gLastCompletedStarNum == 7) {
            name = segmented_to_virtual(actNameTbl[COURSE_STAGES_MAX * 6 + 1]);
        } else {
            name = segmented_to_virtual(actNameTbl[COURSE_NUM_TO_INDEX(gLastCompletedCourseNum) * 6 + gLastCompletedStarNum - 1]);
        }

        // Print course number
        gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);

        INT_TO_STR_DIFF(gLastCompletedCourseNum, strCourseNum);

        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gMenuTextAlpha);
        print_generic_string(65, 165, LANGUAGE_ARRAY(textCourse));
        print_generic_string(CRS_NUM_X2, 165, strCourseNum);

        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);
        print_generic_string(63, 167, LANGUAGE_ARRAY(textCourse));
        print_generic_string(CRS_NUM_X3, 167, strCourseNum);

        gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    } else if (gLastCompletedCourseNum == COURSE_BITDW || gLastCompletedCourseNum == COURSE_BITFS) { // Bowser courses
        name = segmented_to_virtual(courseNameTbl[COURSE_NUM_TO_INDEX(gLastCompletedCourseNum)]);

        // Print course name and clear text
        gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);

        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gMenuTextAlpha);
#ifdef VERSION_EU
        centerX = get_str_x_pos_from_center(153, name, 12.0f);
#endif
        print_generic_string(TXT_NAME_X1, 130, name);
#ifndef VERSION_EU
        print_generic_string(TXT_CLEAR_X1, 130, textClear);
#endif

        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);
        print_generic_string(TXT_NAME_X2, 132, name);
#ifndef VERSION_EU
        print_generic_string(TXT_CLEAR_X2, 132, textClear);
#endif

        gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

        print_hud_course_complete_string(HUD_PRINT_CONGRATULATIONS);
        print_hud_course_complete_coins(118, 111);
        play_star_fanfare_and_flash_hud(2, 0); //! 2 isn't defined, originally for key hud?

        return;
    } else { // Castle secret stars
        name = segmented_to_virtual(actNameTbl[COURSE_STAGES_MAX * 6]);

        print_hud_course_complete_coins(118, 103);
        play_star_fanfare_and_flash_hud(1, 1 << (gLastCompletedStarNum - 1));
    }

    // Print star glyph
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);
    print_hud_lut_string(HUD_LUT_GLOBAL, 55, 77, textSymStar);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    // Print act name and catch text
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);

    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gMenuTextAlpha);
    print_generic_string(76, 145, name);
#if defined(VERSION_JP) || defined(VERSION_SH)
    print_generic_string(220, 145, textCatch);
#endif

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);
    print_generic_string(74, 147, name);
#if defined(VERSION_JP) || defined(VERSION_SH)
    print_generic_string(218, 147, textCatch);
#endif

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

#if defined(VERSION_JP) || defined(VERSION_SH)
    #define X_VAL9 x
    #define TXT_SAVEOPTIONS_X x + 10
    #define TXT_SAVECONT_Y 2
    #define TXT_SAVEQUIT_Y 18
    #define TXT_CONTNOSAVE_Y 38
#else
#ifdef VERSION_EU
    #define X_VAL9 xOffset - 12
    #define TXT_SAVEOPTIONS_X xOffset
#else
    #define X_VAL9 x
    #define TXT_SAVEOPTIONS_X x + 12
#endif
    #define TXT_SAVECONT_Y 0
    #define TXT_SAVEQUIT_Y 20
    #define TXT_CONTNOSAVE_Y 40
#endif

#ifdef VERSION_EU
void render_save_confirmation(s16 y, s8 *index, s16 yOffset)
#else
void render_save_confirmation(s16 x, s16 y, s8 *index, s16 yOffset)
#endif
{
#ifdef VERSION_EU
    u8 textSaveAndContinue[][24] = {
        { TEXT_SAVE_AND_CONTINUE },
        { TEXT_SAVE_AND_CONTINUE_FR },
        { TEXT_SAVE_AND_CONTINUE_DE }
    };
    u8 textSaveAndQuit[][22] = {
        { TEXT_SAVE_AND_QUIT },
        { TEXT_SAVE_AND_QUIT_FR },
        { TEXT_SAVE_AND_QUIT_DE }
    };
    u8 textContinueWithoutSave[][27] = {
        { TEXT_CONTINUE_WITHOUT_SAVING },
        { TEXT_CONTINUE_WITHOUT_SAVING_FR },
        { TEXT_CONTINUE_WITHOUT_SAVING_DE }
    };
    s16 xOffset = get_str_x_pos_from_center(160, LANGUAGE_ARRAY(textContinueWithoutSave), 12.0f);
#else
    u8 textSaveAndContinue[] = { TEXT_SAVE_AND_CONTINUE };
    u8 textSaveAndQuit[] = { TEXT_SAVE_AND_QUIT };
    u8 textContinueWithoutSave[] = { TEXT_CONTINUE_WITHOUT_SAVING };
#endif

    handle_menu_scrolling(MENU_SCROLL_VERTICAL, index, 1, 3);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);

    print_generic_string(TXT_SAVEOPTIONS_X, y + TXT_SAVECONT_Y, LANGUAGE_ARRAY(textSaveAndContinue));
    print_generic_string(TXT_SAVEOPTIONS_X, y - TXT_SAVEQUIT_Y, LANGUAGE_ARRAY(textSaveAndQuit));
    print_generic_string(TXT_SAVEOPTIONS_X, y - TXT_CONTNOSAVE_Y, LANGUAGE_ARRAY(textContinueWithoutSave));

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL9, y - ((*index - 1) * yOffset), 0);

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);

    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

s16 render_course_complete_screen(void) {
    s16 index;
#ifdef VERSION_EU
    gInGameLanguage = eu_get_language();
#endif

    switch (gMenuState) {
        case MENU_STATE_COURSE_COMPLETE_SCREEN_OPENING:
            render_course_complete_lvl_info_and_hud_str();
            if (gCourseCompleteScreenTimer > 100 && gCourseCompleteCoinsEqual == TRUE) {
                gMenuState = MENU_STATE_COURSE_COMPLETE_SCREEN_OPEN;
                level_set_transition(-1, NULL);
                gMenuTextAlpha = 0;
                gMenuLineNum = MENU_OPT_DEFAULT;
            }
            break;

        case MENU_STATE_COURSE_COMPLETE_SCREEN_OPEN:
            shade_screen();
            render_course_complete_lvl_info_and_hud_str();
#ifdef VERSION_EU
            render_save_confirmation(86, &gMenuLineNum, 20);
#else
            render_save_confirmation(100, 86, &gMenuLineNum, 20);
#endif

            if (gCourseCompleteScreenTimer > 110
                && ((gPlayer3Controller->buttonPressed & A_BUTTON)
                 || (gPlayer3Controller->buttonPressed & START_BUTTON)
#ifdef VERSION_EU
                 || (gPlayer3Controller->buttonPressed & Z_TRIG)
#endif
                )) {
                level_set_transition(0, NULL);
                play_sound(SOUND_MENU_STAR_SOUND, gGlobalSoundSource);
                gMenuState = MENU_STATE_DEFAULT;
                gMenuMode = MENU_MODE_NONE;
                index = gMenuLineNum;
                gCourseCompleteScreenTimer = 0;
                gCourseCompleteCoins = 0;
                gCourseCompleteCoinsEqual = FALSE;
                gHudFlash = 0;

                return index;
            }
            break;
    }

    if (gMenuTextAlpha < 250) {
        gMenuTextAlpha += 25;
    }

    gCourseCompleteScreenTimer++;

    return MENU_OPT_NONE;
}
