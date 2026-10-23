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

#include "ui.h"
#include "font_string.h"
#include "dialog.h"

#ifdef VERSION_EU
#undef LANGUAGE_FUNCTION
#define LANGUAGE_FUNCTION gInGameLanguage
#endif

s8 gLastDialogLineNum;
s32 gDialogVariable;
enum DialogBoxPageState {
    DIALOG_PAGE_STATE_NONE,
    DIALOG_PAGE_STATE_SCROLL,
    DIALOG_PAGE_STATE_END
};

enum DialogBoxType {
    DIALOG_TYPE_ROTATE, // used in NPCs and level messages
    DIALOG_TYPE_ZOOM    // used in signposts and wall signs and etc
};
#define DIALOG_BOX_ANGLE_DEFAULT 90.0f
#define DIALOG_BOX_SCALE_DEFAULT 19.0f
f32 gDialogBoxAngle = DIALOG_BOX_ANGLE_DEFAULT;
f32 gDialogBoxScale = DIALOG_BOX_SCALE_DEFAULT;
s16 gDialogScrollOffsetY = 0;
s8 gDialogBoxType = DIALOG_TYPE_ROTATE;
s16 gDialogID = DIALOG_NONE;
s16 gNextDialogPageStartStrIndex = 0;
s16 gDialogPageStartStrIndex = 0;
s8 gDialogWithChoice = FALSE;
s32 gDialogResponse = DIALOG_RESPONSE_NONE;
s16 get_dialog_id(void) {
    return gDialogID;
}

void create_dialog_box(s16 dialog) {
    if (gDialogID == DIALOG_NONE) {
        gDialogID = dialog;
        gDialogBoxType = DIALOG_TYPE_ROTATE;
    }
}

void create_dialog_box_with_var(s16 dialog, s32 dialogVar) {
    if (gDialogID == DIALOG_NONE) {
        gDialogID = dialog;
        gDialogVariable = dialogVar;
        gDialogBoxType = DIALOG_TYPE_ROTATE;
    }
}

void create_dialog_inverted_box(s16 dialog) {
    if (gDialogID == DIALOG_NONE) {
        gDialogID = dialog;
        gDialogBoxType = DIALOG_TYPE_ZOOM;
    }
}

void create_dialog_box_with_response(s16 dialog) {
    if (gDialogID == DIALOG_NONE) {
        gDialogID = dialog;
        gDialogBoxType = DIALOG_TYPE_ROTATE;
        gDialogWithChoice = TRUE;
    }
}

void reset_dialog_render_state(void) {
    level_set_transition(0, NULL);

    if (gDialogBoxType == DIALOG_TYPE_ZOOM) {
        trigger_cutscene_dialog(2);
    }

    gDialogBoxScale = DIALOG_BOX_SCALE_DEFAULT;
    gDialogBoxAngle = DIALOG_BOX_ANGLE_DEFAULT;
    gMenuState = MENU_STATE_DEFAULT;
    gDialogID = DIALOG_NONE;
    gDialogPageStartStrIndex = 0;
    gDialogWithChoice = FALSE;
    gNextDialogPageStartStrIndex = 0;
    gDialogResponse = DIALOG_RESPONSE_NONE;
}
#if defined(VERSION_JP) || defined(VERSION_SH)
    #define X_VAL1 -5.0f
    #define Y_VAL1 2.0
    #define Y_VAL2 4
#else
    #define X_VAL1 -7.0f
    #define Y_VAL1 5.0
    #define Y_VAL2 5.0f
#endif
void render_dialog_box_type(struct DialogEntry *dialog, s8 linesPerBox) {
#ifndef VERSION_CN
    UNUSED u8 filler[4];
#endif

    create_dl_translation_matrix(MENU_MTX_NOPUSH, dialog->leftOffset, dialog->width, 0);

    switch (gDialogBoxType) {
        case DIALOG_TYPE_ROTATE: // Renders a dialog black box with zoom and rotation
            if (gMenuState == MENU_STATE_DIALOG_OPENING || gMenuState == MENU_STATE_DIALOG_CLOSING) {
                create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.0 / gDialogBoxScale, 1.0 / gDialogBoxScale, 1.0f);
                // convert the speed into angle
                create_dl_rotation_matrix(MENU_MTX_NOPUSH, gDialogBoxAngle * 4.0f, 0, 0, 1.0f);
            }
            gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 150);
            break;
        case DIALOG_TYPE_ZOOM: // Renders a dialog white box with zoom
            if (gMenuState == MENU_STATE_DIALOG_OPENING || gMenuState == MENU_STATE_DIALOG_CLOSING) {
                create_dl_translation_matrix(MENU_MTX_NOPUSH, 65.0 - (65.0 / gDialogBoxScale),
                                              (40.0 / gDialogBoxScale) - 40, 0);
                create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.0 / gDialogBoxScale, 1.0 / gDialogBoxScale, 1.0f);
            }
            gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 150);
            break;
    }

    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL1, Y_VAL1, 0);
