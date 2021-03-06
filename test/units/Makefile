CC=gcc
CFLAGS=-std=gnu99 -Wall -Werror -fPIC -I../../src -shared -fno-stack-protector
LD=ld
LDFLAGS=-shared
SOURCES=$(wildcard ../../src/*.c)
OBJECTS=$(SOURCES:../../src/%.c=out/%.o)
DEPS=$(OBJECTS:%.o=%.d)
COMPILING=
CHILI=../../chili all -i
SUITES=chili_run.so chili_main.so chili_suite.so chili_named.so chili_registry.so chili_debugger.so
SUITE_PATHS=$(SUITES:%=./%)

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
	COMPILING = Compiling $< for debug
else
	CFLAGS += -O
	COMPILING = Compiling $<
endif

# Default target
.PHONY: run
run: out $(SUITES)
	@echo Running unit tests
	@$(CHILI) $(SUITE_PATHS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif

out:
	mkdir -p out

$(OBJECTS): out/%.o: ../../src/%.c
	@echo $(COMPILING)
	@$(CC) $(CFLAGS) $< -o $@

# Generate dependency files in out dir.
# Dependencies should be on the form:
#   out/xx.d out/xx.o: xx.c xx.h yy.h
$(DEPS): out/%.d: ../../src/%.c
	@mkdir -p out
	@echo Analyzing dependencies for $<
	@$(CC) -MM $(CPPFLAGS) -MT '$@ $(basename $@).o' $< > $@;

chili_run.so: tests_run.o out/run.o out/redirect.o assert.o
	@echo Linking $@
	@$(LD) $(LDFLAGS) $^ -o $@

chili_main.so: tests_main.o out/main.o stub_command.o assert.o
	@echo Linking $@
	@$(LD) $(LDFLAGS) $^ -o $@

chili_suite.so: tests_suite.o out/suite.o
	@echo Linking $@
	@$(LD) $(LDFLAGS) $^ -o $@

chili_named.so: tests_named.o out/named.o assert.o
	@echo Linking $@
	@$(LD) $(LDFLAGS) $^ -o $@

chili_registry.so: tests_registry.o out/registry.o assert.o
	@echo Linking $@
	@$(LD) $(LDFLAGS) $^ -o $@

chili_debugger.so: tests_debugger.o out/debugger.o assert.o
	@echo Linking $@
	@$(LD) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	@echo Cleaning
	@-rm *.so -f
	@-rm *.o -f
	@-rm out -rf
