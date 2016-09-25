# Simple Hello World example

EXEC = hello
OBJS = hello.o

all: $(EXEC)

$(EXEC): $(OBJS)

install:
	@install -D $(EXEC) $(STAGING)/bin/

clean:
	@$(RM) $(OBJS)

distclean: clean
	@$(RM) $(EXEC)
