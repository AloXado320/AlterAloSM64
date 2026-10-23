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

#ifdef VERSION_EU
#undef LANGUAGE_FUNCTION
#define LANGUAGE_FUNCTION gInGameLanguage
#endif

#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
u8 gDialogCharWidths[256] = { // TODO: Is there a way to auto generate this?
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  6,  6,  6,  6,
    6,  6,  5,  6,  6,  5,  8,  8,  6,  6,  6,  6,  6,  5,  6,  6,
    8,  7,  6,  6,  6,  5,  5,  6,  5,  5,  6,  5,  4,  5,  5,  3,
    7,  5,  5,  5,  6,  5,  5,  5,  5,  5,  7,  7,  5,  5,  4,  4,
    8,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    8,  8,  8,  8,  7,  7,  6,  7,  7,  0,  0,  0,  0,  0,  0,  0,
#ifdef VERSION_EU
    6,  6,  6,  0,  6,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  4,
    5,  5,  5,  5,  6,  6,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,
    5,  5,  5,  0,  6,  6,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  5,  5,  0,  0,  6,  6,  0,  0,  0,  0,  0,  0,  0,  5,  6,
    0,  4,  4,  0,  0,  5,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,
#else
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  6,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
#endif
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
#ifdef VERSION_EU
    7,  5, 10,  5,  9,  8,  4,  0,  0,  0,  0,  5,  5,  6,  5,  0,
#else
    7,  5, 10,  5,  9,  8,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,
#endif
    0,  0,  5,  7,  7,  6,  6,  8,  0,  8, 10,  6,  4, 10,  0,  0
};
#endif
#if defined(VERSION_US) || defined(VERSION_EU)
UNUSED
#endif
#ifdef VERSION_CN
static u8 *alloc_ia8_text_from_i1(u8 *in) {
    s32 i, j;
    u8 l, r;
    u8 bitMask = 0x80;
    u8 *out = alloc_display_list(8 * 8);

    if (out == NULL) {
        return NULL;
    }

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 8;) {
            r = 0;
            l = ((in[i] & (bitMask >> j)) >> (7 - j)) != 0 ? 0xF0 : 0x00;
            j++;
            r = ((in[i] & (bitMask >> j)) >> (7 - j)) != 0 ? 0x0F : 0x00;
            out[i * 4 + j / 2] = l | r;
            CN_DEBUG_PRINTF(("%x, ", out[i * 4 + j / 2]));
            j++;
        }
        CN_DEBUG_PRINTF(("\n"));
    }

    return out;
}
#else
static u8 *alloc_ia8_text_from_i1(u16 *in, s16 width, s16 height) {
    s32 inPos;
    u16 bitMask;
    u8 *out;
    s16 outPos = 0;

    out = (u8 *) alloc_display_list((u32) width * (u32) height);

    if (out == NULL) {
        return NULL;
    }

    for (inPos = 0; inPos < (width * height) / 16; inPos++) {
        bitMask = 0x8000;

        while (bitMask != 0) {
            if (in[inPos] & bitMask) {
                out[outPos] = 0xFF;
            } else {
                out[outPos] = 0x00;
            }

            bitMask /= 2;
            outPos++;
        }
    }

    return out;
}
#endif

#ifdef VERSION_CN
void render_generic_char(u16 c)
#else
void render_generic_char(u8 c)
#endif
{
    void **fontLUT = segmented_to_virtual(main_font_lut);
    void *packedTexture = segmented_to_virtual(fontLUT[c]);
#if defined(VERSION_JP) || defined(VERSION_SH)
    void *unpackedTexture = alloc_ia8_text_from_i1(packedTexture, 8, 16);
#endif

#ifndef VERSION_EU
    gDPPipeSync(gDisplayListHead++);
#endif
#if defined(VERSION_JP) || defined(VERSION_SH)
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_8b, 1, VIRTUAL_TO_PHYSICAL(unpackedTexture));
#else
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, VIRTUAL_TO_PHYSICAL(packedTexture));
#endif
    gSPDisplayList(gDisplayListHead++, dl_ia_text_tex_settings);
#ifdef VERSION_EU
    font_draw_texrect_flip(gDialogX << 2, (gDialogY - 16) << 2,
                           (gDialogX + 8) << 2, gDialogY << 2, 8 << 6, 4 << 6, 1 << 10, 1 << 10);
