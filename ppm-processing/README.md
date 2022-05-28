# PPM Processor
This Program can be used to load and save _.ppm_ images. A multiple of image filters can be applied to the images. Furthermore the program can create _.ppm_ images from scratch and allow the user to draw a selection of geometry onto the image. The user can draw points, lines, rectangles and circles onto the image.
## Quick Start
To run make sure that cmake is installed on the machine.
</br>
In _ppm-processing/_

```
mkdir build
cd build
cmake ..
make
./image_processor
```

# How to use
## Load and Save Images
To create an image use the `ImageGenerator` class. The class has two constructors:
```
// creates an empty image
ImageGenerator image_generator = ImageGenerator();
// loads an image from a ppm file given the path
ImageGenerator image_generator = ImageGenerator(std::string <path_to_file>);
```
Optionally a _.ppm_ image can be loaded into the image generator using the following code:
```
image_generator.read_image_from_file(std::string <path_to_file>);
```
To save the edited image to a _.ppm_ file use the following code:
```
image_generator.save_image_to_file(std::string <path_to_file>);
```
## Data Representation
The images are stored as a two dimensional `std::vector< std::vector<Triple> >` where a `Triple` is a data structure storing three integer values. These are used to represent the color channel values of each of the pixels in the image. As we are using vectors to store the pixels we can access the pixels of an image via the following code:
```
Triple c = image_generator.get_image()[y][x];
```
In this example `c` is the pixel in row y and column x, where row 0 is at the bottom of the image and column 0 is at the left of the image.</br>
To loop through every pixel of the image use the following code:
```
for (size_t i = 0; i < image_generator.get_height(); i++) {
    for (size_t j = 0; j < image_generator.get_width(); j++) { 
        Triple pix = image_generator.get_image()[i][j];
    }
}
```
Note that `image_generator.get_image()` returns the actual vector storing the image and the image can be edited via this getter function.
## Image Filters
The following filters can be applied to a loaded _.ppm_ images. The examples have been applied to this original picture:</br>
![Original](Images/ReadmeImages/bird.ppm?raw=true)
### Color Filter
This filter adds a color to every pixel in the image functionally applying a color filter to the entire image.
```
image_generator.apply_color_filter(Triple filter);
```
![Color Filter](Images/ReadmeImages/colored.ppm?raw=true)
### Inverted Filter
This filter inverts the color of every pixel in the image.
```
image_generator.apply_inverted_filter(Triple filter);
```
![Inverted Filter](Images/ReadmeImages/inverted.ppm?raw=true)
### Grayscale Filter
This filter averages the three color channels of every pixel in the image. This turns the image into a grayscale version of the image.
```
image_generator.apply_grayscale_filter(Triple filter);
```
![Inverted Filter](Images/ReadmeImages/gray.ppm?raw=true)
## Geometry Drawing
Note that for any geometry providing points outside of the image dimensions will increase computation time as the program will compute points that are not displayed to the user. All geometry displayed however, fits the provided arguments.
### Background
The whole image can be filled with a solid color using the following code:
```
image_generator.fill_with_color(Triple color);
```
![Filled](Images/ReadmeImages/filled.ppm?raw=true)
### Drawing a Point
To draw a point onto the image use the following code:
```
image_generator.draw_point(Triple color, int x, int y, int thickness);
```
The `x` and `y` parameters determine the position of the point, while the `thickness` parameter determines the width of the point.</br>
![Points](Images/ReadmeImages/point.ppm?raw=true)
### Drawing a Line
To draw a line onto the image use the following code:
```
image_generator.draw_line(Triple color_a, Triple color_b, int x1, int y1, int x2, int y2, int thickness);
```
The `x1`,`y1`,`x2` and `y2` parameters determine the position of the points that the line connects, while the `thickness` parameter determines the width of the point. The two colors `color_a` and `color_b` are the colors of the line at positions `(x1,y1)` and `(x2,y2)` respectively. The color of the line is interpolated between the two given colors. For monochrome lines provide the same color for both arguments.</br>
![Lines](Images/ReadmeImages/lines.ppm?raw=true)
### Drawing a Rectangle
To draw a rectangle onto the image use the following code:
```
image_generator.draw_rect(Triple color, int x1, int y1, int x2, int y2, int thickness, bool fill);
```
The `x1`,`y1`,`x2` and `y2` parameters determine the position of the points defining the two opposing edge points of the rectangle, while the `thickness` parameter determines the width of the edge of the rectangle. The `fill` parameter determines if the rectangle is filled (true) or empty (false).</br>
![Rectangles](Images/ReadmeImages/rect.ppm?raw=true)
### Drawing a Circle
To draw a circle onto the image use the following code:
```
image_generator.draw_circle(Triple color, int x_c, int y_c, int r, int thickness, bool fill);
```
The `x_c` and `y_c` define the center of the circle while `r` is the circle's radius. The `thickness` parameter determines the width of the edge of the circle. The `fill` parameter determines if the circle is filled (true) or empty (false).</br>
![Circles](Images/ReadmeImages/circle.ppm?raw=true)