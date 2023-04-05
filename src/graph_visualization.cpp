#include "graph_visualization.h"

int main() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow("SDL Tree", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        fprintf(stderr, "SDL_Window Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(
            window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        fprintf(stderr, "SDL_Renderer Error: %s\n", SDL_GetError());
        return 1;
    }

    std::vector<std::vector<int>> input = {};
    input_graph(input);
    std::vector<tree_node> nodes = {};
    id_succ_init(input, nodes);

    transformation_graph(nodes);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].is_dummy == false) {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            SDL_RenderDrawPoint(renderer, nodes[i].x, nodes[i].y);
            SDL_RenderDrawPoint(renderer, nodes[i].x + 1, nodes[i].y + 1);
            SDL_RenderDrawPoint(renderer, nodes[i].x - 1, nodes[i].y + 1);
            SDL_RenderDrawPoint(renderer, nodes[i].x + 1, nodes[i].y - 1);
            SDL_RenderDrawPoint(renderer, nodes[i].x - 1, nodes[i].y - 1);
            SDL_RenderDrawPoint(renderer, nodes[i].x + 1, nodes[i].y);
            SDL_RenderDrawPoint(renderer, nodes[i].x - 1, nodes[i].y);
            SDL_RenderDrawPoint(renderer, nodes[i].x, nodes[i].y + 1);
            SDL_RenderDrawPoint(renderer, nodes[i].x, nodes[i].y - 1);
        } else {
            SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
            SDL_RenderDrawPoint(renderer, nodes[i].x + 1, nodes[i].y);
            SDL_RenderDrawPoint(renderer, nodes[i].x - 1, nodes[i].y);
            SDL_RenderDrawPoint(renderer, nodes[i].x, nodes[i].y + 1);
            SDL_RenderDrawPoint(renderer, nodes[i].x, nodes[i].y - 1);
        }
        for (int j = 0; j < nodes[i].succ.size(); j++) {
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            tree_node succ_node = get_tree_node_by_id(nodes, nodes[i].succ[j]);
            SDL_RenderDrawLine(renderer, nodes[i].x, nodes[i].y, succ_node.x + 2, succ_node.y + 2);
        }
    }

    SDL_RenderPresent(renderer);

    SDL_Event event;
    int quit = 1;
    while (quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 0;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

tree_node get_tree_node_by_id(std::vector<tree_node> &nodes, tree_node::node_id id) {
    tree_node node = {};
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].id == id) {
            node = nodes[i];
            break;
        }
    }
    return node;
}

void input_graph(std::vector<std::vector<int>> &input) {
    for (int i = 0; 1; i++) {
        int input_num;
        std::vector<int> in = {};
        input.push_back(in);
        while (1) {
            scanf("%i", &input_num);
            if (input_num == 0 || input_num == -1) {
                break;
            }
            input[i].push_back(input_num);
        }
        if (input_num == -1) {
            break;
        }
    }
}

void id_succ_init(std::vector<std::vector<int>> &input, std::vector<tree_node> &nodes) {
    for (int i = 0; i < input.size(); i++) {
        tree_node node = {};
        nodes.push_back(node);
        nodes[i].id = input[i][0];
        for (int j = 1; j < input[i].size(); j++) {
            int num = 0;
            nodes[i].succ.push_back(num);
            nodes[i].succ[j - 1] = input[i][j];
        }
    }
}

void pred_reinit(std::vector<tree_node> &nodes) {
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i].pred = {};
    }

    for (int i = 0; i < nodes.size(); i++) {
        for (int j = 0; j < nodes[i].succ.size(); j++) {
            for (int k = 0; k < nodes.size(); k++) {
                if (nodes[k].id == nodes[i].succ[j]) {
                    nodes[k].pred.push_back(nodes[i].id);
                }
            }
        }
    }
}

void deg_reinit(std::vector<tree_node> &nodes) {
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i].deg_p = 0;
        nodes[i].deg_m = 0;
    }

    for (int i = 0; i < nodes.size(); i++) {
        for (int j = 0; j < nodes[i].succ.size(); j++) {
            nodes[i].deg_p++;

            for (int k = 0; k < nodes.size(); k++) {
                if (nodes[k].id == nodes[i].succ[j]) {
                    nodes[k].deg_m++;
                }
            }
        }
    }
}

