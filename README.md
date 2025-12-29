# EIS: Four-Cycle Counting in Graph Streams

[![DOI](https://zenodo.org/badge/1120214769.svg)](https://doi.org/10.5281/zenodo.18089152)


## Compiling

Before building, ensure you have:

- **C++20 compatible compiler** (e.g. `clang++` ≥ 15)
- **CMake** version 3.22 or higher

Clone the repository and build the project using CMake:

```sh
git clone git@github.com:sl-dv/EIS.git
cd EIS/
mkdir build
cd build/
cmake ..
make
```

## Usage

After building, the `build/` directory will contain four executables
`EIS`, `EISm`, `NIS`, `3ES`
corresponding to the algorithms.

All of them expect a edge list of a simple undirected graph with zero-indexed nodes, the graph sample size $k$ and optionally the number of repetitions $r$.
We provide a script `scripts/standardize_instances.py` to standardize instances in the .mtx, .edges or KONECT tsv format.

The `data/` directory contains a small example instance which can be run from the project base directory using
```sh
./build/EIS data/random.standardized -k 20000 -r 10
```

The output consists of $r$ lines containing one estimate each, and a running time overview.

Further, there is an executable `stats` for computing the exact number of four-cycles.

## Reproducibility
To reproduce the experiments in the paper, run

```sh
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

python scripts/run_experiment.py
python scripts/create_plots.py out/exp_1
```

## License

MIT License. See [LICENSE](LICENSE) for details.
