# SPDX-License-Identifier: GPL-2.0
# List of programs to build
hostprogs-y += hello

# Libbpf dependencies
LIBBPF := ../libbpf_kernel/bpf.o

hello-objs := bpf_load.o $(LIBBPF) hello_user.o

# Tell kbuild to always build the programs
always := $(hostprogs-y)
always += hello_kern.o

kernel_srctree := /home/yanxw/linux-source-4.15.0
HOSTCFLAGS += -I$(kernel_srctree)/usr/include
HOSTCFLAGS += -I$(kernel_srctree)/tools/lib/
HOSTCFLAGS += -I$(kernel_srctree)/tools/testing/selftests/bpf/
HOSTCFLAGS += -I$(kernel_srctree)/tools/lib/ -I$(kernel_srctree)/tools/include
HOSTCFLAGS += -I$(kernel_srctree)/tools/perf

HOSTCFLAGS_bpf_load.o += -I$(objtree)/usr/include -Wno-unused-variable
HOSTLOADLIBES_hello += -lelf

# Allows pointing LLC/CLANG to a LLVM backend with bpf support, redefine on cmdline:
#  make samples/bpf/ LLC=~/git/llvm/build/bin/llc CLANG=~/git/llvm/build/bin/clang
LLC ?= llc
CLANG ?= clang

# Detect that we're cross compiling and use the cross compiler
ifdef CROSS_COMPILE
HOSTCC = $(CROSS_COMPILE)gcc
CLANG_ARCH_ARGS = -target $(ARCH)
endif

CC ?= gcc
# Trick to allow make to be run from this directory
$(info curdir is $(CURDIR))
all: $(LIBBPF) hello hello_kern.o

hello:
	gcc -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89  -I$(HOSTCFLAGS) -c -o hello_user.o hello_user.c
	gcc -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89  -I$(HOSTCFLAGS) -c -o bpf_load.o bpf_load.c
	gcc  -o hello bpf_load.o ../libbpf_kernel/bpf.o hello_user.o  -lelf
clean:
	$(MAKE) -C ../../ M=$(CURDIR) clean
	@find $(CURDIR) -type f -name '*~' -delete

$(LIBBPF): FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

$(obj)/syscall_nrs.s:	$(src)/syscall_nrs.c
	$(call if_changed_dep,cc_s_c)

$(obj)/syscall_nrs.h:	$(obj)/syscall_nrs.s FORCE
	$(call filechk,offsets,__SYSCALL_NRS_H__)

clean-files += syscall_nrs.h

FORCE:


# Verify LLVM compiler tools are available and bpf target is supported by llc
.PHONY: verify_cmds verify_target_bpf $(CLANG) $(LLC)

verify_cmds: $(CLANG) $(LLC)
	@for TOOL in $^ ; do \
		if ! (which -- "$${TOOL}" > /dev/null 2>&1); then \
			echo "*** ERROR: Cannot find LLVM tool $${TOOL}" ;\
			exit 1; \
		else true; fi; \
	done

verify_target_bpf: verify_cmds
	@if ! (${LLC} -march=bpf -mattr=help > /dev/null 2>&1); then \
		echo "*** ERROR: LLVM (${LLC}) does not support 'bpf' target" ;\
		echo "   NOTICE: LLVM version >= 3.7.1 required" ;\
		exit 2; \
	else true; fi

$(src)/*.c: verify_target_bpf

$(obj)/tracex5_kern.o: $(obj)/syscall_nrs.h
$(info src is $(src))
$(info obj is $(obj))
# asm/sysreg.h - inline assembly used by it is incompatible with llvm.
# But, there is no easy way to fix it, so just exclude it since it is
# useless for BPF samples.
#$(obj)/%.o: $(src)/%.c
hello_kern.o: hello_kern.c
	$(CLANG) $(NOSTDINC_FLAGS) $(LINUXINCLUDE) $(EXTRA_CFLAGS) -I$(obj) \
	$(CLANG) -nostdinc -isystem /usr/lib/gcc/x86_64-linux-gnu/7/include  \
	-I$(kernel_srctree)/arch/x86/include -I$(kernel_srctree)/arch/x86/include/generated  -I$(kernel_srctree)/include \
	-I$(kernel_srctree)/arch/x86/include/uapi -I$(kernel_srctree)/arch/x86/include/generated/uapi -I$(kernel_srctree)/include/uapi \
	-I$(kernel_srctree)/include/generated/uapi -include $(kernel_srctree)/include/linux/kconfig.h -I$(kernel_srctree)/ubuntu/include \
  	-I$(CURDIR) \
		-I$(kernel_srctree)/tools/testing/selftests/bpf/ \
		-D__KERNEL__ -Wno-unused-value -Wno-pointer-sign \
		-D__TARGET_ARCH_$(ARCH) -Wno-compare-distinct-pointer-types \
		-Wno-gnu-variable-sized-type-not-at-end \
		-Wno-address-of-packed-member -Wno-tautological-compare \
		-Wno-unknown-warning-option $(CLANG_ARCH_ARGS) \
		-O2 -emit-llvm -c $< -o -| $(LLC) -march=bpf -filetype=obj -o $@
