# @file
# @brief Dispatch explicit independent child directories and recursive cleanup.
# Contract: ../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
.RECIPEPREFIX := >
.DEFAULT_GOAL := all
.PHONY: all clean $(SUBDIRECTORIES)

all: $(SUBDIRECTORIES)

$(SUBDIRECTORIES):
>    $(MAKE) -C $@ all

clean:
>    $(foreach directory,$(SUBDIRECTORIES),$(MAKE) -C $(directory) clean &&) true
