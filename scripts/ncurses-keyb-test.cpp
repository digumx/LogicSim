#include <ncurses.h>
#include <string>
#include <chrono>

int main()
{
        initscr();
        cbreak();
        noecho();
        nodelay(stdscr, TRUE);
        keypad(stdscr, TRUE);
        scrollok(stdscr, TRUE);
        idcok(stdscr, TRUE);
        curs_set(0);

        printw("Waiting for 5 seconds.\n");
        refresh();
        std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
        while(std::chrono::steady_clock::now() - t <=
                        std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::seconds(5)));
        printw("Listening.\n");
        refresh();

        while(true)
        {
                int code = getch();
                if(code != ERR)
                {
                        char c = (char) code;
                        printw("Key pressed with code ");
                        printw(std::to_string(code).c_str());
                        printw(" and when attempted to print it looks like ");
                        printw(std::string(1, c).c_str());
                        printw("\n");
                        refresh();
                }
        }

        endwin();
}
