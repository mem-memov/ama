.RECIPEPREFIX := >
.DEFAULT_GOAL := all
.PHONY: all clean $(SUBDIRECTORIES)

all: $(SUBDIRECTORIES)

$(SUBDIRECTORIES):
>    $(MAKE) -C $@ all

clean:
>    $(foreach directory,$(SUBDIRECTORIES),$(MAKE) -C $(directory) clean &&) true
