/** @file
 * @brief Compile independent actual-C sizeof/alignment/offset checks against every planned nominal descriptor and declared field form.
 */
/** Emit readable test-local aliases from exact registered identities, without duplicating the generator's naming algorithm. */
static void probe_aliases(FILE *output) {
    const char *logical[] = {".main", ".plain", ".twin", ".worker", ".palette"};
    const char *aliases[] = {"ROOT", "PLAIN", "TWIN", "WORKER", "NAMESPACE"};
    for (size_t index = 0; index < 5; index++) {
        const ScaffoldType *type = layout_type(logical[index]);
        int written = fprintf(output, "#define %s_ID UINT64_C(%zu)\n", aliases[index], codegen_type_id(&assembly, type));
        assert(written > 0);
        if (!type->plan->requires_pool) { assert(index == 4); continue; }
        written = fprintf(output, "#define %s_STORAGE %s_Storage\n#define %s_TYPE %s_Type\n#define %s_BIND %s_Bind\n#define %s_SCOPE %s_Scope\n",
            aliases[index], type->names[SCAFFOLD_WRITE]->internal_name, aliases[index], type->names[SCAFFOLD_WRITE]->internal_name,
            aliases[index], type->names[SCAFFOLD_WRITE]->internal_name, aliases[index], type->names[SCAFFOLD_WRITE]->internal_name);
        assert(written > 0);
        if (index != 0) {
            written = fprintf(output, "#define %s_STATE %s_State\n#define %s_INTERFACE %s_BindInterface\n"
                "#define %s_DEFAULTS %s_defaults\n#define %s_LITERALS %s_literals\n#define %s_CLONES %s_clones\n"
                "#define %s_DESTROYS %s_destroys\n#define %s_ENTRIES %s_entries\n",
                aliases[index], type->native_prefix->internal_name, aliases[index], type->names[SCAFFOLD_WRITE]->internal_name,
                aliases[index], type->native_prefix->internal_name, aliases[index], type->native_prefix->internal_name,
                aliases[index], type->native_prefix->internal_name, aliases[index], type->native_prefix->internal_name,
                aliases[index], type->native_prefix->internal_name);
            assert(written > 0);
        }
    }
    const char *fields[] = {"first", "second", "other", "items", "reference", "references", "worker"};
    const ScaffoldType *root = layout_type(".main");
    assert(root->plan->field_count == 7);
    for (size_t index = 0; index < 7; index++) {
        const ScaffoldMember *member = layout_member(root, fields[index]);
        assert(member->field->slot == index);
        int written = fprintf(output, "#define FIELD_%s %s\n", fields[index], member->identity->internal_name);
        assert(written > 0);
    }
    int written = fprintf(output, "#define FIELD_input %s\n", layout_member(layout_type(".worker"), "input")->identity->internal_name);
    assert(written > 0);
}
/** Assert descriptors against the C compiler's real layout, independently of emitted descriptor initializer text. */
static void probe_layout_checks(FILE *output) {
    int written = fputs(
        "_Static_assert(_Alignof(PLAIN_STATE)==64 && _Alignof(TWIN_STATE)==64,\"native alignment\");\n"
        "_Static_assert(sizeof(PLAIN_STATE)==sizeof(TWIN_STATE),\"equal representation, distinct nominal types\");\n"
        "_Static_assert(sizeof(((ROOT_STORAGE*)0)->FIELD_reference)==sizeof(AmaHandle),\"reference storage\");\n"
        "_Static_assert(sizeof(((ROOT_STORAGE*)0)->FIELD_items)==sizeof(AmaHandleList),\"owning list storage\");\n"
        "_Static_assert(sizeof(((ROOT_STORAGE*)0)->FIELD_references)==sizeof(AmaHandleList),\"reference list storage\");\n"
        "static void check_layouts(AmaProjectHost *host) {\n"
        "assert(host->world.type_count==4 && host->program.type_count==5 && host->program.constant_count==2);\n"
        "assert(ama_world_type(&host->world,NAMESPACE_ID)==NULL);\n"
        "assert(host->program.types[NAMESPACE_ID-1].object==NULL);\n"
        "assert(PLAIN_ID!=TWIN_ID && &PLAIN_TYPE!=&TWIN_TYPE);\n"
        "assert(ROOT_TYPE.pool.payload_size==sizeof(ROOT_STORAGE) && ROOT_TYPE.native_offset==offsetof(ROOT_STORAGE,payload));\n"
        "assert(PLAIN_TYPE.pool.payload_size==sizeof(PLAIN_STORAGE) && PLAIN_TYPE.native_offset==offsetof(PLAIN_STORAGE,payload));\n"
        "assert(TWIN_TYPE.pool.payload_size==sizeof(TWIN_STORAGE) && TWIN_TYPE.native_offset==offsetof(TWIN_STORAGE,payload));\n"
        "assert(WORKER_TYPE.pool.payload_size==sizeof(WORKER_STORAGE) && WORKER_TYPE.native_offset==offsetof(WORKER_STORAGE,payload));\n"
        "assert(PLAIN_TYPE.pool.alignment>=_Alignof(PLAIN_STORAGE) && WORKER_TYPE.pool.alignment>=_Alignof(WORKER_STORAGE));\n"
        "assert(PLAIN_TYPE.field_count==0 && TWIN_TYPE.field_count==0 && WORKER_TYPE.field_count==1 && ROOT_TYPE.field_count==7);\n"
        "assert(WORKER_TYPE.fields[0].offset==offsetof(WORKER_STORAGE,FIELD_input) && WORKER_TYPE.fields[0].type_id==PLAIN_ID);\n",
        output);
    assert(written != EOF);
    const char *fields[] = {"first", "second", "other", "items", "reference", "references", "worker"};
    const char *kinds[] = {"AMA_FIELD_OWNED", "AMA_FIELD_OWNED", "AMA_FIELD_OWNED", "AMA_FIELD_OWNING_LIST", "AMA_FIELD_REFERENCE", "AMA_FIELD_REFERENCE_LIST", "AMA_FIELD_OWNED"};
    const char *types[] = {"PLAIN_ID", "PLAIN_ID", "TWIN_ID", "PLAIN_ID", "PLAIN_ID", "PLAIN_ID", "WORKER_ID"};
    for (size_t index = 0; index < 7; index++) {
        written = fprintf(output, "assert(ROOT_TYPE.fields[%zu].offset==offsetof(ROOT_STORAGE,FIELD_%s));\n"
            "assert(ROOT_TYPE.fields[%zu].kind==%s && ROOT_TYPE.fields[%zu].type_id==%s);\n",
            index, fields[index], index, kinds[index], index, types[index]);
        assert(written > 0);
    }
    written = fputs("}\n", output);
    assert(written != EOF);
}
