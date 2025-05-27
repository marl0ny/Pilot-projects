#ifndef __EMSCRIPTEN__
#include <png.h>
#include <stdlib.h>
#include <stdio.h>


/*For writing a png file using the PNG library, see the examples
from this resource:

    www.libpng.org/pub/png/libpng-1.2.5-manual.html#section4

*/
int write_rgb8_png(const char *fname,
                   unsigned char *data, int width, int height) {
    png_struct *write_ptr
         = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                   NULL, NULL, NULL);
    if (!write_ptr) {
        fprintf(stderr,
                "write_rgba8_png: png_create_write_struct returned NULL\n");
        return -1;
    }
    png_info *info_ptr = png_create_info_struct(write_ptr);
    if (!info_ptr) {
        fprintf(stderr,
                "write_rgba8_png: png_create_info_struct returned NULL\n");
        png_destroy_write_struct(&write_ptr, NULL);
        return -1;
    }
    FILE *f = fopen(fname, "wb");
    if (!f) {
        perror("fopen");
        return -1;
    }
    if (setjmp(png_jmpbuf(write_ptr))) {
        png_destroy_write_struct(&write_ptr, &info_ptr);
        fclose(f);
        return -1;
    }
    png_init_io(write_ptr, f);
    png_set_IHDR(write_ptr, info_ptr, width, height, 8,
                 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(write_ptr, info_ptr);
    for (int i = height - 1; i >= 0; i--)
        png_write_row(write_ptr, &data[3*i*width]);
    png_write_end(write_ptr, info_ptr);
    fclose(f);
    return 0;
}
#endif