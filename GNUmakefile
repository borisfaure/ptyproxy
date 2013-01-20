
# Common
prefix=     /usr/local
bindir=     $(prefix)/bin

CC= gcc

CFLAGS+= -std=c99 -D_POSIX_C_SOURCE=200112L
CFLAGS+= -Wall -Wextra -Werror -Wshadow -Wno-unused
CFLAGS+= -g

LDFLAGS= -g

# Target: ptyproxy
ptyproxy_SRC= ptyproxy.c
ptyproxy_OBJ= $(subst .c,.o,$(ptyproxy_SRC))
ptyproxy_BIN= $(subst .o,,$(ptyproxy_OBJ))

$(ptyproxy_BIN): CFLAGS+=  -I$(OSSLIBDIR)/include/sys
$(ptyproxy_BIN): LDFLAGS+=
$(ptyproxy_BIN): LDLIBS+=  -lutil

# Rules
all: $(ptyproxy_BIN)

$(ptyproxy_BIN): $(ptyproxy_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) $(ptyproxy_BIN) $(ptyproxy_OBJ)

install: all
	mkdir -p $(bindir)
	install -m 755 $(ptyproxy_BIN) $(bindir)

uninstall:
	$(RM) $(addprefix $(bindir)/,$(ptyproxy_BIN))

tags:
	ctags -o tags -a $(wildcard *.[hc])

.PHONY: all clean install uninstall tags

