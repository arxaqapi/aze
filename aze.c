#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios original_termios;

void disableRawMode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void enableRawMode(void)
{
    tcgetattr(STDIN_FILENO, &original_termios);
    atexit(disableRawMode);

    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    // disable echo with bitwise operations
    // and turn off canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int main(void)
{
    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
    return 0;
}