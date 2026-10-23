#ifndef FONT_STRING_H
#define FONT_STRING_H

#include <PR/ultratypes.h>

#define ASCII_TO_DIALOG(asc)                                       \
    (((asc) >= '0' && (asc) <= '9') ? ((asc) - '0') :              \
     ((asc) >= 'A' && (asc) <= 'Z') ? ((asc) - 'A' + 0x0A) :       \
     ((asc) >= 'a' && (asc) <= 'z') ? ((asc) - 'a' + 0x24) : 0x00)

enum SpecialFontChars {
    GLOBAL_CHAR_SPACE = 0x9E,
#ifdef VERSION_CN
    GLOBAL_CHAR_NEWLINE = 0xFE,
#endif
    GLOBAL_CHAR_TERMINATOR = 0xFF
};

enum DialogMark {
    DIALOG_MARK_NONE,
    DIALOG_MARK_DAKUTEN,
    DIALOG_MARK_HANDAKUTEN
};

// definitions for some of the special characters defined in charmap.txt
enum DialogSpecialChars {
#ifdef VERSION_EU
    DIALOG_CHAR_LOWER_A_GRAVE = 0x60,      // 'a' grave
    DIALOG_CHAR_LOWER_A_CIRCUMFLEX = 0x61, // 'a' circumflex
    DIALOG_CHAR_LOWER_A_UMLAUT = 0x62,     // 'a' umlaut
    DIALOG_CHAR_UPPER_A_GRAVE = 0x64,      // 'A' grave
    DIALOG_CHAR_UPPER_A_CIRCUMFLEX = 0x65, // 'A' circumflex
    DIALOG_CHAR_UPPER_A_UMLAUT = 0x66,     // 'A' umlaut
    DIALOG_CHAR_LOWER_E_GRAVE = 0x70,      // 'e' grave
    DIALOG_CHAR_LOWER_E_CIRCUMFLEX = 0x71, // 'e' circumflex
    DIALOG_CHAR_LOWER_E_UMLAUT = 0x72,     // 'e' umlaut
    DIALOG_CHAR_LOWER_E_ACUTE = 0x73,      // 'e' acute
    DIALOG_CHAR_UPPER_E_GRAVE = 0x74,      // 'E' grave
    DIALOG_CHAR_UPPER_E_CIRCUMFLEX = 0x75, // 'E' circumflex
    DIALOG_CHAR_UPPER_E_UMLAUT = 0x76,     // 'E' umlaut
    DIALOG_CHAR_UPPER_E_ACUTE = 0x77,      // 'E' acute
    DIALOG_CHAR_LOWER_U_GRAVE = 0x80,      // 'u' grave
    DIALOG_CHAR_LOWER_U_CIRCUMFLEX = 0x81, // 'u' circumflex
    DIALOG_CHAR_LOWER_U_UMLAUT = 0x82,     // 'u' umlaut
    DIALOG_CHAR_UPPER_U_GRAVE = 0x84,      // 'U' grave
    DIALOG_CHAR_UPPER_U_CIRCUMFLEX = 0x85, // 'U' circumflex
    DIALOG_CHAR_UPPER_U_UMLAUT = 0x86,     // 'U' umlaut
    DIALOG_CHAR_LOWER_O_CIRCUMFLEX = 0x91, // 'o' circumflex
    DIALOG_CHAR_LOWER_O_UMLAUT = 0x92,     // 'o' umlaut
    DIALOG_CHAR_UPPER_O_CIRCUMFLEX = 0x95, // 'O' circumflex
    DIALOG_CHAR_UPPER_O_UMLAUT = 0x96,     // 'O' umlaut
    DIALOG_CHAR_LOWER_I_CIRCUMFLEX = 0xA1, // 'i' circumflex
    DIALOG_CHAR_LOWER_I_UMLAUT = 0xA2,     // 'i' umlaut
    DIALOG_CHAR_I_NO_DIA = 0xEB,           // 'i' without diacritic
    DIALOG_CHAR_DOUBLE_LOW_QUOTE = 0xF0,   // German opening quotation mark
#endif
#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
    DIALOG_CHAR_SLASH = 0xD0,
    DIALOG_CHAR_MULTI_THE = 0xD1, // 'the'
    DIALOG_CHAR_MULTI_YOU = 0xD2, // 'you'
#endif
    DIALOG_CHAR_PERIOD = 0x6E,
    DIALOG_CHAR_COMMA = 0x6F,
    DIALOG_CHAR_SPACE = 0x9E,
    DIALOG_CHAR_STAR_COUNT = 0xE0, // number of stars
    DIALOG_CHAR_UMLAUT = 0xE9,
    DIALOG_CHAR_MARK_START = 0xEF,
    DIALOG_CHAR_DAKUTEN = DIALOG_CHAR_MARK_START + DIALOG_MARK_DAKUTEN,
    DIALOG_CHAR_PERIOD_OR_HANDAKUTEN = DIALOG_CHAR_MARK_START + DIALOG_MARK_HANDAKUTEN,
    DIALOG_CHAR_STAR_FILLED = 0xFA,
    DIALOG_CHAR_STAR_OPEN = 0xFD,
    DIALOG_CHAR_NEWLINE = 0xFE,
    DIALOG_CHAR_TERMINATOR = 0xFF
};

