/** @file
 * @brief Construct typed C inputs directly through normal ordered entry preparation.
 */
static size_t transport_constructions;
/** Borrow one explicit typed input only during fresh payload construction. */
static AmaStatus construct_transport(void *payload, const void *input) {
    transport_constructions++;
    size_t number = *(const size_t *)input;
    if (number == SIZE_MAX) { return ama_status_make(AMA_INVALID_LITERAL,NULL,0); }
    AmaStatus status = initialize_payload(payload);
    if (status.code == AMA_OK) { ((NativeValue *)payload)->number = number; }
    if (status.code==AMA_OK && number==SIZE_MAX-1) { return ama_status_make(AMA_OK,NULL,1); }
    return status;
}
/** Prepared scalars and lists select one decoder, copy their data and retain no C input borrow. */
static void it_constructs_prepared_transport_scalars_and_lists_without_default_overwrite(void) {
    setup_call();
    root_fields[2].role = AMA_PROGRAM_PARAMETER;
    size_t numbers[] = {123,456,789};
    AmaRecipe recipes[3];
    for (size_t index=0; index<3; index++) {
        recipes[index]=(AmaRecipe){.constructor=AMA_CONSTRUCT_TRANSPORT,
            .from_transport=construct_transport,.transport_input=&numbers[index]};
    }
    AmaFieldRecipe fields[] = {{.count=1,.children=recipes,.supplied=true},
        {.count=2,.children=recipes+1,.supplied=true}};
    AmaProgramInput bindings[] = {
        {.field=0,.source={.kind=AMA_SOURCE_PREPARED,.type_id=1,.prepared=&fields[0]}},
        {.field=2,.source={.kind=AMA_SOURCE_PREPARED,.type_id=2,.prepared=&fields[1]}}
    };
    invocation.inputs=bindings; invocation.input_count=2;
    bool refused=false;
    AmaStatus status=ama_program_acquire(&program,caller,&invocation,&refused);
    assert(status.code==AMA_OK && !refused);
    size_t before=constructed, copies=cloned;
    transport_constructions=0;
    status=ama_program_prepare(&program,caller,&invocation);
    assert(status.code==AMA_OK && constructed==before+8 && cloned==copies+3 && transport_constructions==3);
    RootValue *value=object_payload(target);
    assert(((LeafValue *)object_payload(value->left))->native.number==123);
    assert(((LeafValue *)object_payload(value->right))->native.number==123);
    assert(value->branches.count==2);
    assert(((RootValue *)object_payload(*ama_handle_list_element(&value->branches,0)))->native.number==456);
    assert(((RootValue *)object_payload(*ama_handle_list_element(&value->branches,1)))->native.number==789);
    numbers[0]=0; numbers[1]=0; numbers[2]=0;
    assert(((LeafValue *)object_payload(value->left))->native.number==123);
    status=ama_program_release(&program,caller); assert(status.code==AMA_OK);
    finish_call();
}
/** Decoder rejection remains a technical construction failure and cleans its private prefix. */
static void it_preserves_old_field_when_a_prepared_transport_decoder_fails(void) {
    for (unsigned malformed=0; malformed!=4; malformed++) {
        setup_call();
        size_t number=malformed==1 ? SIZE_MAX-1 : SIZE_MAX;
        AmaRecipe recipe={.constructor=AMA_CONSTRUCT_TRANSPORT,.from_transport=construct_transport,.transport_input=&number};
        AmaFieldRecipe field={.count=1,.children=&recipe,.supplied=true};
        supplied.source=(AmaProgramSource){.kind=AMA_SOURCE_PREPARED,.type_id=1,.prepared=&field};
        if (malformed==2) { supplied.source.type_id=2; }
        if (malformed==3) { supplied.source.prepared=NULL; }
        RootValue *value=object_payload(target); AmaHandle old=value->left;
        bool refused=false;
        AmaStatus status=ama_program_acquire(&program,caller,&invocation,&refused);
        assert(status.code==AMA_OK && !refused);
        transport_constructions=0;
        size_t before=destroyed;
        status=ama_program_prepare(&program,caller,&invocation);
        assert(status.code==(malformed>=2 ? AMA_TYPE_MISMATCH : malformed==1 ? AMA_INVALID_STATE : AMA_INVALID_LITERAL));
        assert(transport_constructions==(malformed>=2 ? 0 : 1) && ama_handle_equal(old,value->left));
        assert(destroyed==before+(malformed==1 ? 1 : 0));
        finish_call();
    }
}
