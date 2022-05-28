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
    ImageGenerator inv = ImageGenerator(src);
    ImageGenerator col = ImageGenerator(src);
    ImageGenerator gra = ImageGenerator(src);
    inv.apply_inverted_filter();
    col.apply_color_filter(Color(255,0,0));
    gra.apply_grayscale_filter();
    inv.save_image_to_file(IMAGE_README + "inverted.ppm");
    col.save_image_to_file(IMAGE_README + "colored.ppm");
    gra.save_image_to_file(IMAGE_README + "gray.ppm");
    
    // drawing
    Color bc = Color(122,0,255);
    Color col_a = Color(0,255,0);
    Color col_b = Color(50, 190, 20);
    Color col_c = Color(50,50,156);
    ImageGenerator ig = ImageGenerator();
    ig.generate_ppm_image(bc, 50, 50);
    ig.save_image_to_file(IMAGE_README + "filled.ppm");
    ig.draw_point(Color(0,255,0), 10, 25, 1);
    ig.draw_point(Color(0,255,0), 35, 25, 3);
    ig.save_image_to_file(IMAGE_README + "point.ppm");
    ig.fill_with_color(bc);
    ig.draw_line(col_a, col_c, 10, 10, 35, 10, 1);
    ig.draw_line(col_c, col_a, 10, 35, 35, 35, 3);
    ig.save_image_to_file(IMAGE_README + "lines.ppm");
    ig.fill_with_color(bc);
    ig.draw_rect(col_a, 10, 40, 20, 20, 1, false);
    ig.draw_rect(col_b, 30, 40, 40, 20, 1, true);
    ig.save_image_to_file(IMAGE_README + "rect.ppm");
    ig.fill_with_color(bc);
    ig.draw_circle(col_a, 13, 20, 10, 1, false);
    ig.draw_circle(col_b, 37, 20, 10, 1, true);
    ig.save_image_to_file(IMAGE_README + "circle.ppm");
}
