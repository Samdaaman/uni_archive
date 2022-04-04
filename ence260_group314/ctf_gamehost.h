/** 
    @file   ctf_gamehost.h
    @author Sam Hogan and Henry Seaton
    @brief  An interface for a module which provides the game logic for a top down capture the flag style game
            This includes functions which are considered major game events such as resetting players
    @date   16th October 2019
 */


#ifndef CTF_GAMEHOST_H
#define CTF_GAMEHOST_H

#include "system.h"
#include "ctf_player.h"

#define OPPONENT_FLAG_ROW 7
#define OPPONENT_FLAG_COL 10
#define GAME_ROWS 7
#define GAME_COLS 10
#define PLAYER_START_ROW 4
#define PLAYER_START_COL 3
#define PLAYER_START_COL_DISADVANTAGE 1

// Updates the overall game based of the position of the two players, this could trigger a player reset
// Events include but are not limited to, scoring and getting tug
void game_event_update(Player *my_player, Player *opponent_player);

// Resets my_player back to their initial start position but does not clear score
void reset_my_player(Player *my_player);

// Resets my player and the game which includes setting my_player->score to zero
void reset_game(Player *my_player, Player *opponent_player);

#endif
