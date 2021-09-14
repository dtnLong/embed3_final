#include "../include/snake.h"
#include "../include/framebf.h"
#include "../include/delay.h"
#include "../include/uart.h"
#include "../include/random.h"
#include "../include/snake_logo.h"
#include "../include/title_logo.h"
#include "../include/util.h"

/* GAME INFO:
    Game resolution (800 x 700)
        x1, y1: 112, 34
        x2, y2: 911, 733 
    Snake square: 20x20
*/

#define SNAKE_WIDTH 19 //snake width = 19 instead of 20 cuz from 0 to 19 there are 20 pixels
#define BACKGROUND_COLOR 0x004c546e
#define BOX_COLOR 0x00e1e4ed
#define FRUIT_COLOR 0x00f04848
#define SELECT_COLOR 0x009f3eb8


/* Game states: 
    WELCOME: loading screen
    MAIN_MENU: main menu having options (PLAY, DIFFICULTY, QUIT)
    GUIDE: how to play
    GAMEPLAY: gameplay screen 
    END: game over
*/
typedef enum {WELCOME, MAIN_MENU, GUIDE, GAMEPLAY, END} STATE;

/* Snake direction: LEFT, RIGHT, UP, DOWN */
typedef enum {LEFT,RIGHT,UP,DOWN} DIRECTION;

/* Penalty types: 
    NONE: no penalty
    FAST: fast snake 
    DIZZY: reverse snake's control
*/
typedef enum {FAST = 2, DIZZY = 1, NONE = 0} PENALTY; 

/* Main menu options: PLAY (default), DIFFICULTY, QUIT */
typedef enum {PLAY = 1, DIFFICULTY = 2, QUIT =3} OPTION;

/* Difficulty: EASY, MEDIUM, HARD */
typedef enum {EASY = 300, MEDIUM = 200, HARD =100} MODE;

/* Options after game over: PLAY_AGAIN, MENU */
typedef enum {PLAY_AGAIN = 1, MENU =2} GAME_OVER_OPTION;

/* Pause state: PAUSE, UNPAUSE */
typedef enum {PAUSE, UNPAUSE} PAUSE_STATE;

/* Initialize settings variables*/
STATE state = WELCOME; 
DIRECTION direction = RIGHT; 
PENALTY penalty = NONE;
OPTION option = PLAY; 
MODE mode = EASY; 
PAUSE_STATE p_state = UNPAUSE;
GAME_OVER_OPTION go_option = PLAY_AGAIN;

/* coordinates */
int x,y; //snake coordinates
int food_x, food_y;
int bomb_x, bomb_y;  

/* game data */
int score = 0;
char score_str[4] = "0000";
int snake_speed;
char game_mode[3][10] = {"hard", "medium", "easy"};
int high_score = 0;

/* snake data */ 
int snake_length = 3;
int snake_coord[1400][2]; //2D array to store coordinates x,y (top left position) of each square of the snake

/* render flags to avoid continuously re-rendering */
int main_menu_flag = 1; 
int guide_flag = 1; 
int title_flag = 1;
int end_flag = 1;

/* penalty data */
int penalty_turn = -1, prev_penalty_turn = -1;
int penalty_effect_duration = 2;

/* reset game screen for screen transition */
void clear_screen() {
    drawRectARGB32(0,0,1023,767,BACKGROUND_COLOR,1); //draw background
  	drawRectARGB32(111,33,912,734,BOX_COLOR,1); //draw inner box
}

/* update head coordinates */
void update_head_coord(){
	snake_coord[0][0] = x;
	snake_coord[0][1] = y;
}

/* generate new food coordinates 
    since food size = snake's square size
    -> food_x = x1 + (snake width + 1) * random value from 0 to 39 (one row can have 40 square blocks)
    -> food_y = y1 + (snake width + 1) * random value from 0 to 34 (one column can have )
    if the generated coordinates coincide with snake coordinates, generate new coordinates again
*/
void generate_food() {
    random:
    food_x = 112 + (SNAKE_WIDTH + 1) * rand(0,40);
    food_y = 34 + (SNAKE_WIDTH + 1) * rand(0,35);
	for (int i = 0; i < snake_length; i++) { 
		if ((snake_coord[i][0] == food_x)  && (snake_coord[i][1] == food_y)) { 
			goto random;
		}
	}
}

/* Render food by
    1. generate new food coordinates
    2. draw circles from the coordinates
 */
