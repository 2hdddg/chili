DEBUG?=0
CC=gcc
CFLAGS=-c -I. -std=gnu99 -Wall -Werror -Wno-error=unused-result
LD=gcc
LDFLAGS=-ldl
SOURCES=$(wildcard src/*.c)
OBJECTS=$(SOURCES:src/%.c=out/%.o)
DEPS=$(OBJECTS:%.o=%.d)
COMPILING=
PREFIX=/usr/local

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
	COMPILING = Compiling $< for debug
else
	CFLAGS += -O
	COMPILING = Compiling $<
endif

# Default target
.PHONY: all
all: chili cscope.out

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

out:
	mkdir -p out

chili: out $(OBJECTS)
	@echo Generating executable chili
	@$(LD) -o chili  $(OBJECTS) $(LDFLAGS)

cscope.out: $(SOURCES)
	@echo Generating cscope.out
	@cscope -b -ssrc

$(OBJECTS): out/%.o: src/%.c
	@echo $(COMPILING)
	@$(CC) $(CFLAGS) $< -o $@

# Generate dependency files in out dir.
# Dependencies should be on the form:
#   out/xx.d out/xx.o: xx.c xx.h yy.h
$(DEPS): out/%.d: src/%.c
	@mkdir -p out
	@echo Analyzing dependencies for $<
	@$(CC) -MM $(CPPFLAGS) -MT '$@ $(basename $@).o' $< > $@;

.PHONY: clean
clean:
	@echo Cleaning
	@-rm chili -f
	@-rm -rf out
	@-rm cscope.out -f

examples: FORCE chili
	@echo Running examples
	@$(MAKE) run -C examples

scenario_tests: FORCE chili
	@$(MAKE) run -C test/scenarios/ --no-print-directory

unit_tests: FORCE chili
	@$(MAKE) run -C test/units/ --no-print-directory

tests: FORCE chili unit_tests scenario_tests

FORCE:
 
.PHONY: install
install: chili
	mkdir -p $(DESTDIR)$(PREFIX)/bin 
	cp $< $(DESTDIR)$(PREFIX)/bin/chili

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/chili

