#ifndef EIS_SAMPLE_HPP
#define EIS_SAMPLE_HPP

#include <unordered_map>
#include <vector>
#include <random>
#include "bicoloredGraph.hpp"

struct Sample {
    using node = int;
    using edge = std::pair<node, node>;

    Sample();
    void setupReservoirSampling(int s);
    void processForReservoirSampling(edge edge);
    void finalizeReservoirSampling();
    void collectInducedEge(edge edge);
    long long estimate();

private:
    BiColoredGraph graph;
    std::unordered_map<int, int> nodeMapping;
    std::vector<std::pair<node, node>> reservoir;
    node nextNode = 0;
    int processedEdges = 0;
    std::random_device rd;
    std::minstd_rand gen;
    int space;
    int removedNodes = 0;
    int removedsampledEdges = 0;
    int streamsize = 0;
    inline node getMappedNode(node original) {
        auto it = nodeMapping.find(original);
        if (it == nodeMapping.end()) {
            nodeMapping[original] = nextNode++;
            return nextNode - 1;
        }
        return it->second;
    }
};
#endif // EIS_SAMPLE_HPP