//===----------------------------------------------------------------------===/ /
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "layout.h"

#include <iostream>
#include <vector>

int portOrderValueDefinition(TreeNode node, int portIndex, bool forwardLayerSweep) {
    int portOrderValue;
    int maxPortIndex = node.pred.size() + node.succ.size();
    if (forwardLayerSweep) {
        //    forward layer sweeps
        portOrderValue = portIndex;
    } else {
        //    backwards layer sweeps
        if (portIndex <= maxPortIndex) {
            portOrderValue = maxPortIndex - portIndex + 1;
        } else {
            portOrderValue = maxPortIndex + node.succ.size() - portIndex + 1;
        }
    }
    return portOrderValue;
}

float rankDefinition(TreeNode node, int nodeIndex, int portIndex, bool forwardLayerSweep) {
    if (forwardLayerSweep) {
        return nodeIndex + portOrderValueDefinition(node, portIndex, forwardLayerSweep) / node.succ.size() + 1;
    } else {
        return nodeIndex + portOrderValueDefinition(node, portIndex, forwardLayerSweep) / node.pred.size() + 1;
    }
}

void sortNodes(Net &net, std::vector<TreeNode::nodeId> &layer) {
    for (int i = 0; i < layer.size() - 1; ++i) {
        for (int j = 0; j < layer.size() - i - 1; ++j) {
            if (net.getNode(layer[j])->barycentricValue > net.getNode(layer[j + 1])->barycentricValue) {
                std::swap(net.getNode(layer[j])->number, net.getNode(layer[j + 1])->number);
                std::swap(layer[j], layer[j + 1]);
            }
        }
    }
}

int getAmountOfLayers(std::vector < TreeNode > &nodes) {
    int amountOfLayers = 0;
    for (int i = 0; i < nodes.size(); ++i) {
        if (nodes[i].layer > amountOfLayers)
            amountOfLayers = nodes[i].layer;
    }
    return amountOfLayers;
}

std::vector<std::vector<TreeNode::nodeId>> Net::getNodesByLayer() {
    int amountOfLayers = getAmountOfLayers(this->nodes);
    std::vector<std::vector<TreeNode::nodeId>> nodesByLayer(amountOfLayers + 1);
    for (TreeNode &node: nodes) {
        std::vector<TreeNode::nodeId> &layer = nodesByLayer[node.layer];
        layer.push_back(node.id);
        node.number = layer.size() - 1;
    }
    return nodesByLayer;
}

int getPortIndex(TreeNode::nodeId id, TreeNode node) {
    for (int i = 0; i < node.succ.size(); ++i) {
        if (node.succ[i] == id)
            return i;
    }
    for (int i = 0; i < node.pred.size(); ++i) {
        if (node.pred[i] == id)
            return i;
    }
}

int crossCounting(Net &net, std::vector<std::vector<TreeNode::nodeId>> &tempNodesByLayer) {
    int crossNum = 0;
    for (int i = 1; i < tempNodesByLayer.size(); ++i) {
        std::vector<std::vector<int>> edges;
        for (int j = 0; j < tempNodesByLayer[i].size(); ++j) {
            TreeNode node = *net.getNode(tempNodesByLayer[i][j]);
            for (int k = 0; k < node.pred.size(); ++k) {
                if (net.getNode(node.pred[k])->layer > node.layer)
                    continue;
                std::vector<int> temp(3);
                temp[0] = net.getNode(node.pred[k])->number;
                temp[1] = j;
                edges.push_back(temp);
            }
            for (int k = 0; k < node.succ.size(); ++k) {
                if (net.getNode(node.succ[k])->layer > node.layer)
                    continue;
                std::vector<int> temp(3);
                temp[0] = net.getNode(node.succ[k])->number;
                temp[1] = j;
                edges.push_back(temp);
            }
        }

        for (int j = 0; j < edges.size() - 1; ++j) {
            for (int k = 0; k < edges.size() - 1; ++k) {
                if (edges[k][0] > edges[k + 1][0])
                    std::swap(edges[k], edges[k + 1]);
            }
        }
        for (int j = 0; j < edges.size() - 1; ++j) {
            for (int k = 0; k < edges.size() - 1; ++k) {
                if (edges[k][1] > edges[k + 1][1]) {
                    std::swap(edges[k][1], edges[k + 1][1]);
                    crossNum += 1;
                }
            }
        }
    }
    return crossNum;
}

