CC ?= cc
GHDL ?= ghdl
PYTHON ?= python3
CFLAGS ?= -O2

FIRMWARE := quartus/software
HOST_TEST := build/host/test_temperature
GHDL_WORK := build/ghdl

.PHONY: all test test-vhdl figures clean

all: $(HOST_TEST)

$(HOST_TEST): tests/test_temperature.c $(FIRMWARE)/temperature.c $(FIRMWARE)/temperature.h $(FIRMWARE)/I2C.h
	mkdir -p "$(@D)"
	$(CC) $(CPPFLAGS) $(CFLAGS) -std=c99 -Wall -Wextra -Werror -pedantic -I$(FIRMWARE) $(filter %.c,$^) $(LDFLAGS) $(LDLIBS) -o "$@"

test: $(HOST_TEST)
	"$(HOST_TEST)"
	$(MAKE) test-vhdl

test-vhdl:
	@if command -v "$(GHDL)" >/dev/null 2>&1; then \
	    set -e; \
	    mkdir -p "$(GHDL_WORK)"; \
	    "$(GHDL)" -a --std=08 --work=altera_mf --workdir="$(GHDL_WORK)" tests/altera_mf_components.vhd; \
	    "$(GHDL)" -s --std=08 --workdir="$(GHDL_WORK)" -P"$(GHDL_WORK)" quartus/DE1_SoC_Basic_Computer.vhd quartus/sdram_pll.vhd; \
	    echo "PASS: GHDL syntax checks for the top level and SDRAM PLL wrapper"; \
	else \
	    echo "SKIP: GHDL unavailable; VHDL syntax checks not run"; \
	fi

figures:
	$(PYTHON) scripts/generate_figures.py

clean:
	$(RM) -r build
