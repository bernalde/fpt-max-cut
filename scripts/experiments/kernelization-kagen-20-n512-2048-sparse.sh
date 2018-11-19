#!/bin/bash

source ./bootstrap.sh

./$selected_build -action "kernelization" -iterations 10 -sample-kagen 20 -num-nodes 512 -num-edges-lo 0 -num-edges-hi 2500 -total-allowed-solver-time -1 \
                  -benchmark-output ../data/output/experiments/kernelization/n512/out > ../data/output/experiments/kernelization/n512/out-exe
./$selected_build -action "kernelization" -iterations 10 -sample-kagen 20 -num-nodes 2048 -num-edges-lo 0 -num-edges-hi 10000 -total-allowed-solver-time -1 \
                  -benchmark-output ../data/output/experiments/kernelization/n2048/out > ../data/output/experiments/kernelization/n2048/out-exe


./$selected_build -action "kernelization" -iterations 10 -sample-kagen 20 -num-nodes 512 -num-edges-lo 0 -num-edges-hi 2500 -total-allowed-solver-time -1 -support-weighted-result \
                  -benchmark-output ../data/output/experiments/kernelization/n512w/out > ../data/output/experiments/kernelization/n512w/out-exe
./$selected_build -action "kernelization" -iterations 10 -sample-kagen 20 -num-nodes 2048 -num-edges-lo 0 -num-edges-hi 10000 -total-allowed-solver-time -1 -support-weighted-result \
                  -benchmark-output ../data/output/experiments/kernelization/n2048w/out > ../data/output/experiments/kernelization/n2048w/out-exe