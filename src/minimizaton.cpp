#include <iostream>
#include <vector>
#include <layout.h>

int o_definition(TreeNode node, int port_index, bool forward_layer_sweep) {
    int o;
    int max_port_index = node.pred.size()+node.succ.size();
    //    forward layer sweeps
    if (forward_layer_sweep) o = port_index;
        //    backwards layer sweeps
    else{
        if(port_index<=max_port_index)
            o=max_port_index-port_index+1;
        else
            o=max_port_index+node.succ.size()-port_index+1;
    }
    return o;
}

float rank_definition(Net &net, int node_index,int port_index, const std::vector<TreeNode::nodeId> &fixed_layout, bool forward_layer_sweep){
    TreeNode node = *net.getNode(fixed_layout[node_index]);
    if (forward_layer_sweep)
        return float(node_index)+float(o_definition(node, port_index,forward_layer_sweep))/float(node.succ.size()+1);
    else
        return float(node_index)+float(o_definition(node, port_index,forward_layer_sweep))/float(node.pred.size()+1);
}

void sort_node(Net &net, std::vector<TreeNode::nodeId> &layer){
    for (int i=0;i<layer.size()-1;++i){
        for (int j=0;j<layer.size()-i-1;++j){
            if (net.getNode(layer[j])->barycentric_value>net.getNode(layer[j+1])->barycentric_value){
                std::swap(layer[j], layer[j+1]);
                std::swap(net.getNode(layer[j])->x, net.getNode(layer[j+1])->x); // only for test viz
                std::swap(net.getNode(layer[j])->number, net.getNode(layer[j+1])->number);
            }
        }
    }
}

int get_amount_of_layers(std::vector<TreeNode> &nodes){
    int amount_of_layers = 0;
    for (int i=0;i<nodes.size();++i){
        if(nodes[i].layer>amount_of_layers)
            amount_of_layers=nodes[i].layer;
    }
    return amount_of_layers;
}

std::vector<std::vector<TreeNode::nodeId>>& get_net_structure(std::vector<TreeNode> &nodes){
    int amount_of_layers = get_amount_of_layers(nodes);
    std::vector<std::vector<TreeNode::nodeId>> net_structure(amount_of_layers+1);
    for (const auto &node : nodes) {
        net_structure[node.layer].push_back(node.id);
    }
    return net_structure;
}

int get_port_index(TreeNode::nodeId id, TreeNode node, bool forward_layer_sweep){
    if (forward_layer_sweep){
        for(int i=0;i<node.succ.size();++i)
            if(node.succ[i]==id)
                return i;
    }
    else{
        for(int i=0;i<node.pred.size();++i)
            if(node.pred[i]==id)
                return i;
    }
}

int cross_counting(Net &net, const std::vector<TreeNode::nodeId> &free_layout, const std::vector<TreeNode::nodeId> &fixed_layout){
    std::vector<std::vector<int>> pi_e;
    for (int i=0;i<free_layout.size();++i){
        TreeNode node = *net.getNode(free_layout[i]);
        for (int j=0;j<node.pred.size();++j){
            std::vector<int> temp (3);
            temp[0] = net.getNode(node.pred[j])->number;
            temp[1] = i;
            pi_e.push_back(temp);
        }
    }
    for (int i=0;i<pi_e.size()-1;++i)
        for (int j=0;j<pi_e.size()-1;++j)
            if (pi_e[j][0]>pi_e[j+1][0]){
                std::swap(pi_e[j], pi_e[j+1]);
            }

    std::vector<int> pi;
    for (int i=0;i<pi_e.size();++i)
        pi.push_back(pi_e[i][1]);

    int cross_num = 0;

    for (int i=0;i<pi.size()-1;++i)
        for (int j=0;j<pi.size()-1;++j)
            if (pi[j]>pi[j+1]){
                std::swap(pi[j], pi[j+1]);
                cross_num+=1;
            }
    return cross_num;
}


