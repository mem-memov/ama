/** @file
 * @brief Accept real static, shared and explicitly unloaded component compositions.
 */
#include "common.c"
#include "host.c"
#include "failures.c"
#include "combined.c"
#include "templates.c"
/** Static and shared linking both keep overlapping runtime/native identities isolated. */
static void it_packages_two_components_with_overlapping_module_names(void) {
    setup_test(); prepare_static_facades();
    PackageNames first,second;
    package_component("first.component","first","/test/fixture/v1.0.0/src/textfacade",&first,false,facade_package_rejections);
    package_component("second.component","second","/test/fixture/v1.0.0/src/integerfacade",&second,true,facade_package_contract);
    assert(strcmp(first.component,second.component)!=0);
    host_part("generated/first.c","first",&first,"first",false);
    host_part("generated/second.c","second",&second,"second",false);
    combined_host(&first,&second);
    file("generated/host.c","int first(void); int second(void); int combined(void); int main(void) { return first() || second() || combined(); }\n");
    link_hosts("generated/first/library.a","generated/second/library.a","generated/static-host");
    link_hosts("generated/first/library.so","generated/second/library.so","generated/shared-host");
    host_part("generated/first.c","first",&first,"first",true);
    host_part("generated/second.c","second",&second,"second",true);
    file("generated/host.c","int first(void); int second(void); int main(void) { return first() || second(); }\n");
    link_hosts(NULL,NULL,"generated/loader-host");
    destroy_test();
}
/** Register real multi-translation-unit host linking, not symbol-name-only assertions. */
void run(void) {
    it_checks_tool_status_and_exact_captured_argument_bytes();
    it_packages_two_components_with_overlapping_module_names();
}
/** Return after both separately linked host executables have run. */
int main(void) { run(); return EXIT_SUCCESS; }
