# Exact command manifests; values are written by Make, never shell-interpolated.
# Callers define CONFIGURATION_DIRECTORY, COMPILE_COMMAND, LINK_COMMAND,
# BINARY and OBJECTS.
# Recursive checks of the same already-built target can overlap. A live Make
# process owns a distinct candidate for each target; the shell reports its
# parent Make PID, not its own short-lived PID. See [COMP-05](../spec/implementation/compiler.md#comp-05).
configuration_process_id := $(shell printf '%s' "$$PPID")
ifeq ($(configuration_process_id),)
$(error Cannot determine the Make process identity for command manifests)
endif
CONFIGURATION_CANDIDATE = $@.$(configuration_process_id).tmp

.PHONY: force-configuration
force-configuration:

$(CONFIGURATION_DIRECTORY):
>    @mkdir -p $@

$(CONFIGURATION_DIRECTORY)/compile.configuration: CONFIGURATION_CONTENT = $(COMPILE_COMMAND)
$(CONFIGURATION_DIRECTORY)/link.configuration: CONFIGURATION_CONTENT = $(LINK_COMMAND) -o $(BINARY) $(OBJECTS)

$(CONFIGURATION_DIRECTORY)/compile.configuration $(CONFIGURATION_DIRECTORY)/link.configuration: force-configuration | $(CONFIGURATION_DIRECTORY)
>    @$(file >$(CONFIGURATION_CANDIDATE),$(CONFIGURATION_CONTENT))if test ! -e $@; then \
>        mv $(CONFIGURATION_CANDIDATE) $@; \
>    else \
>        cmp -s $(CONFIGURATION_CANDIDATE) $@; result=$$?; \
>        case $$result in \
>            0) $(RM) $(CONFIGURATION_CANDIDATE) ;; \
>            1) mv $(CONFIGURATION_CANDIDATE) $@ ;; \
>            *) exit $$result ;; \
>        esac; \
>    fi
