#include "../include/uart.h"
#include "../include/mbox.h"
#include "../include/util.h"
#include "../include/command.h"
#include "../include/framebf.h"

void command_help(char* parameter) {
    // Display command list and command detail
    if (str_compare(parameter, "\0")) {
        uart_puts("For more information on a specific command, type help command-name\n\n");
    } else {
        uart_puts("Command not recognize!\n");
        uart_puts("Type \"help\" to view all command\n\n");
    }
}

void command_cls() {
    uart_puts("\033[H\033[J");
}

void command_scrsize(char (*token)[50]) {
    /*
        Change physical and virtual screen size
    */
    if (token[1][0] == '\0') {
        uart_puts("This command need parameter!\n");
        uart_puts(HELP_MESSAGE);
        return;
    }

    if (str_is_int(token[1]) == 0 || str_is_int(token[2]) == 0) {
        uart_puts("Screen width or height must be a number\n");
        uart_puts(HELP_MESSAGE);
        return;
    }
    
    framebf_init(str_to_int(token[1]), str_to_int(token[2]));
    uart_puts("\n");
}

void command_clrscr() {
    /*
        Clear all pixel of the display to default color
    */
    clear_display();
}

void command_txt() {
    /*
        Display name of team member in 3 different color
    */
	drawString(20, 20, "Dang Truong Nguyen Long", 0x00FFFFFF, 4);
	drawString(20, 100, "Le Ngoc Duy", 0x006800FF, 4);
	drawString(20, 180, "Phan Quoc Binh", 0x00BD1E13, 4);
}

void command_smallimg() {
    draw_small_image(0, 0);
}

void command_largeimg() {
    draw_large_image_controller(0, 0);
}

void command_video(){
    
}

void interpret_command(char* command) {
    char command_token[20][50];
    for (int i = 0; i < 20; i++) {
        command_token[i][0] = '\0';
    }

    str_split(command_token, command, ' ');
    if (str_compare(command_token[0], "help")) {
        command_help(command_token[1]);
    } else if (str_compare(command_token[0], "cls")) {
        command_cls();
    } else if (str_compare(command_token[0], "scrsize")) {
        command_scrsize(command_token);
    } else if (str_compare(command_token[0], "clrscr")) {
        command_clrscr();
    } else if (str_compare(command_token[0], "txt")) {
        command_txt();
    } else if (str_compare(command_token[0], "small_img")) {
        command_smallimg();
    } else if (str_compare(command_token[0], "large_img")) {
        command_largeimg();
    } else if (str_compare(command_token[0], "video")){
        uart_puts("Video command\n");
        command_video();
    } else if (str_compare(command_token[0], "abc")){
        uart_puts("abc command\n");
    } else {
        uart_puts("Command not recognize!\n");
        uart_puts("Type \"help\" to view all command\n\n");
    }
}