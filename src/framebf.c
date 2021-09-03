#include "../include/mbox.h"
#include "../include/uart.h"
#include "../include/font.h"
#include "../include/large_img.h"
#include "../include/video.h"
#include "../include/delay.h"

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

void framebf_init(int width, int heigth) {
    /*
        Set screen resolution based on input width and heigth
    */
    mbox[0] = 35*4; // Length of message in bytes
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_SETPHYWH; //Set physical width-height
    mbox[3] = 8; // Value size in bytes
    mbox[4] = 0; // REQUEST CODE = 0
    mbox[5] = width; // Value(width)
    mbox[6] = heigth; // Value(height)

    mbox[7] = MBOX_TAG_SETVIRTWH; //Set virtual width-height
    mbox[8] = 8;
    mbox[9] = 0;
    mbox[10] = width;
    mbox[11] = heigth;

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

void drawLine(int x1, int y1, int x2, int y2, unsigned int attr)  
{  
    int dx, dy, p, x, y;

    dx = x2-x1;
    dy = y2-y1;
    x = x1;
    y = y1;
    p = 2*dy-dx;

    while (x<x2) {
       if (p >= 0) {
          drawPixelARGB32(x,y,attr);
          y++;
          p = p+2*dy-2*dx;
       } else {
          drawPixelARGB32(x,y,attr);
          p = p+2*dy;
       }
       x++;
    }
}

void drawBomb(int x0, int y0, int radius) {
    int x = radius;
    int y = 0;
    int err = 0;
 
    while (x >= y) {
        drawLine(x0 - y, y0 + x, x0 + y, y0 + x, 0x008ACB88);
        drawLine(x0 - x, y0 + y, x0 + x, y0 + y, 0x00f786b9);
        drawLine(x0 - x, y0 - y, x0 + x, y0 - y, 0x008ACB88);
        drawLine(x0 - y, y0 - x, x0 + y, y0 - x, 0x00f786b9);
        drawPixelARGB32(x0 - y, y0 + x, 0x008ACB88);
        drawPixelARGB32(x0 + y, y0 + x, 0x008ACB88);
        drawPixelARGB32(x0 - x, y0 + y, 0x008ACB88);
        drawPixelARGB32(x0 + x, y0 + y, 0x008ACB88);
        drawPixelARGB32(x0 - x, y0 - y, 0x008ACB88);
        drawPixelARGB32(x0 + x, y0 - y, 0x008ACB88);
        drawPixelARGB32(x0 - y, y0 - x, 0x008ACB88);
        drawPixelARGB32(x0 + y, y0 - x, 0x008ACB88);

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
    
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
};

void drawCircle(int x0, int y0, int radius, unsigned int attr, int outline_attr, int fill) {
    int x = radius;
    int y = 0;
    int err = 0;
 
    while (x >= y) {
        if (fill) {
            drawLine(x0 - y, y0 + x, x0 + y, y0 + x, attr);
            drawLine(x0 - x, y0 + y, x0 + x, y0 + y, attr);
            drawLine(x0 - x, y0 - y, x0 + x, y0 - y, attr);
            drawLine(x0 - y, y0 - x, x0 + y, y0 - x, attr);
        }
        drawPixelARGB32(x0 - y, y0 + x, outline_attr);
        drawPixelARGB32(x0 + y, y0 + x, outline_attr);
        drawPixelARGB32(x0 - x, y0 + y, outline_attr);
        drawPixelARGB32(x0 + x, y0 + y, outline_attr);
        drawPixelARGB32(x0 - x, y0 - y, outline_attr);
        drawPixelARGB32(x0 + x, y0 - y, outline_attr);
        drawPixelARGB32(x0 - y, y0 - x, outline_attr);
        drawPixelARGB32(x0 + y, y0 - x, outline_attr);

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
    
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

void clear_display() {
    drawRectARGB32(0, 0, scr_width, scr_height, default_background_color, 1);
}

// Font display
// void drawChar(unsigned char ch, int x, int y, unsigned int attr, int scale) {
//     /*
//         Display a character.
//         ch: Character to display
//         x, y: Coordinate to display character at
//         attr: color of the character
//         scale: size of the character multiplier
//     */

//     unsigned char *glyph = 0;

//     // Get the current glyph. Each element in the font array is the data of a glyph with size of 8 bit
//     // So the address of the glype will be ASCII value of char * 8
//     if (ch < FONT_NUMGLYPHS) {
//         glyph = (unsigned char *) &font + (ch * FONT_BPG);
//     } else {
//         glyph = (unsigned char *) &font;
//     }

//     // Loop through all row (element) of the glyph
//     for (int i = 1; i <= (FONT_HEIGHT * scale); i++) {
//         // Loop through each bit of the hex value of the current glyph element
//         // When scaling, each bit will have to be display a "scale" amount of time
//         for (int j = 0; j < (FONT_WIDTH * scale); j++) {
//             // Obtain the current bit to be displayed. By using j / scale, each bit will be duplicate by "scale" amount
//             // Ex: scale = 2;
//             // j = 0 -> bit = 0
//             // j = 1 -> bit = 0
//             // j = 2 -> bit = 1
//             // j = 2 -> bit = 1
//             // j = 3 -> bit = 2
//             // j = 3 -> bit = 2
//             // ...
//             unsigned char bit_data = 1 << (j / scale);
//             unsigned int color = attr;

//             // If the current bit is 1 then drawa pixel with the input color else draw black pixel
//             if (*glyph & bit_data) {
//                 drawPixelARGB32(x+j, y+i, color);
//             } else {
//                 drawPixelARGB32(x+j, y+i, default_background_color);
//             }
//         }

//         // When scaling, each row (element) of the glyph will have to be display a "scale" amount of time
//         // By doing this, the current glyph row (element) only increase after the element is display 
//         // Ex: scale = 2
//         // i = 1 -> i % 2 = 1;
//         // i = 2 -> i % 2 = 0; glyph row increase
//         // i = 3 -> i % 2 = 1;
//         // i = 4 -> i % 2 = 0; glyph row increase
//         // ...
//         if (i % scale == 0) {
//             glyph += FONT_BPL;
//         }
//     }
// }

void drawChar(unsigned char ch, int x, int y, unsigned int attr, int scale) {
    /*
        Display a character.
        ch: Character to display
        x, y: Coordinate to display character at
        attr: color of the character
        scale: size of the character multiplier
    */

    if (ch > FONT_NUMGLYPHS) {
        ch = 0;
    }

    int current_glyph_height = 0;

    // Loop through all row (element) of the glyph
    // When scaling, each row will have to be display a "scale" amount of time
    for (int height = 1; height <= (FONT_HEIGHT * scale); height++) {
        // Loop through each bit of the hex value of the current glyph element
        // When scaling, each bit will have to be display a "scale" amount of time
        for (int width = 0; width < (FONT_WIDTH * scale); width++) {
            // Obtain the current bit to be displayed. By using width / scale, each bit will be duplicate by "scale" amount
            // Ex: scale = 2;
            // width = 0 -> bit = 0
            // width = 1 -> bit = 0
            // width = 2 -> bit = 1
            // width = 2 -> bit = 1
            // width = 3 -> bit = 2
            // width = 3 -> bit = 2
            // ...
            unsigned char bit_data = 1 << (width / scale);

            // If the current bit is 1 then drawa pixel with the input color else draw black pixel
            // Each element in the font array contain data for the corresponding ASCII value
            if (font[ch][current_glyph_height] & bit_data) {
                drawPixelARGB32(x + width, y + height, attr);
            } else {
                drawPixelARGB32(x + width, y + height, default_background_color);
            }
        }

        // When scaling, each row (element) of the glyph will have to be display a "scale" amount of time
        // By doing this, the current glyph row (element) only increase after the element is display 
        // Ex: scale = 2
        // height = 1 -> height % 2 = 1;
        // height = 2 -> height % 2 = 0; glyph row increase
        // height = 3 -> height % 2 = 1;
        // height = 4 -> height % 2 = 0; glyph row increase
        // ...
        if (height % scale == 0) {
            current_glyph_height++;
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

void drawChar_bg(unsigned char ch, int x, int y, unsigned int attr, unsigned int bg_attr, int scale) {
    /*
        Display a character.
        ch: Character to display
        x, y: Coordinate to display character at
        attr: color of the character
        scale: size of the character multiplier
    */

    if (ch > FONT_NUMGLYPHS) {
        ch = 0;
    }

    int current_glyph_height = 0;

    // Loop through all row (element) of the glyph
    // When scaling, each row will have to be display a "scale" amount of time
    for (int height = 1; height <= (FONT_HEIGHT * scale); height++) {
        // Loop through each bit of the hex value of the current glyph element
        // When scaling, each bit will have to be display a "scale" amount of time
        for (int width = 0; width < (FONT_WIDTH * scale); width++) {
            // Obtain the current bit to be displayed. By using width / scale, each bit will be duplicate by "scale" amount
            // Ex: scale = 2;
            // width = 0 -> bit = 0
            // width = 1 -> bit = 0
            // width = 2 -> bit = 1
            // width = 2 -> bit = 1
            // width = 3 -> bit = 2
            // width = 3 -> bit = 2
            // ...
            unsigned char bit_data = 1 << (width / scale);

            // If the current bit is 1 then drawa pixel with the input color else draw black pixel
            // Each element in the font array contain data for the corresponding ASCII value
            if (font[ch][current_glyph_height] & bit_data) {
                drawPixelARGB32(x + width, y + height, attr);
            } else {
                drawPixelARGB32(x + width, y + height, bg_attr);
            }
        }

        // When scaling, each row (element) of the glyph will have to be display a "scale" amount of time
        // By doing this, the current glyph row (element) only increase after the element is display 
        // Ex: scale = 2
        // height = 1 -> height % 2 = 1;
        // height = 2 -> height % 2 = 0; glyph row increase
        // height = 3 -> height % 2 = 1;
        // height = 4 -> height % 2 = 0; glyph row increase
        // ...
        if (height % scale == 0) {
            current_glyph_height++;
        }
    }
}

void drawString_bg(int x, int y, char *s, unsigned int attr, unsigned int bg_attr, int scale) {
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
	        drawChar_bg(*s, x, y, attr, bg_attr, scale);
            x += (FONT_WIDTH * scale);
        }
       s++;
    }
}

// Display image
void draw_small_image(int x, int y, int width, int height, int small_img[]) {
    /*
        Draw image have width and height smaller than screen size
        x, y: coordinate of the top left of the image
    */
    
    // Loop through image height
    for (int i = 0; i < height; i++) {
        // Loop through image width
        for (int j = 0; j < width; j++) {
            // The order of the image pixel data is arranged from left to right and top to bottom
            // So the current pixel data in the array is: image width * current height + current width
            drawPixelARGB32(x + j, y + i, small_img[width * i + j]);
        }
    }
}

void draw_large_image(int x, int y) {
    /*
        Draw large image have width larger than screen size
        x, y: coordinate of the top left of the image
    */
    int start_y = 0;
    int end_y = LARGE_IMG_HEIGHT;

    // Clear image artifact left behind on the screen when scrolling
    if (y < 0) {
        drawRectARGB32(0, y + LARGE_IMG_HEIGHT, scr_width, scr_height, default_background_color, 1);
    } else if (y + LARGE_IMG_HEIGHT > scr_height) {
        drawRectARGB32(0, 0, scr_width, y, default_background_color, 1);
    }

    // When y is smaller than 0, the top of the image is cut off,
    // so draw the image from the y cut off of the image which is: -1 * y, to the image height
    if (y < 0) {
        start_y = -1 * y;
        y = 0;
        end_y = LARGE_IMG_HEIGHT;
    // When the bottom y of the image is larger than current screen height, the bottom of the image is cut off,
    // so draw the image from y = 0 of the image to y cut off of the image which is: screen height - y
    } else if (y + LARGE_IMG_HEIGHT > scr_height) {
        start_y = 0;
        end_y = scr_height - y;
    }

    for (int i = start_y; i < end_y; i++) {
        for (int j = 0; j < LARGE_IMG_WIDTH; j++) {
            drawPixelARGB32(x + j, y + i - start_y, large_img[LARGE_IMG_WIDTH * i + j]);
        }
    }
}

void image_viewer(int x, int y) {
    /*
        Image viewer allow scrolling image vertically
        x, y: coordinate of the top left of the image
    */
    uart_puts("Large image viewer \n");
    uart_puts("Press 'w' to scroll up\n");
    uart_puts("Press 's' to scroll down\n");
    uart_puts("Press 'q' to exit\n");

    draw_large_image(x, y);

    char option = 0;
    while (option != 'q') {
        option = uart_getc();
        // When scrolling up, the image will move down so y increase
        // When y is smaller than 0, 
        // The top of the image is still cut off so increase y to see top of the image
        if (option == 'w') {
            if (y < 0) {
                y += 10;
                draw_large_image(x, y);
            }
        // When scrolling down, the image will move up so y decrease
        // When the bottom y of the image is larger than current screen height, 
        // The bottom of the image is still cut off so decrease y to see bottom of the image
        } else if (option == 's') {
            if (y + LARGE_IMG_HEIGHT > scr_height) {
                y -= 10;
                draw_large_image(x, y);
            }
        }
    }
}

// Video
void play_video(int x, int y) {
    /*
        Play a video
        x, y: Top left coordinate of the video
    */

    // Loop through every frame
    for (int frame = 0; frame < VIDEO_TOTAL_FRAME; frame++) {
        // Loop through frame height
        for (int i = 0; i < VIDEO_HEIGHT; i++) {
            // Loop through frame width
            for (int j = 0; j < VIDEO_WIDTH; j++) {
                // The order of the image pixel data is arranged from left to right and top to bottom
                // So the current pixel data in the array is: image width * current height + current width
                drawPixelARGB32(x + j, y + i, video_data[frame][VIDEO_WIDTH * i + j]);
            }
        }

        //The video is 15 FPS so 1 frame is 0.0667 s = 66.7 ms
        wait_msec(67);
    }
}