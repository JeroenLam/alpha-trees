#include "image_generator.h"

using namespace std;

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}

bool d_eq(double a, double b) {
    return abs(a - b) < 0.0001;
}

Triple Barycentric(Point p, Point a, Point b, Point c) {
    Vector v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);
    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;
    return Triple(v,w,u);
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
    return Image;
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

void ImageGenerator::draw_point(Triple color, int x, int y, int thickness) {
    color.x = (int)color.x;
    color.y = (int)color.y;
    color.z = (int)color.z;
    if (thickness > 1)  {
        for (int i = -(thickness - 1); i < (thickness - 1); i++) {
            for (int j = -(thickness - 1); j < (thickness - 1); j++) {
                if (x + i >= 0 && x + i < Image_Width &&
                    y + j >= 0 && y + j < Image_Height
                ) {
                    Image[y + j][x + i] = Color(color);
                }
            }
        }
    } else if (
        x >= 0 && x < Image_Width &&
        y >= 0 && y < Image_Height
    ){
        Image[y][x] = Color(color);
    }
}

void ImageGenerator::draw_line(Triple color_a, Triple color_b, int x1, int y1, int x2, int y2, int thickness) {
    if (x1 == x2 and y1 == y2) {
        draw_point(color_a, x1, y1, thickness);
        return;
    }
    // ensures that (x1,y1) is left of (x2,y2)
    if (x1 > x2) {
        int temp = x1;
        x1 = x2;
        x2 = temp;
        temp = y1;
        y1 = y2;
        y2 = temp;
    }
    Color color = color_a;
    // drawing straight lines
    // vertical
    if (x1 == x2) {
        Color dc = (color_b - color_a) / abs(y1 - y2);
        if (y1 > y2) {
            int temp = y1;
            y1 = y2;
            y2 = temp;
        }
        int x = x1;
        for (int y = y1; y <= y2; y++) {
            draw_point(color, x, y, thickness);
            color += dc;
        }
        return;
    }
    Color dc = (color_b - color_a) / abs(x1 - x2);
    // horizontal
    if (y1 == y2) {
        int y = y1;
        for (int x = x1; x <= x2; x++) {
            draw_point(color, x, y, thickness);
            color += dc;
        }
        return;
    }
    double slope = (double)(y2 - y1) / (double)(x2 - x1);
    double intersect = y1 - slope * x1;
    // Midpoint line drawing algorithm
    // octant 1
    if (slope > 0.0 && slope <= 1.0) {
        int y = y1;
        double d = slope * (x1 + 1.0) + intersect - (y1 + 0.5);
        double a = y1 - y2;
        double b = x2 - x1;
        for (int x = x1; x <= x2; x++) {
            draw_point(color, x, y, thickness);
            color += dc;
            if (d < 0.0) {
                y++;
                d = d + a + b;
            } else {
                d = d + a;
            }
        }
        return;
    }
    // octant 2
    if (slope > 1.0) {
        int x = x1;
        double d = slope * (x1 + 0.5) + intersect - (y1 + 1.0);
        double b = y2 - y1;
        double a = x1 - x2;
        for (int y = y1; y <= y2; y++) {
            draw_point(color, x, y, thickness);
            color += dc;
            if (d < 0.0) {
                x++;
                d = d + a + b;
            } else {
                d = d + a;
            }
        }
        return;
    }
    // octant 3
    if (slope < -1.0) {
        int x = x1;
        double d = slope * (x1 + 0.5) + intersect - (y1 - 1.0);
        double b = y1 - y2;
        double a = x1 - x2;
        for (int y = y1; y >= y2; y--) {
            draw_point(color, x, y, thickness);
            color += dc;
            if (d < 0.0) {
                x++;
                d = d + a + b;
            } else {
                d = d + a;
            }
        }
        return;
    }
    // octant 4
    if (slope < 0.0 && slope >= -1.0) {
        int y = y1;
        double d = slope * (x1 + 1.0) + intersect - (y1 - 0.5);
        double a = y2 - y1;
        double b = x2 - x1;
        for (int x = x1; x <= x2; x++) {
            draw_point(color, x, y, thickness);
            color += dc;
            if (d < 0.0) {
                y--;
                d = d + a + b;
            } else {
                d = d + a;
            }
        }
        return;
    }
}

