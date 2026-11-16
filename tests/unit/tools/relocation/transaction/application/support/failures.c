/** @file
 * @brief Exercise every real application I/O checkpoint and retained originals after failed inverse operations.
 */
/** One attempt's deterministic I/O refusal authority; unrelated operations use the actual filesystem. */
typedef struct { size_t count; size_t failure; bool inverse_failure; } ApplicationFailure;
static const char old_configuration[] = "ama-library | 1\ncomponent | example.component\nabi | ama-c-1\ntarget | test\ncalling-convention | c\nexport | run | .old.a | entry\n";
static const char new_configuration[] = "ama-library | 1\ncomponent | example.component\nabi | ama-c-1\ntarget | test\ncalling-convention | c\nexport | run | .new.a | entry\n";

/** Single-ordinal failures test complete prefixes; inverse_failure refuses importer publication and every inverse rename. */
static bool application_permit(void *context, RelocationIOOperation operation, const char *physical) {
    ApplicationFailure *failure = context;
    failure->count++;
    if (failure->inverse_failure && (operation == RELOCATION_IO_RESTORE
        || (operation == RELOCATION_IO_PUBLISH && strstr(physical,"/imports/unused") != NULL))) { return false; }
    return failure->failure == 0 || failure->failure != failure->count;
}
/** Complete source, external configuration and outside file/directory output effects belong to the same plan. */
static void failure_project(const char *name) {
    prepare(name);
    folder("artifacts"); folder("artifacts/cache"); folder("artifacts/cache/nested");
    file("artifacts/cache/nested/value.bin","directory cache");
    assert(chmod(path("artifacts/cache/nested"),0555) == 0);
    assert(symlink("cycle.link",path("artifacts/cache/cycle.link")) == 0);
    file("artifacts/cache.bin","file cache");
    file("settings.library",old_configuration);
    file("project/old/generated.bin","inside cache");
    const char *outputs[] = {path("artifacts/cache"),path("artifacts/cache.bin"),path("project/old/generated.bin")};
    const char *configurations[] = {path("settings.library")};
    refresh(outputs,3,configurations,1);
}
/** Source paths, file identities and bytes are exactly restored; read-induced/change timestamps are not treated as file contents. */
static void restored(void) {
    untouched(&move_request);
    file_bytes(path("project/old/a"),(const unsigned char *)"@!\n",3);
    file_bytes(path("imports/unused"),(const unsigned char *)"@!\nvalue\n.old.a\n",16);
    file_bytes(path("settings.library"),(const unsigned char *)old_configuration,sizeof(old_configuration)-1);
    file_bytes(path("project/old/generated.bin"),(const unsigned char *)"inside cache",12);
    file_bytes(path("artifacts/cache.bin"),(const unsigned char *)"file cache",10);
    file_bytes(path("artifacts/cache/nested/value.bin"),(const unsigned char *)"directory cache",15);
    for (const RelocationStagedFile *file = staging.files; file != NULL; file = file->next) {
        struct stat metadata;
        assert(lstat(file->before,&metadata) == 0);
        assert(metadata.st_dev == file->metadata.st_dev && metadata.st_ino == file->metadata.st_ino);
        assert(metadata.st_mode == file->metadata.st_mode && metadata.st_uid == file->metadata.st_uid && metadata.st_gid == file->metadata.st_gid);
    }
    struct stat nested;
    assert(lstat(path("artifacts/cache/nested"),&nested) == 0 && (nested.st_mode&07777) == 0555);
    /* Verify the restored read-only mode first, then permit removal of this test-owned fixture. */
    assert(chmod(path("artifacts/cache/nested"),0700) == 0);
}
/** Committed state is a complete move even when retirement of private originals still requires cleanup. */
static void committed(void) {
    struct stat metadata;
    assert(lstat(preview.source,&metadata) != 0 && errno == ENOENT);
    assert(lstat(preview.destination,&metadata) == 0);
    file_bytes(path("imports/unused"),(const unsigned char *)"@!\nvalue\n.new.a\n",16);
    file_bytes(path("settings.library"),(const unsigned char *)new_configuration,sizeof(new_configuration)-1);
    assert(lstat(path("artifacts/cache"),&metadata) != 0 && errno == ENOENT);
    assert(lstat(path("artifacts/cache.bin"),&metadata) != 0 && errno == ENOENT);
}
/** Measure one successful trace, then refuse each ordinal against a fresh independent real fixture. */
static void it_refuses_every_application_io_checkpoint_without_losing_originals(void) {
    failure_project("io-measure");
    ApplicationFailure failure = {0};
    RelocationTransaction transaction;
    application(&transaction);
    RelocationTransactionRequest request = {.staging=&staging,.recovery_directory=path("recovery1"),.io={&failure,application_permit}};
    assert(relocation_transaction_apply(&transaction,&request));
    size_t measured = failure.count;
    relocation_transaction_destroy(&transaction);
    finish_applied();
    fprintf(stderr,"application I/O checkpoints: %zu\n",measured);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        char name[80];
        int length = snprintf(name,sizeof(name),"io-failure-%zu",ordinal);
        assert(length > 0 && (size_t)length < sizeof(name));
        failure_project(name);
        application(&transaction);
        size_t descriptor_baseline = descriptors();
        failure = (ApplicationFailure){.failure=ordinal};
        request = (RelocationTransactionRequest){.staging=&staging,.recovery_directory=path("recovery1"),.io={&failure,application_permit}};
        bool applied = relocation_transaction_apply(&transaction,&request);
        if (applied) { fprintf(stderr,"Unexpected application success at I/O checkpoint %zu/%zu\n",ordinal,measured); }
        assert(!applied && transaction.error.message != NULL);
        assert(descriptors() == descriptor_baseline);
        if (transaction.committed) {
            assert(transaction.state == RELOCATION_TRANSACTION_INCOMPLETE);
            committed();
            struct stat metadata;
            assert(lstat(transaction.journal,&metadata) == 0);
            failure.failure = 0;
            transaction.cleaning = true;
            assert(relocation_transaction_cleanup(&transaction));
            transaction.cleaning = false;
            relocation_transaction_destroy(&transaction);
            finish_applied();
        } else {
            if (transaction.state != RELOCATION_TRANSACTION_ROLLED_BACK) {
                fprintf(stderr,"Incomplete inverse at single I/O failure %zu/%zu: %s; cleanup=%s\n",ordinal,measured,
                    transaction.error.message,transaction.cleanup_error.message == NULL ? "none" : transaction.cleanup_error.message);
            }
            assert(transaction.state == RELOCATION_TRANSACTION_ROLLED_BACK && !transaction.decision_pending);
            restored();
            relocation_transaction_destroy(&transaction);
            done();
        }
    }
}
/** A second failure preserves saved originals and both causes; continuing the known inverse later restores exact source state. */
static void it_retains_originals_after_an_incomplete_inverse(void) {
    failure_project("incomplete-inverse");
    ApplicationFailure failure = {.inverse_failure=true};
    RelocationTransaction transaction;
    application(&transaction);
    RelocationTransactionRequest request = {.staging=&staging,.recovery_directory=path("recovery1"),.io={&failure,application_permit}};
    assert(!relocation_transaction_apply(&transaction,&request));
    assert(!transaction.committed && transaction.state == RELOCATION_TRANSACTION_INCOMPLETE);
    assert(transaction.status == RELOCATION_INCOMPLETE_TRANSACTION);
    assert(transaction.error.message != NULL && transaction.cleanup_error.message != NULL);
    struct stat metadata;
    assert(lstat(transaction.journal,&metadata) == 0 && S_ISDIR(metadata.st_mode));
    bool source_saved = false;
    for (const RelocationTransactionAction *action = transaction.first; action != NULL; action = action->next) {
        if (strcmp(action->path,preview.source) == 0) {
            assert(lstat(action->slot->backup,&metadata) == 0 && metadata.st_ino == action->original_metadata.st_ino);
            source_saved = true;
        }
    }
    assert(source_saved);
    failure.inverse_failure = false;
    transaction.cleaning = true;
    assert(relocation_transaction_rollback(&transaction));
    assert(relocation_transaction_cleanup(&transaction));
    transaction.cleaning = false;
    restored();
    relocation_transaction_destroy(&transaction);
    done();
}