#ifdef VERSION_CN
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.1f, ((f32)(linesPerBox + 1) / Y_VAL2) + 0.1, 1.0f);
#else
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.1f, ((f32) linesPerBox / Y_VAL2) + 0.1, 1.0f);
#endif

    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}
void change_and_flash_dialog_text_color_lines(s8 colorMode, s8 lineNum) {
    u8 color;

    if (colorMode == 1) { // unused
        if (lineNum == 1) {
            gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
        } else {
            if (lineNum == gMenuLineNum) {
                color = sins(gMenuTextColorTransTimer) * 50.0f + 200.0f;
                gDPSetEnvColor(gDisplayListHead++, color, color, color, 255);
            } else {
                gDPSetEnvColor(gDisplayListHead++, 200, 200, 200, 255);
            }
        }
    } else {
        switch (gDialogBoxType) {
            case DIALOG_TYPE_ROTATE:
                break;
            case DIALOG_TYPE_ZOOM:
                gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
                break;
        }
    }
}
#ifdef VERSION_EU
void render_generic_dialog_char_at_pos(struct DialogEntry *dialog, s16 x, s16 y, u8 c) {
    s16 width = (8.0 - (gDialogBoxScale * 0.8));
    s16 height = (16.0 - (gDialogBoxScale * 0.8));
    s16 tmpX = (dialog->leftOffset + (65.0 - (65.0 / gDialogBoxScale)));
    s16 tmpY = ((240 - dialog->width) - ((40.0 / gDialogBoxScale) - 40));
    s16 xCoord = (tmpX + (x / gDialogBoxScale));
    s16 yCoord = (tmpY + (y / gDialogBoxScale));

    void **fontLUT = segmented_to_virtual(main_font_lut);
    void *packedTexture = segmented_to_virtual(fontLUT[c]);
    void *unpackedTexture = alloc_ia4_tex_from_i1(packedTexture, 8, 8);

    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, VIRTUAL_TO_PHYSICAL(unpackedTexture));
    gSPDisplayList(gDisplayListHead++, dl_ia_text_tex_settings);
    font_draw_texrect_flip(xCoord << 2, (yCoord - height) << 2,
                           (xCoord + width) << 2, yCoord << 2, 8 << 6, 4 << 6, 1 << 10, 1 << 10);
}
#endif
#if defined(VERSION_JP) || defined(VERSION_SH)
    #define X_VAL3 5.0f
    #define Y_VAL3 20
#elif defined(VERSION_CN)
    #define X_VAL3 0.0f
    #define Y_VAL3 20
#else
    #define X_VAL3 0.0f
    #define Y_VAL3 16
#endif

#ifdef VERSION_EU
void handle_dialog_scroll_page_state(s8 lineNum, s8 totalLines, s8 *pageState, s8 *xMatrix)
#else
void handle_dialog_scroll_page_state(s8 lineNum, s8 totalLines, s8 *pageState, s8 *xMatrix, s16 *linePos)
#endif
{
#ifndef VERSION_EU
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
#endif

    if (lineNum == totalLines) {
        *pageState = DIALOG_PAGE_STATE_SCROLL;
        return;
    }
#ifdef VERSION_EU
    gDialogY += 16;
#else
    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL3, 2 - (lineNum * Y_VAL3), 0);

    *linePos = 0;
#endif
    *xMatrix = 1;
}
#if defined(VERSION_JP) || defined(VERSION_SH)
void adjust_pos_and_print_period_char(s8 *xMatrix, s16 *linePos) {
    if (*linePos != 0) {
        create_dl_translation_matrix(MENU_MTX_NOPUSH, 10 * *xMatrix, 0, 0);
    }

    create_dl_translation_matrix(MENU_MTX_PUSH, -2.0f, -5.0f, 0);
    render_generic_char(DIALOG_CHAR_PERIOD_OR_HANDAKUTEN);

    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    (*linePos)++;
    *xMatrix = 1;
}
#endif
#ifdef VERSION_EU
void render_star_count_dialog_text(struct DialogEntry *dialog, s8 *linePos)
#else
void render_star_count_dialog_text(s8 *xMatrix, s16 *linePos)
#endif
{
    s8 tensDigit = gDialogVariable / 10;
    s8 onesDigit = gDialogVariable - (tensDigit * 10); // remainder

    if (tensDigit != 0) {
#if defined(VERSION_JP) || defined(VERSION_SH)
        create_dl_translation_matrix(MENU_MTX_NOPUSH, 10 * *xMatrix, 0, 0);
        render_generic_char(tensDigit);
#elif defined(VERSION_US) || defined(VERSION_CN)
        if (*xMatrix != 1) {
            create_dl_translation_matrix(
                MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE] * *xMatrix), 0, 0);
        }

        render_generic_char(tensDigit);
        create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32) gDialogCharWidths[tensDigit], 0, 0);
        *xMatrix = 1;
        (*linePos)++;
