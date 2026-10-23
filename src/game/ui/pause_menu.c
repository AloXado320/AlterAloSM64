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
#include "hud_extra.h"
#include "pause_menu.h"

#ifdef VERSION_EU
#undef LANGUAGE_FUNCTION
#define LANGUAGE_FUNCTION gInGameLanguage
#endif

s8 gDialogCameraAngleIndex = CAM_SELECTION_MARIO;
void change_dialog_camera_angle(void) {
    if (cam_select_alt_mode(0) == CAM_SELECTION_MARIO) {
        gDialogCameraAngleIndex = CAM_SELECTION_MARIO;
    } else {
        gDialogCameraAngleIndex = CAM_SELECTION_FIXED;
    }
}
#ifdef VERSION_EU
u8 gTextCourse[][7] = {
    { TEXT_COURSE },
    { TEXT_COURSE_FR },
    { TEXT_COURSE_DE }
};
#define textCourse gTextCourse
#endif

#if defined(VERSION_JP) || defined(VERSION_SH)
    #define CRS_NUM_X1 93
#elif defined(VERSION_US) || defined(VERSION_CN)
    #define CRS_NUM_X1 100
#elif defined(VERSION_EU)
    #define CRS_NUM_X1 get_string_width(LANGUAGE_ARRAY(textCourse)) + 51
#endif

#if defined(VERSION_EU)
    #define TXT_COURSE_X      48
    #define TXT_STAR_X        89
    #define ACT_NAME_X        107
    #define CRS_NAME_START    3
    #define CRS_NAME_X        108
    #define SECRET_CRS_NAME_X get_str_x_pos_from_center(159, &courseName[CRS_NAME_START], 10.0f)
    #define MYSCORE_X         48
#elif defined(VERSION_CN)
    #define TXT_COURSE_X      63
    #define TXT_STAR_X        98
    #define ACT_NAME_X        112
    #define CRS_NAME_START    6
    #define CRS_NAME_X        117
    #define SECRET_CRS_NAME_X 94
    #define MYSCORE_X         42
#else
    #define TXT_COURSE_X      63
    #define TXT_STAR_X        98
    #define ACT_NAME_X        116
    #define CRS_NAME_START    3
    #define CRS_NAME_X        117
    #define SECRET_CRS_NAME_X 94
    #define MYSCORE_X         62
#endif

void render_pause_my_score_coins(void) {
#ifdef VERSION_EU
    u8 textMyScore[][10] = {
        { TEXT_MY_SCORE },
        { TEXT_MY_SCORE_FR },
        { TEXT_MY_SCORE_DE }
    };
#else
    u8 textCourse[] = { TEXT_COURSE };
    u8 textMyScore[] = { TEXT_MY_SCORE };
#endif
    u8 textStar[] = { TEXT_STAR_DIFF };
    u8 textUnfilledStar[] = { TEXT_UNFILLED_STAR };

    u8 strCourseNum[4];
    void **courseNameTbl;
    u8 *courseName;
    void **actNameTbl;
    u8 *actName;
    u8 courseIndex;
    u8 starFlags;

#ifndef VERSION_EU
    courseNameTbl = segmented_to_virtual(seg2_course_name_table);
    actNameTbl = segmented_to_virtual(seg2_act_name_table);
#endif

    courseIndex = COURSE_NUM_TO_INDEX(gCurrCourseNum);
    starFlags = save_file_get_star_flags(gCurrSaveFileNum - 1, COURSE_NUM_TO_INDEX(gCurrCourseNum));

#ifdef VERSION_EU
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
#endif

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);

    if (courseIndex <= COURSE_NUM_TO_INDEX(COURSE_STAGES_MAX)) {
        print_hud_my_score_coins(1, gCurrSaveFileNum - 1, courseIndex, 178, 103);
        print_hud_my_score_stars(gCurrSaveFileNum - 1, courseIndex, 118, 103);
    }

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);

    if (courseIndex <= COURSE_NUM_TO_INDEX(COURSE_STAGES_MAX)
        && save_file_get_course_star_count(gCurrSaveFileNum - 1, courseIndex) != 0) {
        print_generic_string(MYSCORE_X, 121, LANGUAGE_ARRAY(textMyScore));
    }

    courseName = segmented_to_virtual(courseNameTbl[courseIndex]);

    if (courseIndex <= COURSE_NUM_TO_INDEX(COURSE_STAGES_MAX)) {
        print_generic_string(TXT_COURSE_X, 157, LANGUAGE_ARRAY(textCourse));
        INT_TO_STR_DIFF(gCurrCourseNum, strCourseNum);
        print_generic_string(CRS_NUM_X1, 157, strCourseNum);

        actName = segmented_to_virtual(actNameTbl[COURSE_NUM_TO_INDEX(gCurrCourseNum) * 6 + gDialogCourseActNum - 1]);

        if (starFlags & (1 << (gDialogCourseActNum - 1))) {
            print_generic_string(TXT_STAR_X, 140, textStar);
        } else {
            print_generic_string(TXT_STAR_X, 140, textUnfilledStar);
        }

        print_generic_string(ACT_NAME_X, 140, actName);
#ifndef VERSION_JP
        print_generic_string(CRS_NAME_X, 157, &courseName[CRS_NAME_START]);
    } else {
        print_generic_string(SECRET_CRS_NAME_X, 157, &courseName[CRS_NAME_START]);
#endif
    }

