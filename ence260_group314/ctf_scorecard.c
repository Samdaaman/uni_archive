/**
    @file   ctf_scorecard.c
    @author Sam Hogan and Henry Seaton
    @brief  A module containing functions which track and display game score
    @date   16th October 2019
 */

#include "system.h"
#include "navswitch.h"
#include "display.h"
#include "pacer.h"
#include "../fonts/font3x5_1.h"
#include "ctf_scorecard.h"

static char score_string[3] = {0, 0, '\0'};

// Selects the target score for each game
uint8_t select_score(void)
{
    uint8_t score_int = 0;
    uint8_t score_char = INITIAL_TARGET_SCORE_CHAR;

    display_clear();
    while (score_int == 0)
    {
        pacer_wait();
        navswitch_update();
        tinygl_update();
        if (navswitch_push_event_p (NAVSWITCH_NORTH) && score_char < '9')
        {
            score_char++;
        }
        if (navswitch_push_event_p (NAVSWITCH_SOUTH) && score_char > '1') {
            score_char--;
        }
        if (navswitch_push_event_p (NAVSWITCH_PUSH))
        {
            // Selects the displayed score, converting to int
            score_int = score_char - '0';
        }
        tinygl_draw_char(score_char, TOP_CENTER_POINT);
    }
    return score_int;
}

// Displays player score
void display_score(Player* my_player, uint8_t target_score)
{
    uint8_t score_char = my_player->score + '0';
    score_string[0] = score_char;
    if (my_player->score == target_score) {
        // If a player has reached the target score, display "win".
        score_string[1] = 'W';
    }
    else {
        score_string[1] = 0;
    }
}

// Updates player score while displaying
void score_update(void)
{
    tinygl_update();
    if (score_string[1] != 0) {
        // If a "win" is being displayed, adjusts the text location to fit.
        tinygl_draw_string(score_string, TOP_LEFT_POINT);
    } else {
        tinygl_draw_string(score_string, TOP_CENTER_POINT);
    }
}


