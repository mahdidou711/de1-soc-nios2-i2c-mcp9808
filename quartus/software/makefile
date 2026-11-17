# Adapted from the Intel FPGA Monitor Program build recipe.
# Provide the Nios II EDS root and the Monitor Program build directory.
NIOS2_EDS_ROOT ?=
MONITOR_BUILD_DIR ?=

ifneq ($(MAKECMDGOALS),CLEAN)
ifeq ($(strip $(NIOS2_EDS_ROOT)),)
$(error Set NIOS2_EDS_ROOT to the Nios II EDS installation directory)
endif
ifeq ($(strip $(MONITOR_BUILD_DIR)),)
$(error Set MONITOR_BUILD_DIR to the directory containing nios_cpp_build.ld)
endif
endif

############################################
# Global Defines
DEFINE_COMMA	:= ,

############################################
# Compilation Targets

# Programs
AS		:= nios2-elf-as
CC		:= nios2-elf-gcc
LD		:= nios2-elf-ld
OC		:= nios2-elf-objcopy
RM		:= rm -f

# Flags
USERCCFLAGS	:= -g -O1 -ffunction-sections -fverbose-asm -fno-inline -mno-cache-volatile
ARCHASFLAGS	:= --gstabs -I "$(NIOS2_EDS_ROOT)/components/altera_nios2/sdk/inc/"
ARCHCCFLAGS	:= -mno-hw-mul -mno-hw-div -I "$(NIOS2_EDS_ROOT)/components/altera_nios2/HAL/inc/" -DSYSTEM_BUS_WIDTH=32 -DALT_SINGLE_THREADED -D_JTAG_UART_BASE=268439552u
ARCHLDFLAGS	:= --defsym nasys_stack_top=0x4000000 --defsym nasys_program_mem=0x200 --defsym nasys_data_mem=0x200 --section-start .exceptions=0x20 --section-start .reset=0x0 --section-start .reset=0x0 --section-start .exceptions=0x20
ARCHLDSCRIPT	:= --script="$(MONITOR_BUILD_DIR)/nios_cpp_build.ld"
ASFLAGS		:= $(ARCHASFLAGS)
CCFLAGS		:= -Wall -c $(USERCCFLAGS) $(ARCHCCFLAGS)
LDFLAGS		:= $(patsubst %, -Wl$(DEFINE_COMMA)%, $(ARCHLDFLAGS)) -Wl$(DEFINE_COMMA)$(ARCHLDSCRIPT)
OCFLAGS		:= -O srec

# Files
HDRS		:=
SRCS		:= interval_timer.c amp/lib/src/niosII_jtag_uart.c interrupt.c main_program_timer.c sprint.c I2C_functions.c temperature.c
OBJS		:= $(patsubst %, %.o, $(SRCS))

# Targets
COMPILE: thermometer.srec

thermometer.srec: thermometer.elf
	$(RM) $@
	$(OC) $(OCFLAGS) $< $@

thermometer.elf: $(OBJS)
	$(RM) $@
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.c.o: %.c $(HDRS)
	$(RM) $@
	$(CC) $(CCFLAGS) $< -o $@

%.s.o: %.s $(HDRS)
	$(RM) $@
	$(AS) $(ASFLAGS) $< -o $@

CLEAN: 
	$(RM) thermometer.srec thermometer.elf $(OBJS)

