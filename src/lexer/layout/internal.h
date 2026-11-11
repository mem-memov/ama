#ifndef AMA_LAYOUT_INTERNAL_H
#define AMA_LAYOUT_INTERNAL_H

#include "layout.h"

void layout_classify(LayoutLine *line, bool literal_payloads);
size_t layout_input_literal(SourceSpan text);
LayoutStatus layout_failure(SourceLayout *layout, LayoutStatus status,
    SourceSpan span, const char *message, const char *expected);

#endif