#ifdef VERSION_JP
    print_generic_string(CRS_NAME_X, 157, &courseName[CRS_NAME_START]);
#endif

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

#if defined(VERSION_JP) || defined(VERSION_SH)
    #define TXT1_X 4
    #define TXT2_X 116
    #define Y_VAL7 0
#else
    #define TXT1_X 3
    #define TXT2_X 119
    #define Y_VAL7 2
#endif

void render_pause_camera_options(s16 x, s16 y, s8 *index, s16 xIndex) {
    UNUSED_CN u8 textLakituMario[] = { TEXT_LAKITU_MARIO };
    UNUSED_CN u8 textLakituStop[] = { TEXT_LAKITU_STOP };
#ifdef VERSION_EU
    u8 textNormalUpClose[][20] = {
        { TEXT_NORMAL_UPCLOSE },
        { TEXT_NORMAL_UPCLOSE_FR },
        { TEXT_NORMAL_UPCLOSE_DE }
    };
    u8 textNormalFixed[][17] = {
        { TEXT_NORMAL_FIXED },
        { TEXT_NORMAL_FIXED_FR },
        { TEXT_NORMAL_FIXED_DE },
    };
#else
    u8 textNormalUpClose[] = { TEXT_NORMAL_UPCLOSE };
    u8 textNormalFixed[] = { TEXT_NORMAL_FIXED };
#endif

    handle_menu_scrolling(MENU_SCROLL_HORIZONTAL, index, 1, 2);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);

#ifdef VERSION_CN
    print_generic_string(x + 14, y + 2, textNormalUpClose);
    print_generic_string(x + 124, y + 2, textNormalFixed);
#else
    print_generic_string(x + 14, y + 2, textLakituMario);
    print_generic_string(x + TXT1_X, y - 13, LANGUAGE_ARRAY(textNormalUpClose));
    print_generic_string(x + 124, y + 2, textLakituStop);
    print_generic_string(x + TXT2_X, y - 13, LANGUAGE_ARRAY(textNormalFixed));
#endif

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    create_dl_translation_matrix(MENU_MTX_PUSH, x + ((*index - 1) * xIndex), y + Y_VAL7, 0);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    switch (*index) {
        case CAM_SELECTION_MARIO:
            cam_select_alt_mode(CAM_SELECTION_MARIO);
            break;
        case CAM_SELECTION_FIXED:
            cam_select_alt_mode(CAM_SELECTION_FIXED);
            break;
    }
}

#if defined(VERSION_JP) || defined(VERSION_SH)
    #define X_VAL8 0
    #define Y_VAL8 4
    #define Y_OFFSET1 17
    #define Y_OFFSET2 33
#elif defined(VERSION_US) || defined(VERSION_EU)
    #define X_VAL8 4
    #define Y_VAL8 2
    #define Y_OFFSET1 17
    #define Y_OFFSET2 33
#elif defined(VERSION_CN)
    #define X_VAL8 4
    #define Y_VAL8 2
    #define Y_OFFSET1 20
    #define Y_OFFSET2 38
#endif

