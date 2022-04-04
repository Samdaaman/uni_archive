/**
    @file   game.c
    @author Sam Hogan and Henry Seaton
    @brief  A Capture the Flag game designed for UC Funkits.
            The objective of the game is to get the opponents flag and
            bring it back to your half without getting tagged
    @date   17th October 2019
 */


#include "system.h"
#include "pacer.h"
#include "ctf_player.h"
#include "ctf_ir_coms.h"
#include "ctf_gamehost.h"
#include "navswitch.h"
#include "ctf_scorecard.h"
#include "tinygl.h"
#include "../fonts/font3x5_1.h"

#define SENDING_RATE 30    // for every display update send player info 1/SENDING_RATE of the time
#define PACER_RATE 300
#define TINYGL_SPEED 20

int main (void)
{
    system_init();
    pacer_init(PACER_RATE);
    display_init();
    navswitch_init();

    tinygl_init (PACER_RATE);
    tinygl_font_set (&font3x5_1);
    tinygl_text_speed_set(TINYGL_SPEED); // Magic number
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);

    Player my_player_instance = spawn_player(PLAYER_START_ROW, PLAYER_START_COL);
    Player opponent_player_instance = spawn_player(GAME_ROWS + 1 - PLAYER_START_ROW, GAME_COLS + 1 - PLAYER_START_COL);
    Player *my_player = &my_player_instance;
    Player *opponent_player = &opponent_player_instance;

    uint8_t counter = 0;
    uint8_t target_score = 0;

    bool is_player_1 = ir_coms_init();
    pacer_wait();
    if (!is_player_1)
    {
        target_score = select_score();
        send_target_score(target_score);
    }
    else
    {
        target_score = recieve_target_score();
    }

    while (1)
    {
        pacer_wait ();
        check_and_recieve_opponent(opponent_player);
        if (counter >= SENDING_RATE)
        {
            send_my_player(my_player);
            if (my_player->displaying_score == true)
            {
                // My_player scored
                send_my_player(my_player); // send it again to be sure
                display_score(my_player, target_score);
                while (navswitch_up_p(NAVSWITCH_PUSH))
                {
                    pacer_wait();
                    navswitch_update();
                    score_update();
                }
                my_player->displaying_score = false;
                if (my_player->score == target_score)
                {
                    my_player->trigger_game_reset = true;
                    send_my_player(my_player);
                    reset_game(my_player, opponent_player);
                }

            } else
            {
                game_event_update(my_player, opponent_player);
                player_update(my_player, (opponent_player->col <= GAME_COLS/2));
            }
            counter = 0;
        }
        if (opponent_player->displaying_score == true)
        {
            // Opponent player scored
            reset_my_player(my_player);
            display_score(my_player, target_score);
            while(opponent_player->displaying_score == true)
            {
                pacer_wait();
                check_and_recieve_opponent(opponent_player);
                score_update();
            }
            if (opponent_player->trigger_game_reset == true)
            {
                reset_game(my_player, opponent_player);
            }
        }
        else
        {
            // Nobody scored so continue game operation
            display_players(my_player, opponent_player);
        }
        counter++;
    }
}
