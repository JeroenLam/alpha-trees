#include "source/image_generator.h"
#include "source/paths.h"
#include "source/create_images.h"

using namespace std;

int main(int argc, char const *argv[]) {

    // create_readme_images();
    // create_research_images();
    // create_color_pairs();
    // ImageGenerator ig = ImageGenerator();
    // ig.generate_ppm_image(Triple(0,255,255), 100, 100);
    // ig.save_image_to_file(IMAGE_OUTPUT + "test.ppm");
    create_brightness_boost_images();

    return 0;
}