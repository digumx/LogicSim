/*
 * Implements io throgh ncurses.
 */

#include <ncurses.h>
#include <chrono>

#include <logicsim.hpp>

#include <ncursesio.hpp>

std::chrono::steady_clock::time_point last_update;
bool key_states[256];
bool is_key_pressed;

void lgs::initializeNcursesIO()
{
        initscr();
        cbreak();
        keypad(stdscr, TRUE);
        noecho();
        nodelay(stdscr, TRUE);
        scrollok(stdscr, TRUE);
        is_key_pressed = false;
        last_update = std::chrono::steady_clock::now();
        for(int i = 0; i < 256; ++i)
                key_states[i] = false;
}

void update_key_states()
{
        int code;
        if(std::chrono::steady_clock::now() - last_update >
                        std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(LGS_KEYBOARD_WAIT_TIME)))
        {
/*#ifdef LGS_DEBUG
                printw("Updating key states.\n");
                refresh();
#endif*/
                is_key_pressed = false;
                for(int i = 0; i < 256; i++) key_states[i] = false;
                while((code = getch()) != ERR)
                {
                        key_states[code] = true;
                        is_key_pressed = true;
                }
                last_update = std::chrono::steady_clock::now();
        }
}

bool lgs::getKeyState(int key)
{
        update_key_states();
        return key_states[key];
}

bool lgs::isAnyKeyPressed()
{
        update_key_states();
        return is_key_pressed;
}

int lgs::getAnyPressedKey()
{
       update_key_states();
       int ret = -1;
       for(int i = 0; i < 256; i++)
              if(key_states[i])
              {
                      ret = i;
                      break;
              }
       return ret;
}

void lgs::waitForKey() { while(getch() == ERR); }

void lgs::print(const std::string& str) 
{ 
        printw(str.c_str()); 
        refresh();
}

void lgs::PrintSection::print_update_child()
{
        int l, c;
        getyx(stdscr, l, c);
        beg_lin = l;
        printw(text.c_str());
        printw("\n");
        printw(std::string
}

void lgs::PrintSection::print_update()
{
        
}

void lgs::printAtTop(const std::string& str)
{
        int y, x, max_y, max_x;
        getyx(stdscr, y, x);
        getmaxyx(stdscr, max_y, max_x);
        move(0, 0);
        printw(str.c_str());
        printw("\n");
        printw(std::string(max_x, '-').c_str());
        printw("\n");
        int n_lines = 0;
        for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
                if((*i) == '\n') n_lines++;
        if(y < n_lines+3)   move(n_lines+3, 0);
        else                move(y, x);
        refresh();
}

void lgs::backspace()
{
        int y, x;
        getyx(stdscr, y, x);
        if(x == 0) return;
        move(y, x-1);
        printw(" ");
        move(y, x-1);
        refresh();
}

void lgs::clearScreen()
{ 
        erase();
        refresh();
}

void lgs::exitNcursesMode(bool err)
{
        if(err)
        {
            printw("Critical error occured in LogicSim, press any key to exit.");
            refresh();
            while(getch() == ERR);
        }
        endwin();
        exit(EXIT_FAILURE);
}

