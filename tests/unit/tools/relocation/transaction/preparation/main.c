/** @file
 * @brief Exercise real private file/link preparation and exclusive rename without publishing Ama sources.
 */
#include <fcntl.h>
#include "../preflight/common.c"

/** Retire only the explicitly created primitive fixture paths; this is not the transaction rollback implementation. */
static void retire(RelocationTransactionSlot *slot, const char *link_path, const char *directory_path) {
    if (slot == NULL || !slot->created) { return; }
    struct stat metadata;
    if (slot->prepared != NULL && lstat(slot->prepared,&metadata) == 0) { assert(unlink(slot->prepared) == 0); }
    if (slot->backup != NULL && lstat(slot->backup,&metadata) == 0) { assert(unlink(slot->backup) == 0); }
    if (link_path != NULL && lstat(link_path,&metadata) == 0) { assert(unlink(link_path) == 0); }
    if (directory_path != NULL && lstat(directory_path,&metadata) == 0) { assert(rmdir(directory_path) == 0); }
    assert(rmdir(slot->directory) == 0);
}
/** Binary verification, metadata finalization and literal link preservation never inspect or execute native content. */
static void it_prepares_exact_bytes_attributes_and_literal_links(void) {
    prepare("prepared-primitives");
    size_t baseline = descriptors();
    unsigned char original[8193];
    for (size_t index = 0; index < sizeof(original); index++) { original[index] = (unsigned char)(index%256); }
    const SourceText *text = NULL, *target = NULL;
    assert(source_manager_add(&sources,"binary",original,sizeof(original),&text) == SOURCE_OK);
    assert(source_manager_add(&sources,"link",(const unsigned char *)"../missing target",17,&target) == SOURCE_OK);
    RelocationTransaction transaction;
    application(&transaction);
    RelocationTransactionSlot *slot = relocation_transaction_slot(&transaction,preview.destination);
    assert(slot != NULL && slot->created);
    struct stat expected;
    assert(lstat(path("project/packages/test/fixture/v1.0.0/src/old/a"),&expected) == 0);
    expected.st_mode = (expected.st_mode&~07777)|0640;
    struct stat copied;
    assert(relocation_transaction_file(&transaction,slot->prepared,text,&expected,&copied));
    assert(copied.st_size == (off_t)sizeof(original) && (copied.st_mode&07777) == 0640);
    assert(copied.st_uid == expected.st_uid && copied.st_gid == expected.st_gid);
    assert(copied.st_mtim.tv_sec == expected.st_mtim.tv_sec && copied.st_mtim.tv_nsec == expected.st_mtim.tv_nsec);
    const char *link_path = source_arena_join(&transaction.arena,slot->directory,"/","link");
    const char *directory_path = source_arena_join(&transaction.arena,slot->directory,"/","directory");
    assert(link_path != NULL && directory_path != NULL);
    expected.st_mode = S_IFLNK|0777;
    assert(relocation_transaction_link(&transaction,link_path,target,&expected,&copied));
    assert(S_ISLNK(copied.st_mode));
    assert(mkdir(directory_path,0700) == 0);
    expected.st_mode = S_IFDIR|0550;
    assert(relocation_transaction_directory(&transaction,directory_path,&expected,&copied));
    assert(S_ISDIR(copied.st_mode) && (copied.st_mode&07777) == 0550);
    int descriptor = open(slot->prepared,O_WRONLY);
    assert(descriptor >= 0);
    const unsigned char changed = 127;
    assert(write(descriptor,&changed,1) == 1 && close(descriptor) == 0);
    descriptor = open(slot->prepared,O_RDONLY);
    assert(descriptor >= 0);
    assert(!relocation_transaction_verify(&transaction,descriptor,slot->prepared,text));
    assert(transaction.status == RELOCATION_CHANGED_INPUT);
    assert(close(descriptor) == 0);
    retire(slot,link_path,directory_path);
    relocation_transaction_destroy(&transaction);
    assert(descriptors() == baseline);
    done();
}
/** No-replace publication succeeds only for an absent target; an existing object remains byte-identical. */
static void it_never_overwrites_an_unexpected_rename_target(void) {
    prepare("exclusive-rename");
    RelocationTransaction transaction;
    application(&transaction);
    RelocationTransactionSlot *slot = relocation_transaction_slot(&transaction,preview.destination);
    assert(slot != NULL);
    struct stat expected, copied;
    assert(lstat(path("project/packages/test/fixture/v1.0.0/src/old/a"),&expected) == 0);
    const SourceText *first = NULL, *second = NULL;
    assert(source_manager_add(&sources,"first",(const unsigned char *)"first",5,&first) == SOURCE_OK);
    assert(source_manager_add(&sources,"second",(const unsigned char *)"second",6,&second) == SOURCE_OK);
    assert(relocation_transaction_file(&transaction,slot->prepared,first,&expected,&copied));
    bool renamed = relocation_transaction_rename(&transaction,RELOCATION_IO_PUBLISH,slot->prepared,slot->backup);
    if (!renamed) { fprintf(stderr,"No-replace rename: %s, errno=%d\n",transaction.error.message,errno); }
    assert(renamed);
    assert(relocation_transaction_file(&transaction,slot->prepared,second,&expected,&copied));
    assert(!relocation_transaction_rename(&transaction,RELOCATION_IO_PUBLISH,slot->prepared,slot->backup));
    const SourceText *remaining = NULL;
    assert(source_manager_load(&sources,slot->backup,&remaining) == SOURCE_OK);
    assert(source_length(remaining) == 5 && memcmp(source_bytes(remaining),"first",5) == 0);
    assert(source_manager_load(&sources,slot->prepared,&remaining) == SOURCE_OK);
    assert(source_length(remaining) == 6 && memcmp(source_bytes(remaining),"second",6) == 0);
    retire(slot,NULL,NULL);
    relocation_transaction_destroy(&transaction);
    done();
}
/** A denied ownership operation must not change uid/gid or leak the created file descriptor. */
static bool deny_owner(void *context, RelocationIOOperation operation, const char *physical) {
    (void)context;
    assert(physical != NULL);
    return operation != RELOCATION_IO_OWNER;
}
/** Ownership preparation is refused before chown can affect the private file; original project files stay untouched. */
static void it_refuses_unavailable_prepared_ownership_before_publication(void) {
    prepare("owner-refusal");
    size_t baseline = descriptors();
    RelocationTransaction transaction;
    application(&transaction);
    RelocationTransactionSlot *slot = relocation_transaction_slot(&transaction,preview.destination);
    assert(slot != NULL);
    const SourceText *text = NULL;
    assert(source_manager_add(&sources,"ownership",(const unsigned char *)"bytes",5,&text) == SOURCE_OK);
    struct stat expected, actual;
    assert(lstat(path("project/packages/test/fixture/v1.0.0/src/old/a"),&expected) == 0);
    expected.st_uid ^= 1;
    transaction.io = (RelocationIO){NULL,deny_owner};
    assert(!relocation_transaction_file(&transaction,slot->prepared,text,&expected,&actual));
    assert(transaction.status == RELOCATION_IO_ERROR);
    assert(lstat(slot->prepared,&actual) == 0 && actual.st_uid == geteuid());
    retire(slot,NULL,NULL);
    relocation_transaction_destroy(&transaction);
    assert(descriptors() == baseline);
    done();
}
/** Explicitly exercise production preparation operations and actual kernel rename semantics. */
void run(void) {
    it_refuses_unavailable_prepared_ownership_before_publication();
    it_prepares_exact_bytes_attributes_and_literal_links();
    it_never_overwrites_an_unexpected_rename_target();
}
/** Every primitive fixture retires its private paths and descriptors before process exit. */
int main(void) { run(); return EXIT_SUCCESS; }
