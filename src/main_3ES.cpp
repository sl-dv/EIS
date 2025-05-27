#include "graph.hpp"
#include <iostream>
#include <basics/parms.hpp>
#include <basics/timer.hpp>

int main(int argc, char **argv) {
    Parms.read_parameters(argc,argv); 
    {
        ScopedTimer t1("main");

        Graph graph;
        {
            ScopedTimer t2("IO");
            graph.read_konect(Parms.input());
        }

        int k = Parms.k();
        if (k<=0) throw std::runtime_error("invalid k");
        
        for (int i = 0; i < Parms.reps(); ++i) {
            ScopedTimer t("3ES");
            long long estimate = graph.multipass_baseline(k);
            std::cout << estimate << std::endl;
        }
    }
    ScopedTimer::print_timers();
    return 0;
}

