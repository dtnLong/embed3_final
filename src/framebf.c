// ----------------------------------- framebf.c -------------------------------------
#include "../include/mbox.h"
#include "../include/uart.h"
#include "../include/font.h"
#include "../include/small_img.h"
#include "../include/large_img.h"

//Use RGBA32 (32 bits for each pixel)
#define COLOR_DEPTH 32
//Pixel Order: BGR in memory order (little endian --> RGB in byte order)
#define PIXEL_ORDER 0
//Screen info
unsigned int scr_width, scr_height, pitch;
/* Frame buffer address
* (declare as pointer of unsigned char to access each byte) */
unsigned char *fb;

int default_background_color = 0x00000000;

/**
* Set screen resolution to 1024x768
*/
void framebf_init() {
    mbox[0] = 35*4; // Length of message in bytes
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETPHYWH; //Set physical width-height
    mbox[3] = 8; // Value size in bytes
    mbox[4] = 0; // REQUEST CODE = 0
    mbox[5] = 1024; // Value(width)
    mbox[6] = 768; // Value(height)
    mbox[7] = MBOX_TAG_SETVIRTWH; //Set virtual width-height
    mbox[8] = 8;
    mbox[9] = 0;
    mbox[10] = 1024;
    mbox[11] = 768;
    mbox[12] = MBOX_TAG_SETVIRTOFF; //Set virtual offset
    mbox[13] = 8;
    mbox[14] = 0;
    mbox[15] = 0; // x offset
    mbox[16] = 0; // y offset
    mbox[17] = MBOX_TAG_SETDEPTH; //Set color depth
    mbox[18] = 4;
    mbox[19] = 0;
    mbox[20] = COLOR_DEPTH; //Bits per pixel
    mbox[21] = MBOX_TAG_SETPXLORDR; //Set pixel order
    mbox[22] = 4;
    mbox[23] = 0;
    mbox[24] = PIXEL_ORDER;
    mbox[25] = MBOX_TAG_GETFB; //Get frame buffer
    mbox[26] = 8;
    mbox[27] = 0;
    mbox[28] = 16; //alignment in 16 bytes
    mbox[29] = 0; //will return Frame Buffer size in bytes
    mbox[30] = MBOX_TAG_GETPITCH; //Get pitch
    mbox[31] = 4;
    mbox[32] = 0;
    mbox[33] = 0; //Will get pitch value here
    mbox[34] = MBOX_TAG_LAST;
    // Call Mailbox
    if (mbox_call(ADDR(mbox), MBOX_CH_PROP) //mailbox call is successful ?
        && mbox[20] == COLOR_DEPTH //got correct color depth ?
        && mbox[24] == PIXEL_ORDER //got correct pixel order ?
        && mbox[28] != 0 //got a valid address for frame buffer ?
    ) {
        /* Convert GPU address to ARM address (clear higher address bits)
        * Frame Buffer is located in RAM memory, which VideoCore MMU
        * maps it to bus address space starting at 0xC0000000.
        * Software accessing RAM directly use physical addresses
        * (based at 0x00000000)
        */
        mbox[28] &= 0x3FFFFFFF;
        // Access frame buffer as 1 byte per each address
        fb = (unsigned char *)((unsigned long)mbox[28]);
        uart_puts("Got allocated Frame Buffer at RAM physical address: ");
        uart_hex(mbox[28]);
        uart_puts("\n");
        uart_puts("Frame Buffer Size (bytes): ");
        uart_dec(mbox[29]);
        uart_puts("\n");
        scr_width = mbox[5]; // Actual physical width
        scr_height = mbox[6]; // Actual physical height
        pitch = mbox[33]; // Number of bytes per line
    } else {
        uart_puts("Unable to get a frame buffer with provided setting\n");
    }
}

void drawPixelARGB32(int x, int y, unsigned int attr) {
    int offs = (y * pitch) + (COLOR_DEPTH/8 * x);
    //Access and assign each byte
    /**(fb + offs ) = (attr >> 0 ) & 0xFF; //BLUE
    *(fb + offs + 1) = (attr >> 8 ) & 0xFF; //GREEN
    *(fb + offs + 2) = (attr >> 16) & 0xFF; //RED
    *(fb + offs + 3) = (attr >> 24) & 0xFF; //ALPHA*/
    //Access 32-bit together
    *((unsigned int*)(fb + offs)) = attr;
}

void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill) {
    for (int y = y1; y <= y2; y++ )
    for (int x = x1; x <= x2; x++) {
        if ((x == x1 || x == x2) || (y == y1 || y == y2)) {
            drawPixelARGB32(x, y, attr);
        }
        else if (fill) {
            drawPixelARGB32(x, y, attr);
        }
    }
}

