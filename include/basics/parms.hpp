#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include <basics/cxxopts.hpp>
#include <iostream>

class Parameters
{
public:
    Parameters(){};

    friend std::ostream& operator<<(std::ostream& os, Parameters const & p)
    {
        os<< "---------- Parameters -----------------" << std::endl;
        os<< "\tinput: " << p.input() << std::endl;
        os<< "\tk: " << p.k() << std::endl;
        os<< "\ts: " << p.s() << std::endl;
        os<< "\treps: " << p.reps() << std::endl;
        return os << "---------------------------------------" << std::endl;
    }

    void read_parameters(int argc, char**argv)
    {
        cxxopts::Options options("[EXECUTABLE]", "");
        try{

            options.add_options()
            ("input", "File path for input graph. Mandatory. Can be given positional.", cxxopts::value<std::string>())
            ("k", "Target graph sample size / Number of edges stored.", cxxopts::value<int>()->default_value("20000"))
            ("s", "EISm: Average of s samples which in total use k edges.", cxxopts::value<int>()->default_value("32"))
            ("r,reps", "Repetitions of the algorithm.", cxxopts::value<int>()->default_value("10"))
            ("h,help", "Print this information.");


            options.parse_positional({ "input" });
            options.allow_unrecognised_options();
        
            auto parse_result = options.parse(argc, argv);

            if (parse_result.count("help")) {
                std::cout << options.show_positional_help().help() << std::endl;
                exit(0);
            }


            _input = parse_result["input"].as<std::string>();
            _k = parse_result["k"].as<int>();
            _s = parse_result["s"].as<int>();
            _reps = parse_result["reps"].as<int>();
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing options: " << e.what() << std::endl;
            std::cerr << options.show_positional_help().help() << std::endl;
            exit(0);
        }
    }


    std::string input()     const {return _input;}
    int k()     const {return _k;}
    int s()     const {return _s;}
    int reps()     const {return _reps;}
private:
    std::string     _input;
    int     _k;
    int     _s;
    int     _reps;
};

inline Parameters Parms;

#endif //PARAMETERS_HPP
