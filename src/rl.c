#include "../include/rl.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define DIRECTION_COUNT 8     // 8 possible relative directions to apple
#define SNAKE_DIRECTION_COUNT 4  // 4 possible snake directions
#define DANGER_STATES 2       // 2 states for each danger zone (yes/no)

// Direction to apple (8) * Snake direction (4) * Danger ahead (2) * Danger left (2) * Danger right (2)
#define STATE_COUNT (DIRECTION_COUNT * SNAKE_DIRECTION_COUNT * DANGER_STATES * DANGER_STATES * DANGER_STATES)

float q_table[STATE_COUNT][ACTION_COUNT];
float alpha = 0.1f;      // learning rate
float gama = 0.9f;      // discount factor
float epsilon = 0.9f;    // starting exploration factor (higher for more exploration)
float epsilon_min = 0.05f; // minimum exploration rate
float epsilon_decay = 0.9995f; // decay rate per step

void initialize_q_table() {
    for (int s = 0; s < STATE_COUNT; s++) {
        for (int a = 0; a < ACTION_COUNT; a++) {
            q_table[s][a] = ((float)rand() / RAND_MAX) * 0.1f;
        }
    }
    printf("Q-table initialized with %d states and %d actions.\n", STATE_COUNT, ACTION_COUNT);
}

void save_q_table(int episode) {
    char filename[100];
    sprintf(filename, "q_table_episode_%d.bin", episode);
    FILE *f = fopen(filename, "wb");
    if (f) {
        fwrite(q_table, sizeof(float), STATE_COUNT * ACTION_COUNT, f);
        fclose(f);
        printf("Q-table saved to %s\n", filename);
    } else {
        printf("Failed to save Q-table\n");
    }
}

// choose an action using epsilon-greedy policy
int choose_action(int state, int episode) {
    if ((rand() / (float)RAND_MAX) < epsilon) {
        return rand() % ACTION_COUNT; // Explore
    }
    
    int best = 0;
    for (int i = 1; i < ACTION_COUNT; i++) {
        if (q_table[state][i] > q_table[state][best]) {
            best = i;
        }
    }
    return best;
}

// decay epsilon after each step
void decay_epsilon() {
    epsilon *= epsilon_decay;
    if (epsilon < epsilon_min) epsilon = epsilon_min;
}

void update_q(int state, int action, float reward, int next_state) {
    float old_q = q_table[state][action];
    
    float max_next_q = q_table[next_state][0];
    for (int i = 1; i < ACTION_COUNT; i++) {
        if (q_table[next_state][i] > max_next_q) {
            max_next_q = q_table[next_state][i];
        }
    }
    
    q_table[state][action] = old_q + alpha * (reward + gama * max_next_q - old_q);
}

int get_state(int head_x, int head_y, int apple_x, int apple_y, int dx, int dy, 
             int danger_ahead, int danger_left, int danger_right) {
    
    // convert to block coordinates
    head_x /= 20;
    head_y /= 20;
    apple_x /= 20;
    apple_y /= 20;
    
    // calculate relative position to apple
    int rel_x = apple_x - head_x;
    int rel_y = apple_y - head_y;
    
    // simplify to 8 directions (N, NE, E, SE, S, SW, W, NW)
    int apple_dir;
    if (rel_y < 0 && abs(rel_y) > abs(rel_x)) apple_dir = 0;         // North
    else if (rel_y < 0 && rel_x > 0 && abs(rel_y) == abs(rel_x)) apple_dir = 1; // Northeast
    else if (rel_x > 0 && abs(rel_x) > abs(rel_y)) apple_dir = 2;     // East
    else if (rel_y > 0 && rel_x > 0 && abs(rel_y) == abs(rel_x)) apple_dir = 3; // Southeast
    else if (rel_y > 0 && abs(rel_y) > abs(rel_x)) apple_dir = 4;     // South
    else if (rel_y > 0 && rel_x < 0 && abs(rel_y) == abs(rel_x)) apple_dir = 5; // Southwest
    else if (rel_x < 0 && abs(rel_x) > abs(rel_y)) apple_dir = 6;     // West
    else apple_dir = 7;                                              // Northwest
    
    int snake_dir;
    if (dx == 1 && dy == 0) snake_dir = 0;      // right
    else if (dx == -1 && dy == 0) snake_dir = 1; // left
    else if (dx == 0 && dy == 1) snake_dir = 2;  // down
    else snake_dir = 3;                          // up
    
    return apple_dir * SNAKE_DIRECTION_COUNT * DANGER_STATES * DANGER_STATES * DANGER_STATES + 
           snake_dir * DANGER_STATES * DANGER_STATES * DANGER_STATES + 
           danger_ahead * DANGER_STATES * DANGER_STATES + 
           danger_left * DANGER_STATES + 
           danger_right;
}