#endif
}

#ifdef VERSION_EU
u8 *alloc_ia4_tex_from_i1(u8 *in, s16 width, s16 height) {
    u32 size = (u32) width * (u32) height;
    u8 *out;
    s32 inPos;
    s16 outPos = 0;
    u8 bitMask;

    out = (u8 *) alloc_display_list(size);

    if (out == NULL) {
        return NULL;
    }

    for (inPos = 0; inPos < (width * height) / 4; inPos++) {
        bitMask = 0x80;

        while (bitMask != 0) {
            out[outPos] = (in[inPos] & bitMask) ? 0xF0 : 0x00;
            bitMask /= 2;
            out[outPos] = (in[inPos] & bitMask) ? out[outPos] + 0x0F : out[outPos];
            bitMask /= 2;
            outPos++;
        }
    }

    return out;
}

void render_generic_char_at_pos(s16 xPos, s16 yPos, u8 c) {
    void **fontLUT = segmented_to_virtual(main_font_lut);
    void *packedTexture = segmented_to_virtual(fontLUT[c]);
    void *unpackedTexture = alloc_ia4_tex_from_i1(packedTexture, 8, 8);

    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, VIRTUAL_TO_PHYSICAL(unpackedTexture));
    gSPDisplayList(gDisplayListHead++, dl_ia_text_tex_settings);
    font_draw_texrect_flip(xPos << 2, (yPos - 16) << 2, (xPos + 8) << 2, yPos << 2,
                           8 << 6, 4 << 6, 1 << 10, 1 << 10);
}

void render_lowercase_diacritic(s16 *xPos, s16 *yPos, u8 letter, u8 diacritic) {
    render_generic_char_at_pos(*xPos, *yPos, letter);
    render_generic_char_at_pos(*xPos, *yPos, diacritic + 0xE7);
    *xPos += gDialogCharWidths[letter];
}

void render_uppercase_diacritic(s16 *xPos, s16 *yPos, u8 letter, u8 diacritic) {
    render_generic_char_at_pos(*xPos, *yPos, letter);
    render_generic_char_at_pos(*xPos, *yPos - 4, diacritic + 0xE3);
    *xPos += gDialogCharWidths[letter];
}
#endif // VERSION_EU
#ifdef VERSION_CN
void render_generic_char_cn(u16 c) {
    void **fontLUT = segmented_to_virtual(main_font_lut);
    void *packedTexture = segmented_to_virtual(fontLUT[c]);
    void *unpackedTexture = alloc_ia8_text_from_i1(packedTexture);

    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, VIRTUAL_TO_PHYSICAL(unpackedTexture));
    gSPDisplayList(gDisplayListHead++, dl_ia_text_tex_settings);
}
#endif
#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
// First byte is length, rest is string

/*
 * Place the multi-text string according to the ID passed. (US, EU)
 * 0: 'the'
 * 1: 'you'
 */
#if defined(VERSION_US) || defined(VERSION_CN)
void render_multi_text_string(s8 multiTextID)
#elif defined(VERSION_EU)
void render_multi_text_string(s16 *xPos, s16 *yPos, s8 multiTextID)
#endif
{
    s8 i;
    struct MultiTextEntry textLengths[2] = {
        { { 3, TEXT_THE_RAW } },
        { { 3, TEXT_YOU_RAW } },
    };

    for (i = 0; i < textLengths[multiTextID].str[0]; i++) {
#if defined(VERSION_US) || defined(VERSION_CN)
        render_generic_char(textLengths[multiTextID].str[1 + i]);
        create_dl_translation_matrix(
            MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[textLengths[multiTextID].str[1 + i]]), 0.0f, 0.0f);
#elif defined(VERSION_EU)
        render_generic_char_at_pos(*xPos, *yPos, textLengths[multiTextID].str[1 + i]);
        *xPos += gDialogCharWidths[textLengths[multiTextID].str[1 + i]];
#endif
    }
}
#endif

#if defined(VERSION_JP) || defined(VERSION_SH)
    #define CUR_CHAR str[strPos]
    #define MAX_STRING_WIDTH 18
    #define CHAR_WIDTH_SPACE 5.0f
    #define CHAR_WIDTH_DEFAULT 10.0f
