#pragma once
#include <cstdint>

struct RGB {
  int r, g, b;
};

class Helper{
  private:
    unsigned char* img_data;
    int img_w, img_h, target_w, target_h;
    RGB active_pixels_colors[8];
    int count = 0;
    int min_brightness = 70;

  public:
    Helper(unsigned char* img_data, int img_w, int img_h, int target_w, int target_h)
      :img_data(img_data), img_w(img_w), img_h(img_h), target_w(target_w), target_h(target_h) {}

    void get_pattern(int x, int y, char* utf8){ //char* as an argument
                                                //cause we not gonna use malloc
      int pattern = 0;
      count = 0;  //make sure count = 0 before starting
      if(get_indv_brightness(x, y) > min_brightness){
        pattern |= 1;
        active_pixels_colors[count] = get_indv_color(x, y);
        count++;
      }
      if(get_indv_brightness(x, y+1) > min_brightness){
        pattern |= 2;
        active_pixels_colors[count] = get_indv_color(x, y+1);
        count++;
      }
      if(get_indv_brightness(x, y+2) > min_brightness){
        pattern |= 4;
        active_pixels_colors[count] = get_indv_color(x, y+2);
        count++;
      }
      if(get_indv_brightness(x, y+3) > min_brightness){
        pattern |= 64;
        active_pixels_colors[count] = get_indv_color(x, y+3);
        count++;
      }
      if(get_indv_brightness(x+1, y) > min_brightness){
        pattern |= 8;
        active_pixels_colors[count] = get_indv_color(x+1,y);
        count++;
      }
      if(get_indv_brightness(x+1, y+1) > min_brightness){
        pattern |= 16;
        active_pixels_colors[count] = get_indv_color(x+1, y+1);
        count++;
      }
      if(get_indv_brightness(x+1, y+2) > min_brightness){
        pattern |= 32;
        active_pixels_colors[count] = get_indv_color(x+1, y+2);
        count++;
      }
      if(get_indv_brightness(x+1, y+3) > min_brightness){
        pattern |= 128;
        active_pixels_colors[count] = get_indv_color(x+1,y+3);
        count++;
      }

      uint32_t unicode = 0x2800 + pattern;

      // char utf8[4];
      //conversion code
      //got no idea about this
      utf8[0] = (char)(0xE0 | ((unicode >> 12) & 0x0F));
      utf8[1] = (char)(0x80 | ((unicode >> 6) & 0x3F));
      utf8[2] = (char)(0x80 | (unicode & 0x3F));
      utf8[3] = '\0';

    }

    RGB get_indv_color(int x, int y){
      int img_x = x * img_w / target_w;
      int img_y = y * img_h / target_h;
      int pixel_num = (img_y * img_w + img_x) * 3;
      return {img_data[pixel_num], img_data[pixel_num+1], img_data[pixel_num+2]};
    }

    RGB get_avg_color(){  //must be used after get_pattern for a given (x, y)
                          //cause get_pattern sets its active_pixels_colors and count
      if(count == 0) return {0, 0, 0};

      int sum_r = 0, sum_g = 0, sum_b = 0; 
      
      for(int i = 0; i < count; i++){
        sum_r += active_pixels_colors[i].r;
        sum_g += active_pixels_colors[i].g;
        sum_b += active_pixels_colors[i].b;
      }
      return {sum_r/count, sum_g/count, sum_b/count};
    }

    int get_indv_brightness(int x, int y){
      int img_x = x * img_w / target_w;
      int img_y = y * img_h / target_h;
      int pixel_num = (img_y * img_w + img_x) * 3;
      return img_data[pixel_num]*0.299f + img_data[pixel_num+1]*0.587f + img_data[pixel_num+2]*0.114;
    }
};
