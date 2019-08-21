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


PeripheralInterface& Peripheral::getInputInterface() { return input_interface; };

PeripheralInterface& Peripheral::getOutputInterface() { return output_interface; };

LEDArray::LEDArray(const nlohmann::json& initJson) : Peripheral(initJson)
{
        // Initialize from JSON
        for(nlohmann::json::const_iterator led = initJson.begin(); led != initJson.end(); ++led)
        {
                input_interface.push_back(std::tuple<int, int, bool>((*led)["X"].get<int>(), (*led)["Y"].get<int>(), false));
                led_labels.push_back((*led)["Label"].get<std::string>());
        }

        // Get PrintSection
        section = new PrintSection();
}

void LEDArray::tick() 
{
        std::stringstream str;
        str << "LEDs:\n";
        for(size_t i = 0; i < input_interface.size(); ++i)
                str << led_labels[i] << (std::get<2>(input_interface[i]) ? "1" : "0");
        str << "\n";
        
        section->setText(str.str());
}

BitSwitchArray::BitSwitchArray(const nlohmann::json& initJson) : Peripheral(initJson)
{
        for(nlohmann::json::const_iterator sw = initJson.begin(); sw != initJson.end(); ++sw)
        {
                output_interface.push_back(std::tuple<int, int, bool>((*sw)["X"].get<int>(), (*sw)["Y"].get<int>(), false));
                keys.push_back((*sw)["Key"].get<int>());
        }
}

void BitSwitchArray::tick()
{
        for(PeripheralInterface::size_type i = 0; i < output_interface.size(); i++)
               std::get<2>(output_interface[i]) = getKeyState(keys[i]);
}

Clock::Clock(const nlohmann::json& initJson) : Peripheral(initJson)
{
        output_interface.push_back(std::tuple<int, int, bool>(initJson["X"].get<int>(), initJson["Y"].get<int>(), false));
        period = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                        std::chrono::milliseconds(initJson["Period"].get<int>()));
        previous = std::chrono::steady_clock::now();
}

void Clock::tick()
{
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if(now - previous > period)
        {
                previous = now;
                std::get<2>(output_interface[0]) = !std::get<2>(output_interface[0]);
        }
}

Keyboard::Keyboard(const nlohmann::json& init_json) : Peripheral(init_json)
{
        int key_pressed_x = init_json["Key pressed line"]["X"].get<int>();
        int key_pressed_y = init_json["Key pressed line"]["Y"].get<int>();
        output_interface.push_back(std::tuple<int, int, bool>(key_pressed_x, key_pressed_y, false));
        int x, y;
        for(int i = 0; i < 8; i++)
        {
                x = init_json["Key code lane"][i]["X"].get<int>();
                y = init_json["Key code lane"][i]["Y"].get<int>();
                output_interface.push_back(std::tuple<int, int, bool>(x, y, false));
        }
}

void Keyboard::tick()
{
       if((std::get<2>(output_interface[0]) = lgs::isAnyKeyPressed()))
       {
               int key = lgs::getAnyPressedKey();
               for(int i = 0; i < 8; i++)
               {
                       std::get<2>(output_interface[i+1]) = key%2 == 1;
                       key /= 2;
               }
       } 
}

CharStreamPrinter::CharStreamPrinter(const nlohmann::json& initJson) : Peripheral(initJson)
{
        print_line_prev = false;
        int x = initJson["Print line"]["X"].get<int>();
        int y = initJson["Print line"]["Y"].get<int>();
        input_interface.push_back(std::tuple<int, int, bool>(x, y, false));
        for(int i = 0; i < 8; i++)
        {
                x = initJson["Character lane"][i]["X"].get<int>();
                y = initJson["Character lane"][i]["Y"].get<int>();
                input_interface.push_back(std::tuple<int, int, bool>(x, y, false));
        }
}

void CharStreamPrinter::tick()
{
        if(print_line_prev & !std::get<2>(input_interface[0]))
        {
                unsigned int code = 0;
                for(int i = 7; i >= 0; --i)
                    code = code*2 + (std::get<2>(input_interface[i+1]) ? 1 : 0);
                if(code == 127) lgs::backspace();
                else lgs::print(std::string(1, (char) code));
        }
        print_line_prev = std::get<2>(input_interface[0]);
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
