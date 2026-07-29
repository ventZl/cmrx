# ----------------------------------------------------------------------------
# cmrx makefile for adaptabuild
#
# This is designed to be included as part of a make system designed
# to be expandable and maintainable using techniques found in:
#
# Managing Projects with GNU Make - Robert Mecklenburg - ISBN 0-596-00610-1
# ----------------------------------------------------------------------------

MODULE := cmrx

MODULE_PATH := $(call make_current_module_path)
$(call log_debug,MODULE_PATH is $(MODULE_PATH))

$(MODULE)_PATH := $(MODULE_PATH)
$(call log_debug,$(MODULE)_PATH is $($(MODULE)_PATH))

# FOR THIS TO WORK THE CMRX LIBRARY MUST BE AHEAD OF ANY CMRX
# APPLICATIONS IN adaptabuild_artifacts.mak
#
CMRX_GENLINK_CMSIS := $(PYTHON) $(SRC_PATH)/$(MODULE_PATH)/ld/genlink-cmsis.py

CUSTOM_LINKER_SCRIPT := gen.$(PRODUCT).ld
CUSTOM_LINKER_SCRIPT_PATH := $(SRC_PATH)/$(PRODUCT)/config/$(MCU_VARIANT)

pre_executable::
	$(call log_warning,cmrx pre_executable)
	$(CMRX_GENLINK_CMSIS) --create $(CUSTOM_LINKER_SCRIPT_PATH)/$(MCU_LINKER_SCRIPT) $(CUSTOM_LINKER_SCRIPT_PATH)/$(CUSTOM_LINKER_SCRIPT) $(PRODUCT)

# ----------------------------------------------------------------------------
# Source file lists go here, C dependencies are automatically generated
# by the compiler using the -m option
#
# You can set up a common source path late in the file
#
# Note that each module gets its own, privately scoped variable for building
# ----------------------------------------------------------------------------

# We need both else a previous definition is used :-) Can we make this an include?

SRC_C :=
SRC_ASM :=
SRC_TEST :=

# Here is where we begin to add files to list of sources

SRC_C += src/os/kernel/access.c
SRC_C += src/os/kernel/algo.c
SRC_C += src/os/kernel/context.c
SRC_C += src/os/kernel/isr.c
SRC_C += src/os/kernel/notify.c
SRC_C += src/os/kernel/sched.c
SRC_C += src/os/kernel/signal.c
SRC_C += src/os/kernel/syscall.c
SRC_C += src/os/kernel/timer.c
SRC_C += src/os/kernel/txn.c
SRC_C += src/os/kernel/rpc.c

SRC_C += src/os/arch/arm/cortex.c
SRC_C += src/os/arch/arm/mpu.c
SRC_C += src/os/arch/arm/rpc.c
SRC_C += src/os/arch/arm/runtime.c
SRC_C += src/os/arch/arm/sanitize.c
SRC_C += src/os/arch/arm/sched.c
SRC_C += src/os/arch/arm/signal.c
SRC_C += src/os/arch/arm/static.c

SRC_C += src/lib/irq.c
SRC_C += src/lib/notify.c
SRC_C += src/lib/rpc.c
SRC_C += src/lib/signal.c
SRC_C += src/lib/std.c
SRC_C += src/lib/thread.c
SRC_C += src/lib/timer.c

SRC_C += src/extra/systick.c

# ----------------------------------------------------------------------------
# Set up the module level include path - if you have PRODUCT or MCU
# specific files that must be included first, then make sure you have
#
# $(MODULE)_INCPATH += $(PRODUCT)/config/$(MCU)
#
# early in the list of include paths.

$(MODULE)_INCPATH :=
$(MODULE)_INCPATH += $(PRODUCT)/config/$(MCU_VARIANT)
$(MODULE)_INCPATH += $(MODULE_PATH)/src/os/arch/arm/cmsis
$(MODULE)_INCPATH += $(MODULE_PATH)/src/os
$(MODULE)_INCPATH += $(MODULE_PATH)/include
$(MODULE)_INCPATH += $(MCU_INCPATH)

# ifeq (unittest,$(MAKECMDGOALS))
#   $(MODULE)_INCPATH += $(MODULE_PATH)/src
#   $(MODULE)_INCPATH += $(MODULE_PATH)/cpputest
# endif
#
# # ----------------------------------------------------------------------------
# # NOTE: The default config file must be created somehow - it is normally
# #       up to the developer to specify which defines are needed and how they
# #       are to be configured.
# #
# # By convention we place config files in $(PRODUCT)/config/$(MCU) because
# # that's an easy pace to leave things like HAL config, linker scripts etc
# #
# # This should probably be FIRST in the $(MODULE)_INCPATH in case there is
# # a conflicting file in the include path.

# $(MODULE)_INCPATH += $(PRODUCT)/config/$(MCU)

# ----------------------------------------------------------------------------
# Set any module level compile time defaults here

$(MODULE)_CDEFS += $(MCU_CDEFS)

ifeq (unittest,$(MAKECMDGOALS))
endif

#$(MODULE)_CDEFS :=
#$(MODULE)_CDEFS += CONFIG_SOC_MAX32690
#$(MODULE)_CDEFS += CMSIS_DEVICE_INCLUDE=\"wrap_max32xxx.h\"

##$(MODULE)_INCPATH += third_party/cmsis_core/Include
#$(MODULE)_INCPATH += third_party/hal_adi/MAX/Include
#$(MODULE)_INCPATH += third_party/hal_adi/MAX/Libraries/CMSIS/Device/Maxim/MAX32690/Include
# $(call log_notice,UMM_MALLOC_CFGFILE is $(UMM_MALLOC_CFGFILE))
#
# ifneq  (,$(UMM_MALLOC_CFGFILE))
#     $(MODULE)_CDEFS += UMM_MALLOC_CFGFILE=$(UMM_MALLOC_CFGFILE)
# endif

$(MODULE)_CFLAGS :=
$(MODULE)_CFLAGS += -fomit-frame-pointer

# ifeq (unittest,$(MAKECMDGOALS))
#   $(MODULE)_CDEFS +=
#   $(MODULE)_test_main := cpputest/main.o
# endif

# ----------------------------------------------------------------------------
# Include the adaptabuild library makefile - must be done for each module!

include $(ADAPTABUILD_PATH)/make/library.mak

# # # ----------------------------------------------------------------------------
# # # Include the unit test framework makefile that works for this module
# # # if the target is cpputest
# #
# # ifeq (unittest,$(MAKECMDGOALS))
# #   TESTABLE_MODULES += $(MODULE)_UNITTEST
# #   $(MODULE)_test_main := cpputest/main.o
# #   include $(ADAPTABUILD_PATH)/make/test/cpputest.mak
# # endif
#
# # ----------------------------------------------------------------------------
