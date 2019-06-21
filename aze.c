#include <unistd.h>
#include <stdio.h>

int main(void)
{
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1)
    {
        printf("user input: %c\n", c);
    }
    
    return 0;
}