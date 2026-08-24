#ifndef MY_MACROS
#define MY_MACROS
#define set_color(x, y, z) "\033[38;2;" << x << ";" << y << ";" << z << "m"
#define reset_color "\033[0m\n" 
#endif
