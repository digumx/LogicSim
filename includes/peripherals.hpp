/*
 * This header defines classes for the peripheral system. A peripheral is anything that reads bits from the state and writes bits to the state every tick.
 * Peripherals are attached to the CPUWorker by sending an array of peripherals to the worker. Peripherals execute parallely with the circuit and take 
 * precedence over the circuit.
 */ 

#ifndef LGS_INCLUDE_PERIPHERALS
#define LGS_INCLUDE_PERIPHERALS 

#include <vector>
#include <array>
#include <utility>
#include <chrono>

#include <json.hpp>

namespace lgs
{
        /*
         * Forward declaration of class PrintSection
         */
        class PrintSection;

        /*
         * A container typedef to store the input or output locations of a peripheral. It's a vector of 3-tuples, first 2 members are the 
         * location of the bit, last member is state.
         */
        typedef std::vector<std::tuple<int, int, bool>> PeripheralInterface;
               
        /*
         * An abstract base class for defining the peripheral interface.
         */
        class Peripheral
        {
                protected:
                        PeripheralInterface input_interface, output_interface;
                public:
                        Peripheral(const nlohmann::json& initJson) {}                                   // Force peripherals to provide constructor from json.
                                                                                                        // Should fill input_interface and output_interface.
                        virtual void tick() = 0;                                                        // Do whatever the peripheral does
                        PeripheralInterface& getInputInterface();                                       // Get interfaces between peripheral and circuit
                        PeripheralInterface& getOutputInterface();
        };

        // The following are the peripherals currently supported by LogicSim
        
        /*
         * A peripheral to output the states at a specified list of bit as a line of 0s and 1s in the standard output. The equivalent of connecting
         * status LEDs at specific locations in the circuit.
         *
         * JSON initializer syntax: An array of JSON objects with each member representing an LED. Each memeber has 3 fields, integers "X" and "Y" 
         *                          indicating position of LED within the circuit, and string "Label" providing a label for the LED. The state of
         *                          all LEDs are printed as <Label><State><Label><State>... in a single line. The label can be a null string to 
         *                          produce the effect of printing two or more states consecutively, as <State><State>...
         *
         * NOTE: Note, multiple LEDArrays are allowed, but because of how adding PrintSections are handled, multiple LEDArrays will appear in an order
         *       relative to themselves that is reverse of the order they appear in the JSON file. Their order with respect to any other PrintSection's
         *       order of appearance is undefined. In general, it is currently difficult to control order within PrintSections in general.
         */
        class LEDArray : public Peripheral
        {
                private:
                        std::vector<std::string> led_labels;
                        lgs::PrintSection* section;
                public:
                        LEDArray(const nlohmann::json& initJson);
                        void tick() override;
        };

        /*
         * A peripheral that checks keyboard inputs and based on the key pressed sets certain bits on the logic board. That is, for each key o the keyboard, 
         * it iether sets the value of a particular bit to the state of the key, or ignores it.  This can be thought of as the equivalen of attaching bit 
         * switches at the particular positions in the circuit. A single instance suffices for the entire keyboard, and it is considered the most appropriate
         * configuration, but multiple instances are allowed. Multiple instances can be used to set the state of more than one bits to the state of a key,
         * but using a single instance and circuit logic is considered more appropriate.
         *
         * JSON initializer syntax: An array of JSON objects, each corresponding to a key-bit pair. Each JSON object has 3 integer fields, "Key" whose value is
         *                          the key code for the key, "X" is the x-position of the bit to set, and "Y" is the y-position. Bits with out of bound positions
         *                          are ignored. Key code for the key can be obtained using the ncurses-keyb-test script. 
         *
         * NOTE: The class uses ncurses for keyboard input. Only keystrokes visible to ncurses are supported. Also note that currently, keystrokes corresponding
         *       to interrupt signals are ignored by the way ncurses is initialized.
         */
        class BitSwitchArray : public Peripheral
        {
                private:
                        std::vector<int> keys;
                public:
                        BitSwitchArray(const nlohmann::json& initJson);
                        void tick() override;
        };

