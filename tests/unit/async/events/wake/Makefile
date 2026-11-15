# @file
# @brief Observe actual POSIX notification credits, resource failures and check-to-sleep interleavings.
# Contract: ../../../../../docs/implementation/scheduling-rounds.md.
ROOT := $(abspath ../../../../..)
include ../sources.mk
LINKER_FLAGS += -Wl,--wrap=sem_init,--wrap=sem_destroy,--wrap=sem_post,--wrap=sem_wait
include $(ROOT)/tests/leaf.mk
