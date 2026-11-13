/**
 * @file
 * @brief Sort complete provider identities exactly with an allocation-free heap algorithm.
 *
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) separates
 * native provider codes without hash truncation or discovery-order dependence.
 * A max heap exposes the largest remaining key, then fixes it in the sorted suffix.
 */
#include <string.h>
#include "internal.h"

/** Compare full original key bytes, preserving roots, component boundaries and Unicode spelling. */
int native_status_provider_compare(const IdentityName *left, const IdentityName *right) {
    size_t common = left->encoding_length < right->encoding_length ? left->encoding_length : right->encoding_length;
    int compared = memcmp(left->encoding, right->encoding, common);
    if (compared != 0) { return compared; }
    return (left->encoding_length > right->encoding_length) - (left->encoding_length < right->encoding_length);
}

/** Exchange private table rows, never identities or caller inventory positions. */
static void provider_swap(NativeStatusProvider *left, NativeStatusProvider *right) {
    NativeStatusProvider value = *left;
    *left = *right;
    *right = value;
}

/** Restore a max heap down one path; each descent increases the index and lowers remaining height.
 * root<count/2 proves 2*root+1<count before multiplication, preventing index overflow.
 */
static void provider_sift(NativeStatusProvider *providers, size_t count, size_t root) {
    while (root < count / 2) {
        size_t child = root * 2 + 1;
        if (child + 1 < count && native_status_provider_compare(providers[child].name, providers[child + 1].name) < 0) { child++; }
        if (native_status_provider_compare(providers[root].name, providers[child].name) >= 0) { return; }
        provider_swap(&providers[root], &providers[child]);
        root = child;
    }
}

/** Build the heap bottom-up and grow the ascending fixed suffix one element at a time. */
void native_status_providers_order(NativeStatusProvider *providers, size_t count) {
    for (size_t start = count / 2; start != 0;) { provider_sift(providers, count, --start); }
    for (size_t end = count; end > 1;) {
        provider_swap(&providers[0], &providers[--end]);
        provider_sift(providers, end, 0);
    }
}
