/** @file
 * @brief Execute descriptor lookup and typed payload access across over-aligned pool growth, shared values and all field forms.
 */
/** Read real storage only after nominal/liveness validation; growth must retain every live address and field identity. */
static void probe_value_checks(FILE *output) {
    int written = fputs(
        "static ROOT_STORAGE *check_values(AmaProjectHost *host) {\n"
        "AmaPoolCell *root_cell=NULL; check_status(ama_pool_lookup(&ROOT_TYPE.pool,host->root,false,&root_cell));\n"
        "ROOT_STORAGE *root=ama_pool_payload(&ROOT_TYPE.pool,root_cell);\n"
        "assert(ama_handle_equal(root->FIELD_reference,root->FIELD_first));\n"
        "assert(root->FIELD_references.count==2 && ama_handle_equal(root->FIELD_references.items[0],root->FIELD_first));\n"
        "assert(ama_handle_equal(root->FIELD_references.items[1],root->FIELD_second) && root->FIELD_items.count==17);\n"
        "assert(PLAIN_TYPE.pool.live_count==20 && PLAIN_TYPE.pool.block_count>=3);\n"
        "AmaPoolCell *first=NULL; check_status(ama_pool_lookup(&PLAIN_TYPE.pool,root->FIELD_first,false,&first));\n"
        "PLAIN_STORAGE *storage=ama_pool_payload(&PLAIN_TYPE.pool,first);\n"
        "assert(storage->payload.number==65 && storage->payload.address==(uintptr_t)&storage->payload);\n"
        "assert((uintptr_t)storage%_Alignof(PLAIN_STORAGE)==0 && (uintptr_t)&storage->payload%64==0);\n"
        "for(size_t index=0;index<17;index++) { AmaPoolCell *cell=NULL;\n"
        "check_status(ama_pool_lookup(&PLAIN_TYPE.pool,root->FIELD_items.items[index],false,&cell));\n"
        "PLAIN_STORAGE *item=ama_pool_payload(&PLAIN_TYPE.pool,cell);\n"
        "assert(item->payload.number==68 && item->payload.address==(uintptr_t)&item->payload && (uintptr_t)item%64==0); }\n"
        "AmaHandle extra[19]; for(size_t index=0;index<19;index++) check_status(ama_object_create(&host->world,PLAIN_ID,NULL,&extra[index]));\n"
        "AmaPoolCell *same=NULL; check_status(ama_pool_lookup(&PLAIN_TYPE.pool,root->FIELD_first,false,&same));\n"
        "assert(same==first && ama_pool_payload(&PLAIN_TYPE.pool,same)==storage && storage->payload.number==65);\n"
        "for(size_t index=0;index<19;index++) check_status(ama_object_destroy(&host->world,extra[index]));\n"
        "for(size_t index=0;index<2;index++) { const AmaConstantValue *constant=host->program.constants[index].storage->items;\n"
        "const PLAIN_STATE *value=constant->payload; assert(value->number==67 && value->address==(uintptr_t)value && (uintptr_t)value%64==0); }\n"
        "assert(PLAIN_LITERALS==19 && PLAIN_CLONES==3 && PLAIN_DEFAULTS==20 && PLAIN_DESTROYS==20);\n"
        "assert(WORKER_DEFAULTS==1 && WORKER_ENTRIES==1 && TWIN_LITERALS==1 && TWIN_DEFAULTS==0);\n"
        "return root; }\n",
        output);
    assert(written != EOF);
}