#elif defined(VERSION_CN)
    #define CUR_CHAR strChar
    #define MAX_STRING_WIDTH 18
    #define CHAR_WIDTH_SPACE gDialogCharWidths[DIALOG_CHAR_SPACE]
    #define CHAR_WIDTH_DEFAULT gDialogCharWidths[CUR_CHAR]
#else
    #define CUR_CHAR str[strPos]
    #define MAX_STRING_WIDTH 16
    #define CHAR_WIDTH_SPACE gDialogCharWidths[DIALOG_CHAR_SPACE]
    #define CHAR_WIDTH_DEFAULT gDialogCharWidths[CUR_CHAR]
#endif
/**
 * Prints a generic white string.
 * In JP/EU a IA1 texture is used but in US a IA4 texture is used.
 */
void print_generic_string(s16 x, s16 y, const u8 *str) {
    UNUSED s8 mark = DIALOG_MARK_NONE; // unused in EU
    s32 strPos = 0;
    u8 lineNum = 1;
#ifdef VERSION_EU
    s16 xCoord = x;
    s16 yCoord = 240 - y;
#endif
#ifdef VERSION_CN
    s32 strChar;
#endif

#ifndef VERSION_EU
    create_dl_translation_matrix(MENU_MTX_PUSH, x, y, 0.0f);
#endif

#ifdef VERSION_CN
    while (str[strPos] != DIALOG_CHAR_SPECIAL_MODIFIER || str[strPos + 1] != DIALOG_CHAR_TERMINATOR) {
        strChar = str[strPos] << 8 | str[strPos + 1];
#else
    while (str[strPos] != SPECIAL_CHAR(DIALOG_CHAR_TERMINATOR)) {
#endif
        switch (CUR_CHAR) {
#ifdef VERSION_CN
            case SPECIAL_CHAR(DIALOG_CHAR_NEWLINE):
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                create_dl_translation_matrix(MENU_MTX_PUSH, x, y - (lineNum * MAX_STRING_WIDTH), 0.0f);
                lineNum++;
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_STAR_OPEN): // ???
                create_dl_translation_matrix(MENU_MTX_NOPUSH, 32.0f, 0.0f, 0.0f);
                break;
#endif
#ifdef VERSION_EU
            case SPECIAL_CHAR(DIALOG_CHAR_SPACE):
                xCoord += 5;
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_NEWLINE):
                yCoord += 16;
                xCoord = x;
                lineNum++;
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_A_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_A_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_A_UMLAUT):
                render_lowercase_diacritic(&xCoord, &yCoord, ASCII_TO_DIALOG('a'), CUR_CHAR & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_A_UMLAUT): // @bug grave and circumflex (0x64-0x65) are absent here
                render_uppercase_diacritic(&xCoord, &yCoord, ASCII_TO_DIALOG('A'), CUR_CHAR & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_E_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_E_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_E_UMLAUT):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_E_ACUTE):
                render_lowercase_diacritic(&xCoord, &yCoord, ASCII_TO_DIALOG('e'), CUR_CHAR & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_E_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_E_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_E_UMLAUT):
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_E_ACUTE):
                render_uppercase_diacritic(&xCoord, &yCoord, ASCII_TO_DIALOG('E'), CUR_CHAR & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_U_GRAVE):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_U_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_U_UMLAUT):
                render_lowercase_diacritic(&xCoord, &yCoord, ASCII_TO_DIALOG('u'), CUR_CHAR & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_U_UMLAUT): // @bug grave and circumflex (0x84-0x85) are absent here
                render_uppercase_diacritic(&xCoord, &yCoord, ASCII_TO_DIALOG('U'), CUR_CHAR & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_O_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_O_UMLAUT):
                render_lowercase_diacritic(&xCoord, &yCoord, ASCII_TO_DIALOG('o'), CUR_CHAR & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_UPPER_O_UMLAUT): // @bug circumflex (0x95) is absent here
                render_uppercase_diacritic(&xCoord, &yCoord, ASCII_TO_DIALOG('O'), CUR_CHAR & 0xF);
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_I_CIRCUMFLEX):
            case SPECIAL_CHAR(DIALOG_CHAR_LOWER_I_UMLAUT):
                render_lowercase_diacritic(&xCoord, &yCoord, DIALOG_CHAR_I_NO_DIA, CUR_CHAR & 0xF);
                break;
