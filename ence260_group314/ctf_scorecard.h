/**
    @file   ctf_scorecard.h
    @author Sam Hogan and Henry Seaton
    @brief  A module containing functions which track and display game score
    @date   16th October 2019
 */

#ifndef SCORECARD_H
#define SCORECARD_H

#include "system.h"
#include "tinygl.h"
#include "ctf_player.h"

// Location points for displayed characters
#define TOP_LEFT_POINT (tinygl_point_t){0, 6}
#define TOP_CENTER_POINT (tinygl_point_t){0, 4}
#define INITIAL_TARGET_SCORE_CHAR '3'

// Selects the target score for each game
uint8_t select_score(void);

// Displays player score
void display_score(Player* my_player, uint8_t target_score);

// Updates player score while displaying
void score_update(void);

#endif
