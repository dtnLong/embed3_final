// ----------------------------------- framebf.h -------------------------------------
void framebf_init(int width, int heigth);
void drawPixelARGB32(int x, int y, unsigned int attr);
void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);
void clear_display();
void drawChar(unsigned char ch, int x, int y, unsigned int color, int scale);
void drawString(int x, int y, char *s, unsigned int color, int scale);
void draw_small_image(int x, int y);
void draw_large_image(int x, int y);
void image_viewer(int x, int y);
void play_video(int x, int y);
void drawCircle(int x0, int y0, int radius, unsigned int attr, unsigned int outline_attr, int fill);
void drawLine(int x1, int y1, int x2, int y2, unsigned int attr);
void drawBomb(int x0, int y0, int radius);