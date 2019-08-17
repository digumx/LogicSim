/*
 * This is a small script to transform (flip horizontally, flip vertically, transpose, or a combination of these in sequence) a circuit image.
 */

#include <bitset>
#include <array>
#include <vector>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION 
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

/*
 * Permutes the 20 significant bits of the logic element as given by the permutation array. In the permutation array perm[i] represents 
 * the position the i'th bit is derived from. Keeps the insignifant label bits unchanged.
 */
void permute_significant(const unsigned char r_in, const unsigned char g_in, const unsigned char b_in, 
                unsigned char& r_out, unsigned char& g_out, unsigned char& b_out, std::array<int, 20> perm)
{
        std::bitset<24> logic_element((r_in<<16) + (g_in<<8) + b_in);
        std::bitset<24> out = logic_element;
        for(int i = 0; i < 20; ++i)
                out[i] = logic_element[perm[i]];
        r_out = 0;
        b_out = 0;
        g_out = 0;
        for(int i = 7; i >= 0; --i)
        {
                r_out = r_out*2 + (out[i+16]?1:0);      
                g_out = g_out*2 + (out[i+8]?1:0);      
                b_out = b_out*2 + (out[i]?1:0);      
        }
}

/*
 * The following functions maintain width_w*height_w = width_r*height_r and expect data_w to be allocated with as much space as data_r. Their
 * functions are self explanatory from the name.
 */
void flip_along_vertical(unsigned const char* data_r, int width_r, int height_r, unsigned char* data_w)                           // 0
{
        std::array<int, 20> perm = {0, 4, 2, 6, 1, 5, 3, 7, 8, 12, 10, 14, 9, 13, 11, 15, 18, 17, 16, 19};
        for(int x = 0; x < width_r; ++x)
                for(int y = 0; y < height_r; ++y)
                {
                        int i = y*width_r + x;
                        int j = y*width_r + (width_r-x-1);
                        unsigned char r,g,b;
                        permute_significant(data_r[i*3], data_r[i*3+1], data_r[i*3+2], r, g, b, perm);
                        data_w[j*3] = r;
                        data_w[j*3+1] = g;
                        data_w[j*3+2] = b;
                }
}
void flip_along_horizontal(unsigned const char* data_r, int width_r, int height_r, unsigned char* data_w)                         // 1
{
        std::array<int, 20> perm = {0, 1, 8, 9, 4, 5, 12, 13, 2, 3, 10, 11, 6, 7, 14, 15, 16, 19, 18, 17};
        for(int x = 0; x < width_r; ++x)
                for(int y = 0; y < height_r; ++y)
                {
                        int i = y*width_r + x;
                        int j = (height_r-y-1)*width_r + x;
                        unsigned char r,g,b;
                        permute_significant(data_r[i*3], data_r[i*3+1], data_r[i*3+2], r, g, b, perm);
                        data_w[j*3] = r;
                        data_w[j*3+1] = g;
                        data_w[j*3+2] = b;
                }
}
void transpose(unsigned const char* data_r, int width_r, int height_r, unsigned char* data_w)        // 2
{
        std::array<int, 20> perm = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15, 19, 18, 17, 16};
        for(int x = 0; x < width_r; ++x)
                for(int y = 0; y < height_r; ++y)
                {
                        int i = y*width_r + x;
                        int j = x*height_r + y;
                        unsigned char r,g,b;
                        permute_significant(data_r[i*3], data_r[i*3+1], data_r[i*3+2], r, g, b, perm);
                        data_w[j*3] = r;
                        data_w[j*3+1] = g;
                        data_w[j*3+2] = b;
                }
}

void print_usage()
{
        std::cout << "Usage: " << std::endl;
        std::cout << "transform-circuit <transform string> <input circuit> <output circuit>" << std::endl;
        std::cout << "where," << std::endl;
        std::cout << "\t<transform string>:\tIt is a string representing the transform to be performed on the image in left to right order. Each character "
                << " represents a transform to be performed, as follows: " << std::endl;
        std::cout << "\t\tv:\tFlip along vertical" << std::endl;
        std::cout << "\t\th:\tFlip along horizontal" << std::endl;
        std::cout << "\t\tt:\tTranspose" << std::endl;
        std::cout << "\t\tl:\tRotate left" << std::endl;
        std::cout << "\t\tr:\tRotate right" << std::endl;
        std::cout << "\t\tu:\tRotate 180 degrees" << std::endl;
        std::cout << "\t<input circuit>:\tThe path to input circuit image" << std::endl;
        std::cout << "\t<output circuit>:\tThe path to the output circuit image" << std::endl;
        std::exit(EXIT_FAILURE);
}

int main(int argc, const char** argv)
{
        if(argc < 4)
        {
                std::cout << "Not enough arguements: " << argc << std::endl;
                print_usage();
        }

        const std::string transform_string(argv[1]);
        std::vector<int> transforms;
        for(std::string::const_iterator c = transform_string.begin(); c != transform_string.end(); ++c)
        {
                if((*c) == 'v') transforms.push_back(0);
                else if((*c) == 'h') transforms.push_back(1);
                else if((*c) == 't') transforms.push_back(2);
                else if((*c) == 'l') { transforms.push_back(2); transforms.push_back(1); }
                else if((*c) == 'r') { transforms.push_back(1); transforms.push_back(2); }
                else if((*c) == 'u') { transforms.push_back(0); transforms.push_back(1); }
                else 
                {
                        std::cout << "Unknown transform string character " << (*c) << std::endl;
                        print_usage();
                }
        }

        std::cout << "Loading circuit" << std::endl;
        int width, height, n_channels;
        unsigned char* data1 = stbi_load(argv[2], &width, &height, &n_channels, 3);
        unsigned char* data1_orig = data1;
        if(n_channels != 3)
        {
                std::cout << "Input image does not have 3 channels." << std::endl;
                return EXIT_FAILURE;
        }
        std::cout << "Circuit loaded" << std::endl;

        unsigned char* data2 = new unsigned char[width*height*3];
        unsigned char* data2_orig = data2;

        std::cout << "Processing" << std::endl;
        for(std::vector<int>::iterator i = transforms.begin(); i != transforms.end(); ++i)
        {
                switch((*i))
                {
                        case 0: flip_along_vertical(data1, width, height, data2);
                                break;
                        case 1: flip_along_horizontal(data1, width, height, data2);
                                break;
                        case 2: transpose(data1, width, height, data2);
                                std::swap(width, height);
                                break;
                }
                std::swap(data1, data2);
        }
        std::cout << "Done Processing" << std::endl;

        std::cout << "Writing file" << std::endl;
        stbi_write_png(argv[3], width, height, 3, data1, width*3);
        std::cout << "Done writing file" << std::endl;  

        stbi_image_free(data1_orig);
        delete[] data2_orig;

}
