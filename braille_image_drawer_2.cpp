#include <iostream>
#include <cstdlib>
#include <ctime>
#include "helper_functions.h"
#include "system_info.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "MY_MACROS.hpp"
using namespace std;

int main(int argc, char* argv[]){
    //main(void) has 1 argument it's executable name ./exe_name
    //when file name passed, choose the last argument "$RANDOM_IMAGE"
    const char* file_path = (argc > 1) ? argv[1] : "/home/nischal/Pictures/terminal_pictures/psyduck.png";
    
    int img_w, img_h, channels;
    unsigned char* img_data = stbi_load(file_path, &img_w, &img_h, &channels, 3);
    
    if(!img_data){
      cerr << "Error loading image!" << endl;
      return 1;
    }
    
    int target_length = 90;
    int safe_target_h = target_length - 4;
    int safe_target_w = target_length - 2;
    int target_w =  safe_target_w, target_h = safe_target_h;
    Helper helper(img_data, img_w, img_h, target_length, target_length);

    //Load the system info 
    std::srand(std::time(nullptr));
    ThemeColor rand_color = static_cast<ThemeColor>(std::rand() % 4);
    vector<string> sys_metrics = SysInfo::get_metrics(rand_color);
    int metric_idx = 0;

    for(int y = 0; y < target_h; y+=4){  
       for(int x = 0; x < target_w; x+=2){
        char braille_utf8[4];
        helper.get_pattern(x, y, braille_utf8);
        RGB color = helper.get_avg_color();
        cout << set_color(color.r, color.g, color.b) << braille_utf8;
      }
      cout << "    ";
      //print the sys_metrics's elements one at a time 
      if(metric_idx < sys_metrics.size()){
        cout << sys_metrics[metric_idx];
        metric_idx++;
      }
      cout << "\n";
    }

    //if the number of metrics > lines in the image
    //print the remaining metrics
    while(metric_idx < sys_metrics.size()){
      for(int i = 0; i < target_w / 2; i++) 
        cout << " ";
      cout << "   " << sys_metrics[metric_idx++] << "\n";
    }
    cout << Color::reset() << Color::set_kitty_color(rand_color) << flush;
  stbi_image_free(img_data);
  return 0;
}