#else // i.e. not EU
            case SPECIAL_CHAR(DIALOG_CHAR_DAKUTEN):
                mark = DIALOG_MARK_DAKUTEN;
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_PERIOD_OR_HANDAKUTEN):
                mark = DIALOG_MARK_HANDAKUTEN;
                break;
#ifndef VERSION_CN
            case SPECIAL_CHAR(DIALOG_CHAR_NEWLINE):
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                create_dl_translation_matrix(MENU_MTX_PUSH, x, y - (lineNum * MAX_STRING_WIDTH), 0.0f);
                lineNum++;
                break;
#endif
            case SPECIAL_CHAR(DIALOG_CHAR_PERIOD):
                create_dl_translation_matrix(MENU_MTX_PUSH, -2.0f, -5.0f, 0.0f);
                render_generic_char(DIALOG_CHAR_PERIOD_OR_HANDAKUTEN);
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                break;
#endif

#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
            case SPECIAL_CHAR(DIALOG_CHAR_SLASH):
#ifdef VERSION_EU
                xCoord += gDialogCharWidths[DIALOG_CHAR_SPACE] * 2;
#else
                create_dl_translation_matrix(
                    MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE] * 2), 0.0f, 0.0f);
#endif
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_MULTI_THE):
#ifdef VERSION_EU
                render_multi_text_string(&xCoord, &yCoord, STRING_THE);
#else
                render_multi_text_string(STRING_THE);
#endif
                break;
            case SPECIAL_CHAR(DIALOG_CHAR_MULTI_YOU):
#ifdef VERSION_EU
                render_multi_text_string(&xCoord, &yCoord, STRING_YOU);
#else
                render_multi_text_string(STRING_YOU);
#endif
                break;
#endif

#ifndef VERSION_EU
            case SPECIAL_CHAR(DIALOG_CHAR_SPACE):
                create_dl_translation_matrix(MENU_MTX_NOPUSH, CHAR_WIDTH_SPACE, 0.0f, 0.0f);
                break;
#ifdef VERSION_JP
                break; // ? needed to match
#endif
#endif

            default:
#ifdef VERSION_EU
                render_generic_char_at_pos(xCoord, yCoord, CUR_CHAR);
                xCoord += gDialogCharWidths[CUR_CHAR];
#else
#ifdef VERSION_CN
                if (strChar >= 0x0100) {
                    strChar = (strChar - 0xFF) * 2 + 0xFE;
                    render_generic_char_cn(strChar);
                    create_dl_translation_matrix(MENU_MTX_NOPUSH, 8.0f, 0.0f, 0.0f);
                    render_generic_char_cn(strChar + 1);
                    create_dl_translation_matrix(MENU_MTX_NOPUSH, 8.0f, 0.0f, 0.0f);
                    break;
                }
#endif
                render_generic_char(CUR_CHAR);
                if (mark != DIALOG_MARK_NONE) {
                    create_dl_translation_matrix(MENU_MTX_PUSH, 5.0f, 5.0f, 0.0f);
                    render_generic_char(DIALOG_CHAR_MARK_START + mark);
                    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                    mark = DIALOG_MARK_NONE;
                }

                create_dl_translation_matrix(MENU_MTX_NOPUSH, CHAR_WIDTH_DEFAULT, 0.0f, 0.0f);
#endif
#ifndef VERSION_JP
                break; // what an odd difference. US (and probably later) versions added a useless break here.
#endif
        }

#ifdef VERSION_CN
        strPos += 2;
#else
        strPos++;
#endif
    }

#ifndef VERSION_EU
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
#endif
}
#ifdef VERSION_EU
void print_menu_char_umlaut(s16 x, s16 y, u8 chr) {
    void **fontLUT = segmented_to_virtual(menu_font_lut);

    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_8b, 1, fontLUT[chr]);
    gDPLoadSync(gDisplayListHead++);
    gDPLoadBlock(gDisplayListHead++, G_TX_LOADTILE, 0, 0, 8 * 8 - 1, CALC_DXT(8, G_IM_SIZ_8b_BYTES));
    font_draw_texrect(x << 2, y << 2, (x + 8) << 2, (y + 8) << 2, 1 << 10, 1 << 10);

    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_8b, 1, fontLUT[DIALOG_CHAR_UMLAUT]);
    gDPLoadSync(gDisplayListHead++);
    gDPLoadBlock(gDisplayListHead++, G_TX_LOADTILE, 0, 0, 8 * 8 - 1, CALC_DXT(8, G_IM_SIZ_8b_BYTES));
    font_draw_texrect(x << 2, (y - 4) << 2, (x + 8) << 2, (y + 4) << 2, 1 << 10, 1 << 10);
}
#endif

