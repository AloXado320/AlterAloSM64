#ifndef DIALOG_H
#define DIALOG_H

#include <PR/ultratypes.h>

struct DialogEntry {
    /*0x00*/ u32 unused;
    /*0x04*/ s8 linesPerBox;
    /*0x06*/ s16 leftOffset;
    /*0x08*/ s16 width;
    /*0x0C*/ const u8 *str;
};

// gDialogResponse
enum DialogResponseDefines {
    DIALOG_RESPONSE_NONE,
    DIALOG_RESPONSE_YES,
    DIALOG_RESPONSE_NO,
    DIALOG_RESPONSE_NOT_DEFINED
};

extern s16 gDialogID;
extern s32 gDialogResponse;

s16 get_dialog_id(void);
void create_dialog_box(s16 dialog);
void create_dialog_box_with_var(s16 dialog, s32 dialogVar);
void create_dialog_inverted_box(s16 dialog);
void create_dialog_box_with_response(s16 dialog);
void reset_dialog_render_state(void);
void render_dialog_entries(void);

#endif // DIALOG_H
