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
    // Set physical and virtual screen size of display
    if (token[1][0] == '\0') {
        uart_puts("This command need parameter!\n");
        uart_puts(HELP_MESSAGE);
        return;
    }

    int physical_screen_height = -1;
    int physical_screen_width = -1;
    int virtual_screen_height = -1;
    int virtual_screen_width = -1;

    for (int i = 1; token[i][0] != '\0'; i++) {
        if (i > 4) {
            uart_puts("Invalid extra parameter\n");
            uart_puts(HELP_MESSAGE);
            return;
        } else if (str_compare(token[i], "-p")) {
            if (str_is_int(token[i + 1]) == 0 || str_is_int(token[i + 2]) == 0) {
                uart_puts("Screen width or height must be a number\n");
                uart_puts(HELP_MESSAGE);
                return;
            }
            physical_screen_width = str_to_int(token[i + 1]);
            physical_screen_height = str_to_int(token[i + 2]);
            i += 2;
        } else if (str_compare(token[i], "-v")) {
            if (str_is_int(token[i + 1]) == 0 || str_is_int(token[i + 2]) == 0) {
                uart_puts("Screen width or height must be a number\n");
                uart_puts(HELP_MESSAGE);
                return;
            }
            virtual_screen_width = str_to_int(token[i + 1]);
            virtual_screen_height = str_to_int(token[i + 2]);
            i += 2;
        } else {
            uart_puts("Invalid parameter: ");
            uart_puts(token[i]);
            uart_puts("\n");
            uart_puts(HELP_MESSAGE);
            return;
        }
    }
    if (physical_screen_width != -1 && physical_screen_height != -1) {
        set_physical_size(physical_screen_width, physical_screen_height);
    }
    if (virtual_screen_width != -1 && virtual_screen_height != -1) {
        set_virtual_size(virtual_screen_width, virtual_screen_height);
    }
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
    } else {
        uart_puts("Command not recognize!\n");
        uart_puts("Type \"help\" to view all command\n\n");
    }
}