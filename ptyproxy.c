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
#include <sysexits.h>

int do_proxy(int argc, char **argv)
{
    pid_t child;
    int master, slave;
    struct winsize win;
    struct termios termios;

    /* Get current win size */
    ioctl(0, TIOCGWINSZ, &win);

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
        if (execvp(argv[0], argv)) {
            perror("execl");
            return -1;
        }
    } else {
        struct pollfd pfds[2];
        int log_in, log_out;

        log_in = open("in.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        log_out = open("out.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);

        pfds[0].fd = master;
        pfds[0].events = POLLOUT;
        pfds[0].revents = 0;
        pfds[1].fd = STDIN_FILENO;
        pfds[1].events = POLLIN;
        pfds[1].revents = 0;

        fcntl(master, F_SETFL, O_NONBLOCK);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

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
                do {
                    res = read(master, buf, 4096);
                    if (res > 0) {
                        write(STDOUT_FILENO, buf, res);
                        write(log_out, buf, res);
                    }
                } while (res == 4096);
                pfds[0].revents &= ~POLLOUT;
            }
            /* stdin */
            if (nb_fds && pfds[1].revents & POLLIN) {
                do {
                    res = read(STDIN_FILENO, buf, 4096);
                    if (res > 0) {
                        write(master, buf, res);
                        write(log_in, buf, res);
                    }
                } while (res == 4096);
                pfds[1].revents &= ~POLLIN;
            }
        }
    }
    return 0;
}

static void
usage(void)
{
    fprintf(stderr,
            "ptyproxy prog...\n"
            "prog is the software program to proxy");
    exit(EX_USAGE);
}

int main (int argc, char **argv)
{
    if (argc <= 1) {
        usage();
    }

    do_proxy(--argc, ++argv);

    return EX_OK;
}
