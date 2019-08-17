/*
 * A script to measure the time elapsed between successive key-press events generated when a key is held down in the keyboard.
 * This gives the optimal value for LGS_KEYBOARD_WAIT_TIME.
 */

/*
 * The number of keyboard events over which to average the time measured.
 */
#define N_EVENTS 100

#include <chrono>
#include <string>

#include <ncurses.h>

int main()
{
        initscr();
        cbreak();
        noecho();
        nodelay(stdscr, TRUE);
        keypad(stdscr, TRUE);
        scrollok(stdscr, TRUE);
        idcok(stdscr, TRUE);

        printw("Press and hold a key.\n");
        refresh();
        while(true)
        {
                std::chrono::steady_clock::duration del_t = std::chrono::steady_clock::duration::zero();
                for(int i = 0; i < N_EVENTS; i++)
                {
                        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
                        while(getch() == ERR);
                        del_t += std::chrono::steady_clock::now() - t0;
                }
                del_t /= N_EVENTS;
                printw("Average of time between key events taken over ");
                printw(std::to_string(N_EVENTS).c_str());
                printw(" events is ");
                printw(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(del_t).count()).c_str());
                printw(" milliseconds.\n");
        }
}