#elif defined(VERSION_EU)
        render_generic_dialog_char_at_pos(dialog, gDialogX, gDialogY, tensDigit);
        gDialogX += gDialogCharWidths[tensDigit];
        *linePos = 1;
#endif
    }
#ifndef VERSION_EU
    else {
#if defined(VERSION_JP) || defined(VERSION_SH)
        (*xMatrix)++;
#endif
    }
#endif

#ifndef VERSION_EU
#if defined(VERSION_JP) || defined(VERSION_SH)
    create_dl_translation_matrix(MENU_MTX_NOPUSH, 10 * *xMatrix, 0, 0);
    render_generic_char(onesDigit);
#elif defined(VERSION_US) || defined(VERSION_CN)
    if (*xMatrix != 1) {
        create_dl_translation_matrix(
            MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE] * (*xMatrix - 1)), 0, 0);
    }

    render_generic_char(onesDigit);
    create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32) gDialogCharWidths[onesDigit], 0, 0);
#endif
    (*linePos)++;
    *xMatrix = 1;
#else // VERSION_EU
    render_generic_dialog_char_at_pos(dialog, gDialogX, gDialogY, onesDigit);
    gDialogX += gDialogCharWidths[onesDigit];
    *linePos = 1;
#endif
}
#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
#ifdef VERSION_EU
void render_multi_text_string_lines(s8 multiTextId, s8 lineNum, s8 linesPerBox, UNUSED s8 xMatrix, s8 lowerBound, struct DialogEntry *dialog)
#else
void render_multi_text_string_lines(s8 multiTextId, s8 lineNum, s16 *linePos, s8 linesPerBox, s8 xMatrix, s8 lowerBound)
#endif
{
    s8 i;
    struct MultiTextEntry textLengths[2] = {
        { { 3, TEXT_THE_RAW } },
        { { 3, TEXT_YOU_RAW } },
    };

    if (lineNum >= lowerBound && lineNum <= (lowerBound + linesPerBox)) {
#ifndef VERSION_EU
        if (*linePos != 0 || xMatrix != 1) {
            create_dl_translation_matrix(
                MENU_MTX_NOPUSH, (gDialogCharWidths[DIALOG_CHAR_SPACE] * (xMatrix - 1)), 0, 0);
        }
#endif
        for (i = 0; i < textLengths[multiTextId].str[0]; i++) {
#ifdef VERSION_EU
            render_generic_dialog_char_at_pos(dialog, gDialogX, gDialogY, textLengths[multiTextId].str[1 + i]);
            gDialogX += gDialogCharWidths[textLengths[multiTextId].str[1 + i]];
#else
            render_generic_char(textLengths[multiTextId].str[1 + i]);
            create_dl_translation_matrix(
                MENU_MTX_NOPUSH, (gDialogCharWidths[textLengths[multiTextId].str[1 + i]]), 0, 0);
#endif
        }
    }

#ifndef VERSION_EU
    linePos += textLengths[multiTextId].str[0];
#endif
}
#endif
#ifdef VERSION_EU
void render_dialog_lowercase_diacritic(struct DialogEntry *dialog, u8 chr, u8 diacritic) {
    render_generic_dialog_char_at_pos(dialog, gDialogX, gDialogY, chr);
    render_generic_dialog_char_at_pos(dialog, gDialogX, gDialogY, diacritic + 0xE7);
    gDialogX += gDialogCharWidths[chr];
}