void print_menu_generic_string(s16 x, s16 y, const u8 *str) {
    UNUSED s8 mark = DIALOG_MARK_NONE; // unused in EU
    s32 strPos = 0;
    u32 curX = x;
    u32 curY = y;
    void **fontLUT = segmented_to_virtual(menu_font_lut);

    while (str[strPos] != DIALOG_CHAR_TERMINATOR) {
        switch (str[strPos]) {
#ifdef VERSION_EU
            case DIALOG_CHAR_UPPER_A_UMLAUT:
                print_menu_char_umlaut(curX, curY, ASCII_TO_DIALOG('A'));
                curX += gDialogCharWidths[str[strPos]];
                break;
            case DIALOG_CHAR_UPPER_U_UMLAUT:
                print_menu_char_umlaut(curX, curY, ASCII_TO_DIALOG('U'));
                curX += gDialogCharWidths[str[strPos]];
                break;
            case DIALOG_CHAR_UPPER_O_UMLAUT:
                print_menu_char_umlaut(curX, curY, ASCII_TO_DIALOG('O'));
                curX += gDialogCharWidths[str[strPos]];
                break;
#else
            case DIALOG_CHAR_DAKUTEN:
                mark = DIALOG_MARK_DAKUTEN;
                break;
            case DIALOG_CHAR_PERIOD_OR_HANDAKUTEN:
                mark = DIALOG_MARK_HANDAKUTEN;
                break;
#endif
            case DIALOG_CHAR_SPACE:
                curX += 4;
                break;
            default:
                gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_8b, 1, fontLUT[str[strPos]]);
                gDPLoadSync(gDisplayListHead++);
                gDPLoadBlock(gDisplayListHead++, G_TX_LOADTILE, 0, 0, 8 * 8 - 1, CALC_DXT(8, G_IM_SIZ_8b_BYTES));
                font_draw_texrect(curX << 2, curY << 2, (curX + 8) << 2,
                                  (curY + 8) << 2, 1 << 10, 1 << 10);

#ifndef VERSION_EU
                if (mark != DIALOG_MARK_NONE) {
                    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_8b, 1, fontLUT[DIALOG_CHAR_MARK_START + mark]);
                    gDPLoadSync(gDisplayListHead++);
                    gDPLoadBlock(gDisplayListHead++, G_TX_LOADTILE, 0, 0, 8 * 8 - 1, CALC_DXT(8, G_IM_SIZ_8b_BYTES));
                    font_draw_texrect((curX + 6) << 2, (curY - 7) << 2,
                                      (curX + 6 + 8) << 2, (curY - 7 + 8) << 2, 1 << 10, 1 << 10);

                    mark = DIALOG_MARK_NONE;
                }
#endif
#if defined(VERSION_JP) || defined(VERSION_SH) || defined(VERSION_CN)
                curX += 9;
#else
                curX += gDialogCharWidths[str[strPos]];
#endif
        }
        strPos++;
    }
}
// EU has both get_str_x_pos_from_center and get_str_x_pos_from_center_scale
// JP, US, Shindou and iQue only implement one or the other
#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
s16 get_str_x_pos_from_center(s16 centerPos, u8 *str, UNUSED f32 scale) {
    s16 strPos = 0;
    f32 spacesWidth = 0.0f;

#ifdef VERSION_CN
    while (!(str[strPos] == DIALOG_CHAR_SPECIAL_MODIFIER && str[strPos + 1] == DIALOG_CHAR_TERMINATOR)) {
        u16 character = (str[strPos] << 8) | str[strPos + 1];
        if (character <= 0xFF) {
            spacesWidth += gDialogCharWidths[character];
        } else {
            spacesWidth += 16.0;
        }
        strPos += 2;
    }
#else
    while (str[strPos] != DIALOG_CHAR_TERMINATOR) {
        spacesWidth += gDialogCharWidths[str[strPos]];
        strPos++;
    }
#endif
    // return the x position of where the string starts as half the string's
    // length from the position of the provided center.
    return (s16)(centerPos - (s16)(spacesWidth / 2.0));
}
#endif

