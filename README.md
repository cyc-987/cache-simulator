# How to use

## Prepare
1. Install the required packages
```bash
pip install -r requirements.txt
```
2. Build the program
```bash
make all
```

## `sim`
this is a cache simulator that simulates the behavior of a cache with different configurations.

- usage:
```bash
./sim -bs <block size> -is <instruction cache size> -ds <data cache size> -a <associativity> -<wb/wt> -<wa/nw> <trace file>

./sim -bs <block size> -us <unified cache size> -a <associativity> -<wb/wt> -<wa/nw> <trace file>
```

- examples:
```bash
./sim -bs 16 -is 8192 -ds 8192 -a 1 -wb -wa ./ext_traces/spice.trace
```

## `eval.py`
this program reads the test cases offered in `test_cases.csv` to evaluate the correctness of the cache simulator.

## `performanceEval.ipynb`
this notebook uses the cache simulator to evaluate the performance of the cache with different configurations.