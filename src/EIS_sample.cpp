#include "EIS_sample.hpp"
#include <iostream>

Sample::Sample() : gen(rd()) {}

void Sample::setupReservoirSampling(int s) {
    space = s;
    reservoir.resize(space);
    nodeMapping.clear();
}

void Sample::processForReservoirSampling(edge edge) {
    if (processedEdges < reservoir.size()) {
        reservoir[processedEdges] = edge;
        processedEdges++;
        if (processedEdges == reservoir.size()) {
            std::shuffle(reservoir.begin(), reservoir.end(), gen);
        }
    } else {
        std::uniform_int_distribution<int> dist(0, processedEdges);
        int index = dist(gen);
        if (index < reservoir.size()) {
            reservoir[index] = edge;
        }
        processedEdges++;
    }
    streamsize++;
}

void Sample::finalizeReservoirSampling() {
    for (const auto& [u, v] : reservoir) {
        int mappedU = getMappedNode(u);
        int mappedV = getMappedNode(v);
        graph.addEdge(mappedU, mappedV, 0);
    }
}

void Sample::collectInducedEge(edge edge) {
    auto [u, v] = edge;
    if (not nodeMapping.contains(u) or not nodeMapping.contains(v)) return; // not induced
    int mappedU = getMappedNode(u);
    int mappedV = getMappedNode(v);
    if (graph.degree(mappedU, 0) > 0 and graph.degree(mappedV, 0) > 0) {
        // induced edge
        graph.addEdge(mappedU, mappedV, 1);
    }
    while (graph.m(1) > space) {
        // Delete last sampledEdge until below space
        auto [x, y] = reservoir.back();
        reservoir.pop_back();

        int mappedx = getMappedNode(x);
        int mappedy = getMappedNode(y);

        // Remove edge from the graph
        graph.removeEdge(mappedx, mappedy, 0);
        removedsampledEdges++;

        // Check if nodes have no more sampled edges and delete
        if (graph.degree(mappedx, 0) == 0) {
            graph.removeNode(mappedx);
            removedNodes++;
        }
        if (graph.degree(mappedy, 0) == 0) {
            graph.removeNode(mappedy);
            removedNodes++;
        }
    }
}

long long Sample::estimate() {
    int finalreservoirsize = reservoir.size();
    auto sampleCount = graph.BiColoredChibaNishizeki();

    double prob = 1.0 * finalreservoirsize / streamsize;
    // each four cycle is counted for 2 pairs
    long long estimate = prob > 0 ? std::llround(sampleCount / prob / prob / 2) : 0;
    return estimate;
}