void sortPorts(Net &net, std::vector<std::vector<TreeNode::nodeId>> &nodesByLayer) {
    //    pred
    for (int i = 1; i < nodesByLayer.size(); ++i) {
        for (int j = 0; j < nodesByLayer[i].size(); ++j) {
            TreeNode *node = net.getNode(nodesByLayer[i][j]);
            if (node->pred.size() == 0)
                continue;
            std::vector<float> bPred;
            for (int k = 0; k < node->pred.size(); ++k) {
                int index = net.getNode(node->pred[k])->number;
                int portIndex = getPortIndex(node->id, *net.getNode(node->pred[k]));
                bPred.push_back(rankDefinition( *net.getNode(node->pred[k]), index, portIndex, true) /
                        float(node->succ.size() + node->pred.size()));
            }
            for (int k = 0; k < bPred.size() - 1; ++k) {
                for (int l = 0; l < bPred.size() - 1; ++l) {
                    if (bPred[l] > bPred[l + 1]) {
                        std::swap(node- pred[l], node->pred[l + 1]);
                        std::swap(bPred[l], bPred[l + 1]);
                    }
                }
            }
        }
    }
    //    succ
    for (int i = nodesByLayer.size() - 2; i >= 0; i--) {
        for (int j = 0; j < nodesByLayer[i].size(); ++j) {
            TreeNode *node = net.getNode(nodesByLayer[i][j]);
            std::vector<float> bSucc;
            if (node->succ.size() == 0)
                continue;
            for (int k = 0; k < node->succ.size(); ++k) {
                int index = net.getNode(node->succ[k])->number;
                int portIndex = getPortIndex(node->id, *net.getNode(node->succ[k]));
                bSucc.push_back(rankDefinition( *net.getNode(node->succ[k]), index, portIndex, false) /
                        float(node->succ.size() + node->pred.size()));
            }
            for (int k = 0; k < bSucc.size() - 1; ++k) {
                for (int l = 0; l < bSucc.size() - 1; ++l) {
                    if (bSucc[l] > bSucc[l + 1]) {
                        std::swap(node->succ[l], node->succ[l + 1]);
                        std::swap(bSucc[l], bSucc[l + 1]);
                    }
                }
            }
        }
    }
}

void barycentricValueDefinition(Net &net, TreeNode *node, int increment) {
    int connectionsToAdjacentLayer = 0;
    float rank = 0;
    for (int k = 0; k < node->pred.size(); ++k) {
        if (net.getNode(node->pred[k])->layer + increment != node->layer)
            continue;
        connectionsToAdjacentLayer += 1;
        int index = net.getNode(node->pred[k])->number;
        int portIndex = getPortIndex(node->id, * net.getNode(node->pred[k]));
        if (increment == 1) {
            rank += rankDefinition( *net.getNode(node->pred[k]), index, portIndex, true);
        } else {
            rank += rankDefinition( *net.getNode(node->pred[k]), index, portIndex, false);
        }
    }
    for (int k = 0; k < node->succ.size(); ++k) {
        if (net.getNode(node->succ[k])->layer + increment != node->layer)
            continue;
        connectionsToAdjacentLayer += 1;
        int index = net.getNode(node->succ[k])->number;
        int portIndex = getPortIndex(node->id, *net.getNode(node->succ[k]));
        if (increment == 1) {
            rank += rankDefinition( *net.getNode(node->pred[k]), index, portIndex, true);
        } else {
            rank += rankDefinition( *net.getNode(node->succ[k]), index, portIndex, false);
        }

    }
    node->barycentricValue = rank / connectionsToAdjacentLayer;
}

void layerSweepAlgorithm(Net &net) {
    std::vector<std::vector<TreeNode::nodeId>> nodesByLayer = net.getNodesByLayer();
    std::vector<std::vector<TreeNode::nodeId>> tempNodesByLayer(nodesByLayer.size());
    std::copy(nodesByLayer.begin(), nodesByLayer.end(), tempNodesByLayer.begin());

    int intersections = -1;
    while (true) {
        //            forward layer sweeps
        for (int i = 1; i < tempNodesByLayer.size(); ++i) {
            for (int j = 0; j < tempNodesByLayer[i].size(); ++j) {
                barycentricValueDefinition(net, net.getNode(tempNodesByLayer[i][j]), 1);
            }
            sortNodes(net, tempNodesByLayer[i]);
        }

        int intersectionsAfterFls = crossCounting(net, tempNodesByLayer);

        if (intersections > intersectionsAfterFls || intersections == -1) {
            intersections = intersectionsAfterFls;
            std::copy(tempNodesByLayer.begin(), tempNodesByLayer.end(), nodesByLayer.begin());
        } else {
            break;
        }
        //            backwards layer sweeps
        for (int i = tempNodesByLayer.size() - 2; i >= 0; i--) {
            for (int j = 0; j < tempNodesByLayer[i].size(); ++j) {
                barycentricValueDefinition(net, net.getNode(tempNodesByLayer[i][j]), -1);
            }
            sortNodes(net, tempNodesByLayer[i]);
        }
        int intersectionsAfterBls = crossCounting(net, tempNodesByLayer);

        if (intersections > intersectionsAfterBls || intersections == -1) {
            intersections = intersectionsAfterBls;
            std::copy(tempNodesByLayer.begin(), tempNodesByLayer.end(), nodesByLayer.begin());
        } else {
            break;
        }
    }

    for (int i = 0; i < nodesByLayer.size(); ++i) {
        for (int j = 0; j < nodesByLayer[i].size(); ++j) {
            net.getNode(
                    nodesByLayer[i][j])->number = j; //fix changes after the last iteration of selection of vertex order
        }
    }
    sortPorts(net, nodesByLayer);
    printf("\nintersections: %i\n", intersections);
}