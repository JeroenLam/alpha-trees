#ifndef IMAGE_GENERATOR_PROVIDER_H_
#define IMAGE_GENERATOR_PROVIDER_H_

#include "image_generator.h"

class ImageGeneratorProvider
{
    public:
        ImageGeneratorProvider() {};
        ~ImageGeneratorProvider() {};
        
        ImageGenerator get_image_generator() {
            if (image_generator == NULL) {
                image_generator = new ImageGenerator();
            }
            return *image_generator;
        }

        void close() {
            if (image_generator != NULL) {
                delete(image_generator);
            }
        }

    private:
        ImageGenerator *image_generator = NULL;
};

#endif
