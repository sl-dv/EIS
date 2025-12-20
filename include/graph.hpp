#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <cmath>
#include <optional>


class Graph {
public:

    using node=int;
    using edge=std::pair<node,node>;
    using mapIntLL = std::unordered_map<node, long long>; 


    // Add undirected edge. If incremental is set, allocate new space if unseen node appears
    void addEdge(node u, node v, bool incremental=false);
    void read_from_file(const std::string& filename);

    int n() const;
    int m() const;
    size_t degree(size_t node) const;
    size_t maxdegree() const;

    int computeDegeneracy() const;
    long long ChibaNishizeki();

    long long EIS(int k, int s) const;

    long long NIS(int k) const;

    long long multipass_baseline(int k) const;
    long long countSquaresCompletedByEdge(node u,node v) const;

private:
    std::vector<std::vector<node>> _adjList;
    std::vector<edge> _edgeList;

};

#endif
