TESTS = server client

TEST_DATA = s Tai
CFLAGS = -Wall -Werror -g

# Control the build verbosity                                                   
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

GIT_HOOKS := .git/hooks/applied

.PHONY: all clean

all: $(GIT_HOOKS) $(TESTS)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

OBJS_LIB = \
    tst.o bloom.o

OBJS := \
    $(OBJS_LIB) \
    server.o \
    client.o

deps := $(OBJS:%.o=.%.o.d)

client: client.o $(OBJS_LIB)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS)  -o $@ $^ -lm -pthread


server: server.o $(OBJS_LIB)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS)  -o $@ $^ -lm -pthread

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $< -pthread


clean:
	$(RM) $(TESTS) $(OBJS)
	$(RM) $(deps)
	rm -f  bench_cpy.txt bench_ref.txt ref.txt cpy.txt caculate
	rm -f unit-test/testdict unit-test/bloomtest
	rm -f server.o client.o tst.o server client
-include $(deps)
