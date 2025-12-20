#ifndef BICOLOREDGRAPH_H
#define BICOLOREDGRAPH_H

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <set>
#include <optional>

class BiColoredGraph {
public:

    using node=int;
    using mapIntLL = std::unordered_map<node, long long>; 

    BiColoredGraph(): _num0Edges(0), _num1Edges(0) {};

    void addEdge(int u, int v, int color);
    void removeEdge(int u, int v, int color);
    void removeNode(int u);
    
    int n_max() const; //the max node index after node removal
    int m(std::optional<int> color = std::nullopt) const;
    size_t degree(size_t node, std::optional<int> color = std::nullopt) const;
    long long BiColoredChibaNishizeki();

private: 
    int _num0Edges;     
    int _num1Edges;  
    std::vector<std::vector<node>> _adjList0;   // Adjacency list color0
    std::vector<std::vector<node>> _adjList1;   // Adjacency list color1


};

#endif
