COND_CHOICE = -D COND=ONE

ifeq ($(COND), ONE)
COND_CHOICE = -D COND=ONE
endif

ifeq ($(COND), TWO)
COND_CHOICE = -D COND=TWO
endif

ifeq ($(COND), THREE)
COND_CHOICE = -D COND=THREE
endif

CFLAGS += $(COND_CHOICE)
all:
	gcc $(CFLAGS) test.c -o test; ./test