#ifdef VERSION_CN
#define DIALOG_CHAR_SPECIAL_MODIFIER 0xFF
#define SPECIAL_CHAR(x) (DIALOG_CHAR_SPECIAL_MODIFIER << 8 | (x))
#else
#define SPECIAL_CHAR(x) (x)
#endif

#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
// First byte is length, rest is string
struct MultiTextEntry {
    u8 str[5];
};

#define TEXT_THE_RAW ASCII_TO_DIALOG('t'), ASCII_TO_DIALOG('h'), ASCII_TO_DIALOG('e'), 0x00
#define TEXT_YOU_RAW ASCII_TO_DIALOG('y'), ASCII_TO_DIALOG('o'), ASCII_TO_DIALOG('u'), 0x00

enum MultiStringIDs { STRING_THE, STRING_YOU };

extern u8 gDialogCharWidths[256];
#endif

#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
s16 get_str_x_pos_from_center(s16 centerPos, u8 *str, f32 scale);
#endif
#if defined(VERSION_JP) || defined(VERSION_SH)
#define get_str_x_pos_from_center get_str_x_pos_from_center_scale
#endif
#if defined(VERSION_JP) || defined(VERSION_EU) || defined(VERSION_SH)
s16 get_str_x_pos_from_center_scale(s16 centerPos, u8 *str, f32 scale);
#endif
#if defined(VERSION_US) || defined(VERSION_EU) || defined(VERSION_CN)
s16 get_string_width(u8 *str);
#endif

void int_to_str(s32 num, u8 *dst);
#ifdef VERSION_CN
void int_to_str_2(s32 num, u8 *dst);
#define INT_TO_STR_DIFF int_to_str_2
#else
#define INT_TO_STR_DIFF int_to_str
#endif

#ifdef VERSION_CN
void render_generic_char(u16 c);
#else
void render_generic_char(u8 c);
#endif
#ifdef VERSION_CN
void render_generic_char_cn(u16 c);
#endif
#ifdef VERSION_EU
u8 *alloc_ia4_tex_from_i1(u8 *in, s16 width, s16 height);
void render_generic_char_at_pos(s16 xPos, s16 yPos, u8 c);
void render_lowercase_diacritic(s16 *xPos, s16 *yPos, u8 letter, u8 diacritic);
void render_uppercase_diacritic(s16 *xPos, s16 *yPos, u8 letter, u8 diacritic);
#endif

void print_generic_string(s16 x, s16 y, const u8 *str);
void print_menu_generic_string(s16 x, s16 y, const u8 *str);

void font_draw_texrect(s32 ulx, s32 uly, s32 lrx, s32 lry, u32 dsdx, u32 dtdy);
void font_draw_texrect_flip(s32 ulx, s32 uly, s32 lrx, s32 lry, u32 uls, u32 ult, u32 dsdx, u32 dtdy);
void font_draw_text_quad(void);

#endif // FONT_STRING_H
