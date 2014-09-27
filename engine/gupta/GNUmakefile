# Written by Jelle Geerts (jellegeerts@gmail.com).
#
# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to
# the public domain worldwide. This software is distributed without
# any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication
# along with this software.
# If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

# About this makefile:
#   - The makefile is written in GNU make syntax. You may have to use
#     the GNU make program to be able to use this makefile.
#   - The makefile has been tested to work on these systems:
#     - FreeBSD
#     - Linux
#   - The makefile can benefit from 'make -j N' (where N is an
#     integer).
#   - When no build targets are specified on the 'make' command line,
#     the 'debug' target will be built.
#   - Whether you run 'make', 'make debug', or 'make release', the
#     'clean' target is always processed, such that the output binary
#     is always built with the flags you wanted. That is, if you run
#     'make debug' and afterwards 'make release', you'll end up with a
#     release build, and vice versa. This is done to prevent accidents,
#     where one forgets to run 'make clean' before a 'make release'.
#     Instead of this approach (always forcing the 'clean' target to be
#     processed), one could put debug/release object files in a
#     different directory. That way, debug objects can never end up in
#     a release binary.

# Verbosity switch.
# One can specify VERBOSE=1 on the make command line to enable verbose
# output.
VERBOSE=0

CC ?= gcc
LD ?= ld
MAKE ?= make
RM ?= rm -f
STRIP ?= strip

OUTPUT = gupta

CFLAGS += \
	-Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings \
	-Wmissing-prototypes -Wmissing-declarations -Wredundant-decls \
	-Wnested-externs -Wstrict-prototypes -Wformat=2 -Wundef -pedantic

# "Developer mode" switch.
# If the file '_GNUmakefile-DeveloperMode' exists, it is assumed that
# one is developing code, and so wants extra compilation flags to be
# turned on, like -Werror and -pedantic-errors, such that compilation
# warnings are treated as errors.
# Non-developers such as packagers don't have to do anything, as they
# most likely won't have the file '_GNUmakefile-DeveloperMode', and so
# compilation flags like -Werror won't get in their way while
# compiling.
ifneq ("$(wildcard _GNUmakefile-DeveloperMode)","")
	CFLAGS += -Werror -pedantic-errors -Wa,--fatal-warnings
endif

# Debug build flags.
CFLAGS_DEBUG = -O1 -g -D DEBUG
LDFLAGS_DEBUG =

# Release build flags.
CFLAGS_RELEASE = -O2 -D NDEBUG
LDFLAGS_RELEASE =

CC_WRAPPER = $(CC) $(CFLAGS) $(LDFLAGS) $(INCS) $^ -c -o $@
CC_LINK_WRAPPER = $(CC) $(LDFLAGS) $^ -o $@

SRCS = \
	src/enforce.c \
	src/log.c \
	src/uassert.c \
	src/cecp/cecp.c \
	src/cecp/signal.c \
	src/cecp/stdin_io.c \
	src/engine/board.c \
	src/engine/delta_movement_info.c \
	src/engine/eval.c \
	src/engine/fen.c \
	src/engine/gupta.c \
	src/engine/move.c \
	src/engine/move_deltas.c \
	src/engine/piece.c \
	src/engine/rules.c \
	src/engine/search.c

OBJS = $(patsubst %.c,%.o,$(SRCS))

INCS = -I src

ifeq ($(VERBOSE),1)
	S=
	MAKE_VERBOSITY=
else
	# Prepend commands with an @ so 'make' doesn't echo the
	# commands.
	S=@

	# Pass -s to 'make' for silent operation.
	MAKE_VERBOSITY=-s
endif

# This is the first target, and therefore the default target 'make'
# will process.
.PHONY: debug
debug:
	$(S)$(MAKE) $(MAKE_VERBOSITY) clean
	$(S)echo Compiling debug build ...
	$(S)$(MAKE) $(MAKE_VERBOSITY) $(OUTPUT) "CFLAGS=$(CFLAGS) $(CFLAGS_DEBUG)" "LDFLAGS=$(LDFLAGS) $(LDFLAGS_DEBUG)"
	$(S)echo OK.
	@# Clean up object files, so the source directories aren't
	@# littered with them. They're removed anyway the next time we
	@# build.
	$(S)$(MAKE) $(MAKE_VERBOSITY) clean_objs

# Alias for the 'debug' target.
.PHONY: Debug
Debug: debug

.PHONY: release
release:
	$(S)$(MAKE) $(MAKE_VERBOSITY) clean
	$(S)echo Compiling release build ...
	$(S)$(MAKE) $(MAKE_VERBOSITY) $(OUTPUT) "CFLAGS=$(CFLAGS) $(CFLAGS_RELEASE)" "LDFLAGS=$(LDFLAGS) $(LDFLAGS_RELEASE)"
	$(S)$(STRIP) $(OUTPUT)
	$(S)echo OK.
	@# Clean up object files, so the source directories aren't
	@# littered with them. They're removed anyway the next time we
	@# build.
	$(S)$(MAKE) $(MAKE_VERBOSITY) clean_objs

# Alias for the 'release' target.
.PHONY: Release
Release: release

.PHONY: clean
clean: clean_objs
	$(RM) $(OUTPUT)

.PHONY: clean_objs
clean_objs:
	$(RM) $(OBJS)

%.o: %.c
	$(call CC_WRAPPER)

$(OUTPUT): $(OBJS)
	$(call CC_LINK_WRAPPER)
