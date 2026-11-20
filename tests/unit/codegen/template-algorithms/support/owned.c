/** @file
 * @brief Observe nested partial composition, owning copies, finite live insertion and deletion.
 */
/** A partial pair constructor closes in another producer; same-key instances retain separate lists. */
static void it_executes_nested_partial_list_composition_and_independent_instances(void) {
    algorithm_fixture();
    execute_project("@!\nnumbers[]\n//integer\n |1\n |2\n |3\nwords[]\n//text\n |alpha\n |beta\npair\n/test/fixture/v1.0.0/src/assembly\n .paired\npair !\n < numbers[]\n < words[]\n > reversed[]\n > reversed words[]\nshow number\n/test/fixture/v1.0.0/src/showinteger\nshow text\n/test/fixture/v1.0.0/src/showtext\nreversed[item&]\n{\n number value\n  % item&\n show number !\n  < number value\n}\nreversed words[item&]\n{\n text value\n  % item&\n show text !\n  < text value\n}\nfirst\n/test/fixture/v1.0.0/src/reverse-owned\nsecond\n/test/fixture/v1.0.0/src/reverse-owned\nfirst !\n < numbers[]\nsecond !\n < numbers[]\nfirst\n result[]\n  clear !\nsecond\n result[] >> retained[]\nretained[item&]\n{\n number value\n  % item&\n show number !\n  < number value\n}\nnumbers[item&]\n{\n number value\n  % item&\n show number !\n  < number value\n}\n", "number=3\nnumber=2\nnumber=1\ntext=beta\ntext=alpha\nnumber=3\nnumber=2\nnumber=1\nnumber=1\nnumber=2\nnumber=3\n",true);
}
/** Insert-before-boundary visits n originals; deleting the current entry still reaches its successor. */
static void it_mirrors_and_drains_owned_lists_with_live_boundary_correction(void) {
    algorithm_fixture();
    execute_project("@!\nvalues[]\n//integer\n |1\n |2\n |3\nmirror\n/test/fixture/v1.0.0/src/mirror\nmirror !\n < values[]\n > mirrored[]\ndrain\n/test/fixture/v1.0.0/src/drain-reverse\ndrain !\n < values[]\n > reversed[]\n > remaining\nshow\n/test/fixture/v1.0.0/src/showinteger\nmirrored[item&]\n{\n observed\n  % item&\n show !\n  < observed\n}\nreversed[item&]\n{\n observed\n  % item&\n show !\n  < observed\n}\nshow !\n < remaining\nvalues[]\n length !\n  > original\nshow !\n < original\n", "number=3\nnumber=2\nnumber=1\nnumber=1\nnumber=2\nnumber=3\nnumber=3\nnumber=2\nnumber=1\nnumber=0\nnumber=3\n",true);
}
/** Empty lists carry explicit Types; no implicit sample construction is needed. */
static void it_reverses_explicitly_typed_empty_lists_without_sample_elements(void) {
    algorithm_fixture();
    execute_project("@!\nnumbers[]\n//integer\nwords[]\n//text\npair\n/test/fixture/v1.0.0/src/assembly\n .paired\npair !\n < numbers[]\n < words[]\n > left[]\n > right[]\nleft[]\n length !\n  > size\nshow\n/test/fixture/v1.0.0/src/showinteger\nshow !\n < size\nright[]\n length !\n  > size\nshow !\n < size\n", "number=0\nnumber=0\n",true);
}

/** The tutorial entry is itself executable; observed tests above verify the same composed algorithm values. */
static void it_executes_the_exact_maintained_standalone_example(void) {
    algorithm_fixture();
    char source[8192]; example_source("main",source,sizeof(source));
    execute_project(source,"",true);
}
