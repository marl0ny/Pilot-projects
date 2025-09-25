#include "bmp.hpp"
#include <cstdio>


unsigned int get_bmp_row_byte_size(int width) {
    return ((width*3) % 4 == 0)? 
        (3*width): ((3*width) + (4 - ((3*width) % 4)));
}

BMPHeader::
BMPHeader(int width, int height):
        bm {'B', 'M'},
        _padding {'\0','\0', '\0', '\0'},
        data_offset(54),
        header_size(40),
        width(width), height(height),
        plane_count(1),
        bits_per_pixel(24),
        compression_method(0),
        horizontal_resolution(100),
        vertical_resolution(100),
        color_pallete_count(16777216),
        important_colors_count(0) {
    int row_size = get_bmp_row_byte_size(width);
    this->total_file_size = 54 + row_size*height;
    this->image_size = row_size*height;
}

void print_bmp_header(const BMPHeader &h) {
    printf("Total file size: %d\n", h.total_file_size);
    printf("Data offset: %d\n", h.data_offset);
    printf("Header size: %d\n", h.header_size);
    printf("Width: %d\n", h.width);
    printf("Height: %d\n", h.height);
    printf("Plane count: %d\n", h.plane_count);
    printf("Bits per pixel: %d\n", h.bits_per_pixel);
    printf("Compression method: %d\n", h.compression_method);
    printf("Image size: %d\n", h.image_size);
    printf("Horizontal resolution: %d\n", h.horizontal_resolution);
    printf("Vertical resolution: %d\n", h.vertical_resolution);
    printf("Color pallete count: %d\n", h.color_pallete_count);
    printf("Imporant colors count: %d\n", h.important_colors_count);
}
