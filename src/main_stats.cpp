#include "graph.hpp"
#include <basics/parms.hpp>
#include <basics/timer.hpp>
#include <iostream>

int main(int argc, char **argv) {
    Parms.read_parameters(argc,argv); 
    {
        ScopedTimer t1("main");

        Graph graph;
        graph.read_from_file(Parms.input());

        std::cout << "Number of nodes n: " << graph.n() << "\n";
        std::cout << "Number of edges m: " << graph.m() << "\n";
        std::cout << "Number of four-cycles T: " << graph.ChibaNishizeki() << "\n";
    }
    return 0;
}
