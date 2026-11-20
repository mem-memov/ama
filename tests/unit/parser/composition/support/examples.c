/** @file
 * @brief Exercise language-chapter examples as complete modules or explicitly wrapped body fragments.
 */
/** Canonical forms from modules, declarations, values, calls, continuations, lists and control chapters. */
static const char *const documented_forms[] = {
    "@<\namount\n//integer\n |1\n@>\nresult\n//integer\n@!\nresult\n amount\n",
    "@=\nred\n/test/fixture/v1.0.0/src/palette\n |red # data  \n@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\noutput\n input\n",
    "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@C\n",
    "@!\nis ready\n:\n handle not ready !\n",
    "@!\nis ready\n?\n start work !\n:\n report delay !\n",
    "@!\nis ready\n{\n # loop body pending\n}\ncontinue work !\n",
    "@!\na\n b !\n  condition\n  {\n   step !\n  }\n",
    "@!\nfactory !\nfactory\n next !\n  has item\n  {\n   process item !\n  }\n",
    "@!\na\n b !\n  outer condition\n  {\n   enabled\n   ?\n    c\n     d !\n      inner condition\n      {\n       step !\n      }\n   after inner !\n  }\nafter outer !\n",
    "@!\ndestination\n source\ntarget&\n &item\ncontinuation\n % original\n",
    "@!\nitems[]\n append !\n  value < item\n remove !\n  index < position\n :\n  recover !\n",
    "@!\nitems[item&]\n{\n items[]\n  remove !\n   index < position\n  append !\n   value < item&\n}\n",
    "@!\nsource&[current item&]\n{\n %\n ^\n}\n",
    "@>\nself&\n/test/fixture/v1.0.0/src/main\n &\n@!\ncopy\n source\n",
    "@!\nmodule\n method !\n  > result\n :\n  result\n  //integer\n   |0\n",
    "@!\nmodule !\n < |literal # raw  \n > new variable\n module method %\n  > other\n :\n  recover !\n:\n >\n",
    "@!\n//palette\n red >> color\nitems&[positions[next position]] !\n < &owned value\n > reference&\n",
    "@!\nmessage\n//text\n |First line\n |\n |Third line\n",
    "@!\nсообщение\n//текст\n |Привет # данные  \nобработчик !\n вход < сообщение\n результат > ответ\n",
    "@!\r\nis ready\r\n# comment gap\r\n?\r\n do work !\r\n:\r\n # intentionally empty\r\n",
    "@!\ncondition\n?\n# parent comment before alternative\n:\n >\n",
    "unfinished[\n# Drafts retain lexical checks without full body parsing\n"
};

/** Validate each chapter form structurally and then inspect every borrowed AST subtree. */
static void it_parses_documented_forms_with_exact_original_source_views(void) {
    for (size_t index = 0; index < sizeof(documented_forms) / sizeof(documented_forms[0]); index++) {
        setup_tree();
        ParseStatus status = parse_text(documented_forms[index]);
        if (status != PARSE_OK) { fprintf(stderr, "Example %zu: %s\n%s", index, module.error.diagnostic.message, documented_forms[index]); }
        assert(status == PARSE_OK && module.valid);
        check_tree(documented_forms[index]);
        destroy_tree();
    }
}
