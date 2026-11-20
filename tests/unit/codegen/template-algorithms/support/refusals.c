/** @file
 * @brief Preserve ordinary runtime refusal and reject invalid generic operation/category applications.
 */
/** Missing concrete template references refuse without running the algorithm; an
 * explicit handler handles that attempt, while the next unhandled call fails. */
static void it_preserves_missing_reference_refusal_for_concrete_algorithm_types(void) {
    algorithm_fixture();
    execute_project("@!\nmissing&\n/test/fixture/v1.0.0/src/assembly\n .integers\nshow\n/test/fixture/v1.0.0/src/showinteger\nmissing& !\n:\n show !\n  < |7\nmissing& !\n", "number=7\n",false);
    expect_diagnostic("Ama runtime failure: 7");
}