void set_physical_size(int width, int height) {
    mbox[0] = 30*4; // Length of message in bytes
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_SETPHYWH; //Set physical width-height
    mbox[3] = 8; // Value size in bytes
    mbox[4] = 0; // REQUEST CODE = 0
    mbox[5] = width; // Value(width)
    mbox[6] = height; // Value(height)

    mbox[7] = MBOX_TAG_SETVIRTOFF; //Set virtual offset
    mbox[8] = 8;
    mbox[9] = 0;
    mbox[10] = 0; // x offset
    mbox[11] = 0; // y offset

    mbox[12] = MBOX_TAG_SETDEPTH; //Set color depth
    mbox[13] = 4;
    mbox[14] = 0;
    mbox[15] = COLOR_DEPTH; //Bits per pixel

    mbox[16] = MBOX_TAG_SETPXLORDR; //Set pixel order
    mbox[17] = 4;
    mbox[18] = 0;
    mbox[19] = PIXEL_ORDER;

    mbox[20] = MBOX_TAG_GETFB; //Get frame buffer
    mbox[21] = 8;
    mbox[22] = 0;
    mbox[23] = 16; //alignment in 16 bytes
    mbox[24] = 0; //will return Frame Buffer size in bytes

    mbox[25] = MBOX_TAG_GETPITCH; //Get pitch
    mbox[26] = 4;
    mbox[27] = 0;
    mbox[28] = 0; //Will get pitch value here

    mbox[29] = MBOX_TAG_LAST;

    // Call Mailbox
    if (mbox_call(ADDR(mbox), MBOX_CH_PROP)
        && mbox[15] == COLOR_DEPTH
        && mbox[19] == PIXEL_ORDER
        && mbox[23] != 0
    ) {
        mbox[26] &= 0x3FFFFFFF;
        // Access frame buffer as 1 byte per each address
        fb = (unsigned char *)((unsigned long)mbox[23]);
        uart_puts("Got allocated Frame Buffer at RAM physical address: ");
        uart_hex(mbox[23]);
        uart_puts("\n");
        uart_puts("Frame Buffer Size (bytes): ");
        uart_dec(mbox[24]);
        uart_puts("\n");

        scr_width = mbox[5];
        scr_height = mbox[6];
        pitch = mbox[28];
    } else {
        uart_puts("Unable to get a frame buffer with provided setting\n");
    }
}

void set_virtual_size(int width, int height) {
    mbox[0] = 30*4; // Length of message in bytes
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_SETVIRTWH; //Set virtual width-height
    mbox[3] = 8;
    mbox[4] = 0;
    mbox[5] = width;
    mbox[6] = height;

    mbox[7] = MBOX_TAG_SETVIRTOFF; //Set virtual offset
    mbox[8] = 8;
    mbox[9] = 0;
    mbox[10] = 0; // x offset
    mbox[11] = 0; // y offset

    mbox[12] = MBOX_TAG_SETDEPTH; //Set color depth
    mbox[13] = 4;
    mbox[14] = 0;
    mbox[15] = COLOR_DEPTH; //Bits per pixel

    mbox[16] = MBOX_TAG_SETPXLORDR; //Set pixel order
    mbox[17] = 4;
    mbox[18] = 0;
    mbox[19] = PIXEL_ORDER;

    mbox[20] = MBOX_TAG_GETFB; //Get frame buffer
    mbox[21] = 8;
    mbox[22] = 0;
    mbox[23] = 16; //alignment in 16 bytes
    mbox[24] = 0; //will return Frame Buffer size in bytes

    mbox[25] = MBOX_TAG_GETPITCH; //Get pitch
    mbox[26] = 4;
    mbox[27] = 0;
    mbox[28] = 0; //Will get pitch value here
    mbox[29] = MBOX_TAG_LAST;

    // Call Mailbox
    if (mbox_call(ADDR(mbox), MBOX_CH_PROP)
        && mbox[15] == COLOR_DEPTH
        && mbox[19] == PIXEL_ORDER
        && mbox[23] != 0
    ) {
        mbox[23] &= 0x3FFFFFFF;
        // Access frame buffer as 1 byte per each address
        fb = (unsigned char *)((unsigned long)mbox[23]);
        uart_puts("Got allocated Frame Buffer at RAM physical address: ");
        uart_hex(mbox[23]);
        uart_puts("\n");
        uart_puts("Frame Buffer Size (bytes): ");
        uart_dec(mbox[24]);
        uart_puts("\n");
        pitch = mbox[28];
    } else {
        uart_puts("Unable to get a frame buffer with provided setting\n");
    }
}

void clear_display() {
    drawRectARGB32(0, 0, scr_width, scr_height, default_background_color, 1);
}

/**
 * Display a string using proportional SSFN
 */
