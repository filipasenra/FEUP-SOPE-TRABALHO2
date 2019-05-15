# Project properties
PROGRAM_SERVER = server
PROGRAM_USER = user

#source options
SRCDIR =
C_FILES := $(wildcard $(SRCDIR)/*.c)

#build options
BUILDDIR = build
OBJS_SERVER := server.c dataBase.c creatAccount.c communication.c serverMessage.c log.c box_office.c 
OBJS_USER := user.c userMessage.c log.c communication.c

#compiler options
CFLAGS = -Wall -pedantic
LDFLAGS = -lpthread
LDLIBS = -D_REENTRANT

all: $(PROGRAM_SERVER) $(PROGRAM_USER)

$(PROGRAM_SERVER): .depend $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(OBJS_SERVER) $(LDFLAGS) -o $(PROGRAM_SERVER) $(LDLIBS)

$(PROGRAM_USER): .depend $(OBJS_USER)
	$(CC) $(CFLAGS) $(OBJS_USER) $(LDFLAGS) -o $(PROGRAM_USER) $(LDLIBS)

# Dependency management

depend: .depend

.depend: cmd = gcc -MM -MF depend $(var); echo -n "$(BUILDDIR)/" >> .depend; cat depend >> .depend;
.depend:
	@echo "Generating dependencies..."
	@$(foreach var, $(C_FILES), $(cmd))
	@rm -f depend

-include .depend

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

%: %.c
	$(CC) $(CFLAGS) -o $@ $<


clean:
	rm -f .depend $(PROGRAM_SERVER)
	rm -f .depend $(PROGRAM_USER)
	rm -rf $(BUILDDIR)

$(BUILDDIR):
	mkdir $(BUILDDIR)

.PHONY: clean depend