void print_info(std::vector<tree_node> &nodes) {
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].is_dummy == true) {
            printf("№%i (dummy)| number %i, layer %i, dp %i, dm %i", nodes[i].id, nodes[i].number, nodes[i].layer, nodes[i].deg_p, nodes[i].deg_m);
        } else {
            printf("№%i| number %i, layer %i, dp %i, dm %i", nodes[i].id, nodes[i].number, nodes[i].layer, nodes[i].deg_p, nodes[i].deg_m);
        }
        printf("\nsucc: ");
        for (int j = 0; j < nodes[i].succ.size(); j++) {
            printf("%i ", nodes[i].succ[j]);
        }
        printf("\npred: ");
        for (int j = 0; j < nodes[i].pred.size(); j++) {
            printf("%i ", nodes[i].pred[j]);
        }
        printf("\n");
    }
}

void DFS(int &exit, std::vector<tree_node> &nodes, std::vector<bool> &used_id, tree_node::node_id id, tree_node::node_id prev_id) {
    if (exit == 0) {
        used_id[id - 1] = true;

        int numb = -1;
        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i].id == id) {
                numb = i;
                break;
            }
        }

        for (int i = 0; i < nodes[numb].succ.size(); i++) {
            if (!used_id[nodes[numb].succ[i] - 1]) {
                DFS(exit, nodes, used_id, nodes[numb].succ[i], id);
            } else {
                if (id != prev_id) {
                    printf("Graph has cycle. \n");
                    exit = 1;
                    break;
                }
            }
        }
    }
}

void DFS_cycle_exists(std::vector<tree_node> &nodes) {
    std::vector<bool> used_id = {};
    for (int i = 0; i < nodes.size(); i++) {
        used_id.push_back(false);
    }
    int exit = 0;
    for (int i = 0; i < nodes.size(); i++) {
        if (exit == 1) {
            break;
        }
        if (!used_id[nodes[i].id - 1]) {
            DFS(exit, nodes, used_id, nodes[i].id, -1);
        }
    }
}

void remove_incident_arcs(std::vector<tree_node> &nodes, tree_node::node_id id) {
    for (int i = 0; i < nodes.size(); i++) {
        for (int j = 0; j < nodes[i].succ.size(); j++) {
            if (nodes[i].succ[j] == id) {
                nodes[i].succ.erase(nodes[i].succ.begin() + j);
            }
        }
    }

    for (int i = 0; i < nodes.size(); i++) {
        for (int j = 0; j < nodes[i].pred.size(); j++) {
            if (nodes[i].pred[j] == id) {
                nodes[i].pred.erase(nodes[i].pred.begin() + j);
            }
        }
    }
}

void heapify(std::vector<tree_node> &nodes, int n, int i) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;

    if (l < n && nodes[l].id > nodes[largest].id) {
        largest = l;
    }

    if (r < n && nodes[r].id > nodes[largest].id) {
        largest = r;
    }

    if (largest != i) {
        std::swap(nodes[i], nodes[largest]);
        heapify(nodes, n, largest);
    }
}

void topology_heapSort(std::vector<tree_node> &nodes) {
    int n = 0;
    for (int i = 0; i < nodes.size(); i++) {
        n++;
    }

    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(nodes, n, i);
    }

    for (int i = n - 1; i >= 0; i--) {
        std::swap(nodes[0], nodes[i]);
        heapify(nodes, i, 0);
    }
}

