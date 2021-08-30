#include "../include/mbox.h"
#include "../include/uart.h"
#include "../include/command.h"
#include "../include/util.h"
#include "../include/framebf.h"

void main() {
    // set up serial console
	uart_init();

	// Initialize frame buffer
	framebf_init();

	// Print welcome message
	uart_puts("d88888b d88888b d88888b d888888b .d888b.   j88D  .d888b.  .d88b. \n");
	uart_puts("88'     88'     88'     `~~88~~' VP  `8D  j8~88  88' `8D .8P  88.\n");
	uart_puts("88ooooo 88ooooo 88ooooo    88       odD' j8' 88  `V8o88' 88  d'88\n");
	uart_puts("88~~~~~ 88~~~~~ 88~~~~~    88     .88'   V88888D    d8'  88 d' 88\n");
	uart_puts("88.     88.     88.        88    j88.        88    d8'   `88  d8'\n");
	uart_puts("Y88888P Y88888P Y88888P    YP    888888D     VP   d8'     `Y88P'\n");
	uart_puts("\n");
	uart_puts("\n");
	uart_puts("d8888b.  .d8b.  d8888b. d88888b    .d88b.  .d8888. \n");
	uart_puts("88  `8D d8' `8b 88  `8D 88'       .8P  Y8. 88'  YP \n");
	uart_puts("88oooY' 88ooo88 88oobY' 88ooooo   88    88 `8bo. \n");
	uart_puts("88~~~b. 88~~~88 88`8b   88~~~~~   88    88   `Y8b. \n");
	uart_puts("88   8D 88   88 88 `88. 88.       `8b  d8' db   8D \n");
	uart_puts("Y8888P' YP   YP 88   YD Y88888P    `Y88P'  `8888Y'\n");
	uart_puts("\n");

	wait_msec(2000);

	uart_puts("\n");
	uart_puts("MyBareOS>");
	
	char command[50] = "";
	char prev_c = ' ';
    while(1) {
    	//read each char
    	char c = uart_getc();
		if (c == 8) {
			// When the pressed is backspace, send backspace then space to remove character then backspace again
			// When the command is empty, don't send backspace to not remove "MyBareOS>"
			if (command[0] != '\0') {
				uart_sendc(c);
				uart_sendc(' ');
				uart_sendc(c);
				str_trim_last_char(command);
			}
		} else if (c != ' ' || prev_c != ' ') {
    		uart_sendc(c);
			// When Enter is pressed, process command
			// If not keep reading command
			if (c == 10) {
				interpret_command(command);
				command[0] = '\0';
				prev_c = ' ';
				uart_puts("MyBareOS>");
			} else {
				str_concat_char(command, c);
				prev_c = c;
			}
		} else {
			prev_c = c;
		}
    }
}

// Duy was here muahahahaha