/**
    @file   ctf_player.c
    @author Sam Hogan and Henry Seaton
    @brief  A module containing a player struct and common update functions.
    @date   16th October 2019
 */

#include "ctf_player.h"
#include "navswitch.h"
#include "display.h"

// Creates new player object using default values
Player spawn_player(uint8_t row, uint8_t col)
{
    Player newplayer = {row, col, 0, 0, 0, 0};
    return newplayer;
}

// Updates player location from navswitch input
void player_update(Player* player, bool is_opponent_on_my_side)
{
    navswitch_update();
    if (navswitch_push_event_p(NAVSWITCH_NORTH) && player->row > 1)
    {
        player->row--;
    }
    if (navswitch_push_event_p(NAVSWITCH_SOUTH) && player->row < MAX_ROW)
    {
        player->row++;
    }
    if (navswitch_push_event_p(NAVSWITCH_EAST) && player->col > 1)
    {
        player->col--;
    }
    if (navswitch_push_event_p(NAVSWITCH_WEST) && player->col < MAX_COL_BOTH_SCREENS && !is_opponent_on_my_side)
    {
        player->col++;
    }
    if (navswitch_push_event_p(NAVSWITCH_WEST) && player->col < MAX_COL_ON_SCREEN && is_opponent_on_my_side)
    {
        // Prevents the player moving to other screen if the opponent is onscreen.
        player->col++;
    }
}

// Displays players onscreen
void display_players(Player* my_player, Player* opponent_player)
{
    display_clear();
    display_pixel_set(5 - my_player->col, my_player->row - 1, 1);
    display_pixel_set(5 - opponent_player->col, opponent_player->row - 1, 1);
    static uint8_t flag_flash_counter = 0;
    if (flag_flash_counter & (1 << MAX_COL_ON_SCREEN) && !opponent_player->has_flag)
    {
        display_pixel_set(MAX_COL_ON_SCREEN - MY_FLAG_COL, MY_FLAG_ROW - 1, 1);
    }
    flag_flash_counter ++;
    display_update();
}
