/* includes */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** defines */
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data */
struct editorConfig
{
    int screenrows;
    int screencols;
    struct termios original_termios;
};

struct editorConfig E;


/*** To avoid implicit declaration*/
void clearOnExit(void)
{
    write(STDIN_FILENO, "\x1b[2J", 4);
    write(STDIN_FILENO, "\x1b[H", 3);
}
/*** Output */

void editorDrawRows(void)
{
    // missing terminal size
    for (int y = 0; y < E.screenrows; y++)
    {
        write(STDIN_FILENO, "~\r\n", 3);
    }
    
}
/*** terminal */

void die(const char *s)
{
    clearOnExit();

    perror(s);
    exit(1);
}

void disableRawMode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.original_termios) == -1)
    {
        die("tcsetattr");
    }
}

void enableRawMode(void)
{
    if (tcgetattr(STDIN_FILENO, &E.original_termios) == -1)
    {
        die("tcgetattr");
    }
    atexit(disableRawMode);

    struct termios raw = E.original_termios;
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

int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        return -1;
    } else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void editorRefreshScreen(void)
{
    write(STDIN_FILENO, "\x1b[2J", 4);
    // Escape sequences instruct terminal text formatting tasks, such as coloring text...
    // https://vt100.net/docs/vt100-ug/chapter3.html#ED
    write(STDIN_FILENO, "\x1b[H", 3);

    editorDrawRows();

    write(STDIN_FILENO, "\x1b[H", 3);
    // reposition curser back to top
}

// clearOnExit

/*** input */

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


/*** init */
void initEditor(void)
{
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main(void)
{
    enableRawMode();
    initEditor();

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    
    return 0;
}