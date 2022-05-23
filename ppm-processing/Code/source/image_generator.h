#ifndef IMAGE_GENERATOR_H_
#define IMAGE_GENERATOR_H_

#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <vector>

#include "triple.h"

typedef enum InitType {
    RANDOM,
    WHITE,
    BLACK,
    RGB_SPLIT,
    RGB_SPECTRUM
} InitType;

typedef std::vector< std::vector<Triple> > Matrix;
typedef std::vector<Triple> Row;

class ImageGenerator
{
    public:
        // Constructors
        ImageGenerator();
        ImageGenerator(std::string fname);
        // Getter Functions
        Matrix get_image();
        int get_height();
        int get_width();
        int get_rgb_max();
        std::string get_type();
        // util
        void print_image();
        void generate_ppm_image(Triple color, int width, int height);
        // Image Filter
        void apply_color_filter(Triple filter);
        void apply_inverted_filter();
        void apply_grayscale_filter();
        // Graphics operations e.g. drawing
        void fill_with_color(Triple color);
        void draw_line(Triple color, int x1, int y1, int x2, int y2);
        // IO operations
        void read_image_from_file(std::string fname);
        void save_image_to_file(std::string fname);
    
    private:
        Matrix Image;
        std::string PPM_Type;
        int Image_Width;
        int Image_Height;
        int RGB_Max;
        
        Triple get_next_rgb(std::ifstream &src);
        
};

#endif
