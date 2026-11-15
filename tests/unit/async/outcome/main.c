/** @file
 * @brief Exhaust the cooperative terminal algebra and failure-preserved output contract.
 */
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#define AMA_NATIVE(name) outcome_test_##name
#include "../../../../stdlib/modules/task/outcome+/native/selection.h"
#undef AMA_NATIVE

/** Pending may remain pending or commit once; both terminal states absorb every later request. */
static void it_commits_once_and_preserves_every_terminal_decision(void) {
    for (unsigned state = 0; state < 3; state++) {
        for (unsigned cancel = 0; cancel < 2; cancel++) {
            for (unsigned complete = 0; complete < 2; complete++) {
                unsigned selected = UINT_MAX;
                bool accepted = outcome_test_select(state, cancel, complete, &selected);
                assert(accepted);
                if (state != 0) { assert(selected == state); }
                else if (cancel != 0) { assert(selected == 1); }
                else if (complete != 0) { assert(selected == 2); }
                else { assert(selected == 0); }
                unsigned repeated = UINT_MAX;
                accepted = outcome_test_select(selected, cancel, complete, &repeated);
                assert(accepted && repeated == selected);
            }
        }
    }
    unsigned state = 0;
    bool accepted = outcome_test_select(state, 0, 1, &state);
    assert(accepted && state == 2);
    accepted = outcome_test_select(state, 1, 0, &state);
    assert(accepted && state == 2);
    state = 0;
    accepted = outcome_test_select(state, 1, 0, &state);
    assert(accepted && state == 1);
    accepted = outcome_test_select(state, 0, 1, &state);
    assert(accepted && state == 1);
}

/** Invalid state/input/pointer fails before writing the output, including aliased state/output use. */
static void it_rejects_invalid_transitions_without_changing_the_destination(void) {
    const unsigned invalid[] = {3, UINT_MAX};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        unsigned selected = 91;
        bool accepted = outcome_test_select(invalid[index], 0, 0, &selected);
        assert(!accepted && selected == 91);
        accepted = outcome_test_select(0, invalid[index], 0, &selected);
        assert(!accepted && selected == 91);
        accepted = outcome_test_select(0, 0, invalid[index], &selected);
        assert(!accepted && selected == 91);
    }
    unsigned selected = 91;
    bool accepted = outcome_test_select(0, SIZE_MAX, 0, &selected);
    assert(!accepted && selected == 91);
    accepted = outcome_test_select(0, 0, SIZE_MAX, &selected);
    assert(!accepted && selected == 91);
    accepted = outcome_test_select(0, 0, 0, NULL);
    assert(!accepted);
}

/** Run all finite transitions and explicit out-of-domain cases. */
void run(void) {
    it_commits_once_and_preserves_every_terminal_decision();
    it_rejects_invalid_transitions_without_changing_the_destination();
}

/** Return success only after the transition table and absorbing-state laws hold. */
int main(void) { run(); return 0; }
