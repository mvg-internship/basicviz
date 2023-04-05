#ifndef GRAPH_VISUALIZATION_H_
#define GRAPH_VISUALIZATION_H_

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

enum lib_constants { SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480, DISTANCE = 40 };

struct tree_node {
    using node_id = int;
    std::vector<node_id> succ = {};
    std::vector<node_id> pred = {};
    node_id id = 0;
    int deg_p = 0;
    int deg_m = 0;
    int layer = 0;
    int number = 0;
    bool is_dummy = false;
    int x = 0;
    int y = 0;
};

std::vector<std::vector<tree_node>> positions = {};

tree_node get_tree_node_by_id(std::vector<tree_node> &nodes, tree_node::node_id id);

void input_graph(std::vector<std::vector<int>> &input);
void id_succ_init(std::vector<std::vector<int>> &input, std::vector<tree_node> &nodes);
void pred_reinit(std::vector<tree_node> &nodes);
void deg_reinit(std::vector<tree_node> &nodes);
void print_info(std::vector<tree_node> &nodes);

void DFS(int &exit, std::vector<tree_node> &nodes, std::vector<bool> &used_id, tree_node::node_id id, tree_node::node_id prev_id);
void DFS_cycle_exists(std::vector<tree_node> &nodes);

void remove_incident_arcs(std::vector<tree_node> &nodes, tree_node::node_id id);

void heapify(std::vector<tree_node> &nodes, int n, int i);
void topology_heapSort(std::vector<tree_node> &nodes);
void greedyFAS(std::vector<tree_node> &nodes, std::vector<std::pair<tree_node::node_id, tree_node::node_id>> &deleted_edges);

bool comparator_positions(tree_node first, tree_node second);
void ASAP(std::vector<tree_node> &nodes);

void add_dummy_node(std::vector<tree_node> &nodes, tree_node up, tree_node down, int order);
void add_dummy_nodes(std::vector<tree_node> &nodes, std::vector<std::pair<tree_node::node_id, tree_node::node_id>> &deleted_edges);

void coord_init(std::vector<tree_node> &nodes);

void transformation_graph(std::vector<tree_node> &nodes);

struct Net {
    using node_id = tree_node::node_id;
    std::vector<tree_node> nodes;

    const std::vector<node_id> get_sources();
    const std::vector<node_id> get_sinks();

    const std::vector<node_id> get_successors(node_id id);
    const std::vector<node_id> get_predecessors(node_id id);

    const tree_node get_node(node_id id);
};

#endif // GRAPH_VISUALIZATION_H_