void render_dialog_uppercase_diacritic(struct DialogEntry *dialog, u8 chr, u8 diacritic) {
    render_generic_dialog_char_at_pos(dialog, gDialogX, gDialogY, chr);
    render_generic_dialog_char_at_pos(dialog, gDialogX, gDialogY - 4, diacritic + 0xE3);
    gDialogX += gDialogCharWidths[chr];
}
#endif
u32 ensure_nonnegative(s16 value) {
    if (value < 0) {
        value = 0;
    }

    return value;
}
#ifdef VERSION_JP
void handle_dialog_text_and_pages(s8 colorMode, struct DialogEntry *dialog)
#else
void handle_dialog_text_and_pages(s8 colorMode, struct DialogEntry *dialog, s8 lowerBound)
#endif
{
    UNUSED u64 filler;
#ifdef VERSION_EU
    s16 startY = 14;
#endif
#ifdef VERSION_CN
    s32 strChar;
#else
    u8 strChar;
#endif
    u8 *str = segmented_to_virtual(dialog->str);
    s8 lineNum = 1;
    s8 totalLines;
    s8 pageState = DIALOG_PAGE_STATE_NONE;
    UNUSED s8 mark = DIALOG_MARK_NONE; // only used in JP and SH
    s8 xMatrix = 1;
    s8 linesPerBox = dialog->linesPerBox;
    s16 strIndex;
#ifndef VERSION_EU
    s16 linePos = 0;
#endif

    if (gMenuState == MENU_STATE_DIALOG_SCROLLING) {
        // If scrolling, consider the number of lines for both
        // the current page and the page being scrolled to.
#ifdef VERSION_CN
        totalLines = linesPerBox * 2 - 1;
#else
        totalLines = linesPerBox * 2 + 1;
#endif
    } else {
#ifdef VERSION_CN
        totalLines = linesPerBox;
#else
        totalLines = linesPerBox + 1;
#endif
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);

    strIndex = gDialogPageStartStrIndex;

#ifdef VERSION_EU
    gDialogX = 0;
    gDialogY = startY;
#endif

    if (gMenuState == MENU_STATE_DIALOG_SCROLLING) {
#ifdef VERSION_EU
        gDialogY -= gDialogScrollOffsetY;
#else
        create_dl_translation_matrix(MENU_MTX_NOPUSH, 0, (f32) gDialogScrollOffsetY, 0);
#endif
    }

#ifndef VERSION_EU
    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL3, 2 - lineNum * Y_VAL3, 0);
#endif

    while (pageState == DIALOG_PAGE_STATE_NONE) {
        change_and_flash_dialog_text_color_lines(colorMode, lineNum);

#ifdef VERSION_CN
        strChar = str[strIndex] << 8 | str[strIndex + 1];
#else
        strChar = str[strIndex];
#endif

        switch (strChar) {
            case SPECIAL_CHAR(DIALOG_CHAR_TERMINATOR):
                pageState = DIALOG_PAGE_STATE_END;
#ifndef VERSION_EU
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
#endif
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_NEWLINE):
                lineNum++;
#ifdef VERSION_EU
                handle_dialog_scroll_page_state(lineNum, totalLines, &pageState, &xMatrix);
                gDialogX = 0;
#else
                handle_dialog_scroll_page_state(lineNum, totalLines, &pageState, &xMatrix, &linePos);
#ifdef VERSION_SH
                mark = DIALOG_MARK_NONE;
#endif
#endif
                break;

#ifdef VERSION_EU
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_A_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_A_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_A_UMLAUT):
                render_dialog_lowercase_diacritic(dialog, ASCII_TO_DIALOG('a'), strChar & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_A_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_A_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_A_UMLAUT):
                render_dialog_uppercase_diacritic(dialog, ASCII_TO_DIALOG('A'), strChar & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_E_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_E_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_E_UMLAUT):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_E_ACUTE):
                render_dialog_lowercase_diacritic(dialog, ASCII_TO_DIALOG('e'), strChar & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_E_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_E_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_E_UMLAUT):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_E_ACUTE):
                render_dialog_uppercase_diacritic(dialog, ASCII_TO_DIALOG('E'), strChar & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_U_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_U_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_U_UMLAUT):
                render_dialog_lowercase_diacritic(dialog, ASCII_TO_DIALOG('u'), strChar & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_U_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_U_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_U_UMLAUT):
                render_dialog_uppercase_diacritic(dialog, ASCII_TO_DIALOG('U'), strChar & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_O_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_O_UMLAUT):
                render_dialog_lowercase_diacritic(dialog, ASCII_TO_DIALOG('o'), strChar & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_O_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_O_UMLAUT):
                render_dialog_uppercase_diacritic(dialog, ASCII_TO_DIALOG('O'), strChar & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_I_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_I_UMLAUT):
                render_dialog_lowercase_diacritic(dialog, DIALOG_CHAR_I_NO_DIA, strChar & 0xF);
                break;
#else
            case SPECIAL_CHAR(DIALOG_CHAR_DAKUTEN):
#ifndef VERSION_CN
                mark = DIALOG_MARK_DAKUTEN;
#endif
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_PERIOD_OR_HANDAKUTEN):
#ifndef VERSION_CN
                mark = DIALOG_MARK_HANDAKUTEN;
#endif
                break;
#endif

#ifdef VERSION_CN
            case SPECIAL_CHAR(DIALOG_CHAR_STAR_OPEN): // ???
                xMatrix += 2;
                linePos += 2;
                break;
#endif

            case SPECIAL_CHAR(DIALOG_CHAR_SPACE):
#ifdef VERSION_EU
                gDialogX += gDialogCharWidths[DIALOG_CHAR_SPACE];
