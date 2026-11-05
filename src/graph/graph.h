#ifndef AMA_GRAPH_GRAPH_H
#define AMA_GRAPH_GRAPH_H

#include "common/limits.h"

/* order_out must hold MAX_MODULES indices, with callees before callers. */
int         ensure_graph_node(const char* path);
int         build_and_validate_project_graph(int* order_out);
int         graph_has_node(const char* path);
const char* G_Graph_path(int index);

#endif