void render_food() {
    generate_food();
    drawCircle(food_x + 9, food_y + 9, 10, FRUIT_COLOR, 0, 1);
}

/* Render in-game information
    score: player score
    bomb: current bomb in effect (fast or reverse)
*/
void render_game_info() {
    drawString_bg(122, 8, "Score:", 0xFFFFFFFF, BACKGROUND_COLOR, 2);
    drawString_bg(330, 8, "Bomb:", 0xFFFFFFFF, BACKGROUND_COLOR, 2);
}

/* generate new bomb coordinates 
    since bomb size = snake's square size
    -> bomb_x = x1 + (snake width + 1) * random value from 0 to 39 (one row can have 40 square blocks)
    -> bomb_y = y1 + (snake width + 1) * random value from 0 to 34 (one column can have )
    if the generated coordinates coincide with food coordinates or snake coordinates, generate new coordinates again
*/
void generate_bomb() {
    random:
    bomb_x = 112 + (SNAKE_WIDTH + 1) * rand(0,40);
    bomb_y = 34 + (SNAKE_WIDTH + 1) * rand(0,35);
	if ((bomb_x != x) && (bomb_y != y)) {
        for (int i = 0; i < snake_length; i++) { 
            if ((snake_coord[i][0] == bomb_x)  && (snake_coord[i][1] == bomb_y)) {
                goto random;
            }
        }
    } else {
        goto random;
    }
}

/* Generate penalty turn every 3-5 turns since bomb has effect for 2 turns */
void generate_penalty_turn() {
    prev_penalty_turn = penalty_turn; //store previous penalty turn to check if two turns has passed and player doesnt eat the bomb
    penalty_turn = snake_length + rand(3,6);
}

/* render bomb by
    1. generate the turn that the bomb will appear 
    2. generate new bomb coordinates
    3. draw bomb from the generated coordinates
*/
void render_bomb() {
    generate_penalty_turn();
    generate_bomb();
    drawBomb(bomb_x + 9, bomb_y + 9, 10);
}


/* initialize game settings */
void initialize_game() {
    score = 0;
	snake_length = 3; //initial snake length
	x = 512; //default x 
	y = 374; //default y
    direction = RIGHT; //intial direction
    
    update_head_coord();

    /* initialize snake inital coordinates */
    for (int i = 1; i < snake_length; i++) {
        snake_coord[i][0] = snake_coord[i-1][0] - SNAKE_WIDTH - 1; 
        snake_coord[i][1] = snake_coord[i-1][1];
    }

    generate_penalty_turn(); 
    render_food();
    render_game_info();
}

/* Render new snake head when it mvoves*/
void render_head() {
    drawRectARGB32(snake_coord[0][0],snake_coord[0][1],snake_coord[0][0] + SNAKE_WIDTH, snake_coord[0][1] + SNAKE_WIDTH, 0x00ff8c00, 1);
}

/* clear tail when snake moves */
void clear_tail() {
    drawRectARGB32(snake_coord[snake_length-1][0],snake_coord[snake_length-1][1],snake_coord[snake_length-1][0] + SNAKE_WIDTH, snake_coord[snake_length-1][1] + SNAKE_WIDTH, BOX_COLOR, 1);
}

/* Update body coords, previous block coords = next block coords*/
void update_body_coord() {
    for (int i = snake_length - 1; i>0; i--){
		snake_coord[i][0] = snake_coord[i-1][0];
        snake_coord[i][1] = snake_coord[i-1][1];
	}
}

/* Update snake coords by
    1. update body coords
    2. update head coords base on the direction of the snake is heading
*/ 
void update_snake_coord() {
    update_body_coord();
    if (direction == RIGHT){  
		if (penalty == DIZZY){  //reverse direction if dizzy bomb is in effect
			x -= SNAKE_WIDTH + 1;  
			if (x < 112){ 
				x = 912-SNAKE_WIDTH -1;
			} 
		}
		else {
			x += SNAKE_WIDTH + 1;
			if (x >= 912){
				x = 112;
			} 
		} 
	}
	else if (direction == LEFT){
		if (penalty == DIZZY){
			x += SNAKE_WIDTH + 1;
			if (x >= 912){
				x = 112;
			} 	
		}
		else{
			x -= SNAKE_WIDTH + 1;
			if (x < 112){ 
				x = 912 - SNAKE_WIDTH -1;
			} 
		}
	}
	else if (direction == UP){ 
		if (penalty == DIZZY){
			y += SNAKE_WIDTH + 1;
			if (y >= 734){ 
                y = 34;
			}
		}
		else{
			y -= SNAKE_WIDTH + 1;
			if (y < 34){ 
                y = 734 - SNAKE_WIDTH - 1;
			} 
		}
	}
	else{ 
		if (penalty == DIZZY){
			y -= SNAKE_WIDTH + 1;
			if (y < 34){
                y = 734 - SNAKE_WIDTH - 1;
			} 
		}
		else{
			y += SNAKE_WIDTH + 1;
			if (y >= 734){ 
                y = 34;
			}
		}
	}
    update_head_coord();
}


