/**
    @file   ctf_player.h
    @author Sam Hogan and Henry Seaton
    @brief  A module containing a player struct and common update functions.
    @date   16th October 2019
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "system.h"

// Defines flag position
#define MY_FLAG_ROW 1
#define MY_FLAG_COL 1

// Defines maximum column positions
#define MAX_ROW 7
#define MAX_COL_ON_SCREEN 5
#define MAX_COL_BOTH_SCREENS 10

// Player struct to contain location and data
struct player_s {
    int8_t row; // 1 to 7 (y)
    int8_t col; // 1 to 10, (x)
    bool has_flag;
    uint8_t score;
    bool displaying_score;
    bool trigger_game_reset;
};

typedef struct player_s Player;

// Creates new player object using default values
Player spawn_player(uint8_t row, uint8_t col);

// Updates player location from navswitch input
void player_update(Player* player, bool is_opponent_on_my_side);

// Displays players onscreen
void display_players(Player* my_player, Player* opponent_player);

#endif
