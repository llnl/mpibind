##############################################################
# Edgar A. Leon
# Lawrence Livermore National Laboratory
##############################################################
# Usage:
#   make
#   make HAVE_NVIDIA_GPUS=1
#   make HAVE_AMD_GPUS=1
#   make WITH_NUMA=1
##############################################################

CC     ?= cc
MPICC  ?= mpicc

PROGS   := mpi omp mpi+omp mem-test
OBJS    := cpu.o
CFLAGS  ?= -Wall -Werror
CPPFLAGS :=
LDFLAGS  :=
LDLIBS   :=

HOSTNAME_FULL ?= $(shell hostname)
HOSTNAME_SHORT := $(firstword $(subst ., ,$(HOSTNAME_FULL)))
MACHINE_BASE := $(shell echo $(HOSTNAME_SHORT) | sed 's/[0-9]*$$//')
MACHINE_SUFFIX := -$(MACHINE_BASE)

define COPY_MACHINE_SUFFIX
	cp -f $@ $@$(MACHINE_SUFFIX)
endef

# Auto-generate header dependencies
DEPFLAGS := -MMD -MP

# -----------------------
# Optional NUMA support
# -----------------------
ifneq ($(strip $(WITH_NUMA)),)
CPPFLAGS += -DWITH_NUMA
OBJS     += mem.o
LDLIBS   += -lnuma
endif

# -----------------------
# GPU mode selection
# -----------------------
GPU_MODE := none
ifneq ($(strip $(HAVE_AMD_GPUS)),)
GPU_MODE := amd
else ifneq ($(strip $(HAVE_NVIDIA_GPUS)),)
GPU_MODE := nvidia
endif

ifeq ($(GPU_MODE),amd)
CPPFLAGS    += -DHAVE_GPUS
OBJS        += gpu.o
HIP_LDFLAGS := -L$(shell hipconfig --path)/lib
HIP_LDLIBS  := -lamdhip64
endif

ifeq ($(GPU_MODE),nvidia)
CPPFLAGS    += -DHAVE_GPUS
OBJS        += gpu.o
CUDA_LDLIBS := -lcuda
endif

# -----------------------
# Linker command selection
# -----------------------
# Default (CPU-only)
MPI_LD      := $(MPICC)
MPI_LDFLAGS :=
MPI_LDLIBS  :=

OMP_LD      := $(CC)
OMP_LDFLAGS := -fopenmp
OMP_LDLIBS  :=

MPIOMP_LD      := $(MPICC)
MPIOMP_LDFLAGS := -fopenmp
MPIOMP_LDLIBS  :=

# AMD GPU: use hip runtime libs
ifeq ($(GPU_MODE),amd)
MPI_LDLIBS     += $(HIP_LDFLAGS) $(HIP_LDLIBS)
OMP_LDLIBS     += $(HIP_LDFLAGS) $(HIP_LDLIBS)
MPIOMP_LDLIBS  += $(HIP_LDFLAGS) $(HIP_LDLIBS)
endif

# NVIDIA GPU: use nvcc as linker driver for the GPU-containing binaries
ifeq ($(GPU_MODE),nvidia)
MPI_LD        := nvcc -ccbin $(MPICC) -Xlinker $(CUDA_LDLIBS)
MPI_LDFLAGS   :=
MPI_LDLIBS    :=

OMP_LD        := nvcc
OMP_LDFLAGS   := -Xcompiler -fopenmp
OMP_LDLIBS    :=

MPIOMP_LD      := nvcc -ccbin $(MPICC) -Xlinker $(CUDA_LDLIBS)
MPIOMP_LDFLAGS := -Xcompiler -fopenmp
MPIOMP_LDLIBS  :=
endif

# -----------------------
# Top-level targets
# -----------------------
all: $(PROGS)

.PHONY: all clean

# -----------------------
# Link rules (now one-liners)
# -----------------------
mpi: mpi.o $(OBJS)
	$(MPI_LD) $(MPI_LDFLAGS) $^ -o $@ $(LDFLAGS) $(MPI_LDLIBS) $(LDLIBS)
	$(COPY_MACHINE_SUFFIX)

omp: omp.o $(OBJS)
	$(OMP_LD) $(OMP_LDFLAGS) $^ -o $@ $(LDFLAGS) $(OMP_LDLIBS) $(LDLIBS)
	$(COPY_MACHINE_SUFFIX)

mpi+omp: mpi+omp.o $(OBJS)
	$(MPIOMP_LD) $(MPIOMP_LDFLAGS) $^ -o $@ $(LDFLAGS) $(MPIOMP_LDLIBS) $(LDLIBS)
	$(COPY_MACHINE_SUFFIX)

# Preserving your original behavior: mem-test always links -lnuma
mem-test: mem-test.o mem.o
	$(CC) $^ -o $@ -lnuma
	$(COPY_MACHINE_SUFFIX)

# -----------------------
# C compilation (pattern rule)
# -----------------------
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Target-specific overrides for compiler and flags
mpi.o mpi+omp.o: CC = $(MPICC)
omp.o mpi+omp.o: CFLAGS += -fopenmp

# -----------------------
# GPU object build + deps
# -----------------------
ifeq ($(GPU_MODE),amd)

gpu.cpp: gpu.cu
	hipify-perl $< > $@

gpu.o: gpu.cpp
	hipcc $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

endif

ifeq ($(GPU_MODE),nvidia)

gpu.o: gpu.cu
	# Mixed host/device: generate deps via host compiler flags.
	# Force deterministic gpu.d targeting gpu.o.
	nvcc --Werror all-warnings -x cu \
	     $(CPPFLAGS) \
	     -Xcompiler -MMD -Xcompiler -MP \
	     -Xcompiler -MF -Xcompiler $(@:.o=.d) \
	     -Xcompiler -MT -Xcompiler $@ \
	     -c $< -o $@

endif

# -----------------------
# Auto-include generated deps
# -----------------------
-include $(wildcard *.d)

.PHONY: help

help:
	@echo ""
	@echo "Build targets:"
	@echo "  all          Build all programs (default)"
	@echo "  mpi          Build MPI-only version"
	@echo "  omp          Build OpenMP-only version"
	@echo "  mpi+omp      Build MPI + OpenMP version"
	@echo "  mem-test     Build memory test program"
	@echo "  clean        Remove all build artifacts"
	@echo ""
	@echo "Build options (set on command line):"
	@echo "  HAVE_NVIDIA_GPUS=1   Enable NVIDIA CUDA support"
	@echo "  HAVE_AMD_GPUS=1      Enable AMD HIP support"
	@echo "  WITH_NUMA=1          Enable NUMA support (adds -DWITH_NUMA, links -lnuma)"
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make mpi"
	@echo "  make HAVE_NVIDIA_GPUS=1"
	@echo "  make HAVE_AMD_GPUS=1 mpi+omp"
	@echo "  make WITH_NUMA=1 mem-test"
	@echo ""

# Could remove the -mach copies as well
# $(addsuffix $(MACHINE_SUFFIX),$(PROGS))
clean:
	rm -f *.o *.d *~ $(PROGS) gpu.cpp