/* reverse direction of the snake */
void reverse_direction() { 
    if (direction == RIGHT) {
        direction = LEFT;
    } else if (direction == LEFT) {
        direction = RIGHT;
    } else if (direction == UP) {
        direction = DOWN;
    } else if (direction == DOWN) {
        direction = UP;
    }
}

/* update and display score*/
void update_score(int x) {
    score += x;
    int temp = score;
    for (int i = 3; i >= 0; i--) {
        score_str[i] = temp % 10 + '0';
        temp = temp / 10;
    } 
    drawRectARGB32(238,8,222,24,BACKGROUND_COLOR,1);
    drawString_bg(238, 8, score_str, 0xFFFFFFFF, BACKGROUND_COLOR, 2);
}

/* update on-screen pause info */
void update_pause_info() {
    drawRectARGB32(816,8,912,24,BACKGROUND_COLOR,1);
    if (p_state == PAUSE) {
        drawString_bg(816, 8, "Paused", 0xFFFFFFFF, BACKGROUND_COLOR, 2);
    }
}

/* update on-screen bomb info */
void update_bomb_info() {
    drawRectARGB32(416,8,550,24,BACKGROUND_COLOR,1);
    if (penalty == DIZZY) {
        drawString_bg(426, 8, "Reverse", 0xFFFFFFFF, BACKGROUND_COLOR, 2);
    } else if (penalty == FAST) {
        drawString_bg(426, 8, "Fast", 0xFFFFFFFF, BACKGROUND_COLOR, 2);
    }
}

/* check if snake eats bomb or food */
int snake_eat() {
    /*  Increase snake length if it eats food, update score, and render new food. */
    if (food_x == x && food_y == y) { 
        snake_length++;
        update_score(5);
        render_food();
        if (penalty != NONE) { 
            /* If penalty is in effect decrease the effect duration */
            penalty_effect_duration -= 1;
            /* if penalty is no longer in effect reset penalty to None and reset penalty duration to 2 turns*/
            if (penalty == DIZZY && penalty_effect_duration == 0) {
                penalty = NONE;
                penalty_effect_duration = 2;
                reverse_direction();
            } else if (penalty == FAST && penalty_effect_duration == 0) {
                penalty = NONE;
                snake_speed = mode;
                penalty_effect_duration = 2;
            }
            update_bomb_info();
        } else if (snake_length - prev_penalty_turn == 2) {
            drawCircle(bomb_x + 9, bomb_y+9, 10, BOX_COLOR, BOX_COLOR, 1);
            bomb_x = 0;
            bomb_y = 0;
        } 
        return 1;
    } else if (bomb_x == x && bomb_y == y) { //decrease score and update penalty when a bomb is eaten
        update_score(-5);
        penalty = rand(DIZZY, FAST+1);
        update_bomb_info();
        if (penalty == DIZZY) {
            bomb_x = 0;
            bomb_y = 0;
            reverse_direction();
        } else {
            snake_speed -= 50;
        }
    }
    return 0;
}

/* check if snake eats itself */
int snake_eat_self() { 
    if (snake_length > 4) {
		for (int i = 4; i < snake_length; i++) {
			if ((snake_coord[i][0] == x) && (snake_coord[i][1] == y)) {
				return 1;
			}
		}
	}
	return 0;
}

/* render snake by
    1. check if snake eats food, if yes keep tail else clear tail
    2. update new snake coordinates
    3. render the head
*/
void render_snake() {
    if (!snake_eat()) {
        clear_tail();
    }
    update_snake_coord();
    render_head();
}

/* handle control input from users*/
void handle_control(char input) {
    switch(input) {
        case 'w': //up when snake is going right or left
            if(direction == RIGHT || direction == LEFT){
                direction = UP;
            }
            break;
        case 'a': //left when snake is going up or down
            if(direction == UP || direction == DOWN){
                direction = LEFT;
            }
            break;
        case 's': //down when snake is going right or left
            if(direction == RIGHT || direction == LEFT){						
                direction = DOWN;
            }
            break;
        case 'd': //right when snake is going right or left
            if(direction == UP || direction == DOWN){
                direction = RIGHT;
            }
            break;
        default: 
            break;
    }
}

