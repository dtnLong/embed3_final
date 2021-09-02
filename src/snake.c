#include "../include/snake.h"
#include "../include/framebf.h"
#include "../include/delay.h"
#include "../include/uart.h"
#include "../include/random.h"
#define SNAKE_WIDTH 19
#define BACKGROUND_COLOR 0x004c546e
#define BOX_COLOR 0x00e1e4ed
#define FRUIT_COLOR 0x00f04848

typedef enum {WELCOME, MAIN_MENU, GUIDE, GAMEPLAY, END} STATE;
typedef enum {LEFT,RIGHT,UP,DOWN} DIRECTION;
typedef enum {FAST = 2, REVERSED = 1, NONE = 9} PENALTY; 
STATE state = WELCOME;
DIRECTION direction = RIGHT; 
PENALTY penalty = NONE;

//coordiantes
int x,y;
int food_x, food_y;
int bomb_x, bomb_y;  

//game stats
int score = 0;
int speed = 200;

//snake data 
int snake_length = 3;
int snake_coord[1400][2];

//render flags
int welcome_screen_flag = 1;
int main_menu_flag = 1; 
int guide_flag = 1; 
int score_flag = 1;
int end_flag = 1;

//penalty data
int penalty_turn = -1, prev_penalty_turn = -1;
int penalty_effect_duration = 2;


void init_UI() {
    drawRectARGB32(0,0,1023,767,BACKGROUND_COLOR,1); //draw background
  	drawRectARGB32(111,33,912,734,BOX_COLOR,1); //draw inner box
}

void update_head_coord(){
	snake_coord[0][0] = x;
	snake_coord[0][1] = y;
}

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

void render_food() {
    generate_food();
    drawCircle(food_x + 9, food_y + 9, 10, FRUIT_COLOR, 0, 1);
}

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

void generate_penalty_turn() {
    prev_penalty_turn = penalty_turn;
    penalty_turn = snake_length + rand(3,6);
}

void render_bomb() {
    generate_penalty_turn();
    generate_bomb();
    drawBomb(bomb_x + 9, bomb_y + 9, 10);
}

void initialize_game() {
    score = 0;
	snake_length = 3;
	x = 512;
	y = 374;
    direction = RIGHT;
    
    update_head_coord();

    for (int i = 1; i < snake_length; i++) {
        snake_coord[i][0] = snake_coord[i-1][0] - SNAKE_WIDTH - 1;
        snake_coord[i][1] = snake_coord[i-1][1];
    }

    generate_penalty_turn(); 
    render_food();
}

void render_head() {
    drawRectARGB32(snake_coord[0][0],snake_coord[0][1],snake_coord[0][0] + SNAKE_WIDTH, snake_coord[0][1] + SNAKE_WIDTH, 0x00ff8c00, 1);
}

void clear_tail() {
    drawRectARGB32(snake_coord[snake_length-1][0],snake_coord[snake_length-1][1],snake_coord[snake_length-1][0] + SNAKE_WIDTH, snake_coord[snake_length-1][1] + SNAKE_WIDTH, BOX_COLOR, 1);
}

void update_body_coord() {
    for (int i = snake_length - 1; i>0; i--){
		snake_coord[i][0] = snake_coord[i-1][0];
        snake_coord[i][1] = snake_coord[i-1][1];
	}
}

void update_snake_coord() {
    update_body_coord();
    if (direction == RIGHT){ 
		if (penalty == REVERSED){ 
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
		if (penalty == REVERSED){
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
		if (penalty == REVERSED){
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
		if (penalty == REVERSED){
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

int snake_eat() {
    if (food_x == x && food_y == y) {
        snake_length++;
        render_food();
        if (penalty != NONE) {
            penalty_effect_duration -= 1;
            if (penalty == REVERSED && penalty_effect_duration == 0) {
                penalty = NONE;
                penalty_effect_duration = 2;
                reverse_direction();
            } else if (penalty == FAST && penalty_effect_duration == 0) {
                penalty = NONE;
                speed = 200;
                penalty_effect_duration = 2;
            }
        } else if (snake_length - prev_penalty_turn == 2) {
            uart_puts("asdf");
            drawCircle(bomb_x + 9, bomb_y+9, 10, BOX_COLOR, BOX_COLOR, 1);
            bomb_x = 0;
            bomb_y = 0;
        } 
        return 1;
    } else if (bomb_x == x && bomb_y == y) {
        uart_puts("fads");
        penalty = rand(REVERSED, FAST+1);
        if (penalty == REVERSED) {
            bomb_x = 0;
            bomb_y = 0;
            reverse_direction();
        } else {
            speed -= 50;
        }
    }
    return 0;
}

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

void render_snake() {
    if (!snake_eat()) {
        clear_tail();
    }
    update_snake_coord();
    render_head();
}

void handle_control(char input) {
    switch(input) {
        case 'w': 
            if(direction == RIGHT || direction == LEFT){
                direction = UP;
            }
            break;
        case 'a': 
            if(direction == UP || direction == DOWN){
                direction = LEFT;
            }
            break;
        case 's': 
            if(direction == RIGHT || direction == LEFT){						
                direction = DOWN;
            }
            break;
        case 'd': 
            if(direction == UP || direction == DOWN){
                direction = RIGHT;
            }
            break;
        default: 
            break;
    }
}

void run_snake() {
    init_UI();
    char c;
    while(1) {
        switch(state) {
            case WELCOME:
                //wait_msec(2000);
                if (welcome_screen_flag) { 
                    state = MAIN_MENU;
                }
                break;
            case MAIN_MENU:
                uart_puts("main menu\n");
                if (main_menu_flag) {
                    drawString(328, 310, "Press SPACE to continue", 0x00FFFFFF, 2);
                    main_menu_flag = 0;
                } 
                c = uart_getc();
                if (c == ' ') {
                    state = GUIDE;
                    drawRectARGB32(328, 310, 696, 326, BOX_COLOR, 1);
                    main_menu_flag = 1;
                } 
                break;
            case GUIDE:
                uart_puts("guide\n");
                if (guide_flag) {
                    guide_flag = 0;
                }
                c = uart_getc();
                if (c == ' ') {
                    state = GAMEPLAY;
                    guide_flag = 1;
                } 
                break;
            case GAMEPLAY:
                uart_puts("gameplay\n");
                initialize_game();
                while(1) {
                    c = uart_getc();
                    render_snake();
                    if (c != 0) {
                        handle_control(c);
                        c = 0;
                    }
                    if (snake_length == penalty_turn) {
                        render_bomb();
                    }
                    if (snake_eat_self()) {
                        init_UI();
                        state = END;
                        break;
                    } 
                    wait_msec(speed);
                }
                break;
            case END:
                uart_puts("gameover\n");
                break;
        } 
    }
}