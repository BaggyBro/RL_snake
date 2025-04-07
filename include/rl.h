#ifndef RL_H
#define RL_H

#define ACTION_COUNT 3  // 0=turn left, 1=go straight, 2=turn right

void initialize_q_table();
int choose_action(int state, int episode);
void update_q(int state, int action, float reward, int next_state);
void decay_epsilon();
void save_q_table(int episode);
int get_state(int head_x, int head_y, int apple_x, int apple_y, int dx, int dy, 
             int danger_ahead, int danger_left, int danger_right);

#endif // RL_H
