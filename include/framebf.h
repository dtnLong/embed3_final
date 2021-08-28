// ----------------------------------- framebf.h -------------------------------------
void framebf_init();
void drawPixelARGB32(int x, int y, unsigned int attr);
void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);
void set_physical_size(int width, int height);
void set_virtual_size(int width, int height);
void clear_display();
void set_background(int color);