#else
#if defined(VERSION_JP) || defined(VERSION_SH)
                if (linePos != 0) {
#endif
                    xMatrix++;
#if defined(VERSION_JP) || defined(VERSION_SH)
                }
#endif
                linePos++;
#endif
                break;

#if defined(VERSION_JP) || defined(VERSION_SH)
            case SPECIAL_CHAR(DIALOG_CHAR_PERIOD):
                adjust_pos_and_print_period_char(&xMatrix, &linePos);
                break;
#else
            case SPECIAL_CHAR(DIALOG_CHAR_SLASH):
#ifdef VERSION_EU
                gDialogX += gDialogCharWidths[DIALOG_CHAR_SPACE] * 2;
#else
                xMatrix += 2;
                linePos += 2;
#endif
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_MULTI_THE):
#ifdef VERSION_EU
                render_multi_text_string_lines(STRING_THE, lineNum, linesPerBox, xMatrix, lowerBound, dialog);
#else
                render_multi_text_string_lines(STRING_THE, lineNum, &linePos, linesPerBox, xMatrix, lowerBound);
#endif
                xMatrix = 1;
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_MULTI_YOU):
#ifdef VERSION_EU
                render_multi_text_string_lines(STRING_YOU, lineNum, linesPerBox, xMatrix, lowerBound, dialog);
#else
                render_multi_text_string_lines(STRING_YOU, lineNum, &linePos, linesPerBox, xMatrix, lowerBound);
#endif
                xMatrix = 1;
                break;
#endif

            case SPECIAL_CHAR(DIALOG_CHAR_STAR_COUNT):
#ifdef VERSION_EU
                render_star_count_dialog_text(dialog, &xMatrix);
#else
                render_star_count_dialog_text(&xMatrix, &linePos);
#endif
                break;

#ifdef VERSION_EU
            case SPECIAL_CHAR(DIALOG_CHAR_DOUBLE_LOW_QUOTE):
                render_generic_dialog_char_at_pos(dialog, gDialogX, gDialogY + 8, 0xF6);
                gDialogX += gDialogCharWidths[0xF6];
                break;
#endif

            default: // any other character
#ifdef VERSION_CN
                if (strChar >= 0x0100) {
                    strChar = strChar * 2 - 0x0100;

                    if (lineNum < lowerBound || lineNum > (lowerBound + linesPerBox)) {
                        break;
                    }

                    if (linePos != 0 || xMatrix != 1) {
                        create_dl_translation_matrix(MENU_MTX_NOPUSH, gDialogCharWidths[DIALOG_CHAR_SPACE] * (xMatrix - 1), 0.0f, 0.0f);
                    }

                    render_generic_char_cn(strChar);
                    create_dl_translation_matrix(MENU_MTX_NOPUSH, 8.0f, 0.0f, 0.0f);
                    render_generic_char_cn(strChar + 1);
                    create_dl_translation_matrix(MENU_MTX_NOPUSH, 8.0f, 0.0f, 0.0f);
                } else {
                    if (lineNum < lowerBound || lineNum > (lowerBound + linesPerBox)) {
                        break;
                    }

                    if (linePos != 0 || xMatrix != 1) {
                        create_dl_translation_matrix(MENU_MTX_NOPUSH, gDialogCharWidths[DIALOG_CHAR_SPACE] * (xMatrix - 1), 0.0f, 0.0f);
                    }

                    render_generic_char(strChar);
                    create_dl_translation_matrix(MENU_MTX_NOPUSH, gDialogCharWidths[strChar], 0.0f, 0.0f);
                }

                xMatrix = 1;
                linePos++;
#elif defined(VERSION_JP) || defined(VERSION_SH)
#ifdef VERSION_SH
                if (lineNum >= lowerBound && lineNum <= (lowerBound + linesPerBox)) {
#endif
                    if (linePos != 0) {
                        create_dl_translation_matrix(MENU_MTX_NOPUSH, 10 * xMatrix, 0, 0);
                    }

                    render_generic_char(strChar);
                    xMatrix = 1;
                    linePos++;

                    if (mark != DIALOG_MARK_NONE) {
                        create_dl_translation_matrix(MENU_MTX_PUSH, 5.0f, 7.0f, 0);
                        render_generic_char(DIALOG_CHAR_MARK_START + mark);
                        gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                        mark = DIALOG_MARK_NONE;
                    }
#ifdef VERSION_SH
                }
#endif
#elif defined(VERSION_US)
                if (lineNum >= lowerBound && lineNum <= (lowerBound + linesPerBox)) {
                    if (linePos != 0 || xMatrix != 1) {
                        create_dl_translation_matrix(
                            MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE] * (xMatrix - 1)), 0, 0);
                    }

                    render_generic_char(strChar);
                    create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[strChar]), 0, 0);
                    xMatrix = 1;
                    linePos++;
                }
