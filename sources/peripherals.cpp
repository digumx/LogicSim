/*
 * Provides implementations for Peripheral classes.
 */

#include <iostream>
#include <vector>
#include <utility>
#include <chrono>
#include <string>
#include <sstream>


#include <json.hpp>

#include <ncursesio.hpp>

#include <peripherals.hpp>

using namespace lgs;


LEDArray::LEDArray(const nlohmann::json& initJson) : Peripheral(initJson)
{
        // Initialize from JSON
        for(nlohmann::json::const_iterator led = initJson.begin(); led != initJson.end(); ++led)
        {
                led_pos.push_back(std::pair<int, int>((*led)["X"].get<int>(), (*led)["Y"].get<int>()));
                led_labels.push_back((*led)["Label"].get<std::string>());
        }

#ifndef LGS_DEBUG_LEDARRAY_OFF
        // Get PrintSection
        section = new PrintSection();
#endif
}

void LEDArray::tick(const bool* stateR, bool* stateW, int w, int h) 
{
        std::stringstream str;
        str << "LEDs: ";
        for(size_t i = 0; i < led_pos.size(); ++i)
        {
                str << led_labels[i];
                if(0 <= led_pos[i].first && led_pos[i].first < w && 0 <= led_pos[i].second && led_pos[i].second < h)
                        str << (stateR[led_pos[i].second*w + led_pos[i].first] ? "1" : "0");
                else str << "0";
        }
        str << "\n";
        
        
#ifndef LGS_DEBUG_LEDARRAY_OFF
        section->setText(str.str());
#endif
}

BitSwitchArray::BitSwitchArray(const nlohmann::json& initJson) : Peripheral(initJson)
{
        for(nlohmann::json::const_iterator sw = initJson.begin(); sw != initJson.end(); ++sw)
        {
                switch_pos.push_back(std::pair<int, int>((*sw)["X"].get<int>(), (*sw)["Y"].get<int>()));
                keys.push_back((*sw)["Key"].get<int>());
        }
}

void BitSwitchArray::tick(const bool* stateR, bool* stateW, int w, int h)
{
        for(std::vector<int>::size_type i = 0; i < keys.size(); i++)
               stateW[w*switch_pos[i].second + switch_pos[i].first] = getKeyState(keys[i]);
}

Clock::Clock(const nlohmann::json& initJson) : Peripheral(initJson)
{
        x = initJson["X"].get<int>();
        y = initJson["Y"].get<int>();
        period = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                        std::chrono::milliseconds(initJson["Period"].get<int>()));
        state = false;
        previous = std::chrono::steady_clock::now();
}

void Clock::tick(const bool* stateR, bool* stateW, int w, int h)
{
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if(now - previous > period)
        {
                previous = now;
                state = !state;
        }
        stateW[y*w + x] = state;
}

Keyboard::Keyboard(const nlohmann::json& init_json) : Peripheral(init_json)
{
        key_pressed_x = init_json["Key pressed line"]["X"].get<int>();
        key_pressed_y = init_json["Key pressed line"]["Y"].get<int>();
        for(int i = 0; i < 8; i++)
        {
                key_code_x[i] = init_json["Key code lane"][i]["X"].get<int>();
                key_code_y[i] = init_json["Key code lane"][i]["Y"].get<int>();
        }
}

void Keyboard::tick(const bool* stateR, bool* stateW, int w, int h)
{
       if((stateW[key_pressed_y*w + key_pressed_x] = lgs::isAnyKeyPressed()))
       {
               int key = lgs::getAnyPressedKey();
               for(int i = 0; i < 8; i++)
               {
                       stateW[key_code_y[i]*w + key_code_x[i]] = key%2 == 1;
                       key /= 2;
               }
       } 
}

CharStreamPrinter::CharStreamPrinter(const nlohmann::json& initJson) : Peripheral(initJson)
{
        print_line_prev = false;
        print_line_x = initJson["Print line"]["X"].get<int>();
        print_line_y = initJson["Print line"]["Y"].get<int>();
        for(int i = 0; i < 8; i++)
        {
                char_lane_x[i] = initJson["Character lane"][i]["X"].get<int>();
                char_lane_y[i] = initJson["Character lane"][i]["Y"].get<int>();
        }
}

void CharStreamPrinter::tick(const bool* stateR, bool* stateW, int w, int h)
{
        if(print_line_prev & !stateR[print_line_y*w + print_line_x])
        {
                unsigned int code = 0;
                for(int i = 7; i >= 0; --i)
                    code = code*2 + (stateR[char_lane_y[i]*w + char_lane_x[i]] ? 1 : 0);
                if(code == 127) lgs::backspace();
                else lgs::print(std::string(1, (char) code));
#ifdef DBG_PRINT
                lgs::print("Printing character code ");
                lgs::print(std::to_string(code));
                lgs::print(" which looks like ");
                lgs::print(std::string(1, (char) code));
                lgs::print("\n");
#endif
        }
        print_line_prev = stateR[print_line_y*w + print_line_x];
}

Peripheral* lgs::peripheralFromJson(const nlohmann::json& periJson)
{
        std::string cls = periJson["Class"].get<std::string>();
        if(cls == std::string("LEDArray")) 
                return new LEDArray(periJson["Initializer"]);
        else if(cls == std::string("BitSwitchArray")) return new BitSwitchArray(periJson["Initializer"]);
        else if(cls == std::string("Clock")) return new Clock(periJson["Initializer"]);
        else if(cls == std::string("Keyboard")) return new Keyboard(periJson["Initializer"]);
        else if(cls == std::string("CharStreamPrinter")) return new CharStreamPrinter(periJson["Initializer"]);
        else 
        {
                lgs::print("Unknown Peripheral class: ");
                lgs::print(cls.c_str());
                lgs::print("\n");
                lgs::exitNcursesMode(true);
        }
        return NULL;
}