/* notify the user about in-game commands to pause and return to menu*/
void display_notification() {
    drawString(152, 743, "Press p to pause/unpause, r to return to menu", 0x00FFFFFF, 2);
}

/* main program*/
void run_snake() {
    clear_screen();
    while(1) {
        switch(state) {
            case WELCOME: 
                draw_small_image(112, 34, TITLE_LOGO_WIDTH, TITLE_LOGO_HEIGHT, title_logo);
                draw_small_image(112, 334, SNAKE_LOGO_WIDTH, SNAKE_LOGO_HEIGHT, snake_logo);
                for (int i = 0; i < 3; i++) {
                    drawString_bg(392, 710, "Loading...", BACKGROUND_COLOR, BOX_COLOR, 3);
                    wait_msec(1000);
                    drawString_bg(392, 710, "Loading...", BOX_COLOR, BOX_COLOR, 3);
                    wait_msec(500);
                }
                state = MAIN_MENU;
                clear_screen();
                break;
            case MAIN_MENU:
                if (title_flag == 1) {
                    draw_small_image(112, 34, TITLE_LOGO_WIDTH, TITLE_LOGO_HEIGHT, title_logo);
                    title_flag = 0;
                }
                if (main_menu_flag) {
                    if (option == PLAY) {
                        drawString_bg(463, 354, "Play", SELECT_COLOR, BOX_COLOR, 3);
                        drawString_bg(392, 394, "Difficulty", 0, BOX_COLOR, 3);
                        drawString_bg(463, 438, "Quit", 0, BOX_COLOR, 3);
                        drawString_bg(672, 394, game_mode[mode/100-1], 0, BOX_COLOR, 3);
                    } else if (option == DIFFICULTY){
                        drawString_bg(463, 354, "Play", 0, BOX_COLOR, 3);
                        drawString_bg(392, 394, "Difficulty", SELECT_COLOR, BOX_COLOR, 3);
                        drawString_bg(463, 438, "Quit", 0, BOX_COLOR, 3);
                        drawRectARGB32(672, 394, 816, 418, BOX_COLOR, 1); //Clear mode
                        drawString_bg(672, 394, game_mode[mode/100-1], BACKGROUND_COLOR, BOX_COLOR, 3);
                    } else if (option == QUIT) {
                        drawString_bg(463, 354, "Play", 0, BOX_COLOR, 3);
                        drawString_bg(392, 394, "Difficulty", 0, BOX_COLOR, 3);
                        drawString_bg(463, 438, "Quit", SELECT_COLOR, BOX_COLOR, 3);
                        drawString_bg(672, 394, game_mode[mode/100-1], 0, BOX_COLOR, 3);
                    } 
                    drawString(328, 710, "Press SPACE to continue", 0x00FFFFFF, 2);
                    main_menu_flag = 0;
                } 
                c = uart_getc();
                if (c == ' ') {
                    if (option == PLAY) {
                        state = GUIDE;
                        snake_speed = mode;
                        drawRectARGB32(328, 310, 696, 326, BOX_COLOR, 1);
                        clear_screen();
                        main_menu_flag = 1;
                        title_flag = 1;
                    } else if (option == QUIT) {
                        drawRectARGB32(0, 0, 1023, 767, 0, 1);
                        state = WELCOME;
                        main_menu_flag = 1;
                        title_flag = 1;
                        goto quit;
                    }
                } else if (c == 'w') {
                    option -= 1;
                    if (option < PLAY) {
                        option = QUIT;
                    }
                    main_menu_flag = 1;
                } else if (c == 's') {
                    option += 1;
                    if (option > QUIT) {
                        option = PLAY;
                    }
                    main_menu_flag = 1;
                } else if (c == 'a' && option == DIFFICULTY) {
                    mode += 100;
                    if (mode > EASY) {
                        mode = HARD;
                    }
                    main_menu_flag = 1;
                } else if (c == 'd' && option == DIFFICULTY) {
                    mode -= 100;
                    if (mode < HARD) {
                        mode = EASY;
                    }
                    main_menu_flag = 1;
                }
                break;
            case GUIDE:
                if (guide_flag) {
                    drawString_bg(336, 54, "How to play", BACKGROUND_COLOR, BOX_COLOR, 4);
                    drawString_bg(290, 140, "W", BACKGROUND_COLOR, BOX_COLOR, 3);
                    drawString_bg(242, 174, "A S D", BACKGROUND_COLOR, BOX_COLOR, 3);
                    drawString_bg(450, 140, "Press W/A/S/D to move", 0, BOX_COLOR, 2);
                    drawString_bg(450, 174, "up/left/down/right", 0, BOX_COLOR, 2);
                    drawCircle(295, 300, 20, FRUIT_COLOR, 0, 1);
                    drawString_bg(450, 290, "Fruit: +5 score and increase", 0, BOX_COLOR, 2);
                    drawString_bg(450, 324, "increase snake length", 0, BOX_COLOR, 2);
                    drawBomb(295, 450, 20);
                    drawString_bg(450, 440, "Bomb: -5 score and make", 0, BOX_COLOR, 2);
                    drawString_bg(450, 474, "snake faster or dizzy", 0, BOX_COLOR, 2);
                    drawString_bg(450, 504, "(reverse control)", 0, BOX_COLOR, 2);
                    drawString(328, 710, "Press SPACE to continue", 0x00FFFFFF, 2);
                    guide_flag = 0;
                }
                c = uart_getc();
                if (c == ' ') {
                    state = GAMEPLAY;
                    clear_screen();
                    guide_flag = 1;
                } 
                break;
            case GAMEPLAY:
                display_notification();
                initialize_game();
                while(1) {
                    c = uart_getc();
                    render_snake();
                    if (c == 'r') {
                        state = MAIN_MENU;
                        break;
                    } else if (c == 'p' && p_state == UNPAUSE) {
                        p_state = PAUSE;
                        update_pause_info();
                        while (1) {
                            c = uart_getc();
                            if (c == 'p') {
                                p_state = UNPAUSE;
                                update_pause_info();
                                break;
                            }
                        }
                    } else {
                        handle_control(c);
                    }
                    if (snake_length == penalty_turn) {
                        render_bomb();
                    }
                    if (snake_eat_self()) {
                        clear_screen();
                        state = END;
                        break;
                    } 
                    wait_msec(snake_speed);
                }
                break;
            case END:
                if (end_flag == 1) {
                    drawString_bg(332, 104, "Game Over", BACKGROUND_COLOR, BOX_COLOR, 5);
                    drawString_bg(336, 274, "Score:", 0, BOX_COLOR, 4);
                    drawString_bg(560, 274, score_str, 0, BOX_COLOR, 4);
                    if (score > high_score) {
                        high_score = score;
                        drawString_bg(296, 334, "Congrats, new high score!!!", BACKGROUND_COLOR, BOX_COLOR, 2);
                    } else {
                        char high_score_str[4];
                        int temp = high_score;
                        for (int i = 3; i >= 0; i--) {
                            high_score_str[i] = temp % 10 + '0';
                            temp = temp / 10;
                        } 
                        drawString_bg(384, 334, "High score:", BACKGROUND_COLOR, BOX_COLOR, 2);
                        drawString_bg(576, 334, high_score_str, BACKGROUND_COLOR, BOX_COLOR, 2);
                    }

                    end_flag = 0;
                }
                if (go_option == PLAY_AGAIN) {
                    drawString_bg(182, 470, "Play Again", SELECT_COLOR, BOX_COLOR, 3);
                    drawString_bg(598, 470, "Main Menu", 0, BOX_COLOR, 3);
                } else if (go_option == MENU) {
                    drawString_bg(182, 470, "Play Again", 0, BOX_COLOR, 3);
                    drawString_bg(598, 470, "Main Menu", SELECT_COLOR, BOX_COLOR, 3);
                }
                c = uart_getc();
                if (c == ' ') {
                    if (go_option == PLAY_AGAIN) {
                        clear_screen();
                        state = GAMEPLAY;
                    } else if (go_option == MENU) {
                        clear_screen();
                        state = MAIN_MENU;
                    }
                    end_flag = 1;
                } else if (c == 'a') {
                    go_option -= 1;
                    if (go_option < PLAY_AGAIN) {
                        go_option = MENU;
                    }
                    end_flag = 0;
                } else if (c == 'd') {
                    go_option += 1;
                    if (go_option > MENU) {
                        go_option = PLAY_AGAIN;
                    }
                    end_flag = 0;
                }
                break;
        } 
    }
    quit:;
}