void render_pause_course_options(s16 x, s16 y, s8 *index, s16 yIndex) {
#ifdef VERSION_EU
    u8 textContinue[][10] = {
        { TEXT_CONTINUE },
        { TEXT_CONTINUE_FR },
        { TEXT_CONTINUE_DE }
    };
    u8 textExitCourse[][15] = {
        { TEXT_EXIT_COURSE },
        { TEXT_EXIT_COURSE_FR },
        { TEXT_EXIT_COURSE_DE }
    };
    u8 textCameraAngleR[][24] = {
        { TEXT_CAMERA_ANGLE_R },
        { TEXT_CAMERA_ANGLE_R_FR },
        { TEXT_CAMERA_ANGLE_R_DE }
    };
#else
    u8 textContinue[] = { TEXT_CONTINUE };
    u8 textExitCourse[] = { TEXT_EXIT_COURSE };
    u8 textCameraAngleR[] = { TEXT_CAMERA_ANGLE_R };
#endif

    handle_menu_scrolling(MENU_SCROLL_VERTICAL, index, 1, 3);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);

    print_generic_string(x + 10, y - 2, LANGUAGE_ARRAY(textContinue));
    print_generic_string(x + 10, y - Y_OFFSET1, LANGUAGE_ARRAY(textExitCourse));

    if (*index != MENU_OPT_CAMERA_ANGLE_R) {
        print_generic_string(x + 10, y - Y_OFFSET2, LANGUAGE_ARRAY(textCameraAngleR));
        gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

        create_dl_translation_matrix(MENU_MTX_PUSH, x - X_VAL8, (y - ((*index - 1) * yIndex)) - Y_VAL8, 0);

        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);
        gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
        gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    }

    if (*index == MENU_OPT_CAMERA_ANGLE_R) {
        render_pause_camera_options(x - 42, y - 42, &gDialogCameraAngleIndex, 110);
    }
}

void render_pause_castle_menu_box(s16 x, s16 y) {
    create_dl_translation_matrix(MENU_MTX_PUSH, x - 78, y - 32, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.2f, 0.8f, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 105);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, x + 6, y - 28, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, 90.0f, 0, 0, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, x - 9, y - 101, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, 270.0f, 0, 0, 1.0f);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void highlight_last_course_complete_stars(void) {
    u8 completedCourseIndex;

    if (gLastCompletedCourseNum == COURSE_NONE) {
        completedCourseIndex = 0;
    } else {
        completedCourseIndex = COURSE_NUM_TO_INDEX(gLastCompletedCourseNum);

        if (completedCourseIndex >= COURSE_NUM_TO_INDEX(COURSE_BONUS_STAGES)) {
            completedCourseIndex = COURSE_NUM_TO_INDEX(COURSE_BONUS_STAGES);
        }
    }

    gMenuLineNum = completedCourseIndex;
}

#if defined(VERSION_EU)
    #define PAUSE_X get_str_x_pos_from_center_scale(SCREEN_WIDTH / 2, textPause, 12.0f)
#elif defined(VERSION_CN)
    #define PAUSE_X 113
#else
    #define PAUSE_X 123
#endif

void print_hud_pause_colorful_str(void) {
    u8 textPause[] = { TEXT_PAUSE };

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);

    print_hud_lut_string(HUD_LUT_GLOBAL, PAUSE_X, 81, textPause);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
}

