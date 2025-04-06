
// rl.c

#include "../include/rl.h"
#include <stdlib.h>
#include <stdio.h>

float q_table[STATE_COUNT][ACTION_COUNT];

float alpha = 0.1f;   // learning rate
float gamma = 0.9f;   // discount factor
float epsilon = 0.1f; // exploration factor

int choose_action(int state) {
    if ((rand() / (float)RAND_MAX) < epsilon) {
        return rand() % ACTION_COUNT; // Explore
    }

    // Exploit
    int best = 0;
    for (int i = 1; i < ACTION_COUNT; i++) {
        if (q_table[state][i] > q_table[state][best]) {
            best = i;
        }
    }
    return best;
}

void update_q(int state, int action, int reward, int next_state) {
    float old_q = q_table[state][action];
    float max_next_q = q_table[next_state][0];
    for (int i = 1; i < ACTION_COUNT; i++) {
        if (q_table[next_state][i] > max_next_q) {
            max_next_q = q_table[next_state][i];
        }
    }

    q_table[state][action] = old_q + alpha * (reward + gamma * max_next_q - old_q);
}


int get_state(int head_x, int head_y, int apple_x, int apple_y, int dx, int dy) {
    head_x /= 20;
    head_y /= 20;
    apple_x /= 20;
    apple_y /= 20;

    int rel_x = apple_x - head_x + 40; // Clamp to 0-79
    int rel_y = apple_y - head_y + 40;

    int dx_bin = dx + 1; // -1 to 1 => 0 to 2
    int dy_bin = dy + 1;

    return rel_x + rel_y * 80 + dx_bin * 6400 + dy_bin * 19200;
}

