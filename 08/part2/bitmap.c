#include "bitmap.h"

#include <stdio.h>
#include <string.h>

// based on http://ricardolovelace.com/creating-bitmap-images-with-c-on-windows.html

struct bitmap_file_header {
    unsigned char   bitmap_type[2];
    int             file_size;
    short           reserved1;
    short           reserved2;
    unsigned int    offset_bits;
};

struct bitmap_image_header {
    unsigned int    size_header;
    unsigned int    width;
    unsigned int    height;
    short int       planes;
    short int       bit_count;
    unsigned int    compression;
    unsigned int    image_size;
    unsigned int    ppm_x;
    unsigned int    ppm_y;
    unsigned int    clr_used;
    unsigned int    clr_important;
};

void writeFileHeader( FILE *file, struct bitmap_file_header *header ) {
    fwrite(&header->bitmap_type, 1, 2, file );
    fwrite(&header->file_size, 1, 4, file );
    fwrite(&header->reserved1, 1, 2, file );
    fwrite(&header->reserved2, 1, 2, file );
    fwrite(&header->offset_bits, 1, 4, file );
}

void writeImageHeader( FILE *file, struct bitmap_image_header *header ) {
    fwrite(&header->size_header, 1, 4, file );
    fwrite(&header->width, 1, 4, file );
    fwrite(&header->height, 1, 4, file );
    fwrite(&header->planes, 1, 2, file );
    fwrite(&header->bit_count, 1, 2, file );
    fwrite(&header->compression, 1, 4, file );
    fwrite(&header->image_size, 1, 4, file );
    fwrite(&header->ppm_x, 1, 4, file );
    fwrite(&header->ppm_y, 1, 4, file );
    fwrite(&header->clr_used, 1, 4, file );
    fwrite(&header->clr_important, 1, 4, file );
}

void writeBitmap( int *pixels, size_t width, size_t height ) {
    FILE *image;
    int image_size = width * height;
    int file_size = 54 + 3 * image_size;
    size_t padding = 0;
    if( ( width * 3 ) % 4 != 0 ) {
        padding = 4 - ( ( width * 3 ) % 4 );
        file_size += padding * height;
    }
    struct bitmap_file_header file_header;

    memcpy(&file_header.bitmap_type, "BM", 2);
    file_header.file_size       = file_size;
    file_header.reserved1       = 0;
    file_header.reserved2       = 0;
    file_header.offset_bits     = 54;

    struct bitmap_image_header image_header;

    image_header.size_header     = sizeof( struct bitmap_image_header );
    image_header.width           = width;
    image_header.height          = height;
    image_header.planes          = 1;
    image_header.bit_count       = 24;
    image_header.compression     = 0;
    image_header.image_size      = file_size;
    image_header.ppm_x           = 0;
    image_header.ppm_y           = 0;
    image_header.clr_used        = 0;
    image_header.clr_important   = 0;

    image = fopen( "output.bmp", "w");

    writeFileHeader( image, &file_header );
    writeImageHeader( image, &image_header );

    unsigned char red   = 0;
    unsigned char green = 0;
    unsigned char blue  = 0;
    for (size_t i = 0; i < height; i++) {
        for( size_t j = 0; j < width; j++ ) { 
            if( pixels[i*width + j] == 0 ) {
                red = green = blue = 0;
            } else if ( pixels[i*width + j] == 1 ) {
                red = green = blue = 255;
            } else {
                // do pink if transparent
                red = 255;
                blue = 255;
                green = 0;
            }
            fwrite( &blue, 1, 1, image );
            fwrite( &green, 1, 1, image );
            fwrite( &red, 1, 1, image );
        }
        red = 0;
        fwrite( &red, 1, padding, image );
    }

    fclose(image);
}
