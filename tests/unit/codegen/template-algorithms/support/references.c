/** @file
 * @brief Observe checked alias preservation and membership-only removal in generic reference reversal.
 */
/** Reordering does not copy targets; repeated memberships remain distinct and empty input needs no sample. */
static void it_reverses_reference_memberships_without_copying_or_destroying_targets(void) {
    algorithm_fixture();
    execute_project("@!\nfirst\n//integer\n |1\nsecond\n//integer\n |2\nthird\n//integer\n |3\nreplacement\n//integer\n |9\nreferences&[]\n//integer\n & first\n & second\n & third\n & first\nreverse\n/test/fixture/v1.0.0/src/reverse-references\nreverse !\n < references&[]\n > reversed&[]\nfirst\n replacement\nshow\n/test/fixture/v1.0.0/src/showinteger\nreversed&[item&]\n{\n observed\n  % item&\n show !\n  < observed\n}\nreferences&[item&]\n{\n observed\n  % item&\n show !\n  < observed\n}\nreversed&[]\n remove !\nshow !\n < first\nempty&[]\n//integer\nreverse !\n < empty&[]\n > reversed&[]\nreversed&[]\n length !\n  > size\nshow !\n < size\n", "number=9\nnumber=3\nnumber=2\nnumber=9\nnumber=9\nnumber=2\nnumber=3\nnumber=9\nnumber=9\nnumber=0\n",true);
}