void drawChar(unsigned char ch, int x, int y, unsigned int attr, int scale) {
    /*
        Display a character.
        ch: Character to display
        x, y: Coordinate to display character at
        attr: color of the character
        scale: size of the character multiplier
    */

    unsigned char *glyph = 0;

    // Get the current glyph. Each element in the font array is the data of a glyph with size of 8 bit
    // So the address of the glype will be ASCII value of char * 8
    if (ch < FONT_NUMGLYPHS) {
        glyph = (unsigned char *) &font + (ch * FONT_BPG);
    } else {
        glyph = (unsigned char *) &font;
    }

    // Loop through all row (element) of the glyph
    for (int i = 1; i <= (FONT_HEIGHT * scale); i++) {
        // Loop through each bit of the hex value of the current glyph element
        // When scaling, each bit will have to be display a "scale" amount of time
        for (int j = 0; j < (FONT_WIDTH * scale); j++) {
            // Obtain the current bit to be displayed. By using j / scale, each bit will be duplicate by "scale" amount
            // Ex: scale = 2;
            // j = 0 -> bit = 0
            // j = 1 -> bit = 0
            // j = 2 -> bit = 1
            // j = 2 -> bit = 1
            // j = 3 -> bit = 2
            // j = 3 -> bit = 2
            // ...
            unsigned char bit_data = 1 << (j / scale);
            unsigned int color = attr;

            // If the current bit is 1 then drawa pixel with the input color else draw black pixel
            if (*glyph & bit_data) {
                drawPixelARGB32(x+j, y+i, color);
            } else {
                drawPixelARGB32(x+j, y+i, default_background_color);
            }
        }

        // When scaling, each row (element) of the glyph will have to be display a "scale" amount of time
        // By doing this, the current glyph row (element) only increase after the element is display 
        // Ex: scale = 2
        // i = 1 -> i % 2 = 1;
        // i = 2 -> i % 2 = 0; glyph row increase
        // i = 3 -> i % 2 = 1;
        // i = 4 -> i % 2 = 0; glyph row increase
        // ...
        if (i % scale == 0) {
            glyph += FONT_BPL;
        }
    }
}

void drawString(int x, int y, char *s, unsigned int attr, int scale) {
    /*
        Display string.
        ch: Character to display
        x, y: Coordinate to display character at
        attr: color of the character
        scale: size of the character multiplier
    */

    while (*s) {
        if (*s == '\r') {
            x = 0;
        } else if(*s == '\n') {
            x = 0; 
            y += (FONT_HEIGHT * scale);
        } else {
	        drawChar(*s, x, y, attr, scale);
            x += (FONT_WIDTH * scale);
        }
       s++;
    }
}

void draw_small_image(int x, int y) {
    /*
        Draw image have width and height smaller than screen size
    */
    for (int i = 0; i <= SMALL_IMG_HEIGHT; i++) {
        for (int j = 0; j <= SMALL_IMG_WIDTH; j++) {
            drawPixelARGB32(x + j, y + i, small_img[SMALL_IMG_WIDTH * i + j]);
        }
    }
}

void draw_large_image(int x, int y, int start_y, int end_y) {
    /*
        Draw large image have width larger than screen size
    */

    for (int i = start_y; i <= end_y; i++) {
        for (int j = 0; j <= LARGE_IMG_WIDTH; j++) {
            drawPixelARGB32(x + j, y + i - start_y, large_img[LARGE_IMG_WIDTH * i + j]);
        }
    }
}

void draw_large_image_controller(int x, int y) {
    char option = 0;
    int y_displace = y;
    if (y_displace < 0) {
        draw_large_image(x, 0, -1 * y_displace, LARGE_IMG_HEIGHT);
    } else if (y_displace + LARGE_IMG_HEIGHT > scr_height) {
        draw_large_image(x, y_displace, 0, scr_height - y_displace);
    } else {
        draw_large_image(x, y_displace, 0, LARGE_IMG_HEIGHT);
    }
    while (option != 'q') {
        option = uart_getc();
        if (option == 'w') {
            y_displace -= 20;
            if (y_displace < 0) {
                drawRectARGB32(0, y_displace + LARGE_IMG_HEIGHT, scr_width, scr_height, 0x00000000, 1);
            } else if (y_displace + LARGE_IMG_HEIGHT > scr_height) {
                drawRectARGB32(0, y_displace, scr_width, y_displace, 0x00000000, 1);
            }

            if (y_displace < 0) {
                draw_large_image(x, 0, -1 * y_displace, LARGE_IMG_HEIGHT);
            } else if (y_displace + LARGE_IMG_HEIGHT > scr_height) {
                draw_large_image(x, y_displace, 0, scr_height - y_displace);
            } else {
                draw_large_image(x, y_displace, 0, LARGE_IMG_HEIGHT);
            }
        } else if (option == 's') {
            y_displace += 20;
            if (y_displace < 0) {
                drawRectARGB32(0, y_displace + LARGE_IMG_HEIGHT, scr_width, scr_height, 0x00000000, 1);
            } else if (y_displace + LARGE_IMG_HEIGHT > scr_height) {
                drawRectARGB32(0, y_displace, scr_width, y_displace, 0x00000000, 1);
            }

            if (y_displace < 0) {
                draw_large_image(x, 0, -1 * y_displace, LARGE_IMG_HEIGHT);
            } else if (y_displace + LARGE_IMG_HEIGHT > scr_height) {
                draw_large_image(x, y_displace, 0, scr_height - y_displace);
            } else {
                draw_large_image(x, y_displace, 0, LARGE_IMG_HEIGHT);
            }
        }
    }
}

void wait_msec(unsigned int n) {
    n *= 1000;
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    // calculate expire value for counter
    t+=((f/1000)*n)/1000;
    do{asm volatile ("mrs %0, cntpct_el0" : "=r"(r));}while(r<t);
}