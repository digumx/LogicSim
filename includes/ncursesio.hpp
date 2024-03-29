/*
 * Contains functions for keyboard key-state querying and displaying text output via the ncurses environment. Also handles 
 * entering into and safely exiting from ncurses mode.
 */

#ifndef LGS_INCLUDE_NCURSES_IO
#define LGS_INCLUDE_NCURSES_IO

#include <string>

namespace lgs
{
        /*
         * Called to enter ncurses mode and initialize file scope variables for printing and key listening. Attempting to call
         * any other function defined here before calling this may cause undefined behavior.
         */
        void initializeNcursesIO();

        /*
         * Returns the state of given key with given keycode. 
         */
        bool getKeyState(int key);

        /*
         * Returns if a key has been pressed.
         */
        bool isAnyKeyPressed();

        /*
         * Returns the character code of any one of the keys being pressed. Most useful when only one key is expected to be
         * pressed at a time. Return -1 if no key is being pressed.
         */
        int getAnyPressedKey();

        /*
         * Waits for key press.
         */
        void waitForKey();

        /*
         * Print string to bottommost part of screen.
         */
        void print(const std::string& str);

        /*
         * Represents a section of the screen that can hold text. The text can be added to or replaced.
         * There can be multiple PrintSections, but the total length of text plus margins must fit 
         * comfortably within the screen, otherwise undefined behavior occurs. The bottommost section is 
         * not the part of any PrintSection - it is the exclusive target for print(above).
         * PrintSections can solely be constructed via the getPrintSection... functions.
         */
        class PrintSection
        {
                private:
                        PrintSection* next;
                        bool is_end;
                        int beg_lin;
                        std::string text;

                        void print_update();                        // Print text.
                        void print_update_next();                   // Recursively prints all succeeding sections,
                                                                    // and updates beg_lin for each section. Arguement is
                                                                    // screen width. Returns total length of all print
                                                                    // sections.
                
                public:

                        PrintSection();                             // Construct new print section at top of screen.
                        PrintSection(PrintSection* nxt);            // Construct new print section after nxt.

                        void addToEnd(const std::string& str);              // Add string to end of text.
                        void setText(const std::string& txt);               // Set the test.
                        void addToEnd(const std::string&& str);             // Rvalue reference overloads.
                        void setText(const std::string&& txt);             
                        void addBackspace(size_t n);                        // Delete n characters from the end of the string.
                        void reprint();                                     // Print contents immediately.

        };
         
        /*
         * Deletes the character immediately before the cursor and moves cursor to the left by one character, does nothing if 
         * at end of line. Essentially does what printing a backspace character would do.
         */
        void backspace();

        /*
         * Clears screen, but does not clear out the PrintSections - simply redraws them.
         */
        void clearScreen();

        /*
         * Exits ncurses mode. If the boolean err is true, prints an error message and waits for a keypress before exiting.
         */
        void exitNcursesMode(bool err);
}

#endif
