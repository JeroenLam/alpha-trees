#include "image_generator.h"

using namespace std;

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}

// ---------------------------- Constructors ----------------------------
ImageGenerator::ImageGenerator() {
    Image_Height = 0;
    Image_Width = 0;
    RGB_Max = 0;
    PPM_Type = "";
}

ImageGenerator::ImageGenerator(string fname) {
    read_image_from_file(fname);
}

// ---------------------------- Getters & Setters ----------------------------
Matrix ImageGenerator::get_image() {
    vector< vector<Triple> > copy(Image);
    return copy;
}

int ImageGenerator::get_height() {
    return Image_Height;
}

int ImageGenerator::get_width() {
    return Image_Width;
}

int ImageGenerator::get_rgb_max() {
    return RGB_Max;
}

string ImageGenerator::get_type() {
    return PPM_Type;
}

// ---------------------------- util ----------------------------
void ImageGenerator::print_image() {
    for (size_t i = 0; i < Image_Height; i++) {
        for (size_t j = 0; j < Image_Width; j++) { 
            cout << Image[i][j];
        }
        cout << endl;
    }
}

void ImageGenerator::generate_ppm_image(Triple color, int width, int height) {

    // place header info
    PPM_Type = "P3";
    Image_Width = width;
    Image_Height = height;
    RGB_Max = 255;

    Image.clear();
    for (int y = 0; y < Image_Height; y++) {
        Row curr_row;
        for (int x = 0; x < Image_Width; x++) {
            Color c = Triple(color);
            curr_row.push_back(c);
        }
        Image.push_back(curr_row);
    }
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

// ---------------------------- Image Filters ----------------------------

void ImageGenerator::apply_grayscale_filter() {
    for (int y = 0; y < Image_Height; y++) {
        for (int x = 0; x < Image_Width; x++) {
            Triple src_color = Image[y][x];
            int mean_val = (src_color.r + src_color.g + src_color.b) / 3;
            Image[y][x] = Color(mean_val, mean_val, mean_val);
        }
    }
}

void ImageGenerator::apply_color_filter(Triple filter) {
    int r = 0, g = 0, b = 0;
    for (int y = 0; y < Image_Height; y++) {
        for (int x = 0; x < Image_Width; x++) {
            Triple src_color = Image[y][x];
            r = max(0, min(255, src_color.r + filter.r));
            g = max(0, min(255, src_color.g + filter.g));
            b = max(0, min(255, src_color.b + filter.b));
            Image[y][x] = Color(r, g, b);
        }
    }
}

void ImageGenerator::apply_inverted_filter() {
    int r = 0, g = 0, b = 0;
    for (int y = 0; y < Image_Height; y++) {
        for (int x = 0; x < Image_Width; x++) {
            Triple src_color = Image[y][x];
            r = max(0, (255 - src_color.r));
            g = max(0, (255 - src_color.g));
            b = max(0, (255 - src_color.b));
            Image[y][x] = Color(r, g, b);
        }
    }
}

// ---------------------------- Graphics Operations ----------------------------

void ImageGenerator::fill_with_color(Triple color) {
    for (int y = 0; y < Image_Height; y++) {
        for (int x = 0; x < Image_Width; x++) {
            Image[y][x] = Color(color);
        }
    }
}

void ImageGenerator::draw_line(Triple color, int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x1 > Image_Width ||
        x2 < 0 || x2 > Image_Width ||
        y1 < 0 || y1 > Image_Height ||
        y2 < 0 || y2 > Image_Height
    ) {
        cout << "One or both points of the line exceed image dimensions. Aborting line drawing!" << endl;
        return;
    }
}

// ---------------------------- IO Operations ----------------------------

void ImageGenerator::read_image_from_file(string fname) {
    ifstream image;

    image.open(fname);

    if (image.is_open()) {
        string type = "", width = "", height = "", RGB = "";
        image >> type;
        image >> width;
        image >> height;
        image >> RGB;

        stringstream height_stream(height);
        stringstream width_stream(width);
        stringstream rgb_stream(RGB);

        height_stream >> Image_Height;
        width_stream >> Image_Width;
        rgb_stream >> RGB_Max;
        PPM_Type = type;

        Image.clear();
        for (size_t i = 0; i < Image_Height; i++) {
            Row curr_row;
            for (size_t j = 0; j < Image_Width; j++) {
                Triple next_col = get_next_rgb(image);
                curr_row.push_back(next_col);
            }
            Image.push_back(curr_row);
        }

    } else {
        printf("Error while loading PPM Image\n");
        exit(1);
    }

    image.close();
}

void ImageGenerator::save_image_to_file(std::string fname) {
    ofstream image;

    image.open(fname);

    if (image.is_open()) {
        image << PPM_Type << endl;
        image << Image_Width << " " << Image_Height << endl;
        image << RGB_Max << endl;

        for (size_t i = 0; i < Image_Height; i++) {
            for (size_t j = 0; j < Image_Width; j++) {
                Color curr_col = Image[i][j];
                image << curr_col.r << " " << curr_col.g << " " << curr_col.b << " ";
            }
            image << endl;
        }
        
    } else {
        cout << "Error while trying to write to file" << fname << endl;
        exit(1);
    }

    image.close();
}
