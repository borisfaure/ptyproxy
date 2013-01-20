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
#include <poll.h>

int do_proxy(int argc, char **argv)
{
    pid_t child;
    int master, slave;
    struct winsize win = {
        .ws_col = 80, .ws_row = 24,
        .ws_xpixel = 480, .ws_ypixel = 192,
    };
    struct termios termios;

    /* Ensure that terminal echo is switched off so that we
       do not get back from the spawned process the same
       messages that we have sent it. */
    if (tcgetattr(STDIN_FILENO, &termios) < 0) {
        perror("tcgetattr");
        return -1;
    }
    termios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

    child = forkpty(&master, NULL, &termios, &win);
    if (child == -1) {
        perror("forkpty");
        exit(1);
    }

    if (child == 0) {

        if (execl("/usr/bin/vim", "/usr/bin/vim", NULL)) {
            perror("execl");
            return -1;
        }
    } else {
        struct pollfd pfds[2];

        pfds[0].fd = master;
        pfds[0].events = POLLOUT;
        pfds[0].revents = 0;
        pfds[1].fd = STDIN_FILENO;
        pfds[1].events = POLLIN;
        pfds[1].revents = 0;


        while (1) {
            int res;
            char buf[4096];
            int nb_fds;

            nb_fds = poll(pfds, 2, -1);
            if (nb_fds < 1) {
                perror("poll");
                return -1;
            }

            /* master */
            if (pfds[0].revents & POLLOUT) {
                nb_fds--;
                res = read(master, buf, 4096);
                if (res > 0) {
                    write(STDOUT_FILENO, buf, res);
                }
            }
            /* stdin */
            if (nb_fds && pfds[1].revents & POLLIN) {
                res = read(STDIN_FILENO, buf, 4096);
                if (res > 0) {
                    write(master, buf, res);
                }
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