#else // VERSION_EU
                if (lineNum >= lowerBound && lineNum <= (lowerBound + linesPerBox)) {
                    render_generic_dialog_char_at_pos(dialog, gDialogX, gDialogY, strChar);
                }
                gDialogX += gDialogCharWidths[strChar];
#endif
        }

#ifdef VERSION_JP
        if (linePos == 12) {
            if (str[strIndex + 1] == DIALOG_CHAR_PERIOD) {
                adjust_pos_and_print_period_char(&xMatrix, &linePos);
                strIndex++;
            }

            if (str[strIndex + 1] == DIALOG_CHAR_COMMA) {
                create_dl_translation_matrix(MENU_MTX_NOPUSH, 10 * xMatrix, 0, 0);
                render_generic_char(DIALOG_CHAR_COMMA);
                strIndex++;
            }

            if (str[strIndex + 1] == DIALOG_CHAR_NEWLINE) {
                strIndex++;
            }

            if (str[strIndex + 1] == DIALOG_CHAR_TERMINATOR) {
                pageState = DIALOG_PAGE_STATE_END;
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                break; // exit loop
            } else {
                lineNum++;
                handle_dialog_scroll_page_state(lineNum, totalLines, &pageState, &xMatrix, &linePos);
            }
        }
#endif

#ifdef VERSION_CN
        strIndex += 2;
#else
        strIndex++;
#endif
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    if (gMenuState == MENU_STATE_DIALOG_OPEN) {
        if (pageState == DIALOG_PAGE_STATE_END) {
            gNextDialogPageStartStrIndex = -1;
        } else {
            gNextDialogPageStartStrIndex = strIndex;
        }
    }

    gLastDialogLineNum = lineNum;
}
#if defined(VERSION_JP) || defined(VERSION_SH)
    #define X_VAL4_1 50
    #define X_VAL4_2 25
    #define Y_VAL4_1 1
    #define Y_VAL4_2 20
#elif defined(VERSION_CN)
    #define X_VAL4_1 58
    #define X_VAL4_2 0
    #define Y_VAL4_1 2
    #define Y_VAL4_2 20
#else
    #define X_VAL4_1 56
    #define X_VAL4_2 9
    #define Y_VAL4_1 2
    #define Y_VAL4_2 16
#endif

void render_dialog_triangle_choice(void) {
    if (gMenuState == MENU_STATE_DIALOG_OPEN) {
        handle_menu_scrolling(MENU_SCROLL_HORIZONTAL, &gMenuLineNum, 1, 2);
    }

    create_dl_translation_matrix(MENU_MTX_NOPUSH, ((gMenuLineNum - 1) * X_VAL4_1) + X_VAL4_2, Y_VAL4_1 - (gLastDialogLineNum * Y_VAL4_2), 0);

    if (gDialogBoxType == DIALOG_TYPE_ROTATE) { // White Text
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    } else { // Black Text
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
    }

    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
}
#if defined(VERSION_JP) || defined(VERSION_SH)
    #define X_VAL5 123.0f
    #define Y_VAL5_1 -20
    #define Y_VAL5_2 2
    #define X_Y_VAL6 0.8f
#elif defined(VERSION_US)
    #define X_VAL5 118.0f
    #define Y_VAL5_1 -16
    #define Y_VAL5_2 5
    #define X_Y_VAL6 0.8f
#elif defined(VERSION_EU)
    #define X_VAL5 122.0f
    #define Y_VAL5_1 -16
    #define Y_VAL5_2 3
    #define X_Y_VAL6 0.5f
#elif defined(VERSION_CN)
    #define X_VAL5 118.0f
    #define Y_VAL5_1 -18
    #define Y_VAL5_2 5
    #define X_Y_VAL6 0.8f
#endif

void render_dialog_triangle_next(s8 linesPerBox) {
    s32 globalTimer = gGlobalTimer;

    if (globalTimer & 8) {
        return;
    }

    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL5, (linesPerBox * Y_VAL5_1) + Y_VAL5_2, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, X_Y_VAL6, X_Y_VAL6, 1.0f);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, -90.0f, 0, 0, 1.0f);

    if (gDialogBoxType == DIALOG_TYPE_ROTATE) { // White Text
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    } else { // Black Text
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
    }

    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}
