#include "image_generator.h"

using namespace std;

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}

ImageGenerator::ImageGenerator() {}

void ImageGenerator::generate_ppm_image(string fname, InitType mode, int width, int height) {
    ofstream image;

    image.open(fname);

    srand(time(0));

    if (image.is_open()) {
        // place header info
        image << "P3" << endl;
        image << width << " " << height << endl;
        image << "255" << endl;

        int slope = height / width;
        int rgb_x = 0, rgb_y = 0, rgb_z = 0;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Color c;
                switch (mode) {
                case RANDOM:
                    c = Triple(rand() % 255, rand() % 255, rand() % 255);
                    break;
                case BLACK:
                    c = Triple(0, 0, 0);
                    break;
                case RGB_SPLIT:
                    c = rgb_split_color(x, y, slope, height);
                    break;
                case RGB_SPECTRUM:
                    c = rgb_spectrum_color(rgb_x, rgb_y, rgb_z);
                    if (rgb_y >= 255 == 0) {
                        rgb_x += 10;
                        rgb_y = 0;
                    } else {
                        rgb_y += 10;
                    }
                    if (rgb_x >= 255 == 0) {
                        rgb_z += 10;
                        rgb_x = 0;
                    } else {
                        rgb_x += 10;
                    }
                    if (rgb_z >= 255) {
                        rgb_z = 0;
                    }
                    break;
                case WHITE:
                default:
                    c = Triple(255, 255, 255);
                    break;
                }

                image << c.r << " " << c.g << " " << c.b << endl;
            }
        }
    }
    image.close();
}

void ImageGenerator::generate_ppm_horizontal_split(std::string fname, int split, int width, int height, int transition, Triple color1, Triple color2) {
    ofstream image;

    image.open(fname);


    if (image.is_open()) {
        // place header info
        image << "P3" << endl;
        image << width << " " << height << endl;
        image << "255" << endl;

        int rgb_x = 0, rgb_y = 0, rgb_z = 0;
        Color c;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                c = (x <= split) ? color1 : color2;
                if (transition && x == split) {
                    c = (color2) + y * ((color2 - color1) / width);
                }
                image << c.r << " " << c.g << " " << c.b << endl;
            }
        }
    }
    image.close();
}

void ImageGenerator::apply_grayscale_filter(string src, string dest) {
    ifstream src_img;
    ofstream dst_img;

    src_img.open(src);
    dst_img.open(dest);

    copy_ppm_header(src_img, dst_img);
    
    Color src_color;
    int r = 0, g = 0, b = 0;    
    
    while (!src_img.eof()) {
        src_color = get_next_rgb(src_img);    

        // apply filter
        int mean_val = (src_color.r + src_color.g + src_color.b) / 3;
        printf("MEAN: %d\n", mean_val);
        r = mean_val;
        g = mean_val;
        b = mean_val;

        // write to destination
        dst_img << r << " " << g << " " << b << endl;
    }
    src_img.close();
    dst_img.close();
}

void ImageGenerator::apply_color_filter(string src, string dest, Triple filter) {
    ifstream src_img;
    ofstream dst_img;

    src_img.open(src);
    dst_img.open(dest);

    copy_ppm_header(src_img, dst_img);
    
    Color src_color;
    int r = 0, g = 0, b = 0;    
    
    while (!src_img.eof()) {
        src_color = get_next_rgb(src_img);    

        // apply filter
        r = max(0, min(255, src_color.r + filter.r));
        g = max(0, min(255, src_color.g + filter.g));
        b = max(0, min(255, src_color.b + filter.b));

        // write to destination
        dst_img << r << " " << g << " " << b << endl;
    }
    src_img.close();
    dst_img.close();
}

void ImageGenerator::apply_inverted_filter(string src, string dest) {
    ifstream src_img;
    ofstream dst_img;

    src_img.open(src);
    dst_img.open(dest);

    copy_ppm_header(src_img, dst_img);
    
    Color src_color;
    int r = 0, g = 0, b = 0;    
    
    while (!src_img.eof()) {
        src_color = get_next_rgb(src_img);    

        // invert color
        r = max(0, (255 - src_color.r));
        g = max(0, (255 - src_color.g));
        b = max(0, (255 - src_color.b));

        // write to destination
        dst_img << r << " " << g << " " << b << endl;
    }
    src_img.close();
    dst_img.close();
}

void ImageGenerator::copy_ppm_header(ifstream &src, ofstream &dest) {
    string type = "", width = "", height = "", RGB = "";
    src >> type;
    src >> width;
    src >> height;
    src >> RGB;

    dest << type << endl;
    dest << width << " " << height << endl;
    dest << RGB << endl;
}

Triple ImageGenerator::get_next_rgb(ifstream &src) {
    string red = "", green = "", blue = "";
    int r = 0, g = 0, b = 0;
    // read from source
    src >> red;
    src >> green;
    src >> blue;
    
    // convert to integer
    stringstream r_stream(red);
    stringstream g_stream(green);
    stringstream b_stream(blue);
    
    r_stream >> r;
    g_stream >> g;
    b_stream >> b;

    return Triple(r, g, b);
}

Triple ImageGenerator::rgb_split_color(int x, int y, int slope, int height) {
    // diagonals across the square
    // bottom left to top right
    if (y == -slope * x + height) {
        return Triple(0, 0, 0); 
    }
    // top left to bottom right
    if (y == slope * x) {
        return Triple(0, 0, 0);
    }
    // fill in left triangle
    if ((y > slope * x) && (y < -slope * x + height)) {
        return Triple(255, 0, 0);
    }
    // fill in the right triangle
    if ((y < slope * x) && (y > -slope * x + height)) {
        return Triple(0, 0, 255);
    }
    // fill in the top triangle
    if (x > y) {
        return Triple(0, 255, 0);
    }
    // fill in the bottom triangle
    if (x < y) {
        return Triple(150, 94, 19);
    }
    return Triple(255, 255, 255);
}

Triple ImageGenerator::rgb_spectrum_color(int x, int y, int z) {
    int red = 0, green = 0, blue = 0;
    // x runs over the g channel of the RGB cube
    // y runs over the r channel of the RGB cube
    // z runs over the b channel of the RGB cube
    red = x % 255;
    green = y % 255;
    blue = z % 255;
    return Triple(red, green, blue);
}
