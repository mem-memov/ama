# @file
# @brief Dispatch independent project command and scaffold transaction checks.
# Contract: ../../../../docs/implementation/project-tool.md.
SUBDIRECTORIES := command initialization experiments compiled building
include ../../../parent.mk

# Keep root executable preparation ordered when the parent is run in parallel.
compiled: initialization
