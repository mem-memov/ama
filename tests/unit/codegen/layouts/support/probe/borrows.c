/** @file
 * @brief Exercise generated nominal binders under a real runtime native-borrow admission scope.
 */
/** Simulate a valid current native entry metadata boundary, use actual typed views, reject an equal-layout foreign nominal handle and close authority before host disposal. */
static void probe_borrow_checks(FILE *output) {
    int written = fputs(
        "static void check_borrows(AmaProjectHost *host,ROOT_STORAGE *root) {\n"
        "AmaPoolCell *cell=NULL; check_status(ama_pool_lookup(&WORKER_TYPE.pool,root->FIELD_worker,false,&cell));\n"
        "AmaServiceState previous=cell->metadata.service; AmaHandle current=host->program.current;\n"
        "cell->metadata.service.activity=AMA_INSTANCE_RUNNING; cell->metadata.service.return_link=(AmaReturnLink){.attached=true,.host=true};\n"
        "host->program.current=root->FIELD_worker; AmaNativeBorrowScope borrow={0};\n"
        "check_status(ama_native_borrow_begin(&host->program,root->FIELD_worker,&borrow));\n"
        "WORKER_SCOPE receiver={0}; check_status(WORKER_INTERFACE(&borrow,root->FIELD_worker,&receiver));\n"
        "assert(receiver.read.payload->address==(uintptr_t)receiver.read.payload && (uintptr_t)receiver.read.payload%64==0);\n"
        "assert(receiver.read.FIELD_input->payload->number==65 && (uintptr_t)receiver.read.FIELD_input->payload%64==0);\n"
        "PLAIN_SCOPE plain={0}; check_status(PLAIN_BIND(&borrow,root->FIELD_first,&plain)); assert(plain.read.payload->number==65);\n"
        "TWIN_SCOPE wrong={0}; AmaStatus mismatch=TWIN_BIND(&borrow,root->FIELD_first,&wrong); assert(mismatch.code==AMA_TYPE_MISMATCH && wrong.read.payload==NULL);\n"
        "TWIN_SCOPE twin={0}; check_status(TWIN_BIND(&borrow,root->FIELD_other,&twin)); assert(twin.read.payload->number==66);\n"
        "check_status(ama_native_borrow_end(&borrow)); assert(!host->world.operation_active);\n"
        "cell->metadata.service=previous; host->program.current=current; }\n",
        output);
    assert(written != EOF);
}
