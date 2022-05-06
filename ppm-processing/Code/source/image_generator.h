#ifndef IMAGE_GENERATOR_H_
#define IMAGE_GENERATOR_H_

#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <cmath>

#include "triple.h"

typedef enum InitType {
    RANDOM,
    WHITE,
    BLACK,
    RGB_SPLIT,
    RGB_SPECTRUM
} InitType;

class ImageGenerator
{
    public:
        ImageGenerator();

        void generate_ppm_image(std::string fname, InitType mode, int width, int height);
        void generate_ppm_horizontal_split(std::string fname, int split, int width, int height, int transition, Triple color1, Triple color2);
        void apply_color_filter(std::string src, std::string dest, Triple filter);
        void apply_inverted_filter(std::string src, std::string dest);
        void apply_grayscale_filter(std::string src, std::string dest);
    
    private:
        void copy_ppm_header(std::ifstream &src, std::ofstream &dest);
        
        Triple get_next_rgb(std::ifstream &src);
        Triple rgb_split_color(int x, int y, int slope, int height);
        Triple rgb_spectrum_color(int x, int y, int z);
        
};

#endif
