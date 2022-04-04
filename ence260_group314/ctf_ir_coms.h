/** 
    @file   ctf_ir_coms.h
    @author Sam Hogan and Henry Seaton
    @brief  An interface for a module which provides IR communication between Funkits in a Capture the Flag style game
            Could be adapted to work with other top down two player games 
    @date   16th October 2019
 */

#ifndef CTF_IR_COMS_H
#define CTF_IR_COMS_H

#include "system.h"
#include "ctf_player.h"

#define PLAYER_1 1
#define PLAYER_2 2
#define FIRST_HANDSHAKE 42    // Any number is fine, just to establish initial communication
#define SECOND_HANDSHAKE 43   // Must be different to the FIRST_HANDSHAKE
#define DISPLAY_SCORE_CHAR 70
#define RESET_GAME_CHAR 71
#define FLAG_BIT 7 // 8th bit (MSB) is the flag bit (indicates whether opponent has our flag)

// Checks if there is a player to be recieved and if there is, recieves it
void check_and_recieve_opponent(Player *opponent_player);

// Send the data from my_player to the other funkit
void send_my_player(Player *my_player);

// Initialise the ir_uart sub-module as well as complete the initial handshake to determine player 1 & 2
// Returns true if the current player is player 1, false otherwise
bool ir_coms_init(void);

// Recieves the target score from the other player during the setup phase of the game
uint8_t recieve_target_score(void);

// Send the target score to the opposition player, to be called during the setup phase of the game
void send_target_score(uint8_t target_score);

#endif
