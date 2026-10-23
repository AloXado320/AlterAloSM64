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

#include "font_hud.h"
#include "font_string.h"

#ifdef VERSION_EU
#undef LANGUAGE_FUNCTION
#define LANGUAGE_FUNCTION gInGameLanguage
#endif

#ifdef VERSION_EU
void print_hud_char_umlaut(s16 x, s16 y, u8 chr) {
    void **fontLUT = segmented_to_virtual(main_hud_lut);

    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, fontLUT[chr]);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_load_tex_block);
    font_draw_texrect(x << 2, y << 2, (x + 16) << 2, (y + 16) << 2, 1 << 10, 1 << 10);

    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, fontLUT[GLYPH_UMLAUT]);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_load_tex_block);
    font_draw_texrect(x << 2, (y - 4) << 2, (x + 16) << 2, (y + 12) << 2, 1 << 10, 1 << 10);
}
#endif
/**
 * Prints a hud string depending of the hud table list defined.
 */
void print_hud_lut_string(s8 hudLUT, s16 x, s16 y, const u8 *str) {
    s32 strPos = 0;
    void **hudLUT1 = segmented_to_virtual(menu_hud_lut); // Japanese Menu HUD Color font
    void **hudLUT2 = segmented_to_virtual(main_hud_lut); // 0-9 A-Z HUD Color Font
    u32 curX = x;
    u32 curY = y;

#ifdef VERSION_CN
    u32 xOffset = 0;
#else
    u32 xStride; // X separation

    if (hudLUT == HUD_LUT_JPMENU) {
        xStride = 16;
    } else { // HUD_LUT_GLOBAL
#ifdef VERSION_JP
        xStride = 14;
#else
        xStride = 12; //? Shindou uses this.
#endif
    }
#endif

    while (str[strPos] != GLOBAL_CHAR_TERMINATOR) {
#ifndef VERSION_JP
        switch (str[strPos]) {
#ifdef VERSION_CN
            case GLOBAL_CHAR_NEWLINE:
                curY += 16;
                curX -= xOffset;
                break;
#elif defined(VERSION_EU)
            case GLOBAL_CHAR_SPACE:
                curX += xStride / 2;
                break;
            case HUD_CHAR_A_UMLAUT:
                print_hud_char_umlaut(curX, curY, ASCII_TO_DIALOG('A'));
                curX += xStride;
                break;
            case HUD_CHAR_O_UMLAUT:
                print_hud_char_umlaut(curX, curY, ASCII_TO_DIALOG('O'));
                curX += xStride;
                break;
            case HUD_CHAR_U_UMLAUT:
                print_hud_char_umlaut(curX, curY, ASCII_TO_DIALOG('U'));
                curX += xStride;
                break;
#else
            case GLOBAL_CHAR_SPACE:
                curX += 8;
                break;
#endif
            default:
#endif
                gDPPipeSync(gDisplayListHead++);

#ifndef VERSION_CN
                if (hudLUT == HUD_LUT_JPMENU) {
                    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, hudLUT1[str[strPos]]);
                }

                if (hudLUT == HUD_LUT_GLOBAL) {
#endif
                    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, hudLUT2[str[strPos]]);
#ifndef VERSION_CN
                }
#endif

                gSPDisplayList(gDisplayListHead++, dl_rgba16_load_tex_block);
                font_draw_texrect(curX << 2, curY << 2, (curX + 16) << 2,
                                  (curY + 16) << 2, 1 << 10, 1 << 10);

#ifdef VERSION_CN
                if (str[strPos] < 0x3A) {
                    curX += 12;
                    xOffset += 12;
                } else {
                    curX += 16;
                    xOffset += 16;
                }
#else
                curX += xStride;
#endif
#ifndef VERSION_JP
        }
#endif
        strPos++;
    }
}
u8 gHudSymCoin[] = { GLYPH_COIN, GLYPH_SPACE };
u8 gHudSymX[] = { GLYPH_MULTIPLY, GLYPH_SPACE };

void print_hud_my_score_coins(s32 useCourseCoinScore, s8 fileIndex, s8 courseIndex, s16 x, s16 y) {
    u8 strNumCoins[4];
    s16 numCoins;

    if (!useCourseCoinScore) {
        numCoins = (u16)(save_file_get_max_coin_score(courseIndex) & 0xFFFF);
    } else {
        numCoins = save_file_get_course_coin_score(fileIndex, courseIndex);
    }

    if (numCoins != 0) {
        print_hud_lut_string(HUD_LUT_GLOBAL, x, y, gHudSymCoin);
        print_hud_lut_string(HUD_LUT_GLOBAL, x + 16, y, gHudSymX);
        int_to_str(numCoins, strNumCoins);
        print_hud_lut_string(HUD_LUT_GLOBAL, x + 32, y, strNumCoins);
    }
}

void print_hud_my_score_stars(s8 fileIndex, s8 courseIndex, s16 x, s16 y) {
    u8 strStarCount[4];
    s16 starCount;
    u8 textSymStar[] = { GLYPH_STAR, GLYPH_SPACE };
    UNUSED u16 unused;
    u8 textSymX[] = { GLYPH_MULTIPLY, GLYPH_SPACE };

    starCount = save_file_get_course_star_count(fileIndex, courseIndex);

    if (starCount != 0) {
        print_hud_lut_string(HUD_LUT_GLOBAL, x, y, textSymStar);
        print_hud_lut_string(HUD_LUT_GLOBAL, x + 16, y, textSymX);
        int_to_str(starCount, strStarCount);
        print_hud_lut_string(HUD_LUT_GLOBAL, x + 32, y, strStarCount);
    }
}