void greedyFAS(std::vector<tree_node> &nodes, std::vector<std::pair<tree_node::node_id, tree_node::node_id>> &deleted_edges) {
    topology_heapSort(nodes);
    deg_reinit(nodes);

    std::vector<tree_node> copy_nodes = nodes;
    std::vector<tree_node> s1 = {}, s2 = {};

    while (nodes.size() != 0) {
        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i].deg_p == 0) {
                s2.push_back(copy_nodes[nodes[i].id - 1]);

                remove_incident_arcs(nodes, nodes[i].id);
                nodes.erase(nodes.begin() + i);
                deg_reinit(nodes);

                i = -1;
            }
        }

        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i].deg_m == 0) {
                s1.push_back(copy_nodes[nodes[i].id - 1]);

                remove_incident_arcs(nodes, nodes[i].id);
                nodes.erase(nodes.begin() + i);
                deg_reinit(nodes);

                i = -1;
            }
        }

        if (nodes.size() != 0) {
            int max = nodes[0].deg_p - nodes[0].deg_m;
            int pos_max = 0;
            for (int i = 0; i < nodes.size(); i++) {
                if (nodes[i].deg_p - nodes[i].deg_m > max) {
                    max = nodes[i].deg_p - nodes[i].deg_m;
                    pos_max = i;
                }
            }
            tree_node max_node = get_tree_node_by_id(copy_nodes, nodes[pos_max].id);
            s1.push_back(max_node);

            remove_incident_arcs(nodes, nodes[pos_max].id);
            nodes.erase(nodes.begin() + pos_max);
            deg_reinit(nodes);
        }
    }

    std::vector<tree_node> s = {};
    for (int i = 0; i < s1.size(); i++) {
        s.push_back(s1[i]);
    }
    for (int i = 0; i < s2.size(); i++) {
        s.push_back(s2[s2.size() - 1 - i]);
    }

    for (int i = 0; i < s.size(); i++) {
        for (int j = 0; j < s[i].succ.size(); j++) {
            for (int k = 0; k < i; k++) {
                if (s[i].succ[j] == s[k].id) {
                    std::pair<tree_node::node_id, tree_node::node_id> edge = {s[i].id, s[i].succ[j]};
                    deleted_edges.push_back(edge);

                    s[i].succ.erase(s[i].succ.begin() + j);
                }
            }
        }
    }

    nodes = s;
}

bool comparator_positions(tree_node first, tree_node second) {
    return first.number < second.number;
}

void ASAP(std::vector<tree_node> &nodes) {
    pred_reinit(nodes);
    std::vector<tree_node> copy_nodes = nodes;

    std::vector<tree_node::node_id> rm_id = {};
    for (int i = 0; copy_nodes.size() != 0; i++) {
        for (int j = 0; j < copy_nodes.size(); j++) {
            if (copy_nodes[j].pred.size() == 0) {
                rm_id.push_back(copy_nodes[j].id);
            }
        }

        std::vector<tree_node> layer = {};
        for (int j = 0; j < nodes.size(); j++) {
            for (int k = 0; k < rm_id.size(); k++) {
                if (nodes[j].id == rm_id[k]) {
                    nodes[j].number = k;
                    nodes[j].layer = i;
                    layer.push_back(nodes[j]);
                }
            }
        }
        positions.push_back(layer);

        for (int j = 0; j < copy_nodes.size(); j++) {
            for (int k = 0; k < rm_id.size(); k++) {
                if (copy_nodes[j].id == rm_id[k]) {
                    remove_incident_arcs(copy_nodes, copy_nodes[j].id);
                    copy_nodes.erase(copy_nodes.begin() + j);

                    j = -1;
                    break;
                }
            }
        }
        rm_id.clear();
    }

    for (int i = 0; i < positions.size(); i++) {
        sort(positions[i].begin(), positions[i].end(), comparator_positions);
    }
}

void add_dummy_node(std::vector<tree_node> &nodes, tree_node up, tree_node down, int order) {
    tree_node dummy = {};
    int count_id = 0;
    for (int i = 0; i < nodes.size(); i++) {
        count_id++;
    }
    dummy.succ.push_back(down.id);
    dummy.pred.push_back(up.id);
    int max = -1;
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].id > max) {
            max = nodes[i].id;
        }
    }
    dummy.id = max + 1;
    dummy.layer = up.layer + order;
    dummy.number = positions[dummy.layer].back().number + 1;
    positions[dummy.layer].push_back(dummy);
    dummy.is_dummy = true;

    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].id == up.id) {
            nodes[i].succ.push_back(dummy.id);
            for (int j = 0; j < nodes[i].succ.size(); j++) {
                if (nodes[i].succ[j] == down.id) {
                    nodes[i].succ.erase(nodes[i].succ.begin() + j);
                    break;
                }
            }
            break;
        }
    }

    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].id == down.id) {
            nodes[i].pred.push_back(dummy.id);
            for (int j = 0; j < nodes[i].pred.size(); j++) {
                if (nodes[i].pred[j] == up.id) {
                    nodes[i].pred.erase(nodes[i].pred.begin() + j);
                    break;
                }
            }
            break;
        }
    }

    nodes.push_back(dummy);
}

