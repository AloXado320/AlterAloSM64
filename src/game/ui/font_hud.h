#ifndef FONT_HUD_H
#define FONT_HUD_H

#include <PR/ultratypes.h>

// Japanese File Select uses an unique table
// to print specific Japanese HUD chars
#define HUD_LUT_JPMENU 1
#define HUD_LUT_GLOBAL 2

// For file select JP HUD difference
#if defined(VERSION_JP) || defined(VERSION_SH)
#define HUD_LUT_DIFF HUD_LUT_JPMENU
#else
#define HUD_LUT_DIFF HUD_LUT_GLOBAL
#endif

// They didn't change it everywhere in iQue
#ifdef VERSION_CN
#define HUD_LUT_DIFF2 HUD_LUT_JPMENU
#else
#define HUD_LUT_DIFF2 HUD_LUT_DIFF
#endif

// EU only
enum HudSpecialCharsEU {
    HUD_CHAR_A_UMLAUT = 0x3A,
    HUD_CHAR_O_UMLAUT = 0x3B,
    HUD_CHAR_U_UMLAUT = 0x3C
};

void print_hud_lut_string(s8 hudLUT, s16 x, s16 y, const u8 *str);
void print_hud_my_score_coins(s32 useCourseCoinScore, s8 fileIndex, s8 courseIndex, s16 x, s16 y);
void print_hud_my_score_stars(s8 fileIndex, s8 courseIndex, s16 x, s16 y);

#endif // FONT_HUD_H
