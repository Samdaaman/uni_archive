/** 
    @file   ctf_ir_coms.c
    @author Sam Hogan and Henry Seaton
    @brief  A module for providing IR communication between Funkits in a Capture the Flag style game
            Could be adapted to work with other top down two player games 
    @date   16th October 2019
 */

#include "ctf_ir_coms.h"
#include "ir_uart.h"
#include "navswitch.h"
#include "ctf_gamehost.h"
#include "pacer.h"

// Private function declarations
static uint8_t __player_to_char(Player *my_player);
static void __recieve_opponent_player(Player *opponent_player);
static void __update_opponent_from_char(Player *opponent_player, uint8_t recieved_chr);

// Player number, (0 = unassigned)
static uint8_t player_number = 0;

// Recieves opponent data (position) from the other funkit and updates it accordingly
static void __recieve_opponent_player(Player *opponent_player)
{
    uint8_t recieved_char = ir_uart_getc();
    __update_opponent_from_char(opponent_player, recieved_char);
}

// Updates the opponents data (position) from the recieved char from the other funkit
static void __update_opponent_from_char(Player *opponent_player, uint8_t recieved_chr)
{
    opponent_player->has_flag = recieved_chr & (1 << FLAG_BIT);
    recieved_chr &= ~(1 << FLAG_BIT); // trim off the flag bit
    if (recieved_chr == DISPLAY_SCORE_CHAR)
    {
        opponent_player->displaying_score = true;
    }
    else if (recieved_chr == RESET_GAME_CHAR)
    {
        opponent_player->trigger_game_reset = true;
    }
    else
    {
        // Relative position in opponent players reference frame
        uint8_t rel_pos_x = recieved_chr / GAME_ROWS + 1;
        uint8_t rel_pos_y = recieved_chr % GAME_ROWS;
        // Absolute position in current player reference frame
        opponent_player->col = GAME_COLS +1 - rel_pos_x;
        opponent_player->row = GAME_ROWS - rel_pos_y;
        opponent_player->displaying_score = false;
    }
}

// Send the data from my_player to the other funkit
void send_my_player(Player *my_player)
{
    uint8_t player_chr = __player_to_char(my_player);
    ir_uart_putc(player_chr);
}

// Converts the player data (currently only position) to a char
static uint8_t __player_to_char(Player *my_player)
{
    uint8_t player_chr = 0;
    if (my_player->displaying_score == true)
    {
        player_chr = DISPLAY_SCORE_CHAR;
    }
    else if (my_player->trigger_game_reset == true)
    {
        player_chr = RESET_GAME_CHAR;
    }
    else
    {
        player_chr += (my_player->col - 1) * GAME_ROWS;
        player_chr += my_player->row - 1;      
    }
    if (my_player->has_flag)
    {
        player_chr |= (1 << 7);
    }
    return player_chr; 
}

// Checks if there is a player to be recieved and if there is, recieves it
void check_and_recieve_opponent(Player *opponent_player)
{
    if (ir_uart_read_ready_p() == true)
    {
        __recieve_opponent_player(opponent_player);
    }
}

// Initialise the ir_uart sub-module as well as complete the initial handshake to determine player 1 & 2
// Returns true if the current player is player 1, false otherwise
bool ir_coms_init(void)
{
    ir_uart_init();
    navswitch_init();
    while(!player_number)
    {
        if (ir_uart_read_ready_p())
        {
            uint8_t recieved_chr = ir_uart_getc();
            if (recieved_chr == FIRST_HANDSHAKE)
            {
                player_number = PLAYER_1;
                ir_uart_putc(SECOND_HANDSHAKE);
            } 
            else if (recieved_chr == SECOND_HANDSHAKE)
            {
                player_number = PLAYER_2;
            }
        }
        else
        {
            // Currently you have to press a button to trigger the initial handshake
            navswitch_update();
            if (navswitch_push_event_p(NAVSWITCH_PUSH))
            {
                ir_uart_putc(FIRST_HANDSHAKE);
            }
        }
    }
    return (player_number == PLAYER_1);
}

// Send the target score to the opposition player, to be called during the setup phase of the game
void send_target_score(uint8_t target_score)
{
    ir_uart_putc(target_score);
}

// Recieves the target score from the other player during the setup phase of the game
uint8_t recieve_target_score(void)
{
    while (ir_uart_read_ready_p() == false)
    {
        pacer_wait();
    }
    return ir_uart_getc();
}
