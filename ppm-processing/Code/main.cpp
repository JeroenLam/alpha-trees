#include "source/image_generator_provider.h"
#include "source/paths.h"
#include "source/user_mode.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>

using namespace std;
    
ImageGeneratorProvider provider = ImageGeneratorProvider();

int main(int argc, char const *argv[]) {
    if (argc > 1 && !strcmp("-u", argv[1])) {
        UserMode user_mode = UserMode(provider);
        user_mode.enter_user_mode();
        return 0;
    }

    string src = IMAGE_EXAMPLES + "landscape.ppm";
    string dest = IMAGE_OUTPUT + "gray.ppm";

    cout << "processing..." << endl;
    provider.get_image_generator().apply_grayscale_filter(src, dest);
    // provider.get_image_generator().generate_ppm_horizontal_split(dest, 50, 100, 100, 1, Triple(0,0,0), Triple(255,255,255));
    cout << "image created" << endl;
    // provider.get_image_generator().apply_inverted_filter(src, dest);
    // cout << "filter applied" << endl;

    provider.close();

    return 0;
}
