#include "../include/snake.h"
#include "../include/framebf.h"
#include "../include/delay.h"
#include "../include/uart.h"
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define SNAKE_WIDTH 19
#define BACKGROUND_COLOR 0x004c546e
#define BOX_COLOR 0x00e1e4ed

typedef enum {WELCOME, MAIN_MENU, GUIDE, GAMEPLAY, END} STATE;
typedef enum {LEFT,RIGHT,UP,DOWN} DIRECTION;
typedef enum {FAST, REVERSED, NONE} PENALTY; 
STATE state = WELCOME;
DIRECTION direction = RIGHT; 
PENALTY penalty = NONE;

//coordiantes
int x,y;

//game stats
int score = 0;

//snake data 
int snake_length = 3;
int snake_coord[1400][2];

//render flags
int welcome_screen_flag = 1;
int main_menu_flag = 1; 
int guide_flag = 1; 
int score_flag = 1;
int end_flag = 1;

//game flags


void init_UI() {
    drawRectARGB32(0,0,1023,767,BACKGROUND_COLOR,1); //draw background
  	drawRectARGB32(111,33,912,734,BOX_COLOR,1); //draw inner box
}

void update_head_coord(){
	snake_coord[0][0] = x;
	snake_coord[0][1] = y;
}

void initialize_game() {
    score = 0;
	snake_length = 3;
	x = SCREEN_WIDTH/2;
	y = SCREEN_HEIGHT/2;
    direction = RIGHT;
    update_head_coord();

    for (int i = 1; i < snake_length; i++) {
        snake_coord[i][0] = snake_coord[i-1][0] - SNAKE_WIDTH - 1;
        snake_coord[i][1] = snake_coord[i-1][1];
    }
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
			x -= SNAKE_WIDTH - 1;
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
			x -= SNAKE_WIDTH - 1;
			if (x < 112){ 
				x = 912 - SNAKE_WIDTH -1;
			} 
		}
	}
	else if (direction == UP){ 
		if (penalty == REVERSED){
			y += SNAKE_WIDTH + 1;
			if (y >= 734){ 
                y = 33;
			}
		}
		else{
			y -= SNAKE_WIDTH - 1;
			if (y < 33){ 
                y = 734 - SNAKE_WIDTH - 1;
			} 
		}
	}
	else{ 
		if (penalty == REVERSED){
			y -= SNAKE_WIDTH - 1;
			if (y < 33){
                y = 734 - SNAKE_WIDTH - 1;
			} 
		}
		else{
			y += SNAKE_WIDTH + 1;
			if (y >= 734){ 
                y = 33;
			}
		}
	}
    update_head_coord();
}

void render_snake() {
    clear_tail();
    update_snake_coord();
    render_head();
}

void run_snake() {
    init_UI();
    initialize_game();
    drawRectARGB32(0,0,19, 19, 0x00ff8c00, 1);
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
                if (main_menu_flag) {
                    drawString(328, 310, "Press SPACE to continue", 0x00FFFFFF, 2);
                    main_menu_flag = 0;
                } 
                c = uart_getc();
                if (c == ' ') {
                    state = GUIDE;
                    uart_puts("1");
                    drawRectARGB32(328, 310, 696, 326, BOX_COLOR, 1);
                    main_menu_flag = 1;
                } 
                break;
            case GUIDE:
                if (guide_flag) {
                    guide_flag = 0;
                }
                c = uart_getc();
                if (c == ' ') {
                    state = GAMEPLAY;
                    uart_puts("2");
                    guide_flag = 1;
                } 
                break;
            case GAMEPLAY:
                uart_puts("333");
                while(1) {
                    wait_msec(200);
                    render_snake();
                }
                break;
            case END:
                break;
        } 
    }
}