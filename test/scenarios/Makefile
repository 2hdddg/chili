CC=gcc
CFLAGS=-std=gnu99 -Wall -fPIC
TESTS=$(wildcard *.c)
TARGETS=$(TESTS:%.c=chili_%.so)

.PHONY: run
run: FORCE $(TARGETS)
	@echo Running scenario tests
	@python process.py
	@python execution.py

FORCE:

$(TARGETS): chili_%.so: %.c
	@echo Building $@
	@$(CC) $(CFLAGS) $< -shared -o $@

.PHONY: clean
clean:
	@echo Cleaning
	@-rm *.so -f