        /*
         * A peripheral that produces a clock signal. Although clock circuits can easily be produced in LogicSim, they have two major problems. One is that they
         * cannot synchronize with real life time, and the other is that the size of the circuit will increase at best as a square root of the time period, making
         * long time period clock circuits extremely large. This peripheral solves both these issues.
         *
         * JSON Initializer syntax: The JSON initializer has three integer fields, "X" and "Y" representing the position in the circuit to which the clock is
         *                          connected, and "Period" representing the period in milliseconds of the clock signal produced.
         */
        class Clock : public Peripheral
        {
                private:
                        std::chrono::steady_clock::duration period;
                        std::chrono::steady_clock::time_point previous;

                public:
                        Clock(const nlohmann::json& initJson);
                        void tick() override;
        };

        /*
         * A peripheral that listens to keyboard events and translates them to a form easily interpreted by the circuit.
         * As long as any key is pressed, the bit located by "Key pressed line" is held high, and the 8 bits located by
         * "Key code lane", which combine to represent an 8-bit unsigned integer,  are set to the character code of the 
         * key pressed. When no key is pressed, the "Key pressed line" is held low, and the bits of the "Key code lane" 
         * are allowed to be set by the circuit. If multiple keys are pressed, an arbitrary selection is made and the
         * key code of the selected key is set to "Key code lane" while "Key pressed line" is held high. This behavior 
         * makes this peripheral most useful when only one key is expected to be pressed at one time. If keys are held 
         * down, the "Key pressed line" and "Key code lane" are held to their correspondig appropriate values.
         *
         * JSON Initializer syntax: The JSON initializer has two fields, "Key pressed line" and "Key code lane". The
         *                          "Key pressed line"'s value is itself a JSON object with two integer valued fields,
         *                          "X" and "Y", locating a bit on the circuit that would be held high if and only if
         *                          any key is being pressed on the keyboard. The value of the "Key code lane" is an
         *                          array of length 8. Each element of the array is a JSON object with 2 integer fields,
         *                          "X" and "Y", and the i'th element locates the bit in the circuit to set with the
         *                          value of the i'th bit of the keycode when some key is pressed on the keyboard.
         */
        class Keyboard : public Peripheral
        {
                // First output_interface entry is key pressed lane
                // For next 8, ith entry is i-1th bit of key code.
                public:
                        Keyboard(const nlohmann::json& initJson);
                        void tick() override;
        };

        /*
         * A periperal for producing output to the screen as a character stream, similar to how cout behaves. It prints
         * one character onto the screen at a time, and does not support cursor movements beyond those produced by tab,
         * space, newline and such. It waits for a falling edge transition on the "Print line", that is, a transition from
         * 1 to 0, and on getting such a transition it prints the character whose ASCII code is given by the 8-bit integer
         * formed by the bits in the "Character lane" at the time. Note that in the tick where the "Print line" changes to 
         * a 0 from a 1, whatever code is given by the "Character lane", that is printed. To avoid printing burps, or other
         * temporary garbage in the "Character lane", registers and gates should be used. Note that if the code does not
         * correspond to any printable character, undefined behavior occurs. Also note that code 127 is reserved specifically
         * to denote a backspace character which deletes the last printed character in the same line.
         *
         * JSON Initializer syntax: The JSON initializer has two fields, "Print line" and "Character lane". "Print line"
         *                          is a JSON object with two integer fields, "X" and "Y", locating on the circuit the bit
         *                          whose transition from 1 to 0 will cause a character to be printed. "Character lane" is
         *                          an 8 element array of JSON objects, each with 2 integer fields, "X" and "Y". The i'th
         *                          element of the array locates the point on the board from where the i'th bit of the 
         *                          character code is to be read during printing.
         */
        class CharStreamPrinter : public Peripheral
        {
                private:
                        bool print_line_prev;

                        // The first input_interface member is print line.
                        // For next 8 members, ith member is i-1th bit of character lane.
                public:
                        CharStreamPrinter(const nlohmann::json& initJson);
                        void tick() override;
        };

        /*
         * Factory function that takes a json and produces a Peripheral.
         */
        Peripheral* peripheralFromJson(const nlohmann::json& initJson);

}

#endif
