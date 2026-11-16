/** @file
 * @brief Validate exact JSON scalar/byte transport with an independent decoder and borrowed-stream failure checks.
 */
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "tools/output/json/json.h"

/** Standard-library JSON decoding independently verifies grammar, Unicode/control escaping and exact numeric strings. */
static void it_round_trips_unicode_control_bytes_and_full_width_identities(void) {
    FILE *output = fopen("generated/output.json","wb");
    assert(output != NULL);
    ToolJsonWriter writer;
    tool_json_init(&writer,output);
    unsigned char text[128], bytes[256];
    for (size_t index = 0; index < 32; index++) { text[index] = (unsigned char)index; }
    const unsigned char suffix[] = "\"\\/Имя";
    size_t count = 32;
    for (size_t index = 0; index < sizeof(suffix)-1; index++) { text[count++] = suffix[index]; }
    for (size_t index = 0; index < sizeof(bytes); index++) { bytes[index] = (unsigned char)index; }
    assert(tool_json_raw(&writer,"{\"text\":"));
    assert(tool_json_text(&writer,text,count));
    assert(tool_json_raw(&writer,",\"maximum\":"));
    assert(tool_json_unsigned(&writer,UINTMAX_MAX));
    assert(tool_json_raw(&writer,",\"bytes\":"));
    assert(tool_json_bytes(&writer,bytes,sizeof(bytes)));
    assert(tool_json_raw(&writer,",\"empty\":"));
    assert(tool_json_bytes(&writer,NULL,0));
    assert(tool_json_raw(&writer,",\"flag\":"));
    assert(tool_json_boolean(&writer,true));
    assert(tool_json_raw(&writer,",\"absent\":"));
    assert(tool_json_string(&writer,NULL));
    assert(tool_json_raw(&writer,",\"boundary\":"));
    const unsigned char boundary[] = {0xf4,0x8f,0xbf,0xbf};
    assert(tool_json_text(&writer,boundary,sizeof(boundary)));
    assert(tool_json_raw(&writer,"}\n") && tool_json_finish(&writer));
    assert(fclose(output) == 0);
    char maximum[3*sizeof(uintmax_t)+3];
    int length = snprintf(maximum,sizeof(maximum),"%ju",UINTMAX_MAX);
    assert(length > 0 && (size_t)length < sizeof(maximum));
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) { execlp("python3","python3","verify.py","generated/output.json",maximum,(char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child,&status,0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Invalid UTF-8 has no lossy text fallback and writes no opening quote; subsequent writes retain that first error. */
static void it_rejects_malformed_text_without_mutating_its_bytes(void) {
    const unsigned char values[][4] = {{0xc0,0x80},{0xe2,0x82},{0xed,0xa0,0x80},{0xf4,0x90,0x80,0x80}};
    const size_t lengths[] = {2,2,3,4};
    for (size_t index = 0; index < sizeof(lengths)/sizeof(*lengths); index++) {
        FILE *output = tmpfile();
        assert(output != NULL);
        ToolJsonWriter writer;
        tool_json_init(&writer,output);
        assert(!tool_json_text(&writer,values[index],lengths[index]));
        assert(writer.status == TOOL_JSON_INVALID_UTF8 && ftell(output) == 0);
        assert(!tool_json_raw(&writer,"null") && !tool_json_finish(&writer));
        assert(writer.status == TOOL_JSON_INVALID_UTF8 && fclose(output) == 0);
    }
}
/** Neither a write nor a flush failure closes the borrowed stream or becomes success after a later encoding error. */
static void it_retains_write_and_flush_failures_without_closing_streams(void) {
    FILE *output = fopen("main.c","rb");
    assert(output != NULL);
    ToolJsonWriter writer;
    tool_json_init(&writer,output);
    assert(!tool_json_raw(&writer,"{}"));
    const unsigned char invalid[] = {0xff};
    assert(!tool_json_text(&writer,invalid,sizeof(invalid)));
    assert(writer.status == TOOL_JSON_IO_ERROR && !tool_json_finish(&writer));
    assert(fseek(output,0,SEEK_SET) == 0 && fclose(output) == 0);
    output = fopen("/dev/full","wb");
    assert(output != NULL);
    tool_json_init(&writer,output);
    (void)tool_json_raw(&writer,"{}");
    assert(!tool_json_finish(&writer) && writer.status == TOOL_JSON_IO_ERROR);
    (void)fclose(output);
}
/** Explicitly execute independent grammar/value and real stream failure contracts. */
void run(void) {
    it_round_trips_unicode_control_bytes_and_full_width_identities();
    it_rejects_malformed_text_without_mutating_its_bytes();
    it_retains_write_and_flush_failures_without_closing_streams();
}
/** The C harness owns the test lifecycle; Python is only the independent JSON decoder. */
int main(void) { run(); return EXIT_SUCCESS; }
