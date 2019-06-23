/* includes */
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/* defines */
#define CTRL_KEY(k) ((k) & 0x1f)

/* data */
struct termios original_termios;

/* To avoid implicit declaration*/
void clearOnExit(void)
{
    write(STDIN_FILENO, "\x1b[2J", 4);
    write(STDIN_FILENO, "\x1b[H", 3);
}

/* terminal */

void die(const char *s)
{
    clearOnExit();

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

char editorReadKey(void)
{ // wait for one keypress and return it
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

void editorRefreshScreen(void)
{
    write(STDIN_FILENO, "\x1b[2J", 4);
    // Escape sequences instruct terminal text formatting tasks, such as coloring text...
    // https://vt100.net/docs/vt100-ug/chapter3.html#ED
    write(STDIN_FILENO, "\x1b[H", 3);
}

// clearOnExit

/* input */

// higher level
void editorProcessKeypress()
{ // wait for keypress and handles it
    char c = editorReadKey();
    switch (c)
    {
    case CTRL_KEY('q'):
    clearOnExit();
        exit(0);
        break;
    }
}

/* init */
int main(void)
{
    enableRawMode();

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    
    return 0;
}