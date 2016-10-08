CC=gcc
CFLAGS=-std=gnu99 -Wall -fPIC -I../../src -shared -fno-stack-protector
LD=ld
LDFLAGS=-shared
SOURCES=$(wildcard ../../src/*.c)
OBJECTS=$(SOURCES:../../src/%.c=out/%.o)
DEPS=$(OBJECTS:%.o=%.d)
COMPILING=
CHILI=../../chili -i

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
	COMPILING = Compiling $< for debug
else
	CFLAGS += -O
	COMPILING = Compiling $<
endif

# Default target
.PHONY: run
run: out chili_run.so
	@echo Running unit tests
	@$(CHILI) ./chili_run.so

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

chili_run.so: tests_run.o out/run.o out/redirect.o
	@echo Linking $@
	@$(LD) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	@echo Cleaning
	@-rm *.so -f
	@-rm *.o -f
	@-rm out -rf