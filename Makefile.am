AUTOMAKE_OPTIONS = foreign
CFLAGS = @CFLAGS@
SHELL := /bin/bash
.ONESHELL:
MACHINE := $$(uname -m)
PACK_NAME = $(TARGET)-$(VERSION)-$(OS)-$(MACHINE)
canvil_SOURCES = src/main.c src/canvil_log.c src/command.c src/canvil_core.c src/canvil_env.c src/canvil_toml.c src/canvil_op.c src/canvil.c src/canvil_py_env.c src/canvil_py.c src/canvil_custom.c koliseo/src/koliseo.c
LDADD = $(CANVIL_LDFLAGS)
AM_LDFLAGS = -O2
AM_CFLAGS = $(CANVIL_CFLAGS) -O2 -Werror -Wpedantic -Wall
if DEBUG_BUILD
AM_LDFLAGS += -ggdb -O0
AM_CFLAGS +=
else
AM_LDFLAGS += -s
endif
if NOGIT_BUILD
AM_CFLAGS += -DCANVIL_NOGIT2
endif

koliseo/src:
	@echo -e "Generating ./koliseo/src"
	git submodule update --init koliseo
	@echo -e "Done."

koliseo/src/koliseo.c: koliseo/src

./koliseo/src/koliseo.o:
	@echo -e "    \033[1;35mAM_CFLAGS\033[0m: [ \"\033[1;34m$(AM_CFLAGS)\033[0m\" ]"
	@echo -en "Building ./koliseo/src/koliseo.o:  "
	$(CCOMP) -c ./koliseo/src/koliseo.c $(AM_CFLAGS) -o ./koliseo/src/koliseo.o
	@echo -e "\033[1;33mDone.\033[0m"


tomlc17/srcMakefile:
	@echo -e "Need makefile? Init submodule"
	git submodule update --init tomlc17
	@echo -e "Done."

tomlc17/src/libtomlc17.a: tomlc17/src/Makefile
	@echo -e "Building libtomlc17.a"
	-cd tomlc17/src && CC=$(CCOMP) make
	-cd -
	@echo -e "Done."

%.o: %.c
	$(CCOMP) -c $(CFLAGS) $(AM_CFLAGS) $< -o $@
$(TARGET): $(canvil_SOURCES:.c=.o) tomlc17/src/libtomlc17.a
	@echo -e "    AM_CFLAGS: [ $(AM_CFLAGS) ]"
	@echo -e "    LDADD: [ $(LDADD) ]"
	$(CCOMP) $(CFLAGS) $(AM_CFLAGS) $(canvil_SOURCES:.c=.o) -o $@ $(LDADD) $(AM_LDFLAGS)
clean: tomlc17/src/Makefile koliseo/Makefile
	@echo -en "Cleaning build artifacts:  "
	-rm $(TARGET)
	-rm src/*.o
	-rm static/*.o
	-cd tomlc17/src && make clean && cd -
	-cd koliseo && make clean && cd -
	@echo -e "Done."
cleanob: tomlc17/src/Makefile koliseo/Makefile
	@echo -en "Cleaning object build artifacts:  "
	-rm src/*.o
	-rm static/*.o
	-cd tomlc17/src && make clean && cd -
	-rm koliseo/src/*.o
	@echo -e "Done."
anviltest:
	@echo -en "Running anvil tests."
	./anvil -tX
	@echo -e "Done."

install:
	@echo -en "Installing canvil $(VERSION) globally as 'canvil':    "
	install $(TARGET) /usr/local/bin/canvil
	@echo -e "\033[1;33mDone.\033[0m"

uninstall:
	@echo -en "Uninstalling canvil $(VERSION) globally as 'canvil':    "
	rm /usr/local/bin/canvil
	@echo -e "\033[1;33mDone.\033[0m"

all: $(TARGET)
rebuild: clean all
.DEFAULT_GOAL := all
