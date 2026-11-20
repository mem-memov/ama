/** @file
 * @brief Own a closed two-binding worker, equal-key reuse and an unused partial static export.
 * @see [T187 contract](../../../../../docs/implementation/specialized-planning.md)
 */
#include "../../../compiler/common.c"

/** Load actual source/static/ordinary/planning stages; no synthetic semantic Types supply the expected plan. */
static void setup_specializations(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\nnumber\n//integer\n |7\ntext\n//text\n |hello\nfirst\n/test/fixture/v1.0.0/src/factory\n .numbers\nsecond\n/test/fixture/v1.0.0/src/factory\n .texts\nagain\n/test/fixture/v1.0.0/src/factory\n .numbers\nfirst !\n < number\n > number\nsecond !\n < text\n > text\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.numbers\n/test/fixture/v1.0.0/src/worker\n .slot < //integer\n.texts\n/test/fixture/v1.0.0/src/worker\n .slot < //text\n.unused\n/test/fixture/v1.0.0/src/worker\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/worker", "@=\nshared[]\n.slot\n@<\ninput\n.slot\n@>\noutput\n.slot\n input\n@!\nowned\n.slot\nreference&\n.slot\nitems[]\n.slot\nreferences&[]\n.slot\nitems[item&]\n{\n %\n}\n%\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    bool checked = compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main");
    if (!checked) { fprintf(stderr, "Specialized planning fixture: %s\n", frontend.error.message); }
    assert(checked && frontend.valid && frontend.plan.valid);
}

/** Find an expected fixture declaration by exact UTF-8 bytes, retaining its actual slot and semantic owner. */
static StorageField *fixture_field(PlannedType *type, const char *name) {
    size_t length = strlen(name);
    for (StorageField *field = type->first; field; field = field->next) {
        SourceSpan site = field->symbol->name;
        if (site.end - site.start == length && memcmp(source_bytes(site.source) + site.start, name, length) == 0) { return field; }
    }
    assert(!"Missing expected fixture field");
    return NULL;
}
