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
    ig.draw_triangle(col_a, 80, 240, 202, 300, 350, 200, 2, false);
    ig.draw_triangle(col_b, 65, 150, 330, 180, 280, 55, 1, true);
    ig.save_image_to_file(IMAGE_README + "triangles.ppm");
}

void create_research_images() {
    int height = 400;
    int width = 400;
    int transition_thickness = 5;

    Color col_a = Color(0,0,0);
    Color col_b = Color(255,255,255);

    string experiment_tag = "example_tag_";
    string sub_folder = "TryOut";
    string save_dir = sub_folder + experiment_tag;

    ImageGenerator image = ImageGenerator();
    image.generate_ppm_image(col_a, width, height);
    // horizontal split with and without transition region
    image.draw_rect(col_b, 0, image.get_height() / 2, image.get_width() - 1, 0, 1, true);
    image.save_image_to_file(IMAGE_RESEARCH + save_dir + "horizontal_no_transition.ppm");
    image.draw_line(col_a, col_b, 0, image.get_height() / 2, image.get_width() - 1, image.get_height() / 2, transition_thickness);
    image.save_image_to_file(IMAGE_RESEARCH + save_dir + "horizontal_transition.ppm");
    image.fill_with_color(col_a);
    // vertical split with and without transition region
    image.draw_rect(col_b, 0, 0, image.get_width() / 2, image.get_height() - 1, 1, true);
    image.save_image_to_file(IMAGE_RESEARCH + save_dir + "vertical_no_transition.ppm");
    image.draw_line(col_a, col_b, image.get_width() / 2, 0, image.get_width() / 2, image.get_height() - 1, transition_thickness);
    image.save_image_to_file(IMAGE_RESEARCH + save_dir + "vertical_transition.ppm");
    image.fill_with_color(col_a);
    // diagonal bl->tr split with and without transition region
    image.draw_triangle(col_b, 0, 0, image.get_width() - 1, 0, image.get_width() - 1, image.get_height() - 1, 1, true);
    image.save_image_to_file(IMAGE_RESEARCH + save_dir + "diagonal_bl_tr_no_transition.ppm");
    image.draw_line(col_a, col_b, 0, 0, image.get_width() - 1, image.get_height() - 1, transition_thickness);
    image.save_image_to_file(IMAGE_RESEARCH + save_dir + "diagonal_bl_tr_transition.ppm");
    image.fill_with_color(col_a);
    // diagonal tl->br split with and without transition region
    image.draw_triangle(col_b, 0, image.get_height() - 1, image.get_width() - 1, 0, image.get_width() - 1, image.get_height() - 1, 1, true);
    image.save_image_to_file(IMAGE_RESEARCH + save_dir + "diagonal_tl_br_no_transition.ppm");
    image.draw_line(col_a, col_b, 0, image.get_height() - 1, image.get_width() - 1, 0, transition_thickness);
    image.save_image_to_file(IMAGE_RESEARCH + save_dir + "diagonal_tl_br_transition.ppm");
    image.fill_with_color(col_a);
}
