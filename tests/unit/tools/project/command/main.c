/** @file
 * @brief Check strict borrowed command recognition and the fixed linked scaffold plan.
 * @see [T233 contract](../../../../../docs/implementation/project-tool.md)
 */
#include <assert.h>
#include <string.h>
#include "../../../../../tools/project/command/request.h"
#include "../../../../../tools/project/init/content.h"
/** Argument failure must preserve the previous request and perform no mutation. */
static void it_accepts_only_complete_explicit_initialization(void) {
    const char *args[] = {"ama-project", "init", "a directory; $(literal)", "extra"};
    ProjectInitRequest request = {.destination = "unchanged"};
    for (int count = 0; count < 5; count++) {
        bool result = project_command_request(count, args, &request);
        assert(result == (count == 3));
    }
    assert(request.destination == args[2]);
    args[1] = "compile";
    bool result = project_command_request(3, args, &request);
    assert(!result && request.destination == args[2]);
    args[1] = "init"; args[2] = "";
    result = project_command_request(3, args, &request); assert(!result);
    result = project_command_request(3, NULL, &request); assert(!result);
    args[2] = "path";
    result = project_command_request(3, args, NULL); assert(!result);
    args[0] = NULL;
    result = project_command_request(3, args, &request); assert(!result);
}
/** The immutable plan has no arbitrary executable or invented package identity. */
static void it_describes_only_three_locally_linked_guidance_files(void) {
    size_t count = 0;
    const ProjectInitFile *files = project_init_content(&count);
    assert(count == 3);
    assert(!strcmp(files[0].path, "README.md"));
    assert(!strcmp(files[1].path, "AGENTS.md"));
    assert(!strcmp(files[2].path, "packages/README.md"));
    assert(strstr(files[0].text, "[agent guidance](AGENTS.md)"));
    assert(strstr(files[0].text, "[package layout](packages/README.md)"));
    assert(strstr(files[1].text, "Static names begin with one dot"));
    assert(strstr(files[2].text, "[overview](../README.md)"));
    assert(strstr(files[2].text, "package.ini"));
    assert(strstr(files[1].text, "src must not explicitly depend on tests"));
}
/** Execute every named assertion case explicitly in the intended order. */
void run(void) {
    it_accepts_only_complete_explicit_initialization();
    it_describes_only_three_locally_linked_guidance_files();
}
/** Adapt the harness process to its explicit assertion cases and child controls. */
int main(void) { run(); return 0; }
