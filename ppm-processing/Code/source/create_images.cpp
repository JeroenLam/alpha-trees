#include "image_generator.h"
#include "paths.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>

using namespace std;

void create_readme_images() {
    string src = IMAGE_EXAMPLES + "bird.ppm";
    string dest = IMAGE_README + "inv.ppm";

    // color filters
    // ImageGenerator inv = ImageGenerator(src);
    // ImageGenerator col = ImageGenerator(src);
    // ImageGenerator gra = ImageGenerator(src);
    // inv.apply_inverted_filter();
    // col.apply_color_filter(Color(255,0,0));
    // gra.apply_grayscale_filter();
    // inv.save_image_to_file(IMAGE_README + "inverted.ppm");
    // col.save_image_to_file(IMAGE_README + "colored.ppm");
    // gra.save_image_to_file(IMAGE_README + "gray.ppm");
    
    // drawing
    Color bc = Color(0,0,255);
    Color col_a = Color(255, 165, 0);
    Color col_b = Color(200, 100, 0);
    Color col_c = Color(150,25,125);
    ImageGenerator ig = ImageGenerator();
    ig.generate_ppm_image(bc, 400, 400);
    ig.save_image_to_file(IMAGE_README + "filled.ppm");
    ig.draw_point(col_a, 100, 200, 5);
    ig.draw_point(col_b, 300, 200, 15);
    ig.save_image_to_file(IMAGE_README + "point.ppm");
    ig.fill_with_color(bc);
    ig.draw_line(col_a, col_c, 100, 100, 300, 100, 1);
    ig.draw_line(col_c, col_a, 100, 300, 300, 300, 5);
    ig.save_image_to_file(IMAGE_README + "lines.ppm");
    ig.fill_with_color(bc);
    ig.draw_rect(col_a, 100, 350, 200, 200, 2, false);
    ig.draw_rect(col_b, 280, 350, 350, 280, 1, true);
    ig.save_image_to_file(IMAGE_README + "rect.ppm");
    ig.fill_with_color(bc);
    ig.draw_circle(col_a, 80, 200, 50, 2, false);
    ig.draw_circle(col_b, 250, 200, 50, 1, true);
    ig.save_image_to_file(IMAGE_README + "circle.ppm");
}
