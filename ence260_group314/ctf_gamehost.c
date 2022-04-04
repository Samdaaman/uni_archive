/** 
    @file   ctf_gamehost.c
    @author Sam Hogan and Henry Seaton
    @brief  A module which provides the game logic for a top down capture the flag style game
            This includes functions which are considered major game events such as resetting players
    @date   16th October 2019
 */


#include "ctf_gamehost.h"

// Private method, resets my_player by reseting its position to one with a disadvantage but does not reset the score
static void __reset_my_player_with_disadvantage(Player *my_player);

// Updates the overall game based of the position of the two players, this could trigger a player reset
// Events include but are not limited to, scoring and getting tug
void game_event_update(Player *my_player, Player *opponent_player)
{
    // Check and see if the opponents flag has been taken
    if (my_player->row == OPPONENT_FLAG_ROW && my_player->col == OPPONENT_FLAG_COL)
        my_player->has_flag = true;

    // Check to see if a player was tug
    if (opponent_player->row == my_player->row && opponent_player->col == my_player->col)
    {
        if (my_player->col > GAME_COLS/2)
            // Our player was tug by the opponent so send my player back to the start
            __reset_my_player_with_disadvantage(my_player);
    }
    // Check (ultra rare case) both players crossed at the same time
    if (my_player->col > GAME_COLS/2 && opponent_player->col <= GAME_COLS/2)
    {
        // Resolve the glitch in the matrix by sending both players back to their respective halves
        my_player->col = GAME_COLS/2;
    }
    if (my_player->col <= GAME_COLS/2 && my_player->has_flag == true)
    {
        // my_player scored a point, reset the game
        my_player->score ++;
        my_player->displaying_score = true;
        __reset_my_player_with_disadvantage(my_player);
    }

}

// Resets my_player back to their initial start position but does not clear score
void reset_my_player(Player *my_player)
{
    my_player->has_flag = false;
    my_player->row = PLAYER_START_ROW;
    my_player->col = PLAYER_START_COL;
}

// Private method, resets my_player by reseting its position to one with a disadvantage but does not reset the score
static void __reset_my_player_with_disadvantage(Player *my_player)
{
    my_player->has_flag = false;
    my_player->row = PLAYER_START_ROW;
    my_player->col = PLAYER_START_COL_DISADVANTAGE;
}

// Resets my player and the game which includes setting my_player->score to zero
void reset_game(Player *my_player, Player *opponent_player)
{
    reset_my_player(my_player);
    my_player->trigger_game_reset = false;
    my_player->score = 0;
    opponent_player->has_flag = false;
    opponent_player->displaying_score = false;
    opponent_player->trigger_game_reset = false;
    opponent_player->score = 0;
}