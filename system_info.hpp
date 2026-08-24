#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <filesystem>

namespace fs = std::filesystem;

enum class ThemeColor { RED, GREEN, BLUE, YELLOW };

struct Color {
    static std::string get(ThemeColor theme) {
        switch (theme) {
            case ThemeColor::RED:    return "\033[1;31m";
            case ThemeColor::GREEN:  return "\033[1;32m";
            case ThemeColor::BLUE:   return "\033[38;2;94;241;255m";
            case ThemeColor::YELLOW: return "\033[1;33m";
        }
        return "\033[1;31m";
    }

    //set kitty's active window's default color permanently
    static std::string set_kitty_color(ThemeColor theme){
      switch(theme){
        case ThemeColor::RED: return "\033]10;red\007";
        case ThemeColor::GREEN: return "\033]10;green\007";
        case ThemeColor::BLUE: return "\033]10;#5ef1ff\007";
        case ThemeColor::YELLOW: return "\033]10;yellow\007";
      }
      return "\033]10;yellow\007";
    }


    static std::string reset()   { return "\033[0m"; }
    static std::string dim()     { return "\033[2m"; }
    static std::string accent()  { return "\033[1;37m"; }
    static std::string green()   { return "\033[1;32m"; }
};

struct SysInfo {
    // Helper to execute small shell commands and return stdout as a string
    static std::string exec(const char* cmd) {
        char buffer[128];
        std::string result = "";
        FILE* pipe = popen(cmd, "r");
        if (!pipe) return "";
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);
        if (!result.empty() && result.back() == '\n') result.pop_back();
        return result;
    }

    static std::vector<std::string> get_metrics(ThemeColor theme) {
        std::vector<std::string> lines;

        std::string primary = Color::get(theme);
        std::string reset   = Color::reset();
        std::string dim     = Color::dim();
        std::string accent  = Color::accent();
        std::string green   = Color::green();

        // 1. User & Host
        char host[256];
        gethostname(host, sizeof(host));
        std::string user = getenv("USER") ? getenv("USER") : "user";
        lines.push_back(primary + user + accent + "@" + primary + host + reset);

        std::string plain_host = user + "@" + host;
        lines.push_back(dim + std::string(plain_host.length(), '-') + reset);

        // 2. OS
        std::ifstream os_file("/etc/os-release");
        std::string line, os_name = "Linux";
        while (std::getline(os_file, line)) {
            if (line.find("PRETTY_NAME=") == 0) {
                os_name = line.substr(13, line.length() - 14);
                break;
            }
        }
        lines.push_back(primary + "OS: " + reset + accent + os_name + reset);

        // 3. Host Model
        std::ifstream host_file("/sys/devices/virtual/dmi/id/product_name");
        std::string host_model = "Latitude 7400";
        if (std::getline(host_file, line) && !line.empty()) host_model = line;
        lines.push_back(primary + "Host: " + reset + accent + host_model + reset);

        // 4. Kernel
        struct utsname uts;
        uname(&uts);
        lines.push_back(primary + "Kernel: " + reset + accent + std::string(uts.release) + reset);

        // 5. Uptime
        std::ifstream uptime_file("/proc/uptime");
        double total_seconds = 0;
        if (uptime_file >> total_seconds) {
            int days = total_seconds / 86400;
            int hours = ((int)total_seconds % 86400) / 3600;
            int mins = ((int)total_seconds % 3600) / 60;
            std::string up_str = "";
            if (days > 0) up_str += std::to_string(days) + "d, ";
            if (hours > 0) up_str += std::to_string(hours) + " hours, ";
            up_str += std::to_string(mins) + " mins";
            lines.push_back(primary + "Uptime: " + reset + accent + up_str + reset);
        }

        // 6. Packages & Snaps
        std::string dpkg_cnt = exec("dpkg-query -f '.\n' -W 2>/dev/null | wc -l");
        std::string snap_cnt = exec("snap list 2>/dev/null | tail -n +2 | wc -l");
        if (dpkg_cnt.empty()) dpkg_cnt = "0";
        if (snap_cnt.empty()) snap_cnt = "0";
        lines.push_back(primary + "Packages: " + reset + accent + dpkg_cnt + " (dpkg), " + snap_cnt + " (snap)" + reset);

        // 7. Shell
        std::string shell = getenv("SHELL") ? getenv("SHELL") : "";
        size_t slash = shell.find_last_of('/');
        if (slash != std::string::npos) shell = shell.substr(slash + 1);
        std::string shell_ver = exec((shell + " --version 2>/dev/null | head -n1 | awk '{print $4}'").c_str());
        lines.push_back(primary + "Shell: " + reset + accent + shell + " " + shell_ver + reset);

        // 8. Window Manager (Hyprland / Desktop)
        std::string wm = getenv("HYPRLAND_INSTANCE_SIGNATURE") ? "Hyprland" : 
                        (getenv("XDG_CURRENT_DESKTOP") ? getenv("XDG_CURRENT_DESKTOP") : "Wayland");
        std::string wm_ver = (wm == "Hyprland") ? exec("hyprctl version 2>/dev/null | grep -i 'Tag' | awk '{print $2}'") : "";
        lines.push_back(primary + "WM: " + reset + accent + wm + (wm_ver.empty() ? "" : " " + wm_ver) + reset);

        // 9. Terminal & Terminal Font
        std::string term = getenv("TERM_PROGRAM") ? getenv("TERM_PROGRAM") : (getenv("TERM") ? getenv("TERM") : "kitty");
        lines.push_back(primary + "Terminal: " + reset + accent + term + reset);

        // Parse Kitty config for font name if using Kitty
        std::string font_name = "Iosevka (12pt)";
        std::string kitty_conf = std::string(getenv("HOME") ? getenv("HOME") : "") + "/.config/kitty/kitty.conf";
        std::ifstream kfile(kitty_conf);
        while (std::getline(kfile, line)) {
            if (line.find("font_family") == 0) {
                font_name = line.substr(11);
                size_t first = font_name.find_first_not_of(" \t");
                if (first != std::string::npos) font_name = font_name.substr(first);
                break;
            }
        }
        lines.push_back(primary + "Terminal Font: " + reset + accent + font_name + reset);

        // 10. CPU (/proc/cpuinfo)
        std::ifstream cpu_file("/proc/cpuinfo");
        std::string cpu_model = "Intel Core i7-8665U";
        int cpu_cores = 0;
        while (std::getline(cpu_file, line)) {
            if (line.find("model name") == 0) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) cpu_model = line.substr(colon + 2);
            }
            if (line.find("processor") == 0) cpu_cores++;
        }
        lines.push_back(primary + "CPU: " + reset + accent + cpu_model + " (" + std::to_string(cpu_cores) + ")" + reset);

        // 11. GPU
        std::string gpu = exec("lspci 2>/dev/null | grep -i 'vga\\|3d\\|display' | cut -d ':' -f3 | xargs");
        if (gpu.empty()) gpu = "Intel UHD Graphics 620";
        lines.push_back(primary + "GPU: " + reset + accent + gpu + reset);

        // 12. Memory
        std::ifstream mem_file("/proc/meminfo");
        long total_mem = 0, avail_mem = 0;
        while (std::getline(mem_file, line)) {
            if (line.find("MemTotal:") == 0) sscanf(line.c_str(), "MemTotal: %ld kB", &total_mem);
            if (line.find("MemAvailable:") == 0) sscanf(line.c_str(), "MemAvailable: %ld kB", &avail_mem);
        }
        if (total_mem > 0) {
            long used_mem = total_mem - avail_mem;
            double used_gb = used_mem / 1024.0 / 1024.0;
            double total_gb = total_mem / 1024.0 / 1024.0;
            int mem_pct = (int)((double)used_mem / total_mem * 100);
            char mem_str[128];
            snprintf(mem_str, sizeof(mem_str), "%.2f GiB / %.2f GiB (%s%d%%%s)", 
                     used_gb, total_gb, green.c_str(), mem_pct, accent.c_str());
            lines.push_back(primary + "Memory: " + reset + accent + std::string(mem_str) + reset);
        }

        // 13. Disk Space (statvfs for /)
        struct statvfs stat;
        if (statvfs("/", &stat) == 0) {
            double total_disk = (double)stat.f_blocks * stat.f_frsize / (1024 * 1024 * 1024);
            double free_disk = (double)stat.f_bfree * stat.f_frsize / (1024 * 1024 * 1024);
            double used_disk = total_disk - free_disk;
            int disk_pct = (int)(used_disk / total_disk * 100);
            char disk_str[128];
            snprintf(disk_str, sizeof(disk_str), "%.2f GiB / %.2f GiB (%s%d%%%s) - ext4", 
                     used_disk, total_disk, green.c_str(), disk_pct, accent.c_str());
            lines.push_back(primary + "Disk (/): " + reset + accent + std::string(disk_str) + reset);
        }

        // 14. Battery (/sys/class/power_supply/BAT*)
        std::string bat_path = "/sys/class/power_supply/BAT0/";
        if (!fs::exists(bat_path)) bat_path = "/sys/class/power_supply/BAT1/";
        
        if (fs::exists(bat_path)) {
            std::ifstream cap_file(bat_path + "capacity");
            std::ifstream stat_file(bat_path + "status");
            std::string cap = "100", status = "AC Connected";
            
            if (cap_file >> cap) {}
            if (stat_file >> status) {}

            lines.push_back(primary + "Battery: " + reset + green + cap + "%" + accent + " [" + status + "]" + reset);
        }

        return lines;
    }
};

