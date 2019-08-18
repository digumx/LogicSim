/*
 * Implements io throgh ncurses.
 */

#include <ncurses.h>
#include <chrono>

#include <logicsim.hpp>

#include <ncursesio.hpp>


// For key tracking
std::chrono::steady_clock::time_point last_update;
bool key_states[256];
bool is_key_pressed;

// For print sections
lgs::PrintSection* head = NULL;
bool is_empty = true;

void lgs::initializeNcursesIO()
{
        // Curses mode
        initscr();
        cbreak();
        keypad(stdscr, TRUE);
        noecho();
        nodelay(stdscr, TRUE);
        scrollok(stdscr, TRUE);
        curs_set(0);

        // Keypress detection
        is_key_pressed = false;
        last_update = std::chrono::steady_clock::now();
        for(int i = 0; i < 256; ++i)
                key_states[i] = false;

        // Print sections
        int max_l, max_c;
        getmaxyx(stdscr, max_l, max_c);
        printw(std::string(max_l+1, '\n').c_str());
        refresh();
}

void update_key_states()
{
        int code;
        if(std::chrono::steady_clock::now() - last_update >
                        std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(LGS_KEYBOARD_WAIT_TIME)))
        {
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

void lgs::PrintSection::print_update_next()
{
        int l, c, max_l, max_c;
        getmaxyx(stdscr, max_l, max_c);
        getyx(stdscr, l, c);
        beg_lin = l;
        printw(text.c_str());
        printw("\n");
        printw(std::string(max_c, '-').c_str());
        printw("\n");
        refresh();
        if(!is_end) next->print_update_next();
        return;
}

void lgs::PrintSection::print_update()
{
        int l, c;
        getyx(stdscr, l, c);
        move(beg_lin, 0);
        print_update_next();
        move(l, c);
}

lgs::PrintSection::PrintSection()
{
        next = head;
        is_end = is_empty;
        is_empty = false;
        head = this;
        beg_lin = 0;
        text = "";
}

lgs::PrintSection::PrintSection(PrintSection* section)
{
        next = section->next;
        section->next = this;
        is_end = next->is_end;
        beg_lin = 0;
        text = "";
        section->print_update();
}

void lgs::PrintSection::addToEnd(const std::string& str)
{
        text += str;
        print_update();
}

void lgs::PrintSection::setText(const std::string& str)
{
        text = str;
        print_update();
}

void lgs::PrintSection::addToEnd(const std::string&& str)
{
        text += str;
        print_update();
}

void lgs::PrintSection::setText(const std::string&& str)
{
        text = str;
        print_update();
}

void lgs::PrintSection::addBackspace(size_t n)
{
        n = text.size() < n ? text.size() : n;
        text.erase(text.end() - n, text.end());
        print_update();
}

void lgs::PrintSection::reprint() { print_update(); }

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
        int max_l, max_c;
        getmaxyx(stdscr, max_l, max_c);
        printw(std::string(max_l+1, '\n').c_str());
        refresh();
        if(!is_empty) head->reprint();
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

