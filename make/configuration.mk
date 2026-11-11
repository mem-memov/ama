# Exact command manifests; values are written by Make, never shell-interpolated.
# Callers define CONFIGURATION_DIRECTORY, COMPILE_COMMAND, LINK_COMMAND,
# BINARY and OBJECTS.
.PHONY: force-configuration
force-configuration:

$(CONFIGURATION_DIRECTORY):
>    @mkdir -p $@

$(CONFIGURATION_DIRECTORY)/compile.configuration: CONFIGURATION_CONTENT = $(COMPILE_COMMAND)
$(CONFIGURATION_DIRECTORY)/link.configuration: CONFIGURATION_CONTENT = $(LINK_COMMAND) -o $(BINARY) $(OBJECTS)

$(CONFIGURATION_DIRECTORY)/compile.configuration $(CONFIGURATION_DIRECTORY)/link.configuration: force-configuration | $(CONFIGURATION_DIRECTORY)
>    @$(file >$@.tmp,$(CONFIGURATION_CONTENT))if test ! -e $@; then \
>        mv $@.tmp $@; \
>    else \
>        cmp -s $@.tmp $@; result=$$?; \
>        case $$result in \
>            0) $(RM) $@.tmp ;; \
>            1) mv $@.tmp $@ ;; \
>            *) exit $$result ;; \
>        esac; \
>    fi