void ImageGenerator::draw_rect(Triple color, int x1, int y1, int x2, int y2, int thickness, bool fill){
    // if not filled draw lines between the points
    if (not fill) {
        draw_line(color, color, x1, y1, x2, y1, thickness);
        draw_line(color, color, x2, y1, x2, y2, thickness);
        draw_line(color, color, x2, y2, x1, y2, thickness);
        draw_line(color, color, x1, y2, x1, y1, thickness);
        return;
    }
    thickness = 1;
    // draw a line from left to right for each row of the rectangle
    if (y1 > y2) {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }
    for (size_t y = y1; y < y2; y++) {
        draw_line(color, color, x1, y, x2, y, thickness);
    }
}

void ImageGenerator::draw_circle(Triple color, int x_c, int y_c, int r, int thickness, bool fill) {
    draw_point(color, x_c, y_c + r, thickness);
    draw_point(color, x_c, y_c - r, thickness);
    draw_point(color, x_c - r, y_c, thickness);
    draw_point(color, x_c + r, y_c, thickness);
    if (fill) {
        draw_line(color, color, x_c, y_c + r, x_c, y_c - r, 1);
        thickness = 1;
    }
    int x = 0;
    int y = r;
    int d = 1 - r;
    while (x < y - 1) {
        if (d < 0) {
            d = d + (2 * x) + 3;
        } else {
            d = d + (2 * x) - (2 * y) + 5;
            y--;
        }
        x++;
        draw_point(color, x_c + x, y_c + y, thickness);
        draw_point(color, x_c - x, y_c + y, thickness);
        draw_point(color, x_c - x, y_c - y, thickness);
        draw_point(color, x_c + x, y_c - y, thickness);
        draw_point(color, x_c + y, y_c + x, thickness);
        draw_point(color, x_c + y, y_c - x, thickness);
        draw_point(color, x_c - y, y_c + x, thickness);
        draw_point(color, x_c - y, y_c - x, thickness);
        if (fill) {
            draw_line(color, color, x_c + x, y_c + y, x_c + x, y_c - y, 1);
            draw_line(color, color, x_c - x, y_c + y, x_c - x, y_c - y, 1);
            draw_line(color, color, x_c + y, y_c + x, x_c + y, y_c - x, 1);
            draw_line(color, color, x_c - y, y_c + x, x_c - y, y_c - x, 1);
        }
    }
}

void ImageGenerator::draw_triangle(Triple color, int x1, int y1, int x2, int y2, int x3, int y3, int thickness, bool fill) {
    draw_line(color, color, x1, y1, x2, y2, thickness);
    draw_line(color, color, x1, y1, x3, y3, thickness);
    draw_line(color, color, x3, y3, x2, y2, thickness);
    if (not fill) return;
    thickness = 1;
    int max_x = max(x1, max(x2, x3));
    int max_y = max(y1, max(y2, y3));
    int min_x = min(x1, min(x2, x3));
    int min_y = min(y1, min(y2, y3));
    for (size_t x = min_x; x <= max_x; x++) {
        for (size_t y = min_y; y <= max_y; y++) {
            Triple bar_coor = Barycentric(
                Point(x, y, 0),
                Point(x1, y1, 0),
                Point(x2, y2, 0),
                Point(x3, y3, 0)
            );
            if (bar_coor.x >= 0.0 and bar_coor.y >= 0.0 and bar_coor.z >= 0.0) {
                draw_point(color, x, y, 1);
            }
        }
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
        reverse(Image.begin(), Image.end());

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

        reverse(Image.begin(), Image.end());
        for (size_t i = 0; i < Image_Height; i++) {
            for (size_t j = 0; j < Image_Width; j++) {
                Color curr_col = Image[i][j];
                image << curr_col.r << " " << curr_col.g << " " << curr_col.b << " ";
            }
            image << endl;
        }
        reverse(Image.begin(), Image.end());
        
    } else {
        cout << "Error while trying to write to file" << fname << endl;
        exit(1);
    }

    image.close();
}