void handle_special_dialog_text(s16 dialogID) { // dialog ID tables, in order
    // King Bob-omb (Start), Whomp (Start), King Bob-omb (throw him out), Eyerock (Start), Wiggler (Start)
    s16 dialogBossStart[] = { DIALOG_017, DIALOG_114, DIALOG_128, DIALOG_117, DIALOG_150 };
    // Koopa the Quick (BoB), Koopa the Quick (THI), Penguin Race, Fat Penguin Race (120 stars)
    s16 dialogRaceSound[] = { DIALOG_005, DIALOG_009, DIALOG_055, DIALOG_164 };
    // Red Switch, Green Switch, Blue Switch, 100 coins star, Bowser Red Coin Star
    s16 dialogStarSound[] = { DIALOG_010, DIALOG_011, DIALOG_012, DIALOG_013, DIALOG_014 };
    // King Bob-omb (Start), Whomp (Defeated), King Bob-omb (Defeated, missing in JP), Eyerock (Defeated), Wiggler (Defeated)
#if BUGFIX_KING_BOB_OMB_FADE_MUSIC
    s16 dialogBossStop[] = { DIALOG_017, DIALOG_115, DIALOG_116, DIALOG_118, DIALOG_152 };
#else
    //! @bug JP misses King Bob-omb defeated DIALOG_116, meaning that the boss music will still
    //! play after King Bob-omb is defeated until BoB loads it's music after the star cutscene
    s16 dialogBossStop[] = { DIALOG_017, DIALOG_115, DIALOG_118, DIALOG_152 };
#endif
    s16 i;

    for (i = 0; i < (s16) ARRAY_COUNT(dialogBossStart); i++) {
        if (dialogBossStart[i] == dialogID) {
            seq_player_unlower_volume(SEQ_PLAYER_LEVEL, 60);
            play_music(SEQ_PLAYER_LEVEL, SEQUENCE_ARGS(4, SEQ_EVENT_BOSS), 0);
            return;
        }
    }

    for (i = 0; i < (s16) ARRAY_COUNT(dialogRaceSound); i++) {
        if (dialogRaceSound[i] == dialogID && gMenuLineNum == DIALOG_RESPONSE_YES) {
            play_race_fanfare();
            return;
        }
    }

    for (i = 0; i < (s16) ARRAY_COUNT(dialogStarSound); i++) {
        if (dialogStarSound[i] == dialogID && gMenuLineNum == DIALOG_RESPONSE_YES) {
            play_sound(SOUND_MENU_STAR_SOUND, gGlobalSoundSource);
            return;
        }
    }

    for (i = 0; i < (s16) ARRAY_COUNT(dialogBossStop); i++) {
        if (dialogBossStop[i] == dialogID) {
            seq_player_fade_out(SEQ_PLAYER_LEVEL, 1);
            return;
        }
    }
}
#if defined(VERSION_JP) || defined(VERSION_SH)
    #define DIAG_VAL1 20
    #define DIAG_VAL2 240
    #define DIAG_VAL3 130
    #define DIAG_VAL4 4
#else
    #define DIAG_VAL1 16
#if defined(VERSION_US) || defined(VERSION_CN)
    #define DIAG_VAL2 240
#else
    #define DIAG_VAL2 238
#endif
    #define DIAG_VAL3 132
    #define DIAG_VAL4 5
#endif

