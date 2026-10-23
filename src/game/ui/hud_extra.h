#ifndef HUD_EXTRA_H
#define HUD_EXTRA_H

#include <PR/ultratypes.h>

extern s8 gRedCoinsCollected;

void reset_red_coins_collected(void);
void render_hud_cannon_reticle(void);
void print_animated_red_coin(s16 x, s16 y);
void render_pause_red_coins(void);

#endif // HUD_EXTRA_H