void render_pause_castle_course_stars(s16 x, s16 y, s16 fileIndex, s16 courseIndex) {
    s16 hasStar = 0;

#ifdef VERSION_CN
    u8 str[60];
#else
    u8 str[30];
#endif

    u8 textStar[] = { TEXT_STAR };

    u8 starFlags = save_file_get_star_flags(fileIndex, courseIndex);
    u16 starCount = save_file_get_course_star_count(fileIndex, courseIndex);

    u16 nextStar = 0;

    if (starFlags & (1 << 6)) {
        starCount--;
        print_generic_string(x + 89, y - 5, textStar);
    }

    while (hasStar != starCount) {
        if (starFlags & (1 << nextStar)) {
#ifdef VERSION_CN
            str[nextStar * 4] = 0x00;
            str[nextStar * 4 + 1] = DIALOG_CHAR_STAR_FILLED;
#else
            str[nextStar * 2] = DIALOG_CHAR_STAR_FILLED;
#endif
            hasStar++;
        } else {
#ifdef VERSION_CN
            str[nextStar * 4] = 0x00;
            str[nextStar * 4 + 1] = DIALOG_CHAR_STAR_OPEN;
#else
            str[nextStar * 2] = DIALOG_CHAR_STAR_OPEN;
#endif
        }

#ifdef VERSION_CN
        str[nextStar * 4 + 2] = DIALOG_CHAR_SPECIAL_MODIFIER;
        str[nextStar * 4 + 3] = DIALOG_CHAR_SPACE;
#else
        str[nextStar * 2 + 1] = DIALOG_CHAR_SPACE;
#endif
        nextStar++;
    }

    if (starCount == nextStar && starCount != 6) {
#ifdef VERSION_CN
        str[nextStar * 4] = DIALOG_CHAR_SPECIAL_MODIFIER;
        str[nextStar * 4 + 1] = DIALOG_CHAR_SLASH; //! Meant to be DIALOG_CHAR_STAR_OPEN?
        str[nextStar * 4 + 2] = DIALOG_CHAR_SPECIAL_MODIFIER;
        str[nextStar * 4 + 3] = DIALOG_CHAR_SPACE;
#else
        str[nextStar * 2] = DIALOG_CHAR_STAR_OPEN;
        str[nextStar * 2 + 1] = DIALOG_CHAR_SPACE;
#endif
        nextStar++;
    }

#ifdef VERSION_CN
    str[nextStar * 4] = DIALOG_CHAR_SPECIAL_MODIFIER;
    str[nextStar * 4 + 1] = DIALOG_CHAR_TERMINATOR;
#else
    str[nextStar * 2] = DIALOG_CHAR_TERMINATOR;
#endif

    print_generic_string(x + 14, y + 13, str);
}