void render_dialog_entries(void) {
#ifdef VERSION_EU
    s8 lowerBound;
#endif
    void **dialogTable;
    struct DialogEntry *dialog;
#if defined(VERSION_US) || defined(VERSION_SH) || defined(VERSION_CN)
    s8 lowerBound;
#endif

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

    // if the dialog entry is invalid, set the ID to DIALOG_NONE.
    if (dialog == segmented_to_virtual(NULL)) {
        gDialogID = DIALOG_NONE;
        return;
    }

#ifdef VERSION_EU
    gDialogX = 0;
    gDialogY = 0;
#endif

    switch (gMenuState) {
        case MENU_STATE_DIALOG_OPENING:
            if (gDialogBoxAngle == DIALOG_BOX_ANGLE_DEFAULT) {
                play_dialog_sound(gDialogID);
                play_sound(SOUND_MENU_MESSAGE_APPEAR, gGlobalSoundSource);
            }

            if (gDialogBoxType == DIALOG_TYPE_ROTATE) {
                gDialogBoxAngle -= 7.5;
                gDialogBoxScale -= 1.5;
            } else {
                gDialogBoxAngle -= 10.0;
                gDialogBoxScale -= 2.0;
            }

            if (gDialogBoxAngle == 0.0f) {
                gMenuState = MENU_STATE_DIALOG_OPEN;
                gMenuLineNum = 1;
            }
#ifndef VERSION_JP
            lowerBound = 1;
#endif
            break;

        case MENU_STATE_DIALOG_OPEN:
            gDialogBoxAngle = 0.0f;

            if ((gPlayer3Controller->buttonPressed & A_BUTTON)
             || (gPlayer3Controller->buttonPressed & B_BUTTON)) {
                if (gNextDialogPageStartStrIndex == -1) {
                    handle_special_dialog_text(gDialogID);
                    gMenuState = MENU_STATE_DIALOG_CLOSING;
                } else {
                    gMenuState = MENU_STATE_DIALOG_SCROLLING;
                    play_sound(SOUND_MENU_MESSAGE_NEXT_PAGE, gGlobalSoundSource);
                }
            }
#ifndef VERSION_JP
            lowerBound = 1;
#endif
            break;

        case MENU_STATE_DIALOG_SCROLLING:
            gDialogScrollOffsetY += dialog->linesPerBox * 2;

            if (gDialogScrollOffsetY >= dialog->linesPerBox * DIAG_VAL1) {
                gDialogPageStartStrIndex = gNextDialogPageStartStrIndex;
                gMenuState = MENU_STATE_DIALOG_OPEN;
                gDialogScrollOffsetY = 0;
            }
#ifndef VERSION_JP
            lowerBound = (gDialogScrollOffsetY / DIAG_VAL1) + 1;
#endif
            break;

        case MENU_STATE_DIALOG_CLOSING:
            if (gDialogBoxAngle == 20.0f) {
                level_set_transition(0, NULL);
                play_sound(SOUND_MENU_MESSAGE_DISAPPEAR, gGlobalSoundSource);

                if (gDialogBoxType == DIALOG_TYPE_ZOOM) {
                    trigger_cutscene_dialog(2);
                }

                gDialogResponse = gMenuLineNum;
            }

            gDialogBoxAngle += 10.0f;
            gDialogBoxScale += 2.0f;

            if (gDialogBoxAngle == DIALOG_BOX_ANGLE_DEFAULT) {
                gMenuState = MENU_STATE_DEFAULT;
                gDialogID = DIALOG_NONE;
                gDialogPageStartStrIndex = 0;
                gDialogWithChoice = FALSE;
                gNextDialogPageStartStrIndex = 0;
                gDialogResponse = DIALOG_RESPONSE_NONE;
            }
#ifndef VERSION_JP
            lowerBound = 1;
#endif
            break;
    }

    render_dialog_box_type(dialog, dialog->linesPerBox);

#ifdef VERSION_CN
// This isn't really a diff. The iQue compiler doesn't allow the use of ifdefs inside a macro for some reason, so those were eliminated.
#ifdef WIDESCREEN
#define ulx 0
#define lrx SCREEN_WIDTH
#else
#define ulx ensure_nonnegative(dialog->leftOffset)
#define lrx ensure_nonnegative(dialog->leftOffset + DIAG_VAL3)
#endif
    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, ulx, ensure_nonnegative(DIAG_VAL2 - dialog->width),
        lrx, ensure_nonnegative((256 - dialog->width) + (dialog->linesPerBox * 80 / DIAG_VAL4)));
#undef ulx
#undef lrx
#else
    gDPSetScissor(
        gDisplayListHead++, G_SC_NON_INTERLACE,
        // Horizontal scissoring isn't really required and can potentially mess up widescreen enhancements.
#ifdef WIDESCREEN
        0,
#else
        ensure_nonnegative(dialog->leftOffset),
#endif
        ensure_nonnegative(DIAG_VAL2 - dialog->width),
#ifdef VERSION_EU
#ifdef WIDESCREEN
        SCREEN_WIDTH,
#else
        ensure_nonnegative(dialog->leftOffset + (DIAG_VAL3 / gDialogBoxScale)),
#endif
        ensure_nonnegative((240 - dialog->width) + (dialog->linesPerBox * 80 / DIAG_VAL4 / gDialogBoxScale))
#else
#ifdef WIDESCREEN
        SCREEN_WIDTH,
#else
        ensure_nonnegative(dialog->leftOffset + DIAG_VAL3),
#endif
        ensure_nonnegative((240 - dialog->width) + (dialog->linesPerBox * 80 / DIAG_VAL4))
#endif
    );
#endif

#ifdef VERSION_JP
    handle_dialog_text_and_pages(0, dialog);
#else
    handle_dialog_text_and_pages(0, dialog, lowerBound);
#endif

    if (gNextDialogPageStartStrIndex == -1 && gDialogWithChoice == TRUE) {
        render_dialog_triangle_choice();
    }

    #ifdef VERSION_EU
    #undef BORDER_HEIGHT
    #define BORDER_HEIGHT 8
    #endif
    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 2, 2, SCREEN_WIDTH - BORDER_HEIGHT / 2, SCREEN_HEIGHT - BORDER_HEIGHT / 2);
    #ifdef VERSION_EU
    #undef BORDER_HEIGHT
    #define BORDER_HEIGHT 1
    #endif

    if (gNextDialogPageStartStrIndex != -1 && gMenuState == MENU_STATE_DIALOG_OPEN) {
        render_dialog_triangle_next(dialog->linesPerBox);
    }
}
