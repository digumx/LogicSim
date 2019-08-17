/*
 * A small standalone script to convert from old legacy RGB packing to new RGB packing
 */

#include <iostream>
#include <bitset>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int main(int argc, char** argv)
{
        if(argc < 3)
        {
                std::cerr << "usage: ./convert-format <input file path> <output file path>" << std::endl;
                return EXIT_FAILURE;
        }

        int w,h,c;
        unsigned char* in_img = stbi_load(argv[1], &w, &h, &c, 3);
        unsigned char* out_img = new unsigned char[w*h*3];
        for(int i = 0; i < w*h; i++)
        {
                std::bitset<8> r_in(in_img[i*3]);
                std::bitset<8> g_in(in_img[i*3+1]);
                std::bitset<8> b_in(in_img[i*3+2]);
                std::bitset<8> r_out, g_out, b_out;
                for(int k = 0; k < 4; k++) r_out[k] = r_in[k+4];
                for(int k = 0; k < 6; k++) g_out[k] = g_in[k+2];
                for(int k = 0; k < 2; k++) g_out[k+6] = r_in[k+2];
                for(int k = 0; k < 7; k++) b_out[k] = b_in[k+1];
                b_out[7] = g_in[1];
                out_img[i*3] = (char) r_out.to_ulong();
                out_img[i*3+1] = (char) g_out.to_ulong();
                out_img[i*3+2] = (char) b_out.to_ulong();
        }
        stbi_image_free(in_img);
        stbi_write_png(argv[2], w, h, 3, out_img, w*3);
}
