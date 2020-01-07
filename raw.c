#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

struct termios orig_termios;

void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enter_raw_mode()
{
    struct termios term;

    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    term = orig_termios;

    term.c_lflag &= ~(ECHO | ICANON);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

int main(void)
{
    char c;
    enter_raw_mode();
    do {
        read(STDIN_FILENO, &c, 1);
        fprintf(stderr, "0x%x\n", c);
    } while(c != 'q');
}

