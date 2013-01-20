#define _GNU_SOURCE 1
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pty.h>
#include <termios.h>

int do_proxy(int argc, char **argv)
{
    pid_t child;
    int master;
    struct winsize win = {
        .ws_col = 80, .ws_row = 24,
        .ws_xpixel = 480, .ws_ypixel = 192,
    };

    child = forkpty(&master, NULL, NULL, &win);
    if (child == -1) {
        perror("forkpty");
        exit(1);
    }

    if (child == 0) {
        struct termios termios;

        /* Ensure that terminal echo is switched off so that we
           do not get back from the spawned process the same
           messages that we have sent it. */
        if (tcgetattr(STDIN_FILENO, &termios) < 0) {
            perror("tcgetattr");
            return -1;
        }

        termios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
        termios.c_oflag &= ~(ONLCR);

        if (tcsetattr(STDIN_FILENO, TCSANOW, &termios) < 0) {
            perror("tcsetattr");
            return -1;
        }

        if (execl("/usr/bin/vim", "/usr/bin/vim", NULL)) {
            perror("execl");
            return -1;
        }
    } else {
        while (1) {
            int res;
            int c;

            res = read(master, &c, 1);
            if (res == 1) {
                write(STDOUT_FILENO, &c, 1);
            }
        }
    }
    return 0;
}

int main (int argc, char **argv)
{

    //if (argc > 1) {
        return do_proxy(argc, argv);
    //}
    return -1;
}