void render_pause_castle_main_strings(s16 x, s16 y) {
#ifdef VERSION_EU
    void **courseNameTbl;
#else
    void **courseNameTbl = segmented_to_virtual(seg2_course_name_table);
#endif

#ifdef VERSION_EU
    u8 textCoin[] = { TEXT_COIN };
    u8 textX[] = { TEXT_VARIABLE_X };
#else
    u8 textCoin[] = { TEXT_COIN_X };
#endif

    void *courseName;

    u8 strVal[8];
    s16 prevCourseIndex = gMenuLineNum;

#ifdef VERSION_EU
    switch (gInGameLanguage) {
        case LANGUAGE_ENGLISH:
            courseNameTbl = segmented_to_virtual(course_name_table_eu_en);
            break;
        case LANGUAGE_FRENCH:
            courseNameTbl = segmented_to_virtual(course_name_table_eu_fr);
            break;
        case LANGUAGE_GERMAN:
            courseNameTbl = segmented_to_virtual(course_name_table_eu_de);
            break;
    }
#endif

    handle_menu_scrolling(
        MENU_SCROLL_VERTICAL, &gMenuLineNum,
        COURSE_NUM_TO_INDEX(COURSE_MIN) - 1, COURSE_NUM_TO_INDEX(COURSE_BONUS_STAGES) + 1
    );

    if (gMenuLineNum == COURSE_NUM_TO_INDEX(COURSE_BONUS_STAGES) + 1) {
        gMenuLineNum = COURSE_NUM_TO_INDEX(COURSE_MIN); // Exceeded max, set to min
    }

    if (gMenuLineNum == COURSE_NUM_TO_INDEX(COURSE_MIN) - 1) {
        gMenuLineNum = COURSE_NUM_TO_INDEX(COURSE_BONUS_STAGES); // Exceeded min, set to max
    }

    if (gMenuLineNum != COURSE_NUM_TO_INDEX(COURSE_BONUS_STAGES)) {
        while (save_file_get_course_star_count(gCurrSaveFileNum - 1, gMenuLineNum) == 0) {
            if (gMenuLineNum >= prevCourseIndex) {
                gMenuLineNum++;
            } else {
                gMenuLineNum--;
            }

            if (gMenuLineNum == COURSE_NUM_TO_INDEX(COURSE_STAGES_MAX) + 1
                || gMenuLineNum == COURSE_NUM_TO_INDEX(COURSE_MIN) - 1) {
                gMenuLineNum = COURSE_NUM_TO_INDEX(COURSE_BONUS_STAGES);
                break;
            }
        }
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gMenuTextAlpha);

    if (gMenuLineNum <= COURSE_NUM_TO_INDEX(COURSE_STAGES_MAX)) { // Main courses
        courseName = segmented_to_virtual(courseNameTbl[gMenuLineNum]);
        render_pause_castle_course_stars(x, y, gCurrSaveFileNum - 1, gMenuLineNum);
        print_generic_string(x + 34, y - 5, textCoin);
#ifdef VERSION_EU
        print_generic_string(x + 44, y - 5, textX);
#endif
        INT_TO_STR_DIFF(save_file_get_course_coin_score(gCurrSaveFileNum - 1, gMenuLineNum), strVal);
        print_generic_string(x + 54, y - 5, strVal);
#ifdef VERSION_EU
        print_generic_string(x - 17, y + 30, courseName);
#endif
    } else { // Castle secret stars
        u8 textStarX[] = { TEXT_STAR_X };
        courseName = segmented_to_virtual(courseNameTbl[COURSE_MAX]);
        print_generic_string(x + 40, y + 13, textStarX);
        INT_TO_STR_DIFF(save_file_get_total_star_count(gCurrSaveFileNum - 1, COURSE_BONUS_STAGES - 1, COURSE_MAX - 1), strVal);
        print_generic_string(x + 60, y + 13, strVal);
#ifdef VERSION_EU
        print_generic_string(get_str_x_pos_from_center(x + 51, courseName, 10.0f), y + 30, courseName);
#endif
    }

#ifndef VERSION_EU
    print_generic_string(x - 9, y + 30, courseName);
#endif

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}
s16 render_pause_screen(void) {
    s16 index;

#ifdef VERSION_EU
    gInGameLanguage = eu_get_language();
#endif

    switch (gMenuState) {
        case MENU_STATE_PAUSE_SCREEN_OPENING:
            gMenuLineNum = MENU_OPT_DEFAULT;
            gMenuTextAlpha = 0;
            level_set_transition(-1, NULL);
            play_sound(SOUND_MENU_PAUSE, gGlobalSoundSource);

            if (gCurrCourseNum >= COURSE_MIN && gCurrCourseNum <= COURSE_MAX) {
                change_dialog_camera_angle();
                gMenuState = MENU_STATE_PAUSE_SCREEN_COURSE;
            } else {
                highlight_last_course_complete_stars();
                gMenuState = MENU_STATE_PAUSE_SCREEN_CASTLE;
            }
            break;

        case MENU_STATE_PAUSE_SCREEN_COURSE:
            shade_screen();
            render_pause_my_score_coins();
            render_pause_red_coins();

            if (gMarioStates[0].action & ACT_FLAG_PAUSE_EXIT) {
                render_pause_course_options(99, 93, &gMenuLineNum, 15);
            }

#ifdef VERSION_EU
            if (gPlayer3Controller->buttonPressed & (A_BUTTON | START_BUTTON | Z_TRIG))
#else
            if ((gPlayer3Controller->buttonPressed & A_BUTTON)
             || (gPlayer3Controller->buttonPressed & START_BUTTON))
#endif
            {
                level_set_transition(0, NULL);
                play_sound(SOUND_MENU_PAUSE_2, gGlobalSoundSource);
                gMenuState = MENU_STATE_DEFAULT;
                gMenuMode = MENU_MODE_NONE;

                if (gMenuLineNum == MENU_OPT_EXIT_COURSE) {
                    index = gMenuLineNum;
                } else { // MENU_OPT_CONTINUE or MENU_OPT_CAMERA_ANGLE_R
                    index = MENU_OPT_DEFAULT;
                }

                return index;
            }
            break;

        case MENU_STATE_PAUSE_SCREEN_CASTLE:
            shade_screen();
            print_hud_pause_colorful_str();
            render_pause_castle_menu_box(160, 143);
            render_pause_castle_main_strings(104, 60);

#ifdef VERSION_EU
            if (gPlayer3Controller->buttonPressed & (A_BUTTON | START_BUTTON | Z_TRIG))
#else
            if ((gPlayer3Controller->buttonPressed & A_BUTTON)
             || (gPlayer3Controller->buttonPressed & START_BUTTON))
#endif
            {
                level_set_transition(0, NULL);
                play_sound(SOUND_MENU_PAUSE_2, gGlobalSoundSource);
                gMenuMode = MENU_MODE_NONE;
                gMenuState = MENU_STATE_DEFAULT;

                return MENU_OPT_DEFAULT;
            }
            break;
    }

    if (gMenuTextAlpha < 250) {
        gMenuTextAlpha += 25;
    }

    return MENU_OPT_NONE;
}
