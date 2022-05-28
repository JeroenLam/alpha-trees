#ifndef IMAGE_GENERATOR_H_
#define IMAGE_GENERATOR_H_

#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <vector>
#include <tuple>

#include "triple.h"

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
        void draw_point(Triple color, int x, int y, int thickness);
        void draw_line(Triple color_a, Triple color_b, int x1, int y1, int x2, int y2, int thickness);
        void draw_rect(Triple color, int x1, int y1, int x2, int y2, int thickness, bool fill);
        void draw_circle(Triple color, int x_c, int y_c, int r, int thickness, bool fill);
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
