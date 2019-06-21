/* includes */
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/* data */
struct termios original_termios;

/* terminal */

void die(const char *s)
{
    perror(s);
    exit(1);
}

void disableRawMode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
    {
        die("tcsetattr");
    }
}

void enableRawMode(void)
{
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
    {
        die("tcgetattr");
    }
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

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        die("tcseattr");
    }
}

/* init */
int main(void)
{
    enableRawMode();

    while (1)
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c ,1) == -1 && errno != EAGAIN)
        {
            die("read");
        }
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