void add_dummy_nodes(std::vector<tree_node> &nodes, std::vector<std::pair<tree_node::node_id, tree_node::node_id>> &deleted_edges) {
    int flag = 1;
    while (flag) {
        flag = 0;
        for (int i = 0; i < nodes.size(); i++) {
            for (int j = 0; j < nodes[i].succ.size(); j++) {
                tree_node succ_node = get_tree_node_by_id(nodes, nodes[i].succ[j]);
                if (nodes[i].layer + 1 < succ_node.layer) {
                    add_dummy_node(nodes, nodes[i], succ_node, 1);
                    flag = 1;
                    break;
                }
            }
            if (flag == 1) {
                break;
            }
        }
    }

    for (int i = 0; i < deleted_edges.size(); i++) {
        tree_node first_node = get_tree_node_by_id(nodes, deleted_edges[i].first);
        tree_node second_node = get_tree_node_by_id(nodes, deleted_edges[i].second);
        
        if (first_node.layer == second_node.layer + 1) {
            for (int j = 0; j < nodes.size(); j++) {
                if (nodes[j].id == first_node.id) {
                    nodes[j].succ.push_back(second_node.id);
                }
                if (nodes[j].id == second_node.id) {
                    nodes[j].pred.push_back(first_node.id);
                }
            }
        } else {
            int count_dummy = first_node.layer - (second_node.layer + 1);
            add_dummy_node(nodes, first_node, second_node, -1);
            for (int j = 1; j < count_dummy; j++) {
                add_dummy_node(nodes, nodes.back(), second_node, -1);
            }
        }
    }
}

void coord_init(std::vector<tree_node> &nodes) {
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i].y = (nodes[i].layer + 1) * DISTANCE;
        nodes[i].x = (nodes[i].number + 1) * DISTANCE;
    }
}

void transformation_graph(std::vector<tree_node> &nodes) {
    std::vector<std::pair<tree_node::node_id, tree_node::node_id>> deleted_edges = {};
    greedyFAS(nodes, deleted_edges);
    
    ASAP(nodes);
    
    add_dummy_nodes(nodes, deleted_edges);
    
    coord_init(nodes);
}

const std::vector<tree_node::node_id> Net::get_sources() {
    std::vector<tree_node::node_id> ids_sources = {};
    deg_reinit(nodes);
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].deg_m == 0) {
            ids_sources.push_back(nodes[i].id);
        }
    }
    return ids_sources;
}

const std::vector<tree_node::node_id> Net::get_sinks() {
    std::vector<tree_node::node_id> ids_sinks = {};
    deg_reinit(nodes);
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].deg_p == 0) {
            ids_sinks.push_back(nodes[i].id);
        }
    }
    return ids_sinks;
}

const std::vector<tree_node::node_id> Net::get_successors(tree_node::node_id id) {
    std::vector<tree_node::node_id> ids_successors = {};
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].id == id) {
            ids_successors = nodes[i].succ;
            break;
        }
    }
    return ids_successors;
}

const std::vector<tree_node::node_id> Net::get_predecessors(tree_node::node_id id) {
    std::vector<tree_node::node_id> ids_predecessors = {};
    for (int i = 0; i < nodes.size(); i++) {
        for (int j = 0; j < nodes[i].succ.size(); j++) {
            if (nodes[i].succ[j] == id) {
                ids_predecessors.push_back(nodes[i].id);
            }
        }
    }
    return ids_predecessors;
}

const tree_node Net::get_node(tree_node::node_id id) {
    tree_node node = {};
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].id == id) {
            node = nodes[i];
            break;
        }
    }
    return node;
}
