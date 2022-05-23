#ifndef USER_INTERACTION_H_
#define USER_INTERACTION_H_

#include <sstream>
#include <string>
#include <iostream>
#include <regex>

class UserMode
{
    public:
        UserMode();
        ~UserMode();

        void enter_user_mode();
    
    private:

        int get_rgb(const std::string s);

};

#endif
