#include <stdio.h>
#include <ctype.h>
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
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // XOFF XON, control characters, input flag
    raw.c_oflag &= ~(OPOST);
    // post processing of output
    // required for new line : "\r\n"
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // disable echo with bitwise operations
    // and turn off canonical mode
    raw.c_cc[VMIN] = 0; 
    // min amount of time before read() returns
    raw.c_cc[VTIME] = 1;
    // max amount of time before read() returns

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int main(void)
{
    enableRawMode();

    while (1)
    {
        char c = '\0';
        read(STDIN_FILENO, &c ,1);
        if (iscntrl(c))
        {
            printf("%d\r\n", c);
        } else 
        {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }
    
    return 0;
}