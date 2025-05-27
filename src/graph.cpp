#include "graph.hpp"
#include "bicoloredGraph.hpp"
#include "tabulation_hashing.hpp"
#include "EIS_sample.hpp"
#include "basics/timer.hpp"
#include "basics/parms.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <set>
#include <queue>
#include <random>
#include <iostream>


void Graph::addEdge(node u, node v, bool incremental)
{
    if (u >= _adjList.size() || v >= _adjList.size()) {
        if (not incremental) {
            throw std::out_of_range("Node index out of range.");
        }
        _adjList.resize(std::max({u+1,v+1}));
    }
    if (u == v) {
        return;
    }
    _adjList[u].push_back(v);
    _adjList[v].push_back(u);
    _edgeList.emplace_back(u, v);
}

void Graph::read_konect(const std::string& filename) {
    // Reads a graph from a KONECT format file
    // Removes loops. Makes graph undirected.
    // Does not check for duplicate edges
    // Can handle bip, sym and asym formats

    std::ifstream infile(filename);
    if (!infile.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string line;
    bool is_bipartite = false;
    int n = -1, n_left = -1, n_right = -1, m = -1;
    int maxNode = -1;
    std::vector<std::pair<int, int>> edges;

    while (std::getline(infile, line)) {
        if (line.empty()) continue;

        if (line[0] == '%') {
            if (line.find("bip") != std::string::npos) {
                is_bipartite = true;
            }
            // Second line with graph sizes
            if (line.size() > 2 && std::isdigit(line[2])) {
                std::istringstream meta(line.substr(1)); // remove '%'
                int dummy;
                if (is_bipartite) {
                    meta >> m >> n_left >> n_right;
                    n = n_left + n_right;
                } else {
                    meta >> m >> n >> dummy;
                }
                _adjList.resize(n);
            }
            continue;
        }

        std::istringstream iss(line);
        int u, v;
        if (!(iss >> u >> v)) continue;

        u -= 1; // Convert to 0-based index
        v -= 1;

        if (is_bipartite) {
            if (n_left <= 0 || n_right <= 0) {
                throw std::runtime_error("Invalid bipartite partition sizes from header.");
            }
            v += n_left; // Offset second partition
        }

        maxNode = std::max({maxNode, u, v});
        addEdge(u, v);
    }

    infile.close();

    if (_edgeList.size()!=m)
        throw std::runtime_error("Number of edges mismatch.");

    for (auto& neighbors : _adjList) {
        std::sort(neighbors.begin(), neighbors.end());
        neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
        neighbors.shrink_to_fit();
    }
    //std::cout << "Finished IO.  n "<< n << "\tm "<< m <<std::endl;
}

int Graph::n() const {
    return _adjList.size();
}

int Graph::m() const {
    return _edgeList.size();
}

size_t Graph::degree(size_t node) const {
    return _adjList[node].size();
}
size_t Graph::maxdegree() const {
    size_t maxd = 0;
    for (size_t u = 0; u < _adjList.size(); ++u) {
        maxd = std::max(degree(u),maxd);
    }
    return maxd;
}

int Graph::computeDegeneracy() const {
    //ScopedTimer t1("Graph::computeDegeneracy");
    
    std::vector<int> degree(_adjList.size());
    std::vector<bool> removed(_adjList.size(), false);

    for (size_t u = 0; u < _adjList.size(); ++u) {
        degree[u] = _adjList[u].size();
    }

    // Min-heap to process nodes in order of degree
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> minHeap;
    for (size_t u = 0; u < degree.size(); ++u) {
        minHeap.emplace(degree[u], u);
    }

    int maxMinDegree = 0;

    // Peeling process
    while (!minHeap.empty()) {
        auto [currentDegree, u] = minHeap.top();
        minHeap.pop();

        if (removed[u]) continue;

        removed[u] = true;
        maxMinDegree = std::max(maxMinDegree, currentDegree);

        // Update degrees of neighbors
        for (int v : _adjList[u]) {
            if (!removed[v]) {
                degree[v]--;
                minHeap.emplace(degree[v], v);
            }
        }
    }

    return maxMinDegree;
}

long long Graph::ChibaNishizeki()
{
    //ScopedTimer t1("Graph::ChibaNishizeki");
    long long totalC4 = 0;

    //Sort nodes by degree
    std::vector<int> nodes(n());
    {
        std::iota(nodes.begin(), nodes.end(), 0);
        std::sort(nodes.begin(), nodes.end(), [this](int a, int b) {
            return (degree(a) > degree(b)) || (degree(a) == degree(b) && a > b);
        });
    }

    std::vector<std::vector<int>> adjListCopy = _adjList;

    mapIntLL commonneighbors;
#ifdef USE_SPARSEHASH
    commonneighbors.set_empty_key(-1); // this is needed for sparsehash map
#endif

    for (int u : nodes) {
        commonneighbors.clear();
        for (int v : adjListCopy[u]) {
            for (auto it = adjListCopy[v].begin(); it != adjListCopy[v].end();) {
                if (*it == u) {
                    it = adjListCopy[v].erase(it); // We can safely remove u already here. Erase returns it to next item.
                    continue;
                }
                commonneighbors[*it]++;
                it++;
            }
        }

        for (auto it = commonneighbors.begin(); it != commonneighbors.end(); ++it) {
            const long long w = it->second;
            if (w >= 2) {
                long long squares = (1LL *w * (w - 1)) / 2;
                totalC4 += squares;
            }
        }
    }
    return totalC4;

}

long long Graph::EIS(int k, int s) const
{
    std::random_device rd;
    std::mt19937 gen(rd());


    int reservoirsize = std::min({k/s,m()});

    std::vector<Sample> samples(s);
    
    //std::cout << "Using "<<s<<" samples of size "<<reservoirsize<<".\n";

    {
        //ScopedTimer t3("EIS::1st-pass");
        for (auto& sample : samples) {
            sample.setupReservoirSampling(reservoirsize);
        }
        for (const auto& edge : _edgeList) {
            for (auto& sample : samples) {
                sample.processForReservoirSampling(edge);
            }
        }
        for (auto& sample : samples) {
            sample.finalizeReservoirSampling();
        }
    }

    {
        //ScopedTimer t3("EIS::2nd-pass");
        //2nd Pass
        for (const auto& edge : _edgeList) {
            for (auto& sample : samples) {
                sample.collectInducedEge(edge);
            }
        }
    }
    std::vector<long long> estimates;
    for (auto& sample : samples) {
        estimates.push_back(sample.estimate());
    }

    long long mean = std::accumulate(estimates.begin(), estimates.end(), 0LL) / estimates.size();
    return mean;
}


long long Graph::NIS(int k) const
{
    //ScopedTimer t1("NIS");
    TabHash tabHash;

    uint32_t max_hash_value = std::numeric_limits<uint32_t>::max();

    auto hash = [&](node x) -> uint32_t {
        return tabHash.Simple(x);
    };

    Graph sampleGraph;
    std::unordered_map<int, int> sampleNodesReMapping;
    node nextNode = 0;

    auto getMappedNode = [&](node original) -> node {
        if (sampleNodesReMapping.find(original) == sampleNodesReMapping.end()) {
            sampleNodesReMapping[original] = nextNode++;
        }
        return sampleNodesReMapping[original];
    };


    //Store edges in a bst or heap of size k indexed by hash cutoff threshold
    std::set<std::pair<uint32_t, std::pair<node, node>>> bst;
    auto current_threshold = max_hash_value;

    //1st Pass
    for (const auto& edge : _edgeList) {
        auto [u, v] = edge;
        uint32_t score = std::max(hash(u),hash(v));

        if (score > current_threshold) 
            continue;

        bst.insert({score, edge});

        // If the BST exceeds size k, remove the edges with largest score
        if (bst.size() > k) {
            auto max_score = bst.rbegin()->first;

            // Find first element with max score
            auto it = bst.lower_bound({max_score, {0, 0}}); 
           
            // Remove all elements with the same max score
            while (it != bst.end()) {
                it = bst.erase(it);
            }
            current_threshold = max_score - 1;
        }
    }

    int collectedEdges = 0;
    for (const auto& [score, edge] : bst) {
        auto [u, v] = edge;
        int mappedU = getMappedNode(u);
        int mappedV = getMappedNode(v);
        sampleGraph.addEdge(mappedU,mappedV,true);
        collectedEdges++;
    }

    long long sampleCount=sampleGraph.ChibaNishizeki();

    double prob = std::sqrt(1.0 * collectedEdges / m());

    long long estimate = std::llround(1.0*sampleCount/prob/prob/prob/prob);
    return estimate;
}


long long Graph::multipass_baseline(int k) const
{
    //ScopedTimer t1("multipass_baseline");

    std::random_device rd;
    std::mt19937 gen(rd());


    Graph sampleGraph;
    std::unordered_map<int, int> sampleNodesReMapping;
    node nextNode = 0;

    auto getMappedNode = [&](node original) -> node {
        if (sampleNodesReMapping.find(original) == sampleNodesReMapping.end()) {
            sampleNodesReMapping[original] = nextNode++;
        }
        return sampleNodesReMapping[original];
    };


    int reservoirsize = std::min({k,m()});

    std::vector<std::pair<node,node>> sampledEdgeReservoir(reservoirsize);
    int processedEdges = 0;

    {
        //ScopedTimer t3("multipass_baseline::1st-pass");
        //1st Pass
        for (const auto& edge : _edgeList) {
            if (processedEdges < reservoirsize) {
                sampledEdgeReservoir[processedEdges] = edge;
                processedEdges++;
                if (processedEdges == reservoirsize) {
                    std::shuffle(sampledEdgeReservoir.begin(), sampledEdgeReservoir.end(), gen);
                } 
                continue;
            }
            else {
                std::uniform_int_distribution<int> dist(0, processedEdges);
                int index = dist(gen);
                if (index < reservoirsize) {
                    sampledEdgeReservoir[index] = edge;
                }
            }
            processedEdges++;
        }

        for (const auto& [u, v] : sampledEdgeReservoir) {
            int mappedU = getMappedNode(u);
            int mappedV = getMappedNode(v);
            sampleGraph.addEdge(mappedU, mappedV, true);
        }
    }

    long long sampleCount = 0;
    {
        //ScopedTimer t3("multipass_baseline::2nd-pass");
        //2nd Pass
        for (const auto& edge : _edgeList) {
            auto [u, v] = edge;
            if (not sampleNodesReMapping.contains(u) or not sampleNodesReMapping.contains(v)) continue; //not induced
            int mappedU = getMappedNode(u);
            int mappedV = getMappedNode(v);
            sampleCount += sampleGraph.countSquaresCompletedByEdge(mappedU,mappedV);
        }
    }

    double prob = 1.0 * sampleGraph.m() / m();

    long long estimate = std::llround(sampleCount/prob/prob/prob/4);

    return estimate;
}


long long Graph::countSquaresCompletedByEdge(node u,node v) const {
    //ScopedTimer t("Graph::countSquaresCompletedByEdge");

    long long count=0;
    for (int neighborU : _adjList[u]) {
        if (neighborU==v) continue;
        for (int neighborV : _adjList[v]) {
            if (neighborV==u) continue;
            if (std::find(_adjList[neighborU].begin(), _adjList[neighborU].end(), neighborV)!=_adjList[neighborU].end()) {
                count++;
            }
        }
    }
    return count;
}