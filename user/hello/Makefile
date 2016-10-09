# Simple Hello World example

EXEC = hello
OBJS = hello.o
CPPFLAGS = -D_GNU_SOURCE -D_DEFAULT_SOURCE -D_BSD_SOURCE

all: $(EXEC)

$(EXEC): $(OBJS)

install:
	@install -D $(EXEC) $(STAGING)/bin/

clean:
	@$(RM) $(OBJS)

distclean: clean
	@$(RM) $(EXEC)