void layer_sweep_algorithm(Net &net, std::vector<TreeNode> &nodes){

    std::vector<std::vector<TreeNode::nodeId>> net_structure = get_net_structure(nodes);

    std::vector<std::vector<TreeNode::nodeId>> temp_net_structure(net_structure.size());
    std::copy(net_structure.begin(), net_structure.end(), temp_net_structure.begin());



    for (int i=0;i<temp_net_structure.size();++i){
        for(int j=0;j<temp_net_structure[i].size();++j){
            TreeNode *node = net.getNode(temp_net_structure[i][j]);
            for (int k=0;k<node->succ.size();++k){
                if (net.getNode(node->succ[k])->layer<node->layer){
                    node->pred.push_back(node->succ[k]);
                    node->succ.erase(node->succ.begin()+k);
                }
            }

            for (int k=0;k<node->pred.size();++k){
                if (net.getNode(node->pred[k])->layer>node->layer){
                    node->succ.push_back(node->pred[k]);
                    node->pred.erase(node->pred.begin()+k);
                }
            }
        }
    }

    while(true){
        //            forward layer sweeps
        for (int i=1;i<temp_net_structure.size();++i){
            for(int j=0;j<temp_net_structure[i].size();++j){
                TreeNode *node = net.getNode(temp_net_structure[i][j]);
                float rank = 0;
                std::vector<float> b_pred(node->pred.size());
                for (int k=0;k<node->pred.size();++k){
                    int index = net.getNode(node->pred[k])->number;
                    int port_index = get_port_index(node->id, *net.getNode(node->pred[k]), true);
                    rank+=rank_definition(net,index,port_index,temp_net_structure[i-1],true);
                }
                node->barycentric_value = rank/float(node->pred.size());
            }
            sort_node(net, temp_net_structure[i]);
        }

        int intersections_after_fls = 0;
        for (int i=1;i<temp_net_structure.size();++i)
            intersections_after_fls+=cross_counting(net,temp_net_structure[i],temp_net_structure[i-1]);

        if (net.crossings>intersections_after_fls || net.crossings==-1){
            net.crossings = intersections_after_fls;
            std::copy(temp_net_structure.begin(), temp_net_structure.end(), net_structure.begin());
        }else
            break;
        //    backwards layer sweeps
        for (int i=temp_net_structure.size()-2;i>=0;i--){
            for(int j=0;j<temp_net_structure[i].size();++j){
                TreeNode *node = net.getNode(temp_net_structure[i][j]);
                float rank = 0;
                for (int z=0;z<node->succ.size();++z){
                    int index = net.getNode(node->succ[z])->number;
                    int port_index = get_port_index(node->id, *net.getNode(node->succ[z]), false);
                    rank+=rank_definition(net,index,port_index,temp_net_structure[i+1],false);
                }
                node->barycentric_value = rank/float(node->succ.size());
            }
            sort_node(net, temp_net_structure[i]);
        }
        int intersections_after_bls = 0;
        for (int i=1;i<temp_net_structure.size();++i)
            intersections_after_bls+=cross_counting(net,temp_net_structure[i],temp_net_structure[i-1]);

        if (net.crossings>intersections_after_bls || net.crossings==-1){
            net.crossings = intersections_after_bls;
            std::copy(temp_net_structure.begin(), temp_net_structure.end(), net_structure.begin());
        }
        else
            break;
    }
    for (int i=0;i<net_structure.size();++i){
        for (int j=0;j<net_structure[i].size();++j){
            net.get_node(net_structure[i][j])->number = j;
        }
    }

    printf("\ncrosses: %i\n", net.crossings);

    //    pred
    for (int i=1;i<temp_net_structure.size();++i){
        for(int j=0;j<temp_net_structure[i].size();++j){
            TreeNode *node = net.getNode(net_structure[i][j]);
            if (node->pred.size()==0) continue;
            std::vector<float> b_pred;
            for (int k=0;k<node->pred.size();++k){
                int index = net.getNode(node->pred[k])->number;
                int port_index = get_port_index(node->id, *net.getNode(node->pred[k]),true);
                b_pred.push_back(rank_definition(net,index,port_index,temp_net_structure[i-1],true)/float(node->succ.size()+node->pred.size()));
            }
            for (int k=0;k<b_pred.size()-1;++k){
                for (int l=0;l<b_pred.size()-1;++l){
                    if (b_pred[l]>b_pred[l+1]){
                        std::swap(b_pred[l], b_pred[l+1]);
                        std::swap(node->pred[l], node->pred[l+1]);
                    }
                }
            }
        }
    }
    //    succ
    for (int i=temp_net_structure.size()-2;i>=0;i--){
        for(int j=0;j<temp_net_structure[i].size();++j){
            TreeNode *node = net.getNode(temp_net_structure[i][j]);
            std::vector<float> b_succ;
            if (node->succ.size()==0) continue;
            for (int k=0;k<node->succ.size();++k){
                int index = net.getNode(node->succ[k])->number;
                int port_index = get_port_index(node->id, *net.getNode(node->succ[k]), false);
                b_succ.push_back(rank_definition(net,index,port_index,temp_net_structure[i+1],false)/float(node->succ.size()+node->pred.size()));
            }
            for (int k=0;k<b_succ.size()-1;++k){
                for (int l=0;l<b_succ.size()-1;++l){
                    if (b_succ[l]>b_succ[l+1]){
                        std::swap(b_succ[l],b_succ[l+1]);
                        std::swap(node->succ[l],node->succ[l+1]);
                    }
                }
            }
        }
    }
}