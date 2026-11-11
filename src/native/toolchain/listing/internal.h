#ifndef AMA_NATIVE_LISTING_INTERNAL_H
#define AMA_NATIVE_LISTING_INTERNAL_H

#include <stdbool.h>
#include "../symbols.h"

bool ama_listing_record(AmaByteView line, AmaByteView *symbol);
bool ama_listing_matches(AmaByteView symbol, AmaByteView entry);

#endif
