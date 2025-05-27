# EIS: Four-Cycle Counting in Graph Streams

## Compiling

Before building, ensure you have:

- **C++20 compatible compiler** (e.g. `clang++` ≥ 15)
- **CMake** version 3.22 or higher
- (Optional) **Google SparseHash** library for faster hashmaps
    - Install via package manager or from [source](https://github.com/sparsehash/sparsehash)
        - Ubuntu: `sudo apt-get install libsparsehash-dev`
        - macOS: `brew install google-sparsehash`

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

All of them expect an input file in the [KONECT tsv](http://konect.cc/) format, the graph sample size $k$ and optionally the number of repetitions $r$.

The `data/` directory contains a small example instance which can be run from the project base directory using
```sh
./build/EIS data/out.caida -k 20000 -r 10
```

The output consists of $r$ lines containing one estimate each, and a running time overview.

## License

MIT License. See [LICENSE](LICENSE) for details.
