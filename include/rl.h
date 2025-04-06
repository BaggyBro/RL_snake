#ifndef RL_H
#define RL_H

// Constants
#define STATE_COUNT 57600  // May adjust depending on how you encode states
#define ACTION_COUNT 3       // 0: Left, 1: Forward, 2: Right

// RL core functions
int choose_action(int state);
void update_q(int state, int action, int reward, int next_state);

// Helper to encode game state into int
int get_state(int head_x, int head_y, int apple_x, int apple_y, int dx, int dy);

#endif
