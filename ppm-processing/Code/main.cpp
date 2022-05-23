#include "source/image_generator.h"
#include "source/paths.h"
#include "source/user_mode.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>

using namespace std;

int main(int argc, char const *argv[]) {
    if (argc > 1 && !strcmp("-u", argv[1])) {
        UserMode user_mode = UserMode();
        user_mode.enter_user_mode();
        return 0;
    }


    string src = IMAGE_EXAMPLES + "bird.ppm";
    string dest = IMAGE_OUTPUT + "output.ppm";

    ImageGenerator image = ImageGenerator();
    image.generate_ppm_image(Color(0,255,255), 100, 100);

    int line_thickness = 1;
    int line[4] = {
        14,
        25,
        67,
        12
    };
    // int line[4] = {
    //     0,
    //     9,
    //     4,
    //     0
    // };
    image.draw_line(Color(0,0,0), line[0], line[1], line[2], line[3], line_thickness);

    image.draw_line(Color(0,0,0), line[2], line[1], line[3], line[0], line_thickness);

    image.save_image_to_file(dest);
    

    return 0;
}
