CC=gcc
CFLAGS=-std=gnu99 -Wall
TESTS=$(wildcard *.c)
TARGETS=$(TESTS:%.c=chili_%.so)

.PHONY:
run: $(TARGETS)
	@for TEST in $(TARGETS) ; do \
		../chili ./$$TEST ; \
	done

$(TARGETS): chili_%.so: %.c
	@echo Building $@
	@$(CC) $(CFLAGS) $< -shared -o $@

.PHONY: clean
clean:
	@echo Cleaning
	@-rm *.so -f