#if defined(VERSION_JP) || defined(VERSION_EU) || defined(VERSION_SH)
s16 get_str_x_pos_from_center_scale(s16 centerPos, u8 *str, f32 scale) {
    s16 strPos = 0;
    f32 charsWidth = 0.0f;
    f32 spacesWidth = 0.0f;

    while (str[strPos] != DIALOG_CHAR_TERMINATOR) {
        //! EU checks for dakuten and handakuten despite dialog code unable to handle it
        if (str[strPos] == DIALOG_CHAR_SPACE) {
            spacesWidth += 1.0;
        } else if (str[strPos] != DIALOG_CHAR_DAKUTEN
                   && str[strPos] != DIALOG_CHAR_PERIOD_OR_HANDAKUTEN) {
            charsWidth += 1.0;
        }
        strPos++;
    }
    // return the x position of where the string starts as half the string's
    // length from the position of the provided center.
    return (f32) centerPos - (scale * (charsWidth / 2.0)) - ((scale / 2.0) * (spacesWidth / 2.0));
}
#endif

#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
s16 get_string_width(u8 *str) {
    s16 strPos = 0;
    s16 width = 0;

#ifdef VERSION_CN
    while (!(str[strPos] == DIALOG_CHAR_SPECIAL_MODIFIER && str[strPos + 1] == DIALOG_CHAR_TERMINATOR)) {
        u16 character = (str[strPos] << 8) | str[strPos + 1];
        if (character <= 0xFF) {
            width += gDialogCharWidths[character];
        } else {
            width += 16;
        }
        strPos += 2;
    }
#else
    while (str[strPos] != DIALOG_CHAR_TERMINATOR) {
        width += gDialogCharWidths[str[strPos]];
        strPos++;
    }
#endif
    return width;
}
#endif
void int_to_str(s32 num, u8 *dst) {
    s32 digit1;
    s32 digit2;
    s32 digit3;

    s8 pos = 0;

    if (num > 999) {
        dst[0] = 0x00; dst[1] = DIALOG_CHAR_TERMINATOR;
        return;
    }

    digit1 = num / 100;
    digit2 = (num - digit1 * 100) / 10;
    digit3 = (num - digit1 * 100) - (digit2 * 10);

    if (digit1 != 0) {
        dst[pos] = digit1; pos++;
    }

    if (digit2 != 0 || digit1 != 0) {
        dst[pos] = digit2; pos++;
    }

    dst[pos] = digit3;
    pos++;
    dst[pos] = DIALOG_CHAR_TERMINATOR;
}

#ifdef VERSION_CN
void int_to_str_2(s32 num, u8 *dst) {
    s32 digit1;
    s32 digit2;
    s32 digit3;

    s8 pos = 0;

    if (num > 999) {
        dst[0] = 0x00;
        dst[1] = 0x00;
        dst[2] = DIALOG_CHAR_SPECIAL_MODIFIER;
        dst[3] = DIALOG_CHAR_TERMINATOR;
        return;
    }

    digit1 = num / 100;
    digit2 = (num - digit1 * 100) / 10;
    digit3 = (num - digit1 * 100) - (digit2 * 10);

    if (digit1 != 0) {
        dst[pos] = 0x00;
        pos++;
        dst[pos] = digit1;
        pos++;
    }

    if (digit2 != 0 || digit1 != 0) {
        dst[pos] = 0x00;
        pos++;
        dst[pos] = digit2;
        pos++;
    }

    dst[pos] = 0x00;
    pos++;
    dst[pos] = digit3;
    pos++;
    dst[pos] = DIALOG_CHAR_SPECIAL_MODIFIER;
    pos++;
    dst[pos] = DIALOG_CHAR_TERMINATOR;
}
#endif

void font_draw_texrect(s32 ulx, s32 uly, s32 lrx, s32 lry, u32 dsdx, u32 dtdy) {
    gSPTextureRectangle(gDisplayListHead++, ulx, uly, lrx, lry, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);
}

void font_draw_texrect_flip(s32 ulx, s32 uly, s32 lrx, s32 lry, u32 uls, u32 ult, u32 dsdx, u32 dtdy) {
    gSPTextureRectangleFlip(gDisplayListHead++, ulx, uly, lrx, lry, G_TX_RENDERTILE, uls, ult, dsdx, dtdy);
}
