#include "user_mode.h"

UserMode::UserMode(ImageGeneratorProvider &igp) {
    provider = igp;
}

UserMode::~UserMode() {
    printf("Deconstructor UserMode called\n");
    provider.close();
}

int UserMode::get_rgb(const std::string s) {
    if (s.length() > 3) return -1;

    for (int i = 0; i < s.length(); i++) {
        if (!isdigit(s[i])) return -1;
    }
    
    std::stringstream rgbstream(s);
    int rgb = 0;
    rgbstream >> rgb;
    if (rgb < 0 || rgb > 255) {
        return -1;
    }
    return rgb;
}


void UserMode::enter_user_mode() {
    const std::regex file_path_regex("(([A-Za-z0-9]+|..)/)*([A-Za-z0-9]+).ppm");
    const std::regex filter_choice_regex("[1-2]");

    std::string input_line = "";
    std::string src_path = "";
    std::string dst_path = "";
    int filter_choice = 0;
    int r = -1, g = -1, b = -1;

    
    do {
        std::cout << "Enter path to ppm file that should be edited:" << std::endl;
        std::getline(std::cin, input_line);
    } while (!regex_match(input_line, file_path_regex));
    src_path = input_line;

    do {
        std::cout << "Enter path to save file:" << std::endl;
        std::getline(std::cin, input_line);
    } while (!regex_match(input_line, file_path_regex));
    dst_path = input_line;

    do {
        std::cout << "What filter should be used:" << std::endl;
        std::cout << "1: Color Filter" << std::endl;
        std::cout << "2: Inversion Filter" << std::endl;
        std::getline(std::cin, input_line);
    } while (!regex_match(input_line, filter_choice_regex));
    filter_choice = std::stoi(input_line);

    switch (filter_choice) {
    case 1:
        r = -1;
        g = -1; 
        b = -1;
        while (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            printf("Red Channel:\n");
            scanf("%d", &r);
            printf("Greem Channel:\n");
            scanf("%d", &g);
            printf("Blue Channel:\n");
            scanf("%d", &b);
        }
        std::cout << "Processing..." << std::endl;
        provider.get_image_generator().apply_color_filter(src_path, dst_path, Triple(r, g, b));
        std::cout << "Color filter (" << r << "," << g << "," << b << ") applied" << std::endl;
        std::cout << "File stored at:" << dst_path << std::endl;
        break;
    case 2:
        std::cout << "Processing..." << std::endl;
        provider.get_image_generator().apply_inverted_filter(src_path, dst_path);
        std::cout << "Inversion filter applied" << std::endl;
        std::cout << "File stored at:" << dst_path << std::endl;
        break;
    default:
        std::cout << "Invalid filter! Abort!" << std::endl;
        break;
    }

    return;
}
