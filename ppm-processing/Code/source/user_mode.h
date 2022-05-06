#ifndef USER_INTERACTION_H_
#define USER_INTERACTION_H_

#include <sstream>
#include <string>
#include <iostream>
#include <regex>

#include "image_generator_provider.h"

class UserMode
{
    public:
        UserMode(ImageGeneratorProvider &igp);
        ~UserMode();

        void enter_user_mode();
    
    private:
        ImageGeneratorProvider provider;

        int get_rgb(const std::string s);

};

#endif
