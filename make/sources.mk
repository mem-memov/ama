# @file
# @brief Enumerate sorted C inputs recursively under a finite ordinary source tree.
# Contract: ../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
# Enumerate every C source under a finite, ordinary directory tree.
# Sorting makes discovery independent of directory enumeration order.
source_files = $(sort $(wildcard $(1)/*.c) $(foreach directory,$(wildcard $(1)/*/),$(call source_files,$(patsubst %/,%,$(directory)))))
