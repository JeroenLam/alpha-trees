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
    image.generate_ppm_image(Color(255,255,255), 10, 10);

    

    image.save_image_to_file(dest);
    

    return 0;
}
