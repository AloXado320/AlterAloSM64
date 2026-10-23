#ifndef UI_H
#define UI_H

#include <PR/ultratypes.h>

#define MENU_MTX_PUSH   1
#define MENU_MTX_NOPUSH 2

#define MENU_SCROLL_VERTICAL   1
#define MENU_SCROLL_HORIZONTAL 2

enum MenuState {
    MENU_STATE_0,
    MENU_STATE_1,
    MENU_STATE_2,
    MENU_STATE_3,
    MENU_STATE_DEFAULT = MENU_STATE_0,

    // Dialog
    MENU_STATE_DIALOG_OPENING = MENU_STATE_0,
    MENU_STATE_DIALOG_OPEN = MENU_STATE_1,
    MENU_STATE_DIALOG_SCROLLING = MENU_STATE_2,
    MENU_STATE_DIALOG_CLOSING = MENU_STATE_3,

    // Pause Screen
    MENU_STATE_PAUSE_SCREEN_OPENING = MENU_STATE_0,
    MENU_STATE_PAUSE_SCREEN_COURSE = MENU_STATE_1,
    MENU_STATE_PAUSE_SCREEN_CASTLE = MENU_STATE_2,

    // Course Complete Screen
    MENU_STATE_COURSE_COMPLETE_SCREEN_OPENING = MENU_STATE_0,
    MENU_STATE_COURSE_COMPLETE_SCREEN_OPEN = MENU_STATE_1
};

enum MenuMode {
    MENU_MODE_NONE = -1,
    MENU_MODE_UNUSED_0,
    MENU_MODE_RENDER_PAUSE_SCREEN,
    MENU_MODE_RENDER_COURSE_COMPLETE_SCREEN,
    MENU_MODE_UNUSED_3
};

extern s8 gMenuState;
extern s16 gMenuMode;
extern u16 gMenuTextAlpha;
extern u16 gMenuTextColorTransTimer;
extern s8 gMenuLineNum;
extern s8 gDialogCourseActNum;

#ifdef VERSION_EU
extern s32 gInGameLanguage;
extern s16 gDialogX;
extern s16 gDialogY;
// Shared "Course" text, one entry per EU language. Defined in pause_menu.c
// and printed by both the pause menu and the course complete screen.
extern u8 gTextCourse[][7];
#endif

void create_dl_identity_matrix(void);
void create_dl_translation_matrix(s8 pushOp, f32 x, f32 y, f32 z);
void create_dl_rotation_matrix(s8 pushOp, f32 a, f32 x, f32 y, f32 z);
void create_dl_scale_matrix(s8 pushOp, f32 x, f32 y, f32 z);
void create_dl_ortho_matrix(void);

void handle_menu_scrolling(s8 scrollDirection, s8 *currentIndex, s8 minIndex, s8 maxIndex);
void set_menu_mode(s16 mode);
void shade_screen(void);
s16 render_menus_and_dialogs(void);

#endif // UI_H
