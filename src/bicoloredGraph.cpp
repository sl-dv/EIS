#include "bicoloredGraph.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include <set>
#include <unordered_map>
#include <vector>
#include <numeric>

#include <iostream>
#include "basics/timer.hpp"
#include "basics/parms.hpp"


void BiColoredGraph::addEdge(int u, int v, int color)
{

    if (u >= _adjList0.size() || v >= _adjList0.size()) {
        _adjList0.resize(std::max({u+1,v+1}));
        _adjList1.resize(std::max({u+1,v+1}));
    }
    if (color==0) {
        _adjList0[u].push_back(v);
        _adjList0[v].push_back(u); 
        _num0Edges++;
    }
    else {
        _adjList1[u].push_back(v);
        _adjList1[v].push_back(u); 
        _num1Edges++;
    }
}


void BiColoredGraph::removeEdge(int u, int v, int color) {
    if (color == 0) {
        _adjList0[u].erase(std::remove(_adjList0[u].begin(), _adjList0[u].end(), v), _adjList0[u].end());
        _adjList0[v].erase(std::remove(_adjList0[v].begin(), _adjList0[v].end(), u), _adjList0[v].end());
        _num0Edges--;
    } else {
        _adjList1[u].erase(std::remove(_adjList1[u].begin(), _adjList1[u].end(), v), _adjList1[u].end());
        _adjList1[v].erase(std::remove(_adjList1[v].begin(), _adjList1[v].end(), u), _adjList1[v].end());
        _num1Edges--;
    }
}

void BiColoredGraph::removeNode(int u) {
    for (int v : _adjList0[u]) {
        _adjList0[v].erase(std::remove(_adjList0[v].begin(), _adjList0[v].end(), u), _adjList0[v].end());
        _num0Edges--;
    }
    for (int v : _adjList1[u]) {
        _adjList1[v].erase(std::remove(_adjList1[v].begin(), _adjList1[v].end(), u), _adjList1[v].end());
        _num1Edges--;
    }
    _adjList0[u].clear();
    _adjList1[u].clear();
}


int BiColoredGraph::n_max() const {
    return _adjList0.size();
}

int BiColoredGraph::m(std::optional<int> color) const {
    if (color.has_value()) {
        return color.value() == 0 ? _num0Edges : _num1Edges;
    }
    return _num0Edges+ _num1Edges;
}

size_t BiColoredGraph::degree(size_t node, std::optional<int> color) const{
    if (color.has_value()) {
        return color.value() == 0 ? _adjList0[node].size() : _adjList1[node].size();
    }
    return _adjList0[node].size()+ _adjList1[node].size();
}

long long BiColoredGraph::BiColoredChibaNishizeki()
{
    //ScopedTimer t1("BiColoredChibaNishizeki");
    long long totalC4 = 0;

    //Any edge (u,v) can exist in 2 colors 0 and 1.
    //This is ChibaNishizeki, but counting two types of wedges: 0-1-wedges and 1-0-wedges.
    //We only count squares with alternatingly colored edges.
    //We do not want to count 4Tours i.e. abcb
    //This function returns the number of colored squares, i.e. up to twice the number of squares in the underlying simple graph


    //Sort nodes by degree
    std::vector<int> nodes(n_max());
    {
        std::iota(nodes.begin(), nodes.end(), 0);
        std::sort(nodes.begin(), nodes.end(), [this](int a, int b) {
            return (degree(a) > degree(b)) || (degree(a) == degree(b) && a > b);
        });
    }

    std::vector<std::vector<int>> adjList0Copy = _adjList0;
    std::vector<std::vector<int>> adjList1Copy = _adjList1;

    mapIntSet wedges01;
    mapIntSet wedges10;

#ifdef USE_SPARSEHASH
    wedges01.set_empty_key(-1);
    wedges10.set_empty_key(-1);
#endif

    for (int u : nodes) {
        wedges01.clear();
        wedges10.clear();
        for (int v : adjList0Copy[u]) {
            for (auto it = adjList1Copy[v].begin(); it != adjList1Copy[v].end();) {
                if (*it == u) {
                    it = adjList1Copy[v].erase(it); // We can safely remove u already here. Erase returns iterator to next item.
                    continue;
                }
                int w = *it;
                wedges01[w].emplace(v);
                ++it;
            }
            adjList0Copy[v].erase(std::remove(adjList0Copy[v].begin(), adjList0Copy[v].end(), u), adjList0Copy[v].end());
        }
        for (int v : adjList1Copy[u]) {
            for (auto it = adjList0Copy[v].begin(); it != adjList0Copy[v].end();) {
                if (*it == u) {
                    it = adjList0Copy[v].erase(it); // We can safely remove u already here. Erase returns iterator to next item.
                    continue;
                }
                int w = *it;
                wedges10[w].emplace(v);
                ++it;
            }
            adjList1Copy[v].erase(std::remove(adjList1Copy[v].begin(), adjList1Copy[v].end(), u), adjList1Copy[v].end());
        }

        for (auto& [w, nodes01] : wedges01) {
            std::set<node> & nodes10 = wedges10[w];
            std::set<node> intersection;
            std::set_intersection(
                nodes01.begin(), nodes01.end(),
                nodes10.begin(), nodes10.end(),
                std::inserter(intersection, intersection.begin())
            );

            //This is the number of bicolored squares. Each square can have 2 bicolorings.
            totalC4 += 1LL * nodes01.size()*nodes10.size() - intersection.size();
        }  
    }

   return totalC4;

}