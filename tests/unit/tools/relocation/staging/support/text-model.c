/** @file
 * @brief Exhaust a finite independent edit algebra including adjacent spans, contractions and boundary translation.
 */
/** Compare cursor-based production rewriting with direct formatted slices and signed mathematical displacement. */
static void it_matches_the_complete_two_span_text_model(void) {
    setup("text-model");
    const SourceText *source = NULL;
    const char original[] = "abcdef";
    assert(source_manager_add(&sources,"original",(const unsigned char *)original,6,&source) == SOURCE_OK);
    const char *replacements[] = {"","X","long replacement"};
    for (size_t first = 0; first < 6; first++) {
        for (size_t first_end = first+1; first_end <= 6; first_end++) {
            for (size_t second = first_end; second < 6; second++) {
                for (size_t second_end = second+1; second_end <= 6; second_end++) {
                    for (size_t a = 0; a < 3; a++) {
                        for (size_t b = 0; b < 3; b++) {
                            RelocationPathEdit left = {.original={source,first,first_end},.replacement=replacements[a]};
                            RelocationPathEdit right = {.original={source,second,second_end},.replacement=replacements[b]};
                            const RelocationPathEdit *edits[] = {&left,&right};
                            char expected[80];
                            int length = snprintf(expected,sizeof(expected),"%.*s%s%.*s%s%s",
                                (int)first,original,left.replacement,(int)(second-first_end),original+first_end,
                                right.replacement,original+second_end);
                            assert(length >= 0 && (size_t)length < sizeof(expected));
                            RelocationStaging staging;
                            relocation_staging_init(&staging,sources.allocator);
                            const SourceText *rewritten = NULL;
                            assert(relocation_staging_rewrite(&staging,source,"final",edits,2,&rewritten));
                            exact(rewritten,expected);
                            for (size_t position = 0; position <= 6; position++) {
                                size_t mapped;
                                bool valid = relocation_staging_offset(source,edits,2,position,&mapped);
                                bool interior = (position > first && position < first_end)
                                    || (position > second && position < second_end);
                                assert(valid == !interior);
                                if (valid) {
                                    long displacement = (long)position;
                                    if (position >= first_end) { displacement += (long)strlen(left.replacement)-(long)(first_end-first); }
                                    if (position >= second_end) { displacement += (long)strlen(right.replacement)-(long)(second_end-second); }
                                    assert(displacement >= 0 && mapped == (size_t)displacement && mapped <= source_length(rewritten));
                                }
                            }
                            relocation_staging_destroy(&staging);
                        }
                    }
                }
            }
        }
    }
    RelocationStaging empty;
    relocation_staging_init(&empty,sources.allocator);
    RelocationPathEdit whole = {.original={source,0,6},.replacement=""};
    const RelocationPathEdit *whole_edits[] = {&whole};
    const SourceText *empty_text = NULL;
    assert(relocation_staging_rewrite(&empty,source,"empty",whole_edits,1,&empty_text));
    exact(empty_text,"");
    size_t final_boundary;
    assert(relocation_staging_offset(source,whole_edits,1,6,&final_boundary) && final_boundary == 0);
    relocation_staging_destroy(&empty);
    RelocationPathEdit overlap[] = {{.original={source,1,4},.replacement="x"},
        {.original={source,3,5},.replacement="y"}};
    const RelocationPathEdit *edits[] = {&overlap[0],&overlap[1]};
    RelocationStaging staging;
    relocation_staging_init(&staging,sources.allocator);
    const SourceText *unchanged = source;
    assert(!relocation_staging_rewrite(&staging,source,"final",edits,2,&unchanged));
    assert(staging.status == RELOCATION_REFERENCE_CONFLICT && unchanged == source);
    relocation_staging_destroy(&staging);
    finish();
}
