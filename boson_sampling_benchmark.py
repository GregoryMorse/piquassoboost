#
#  Copyright 2021 Budapest Quantum Computing Group
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import time
import pytest
import itertools

import numpy as np

import piquasso as pq
import piquassoboost as pqb

from mpi4py import MPI
from scipy.stats import unitary_group



def print_histogram(samples):
    hist = dict()

    for sample in samples:
        key = tuple(sample)
        if key in hist.keys():
            hist[key] += 1
        else:
            hist[key] = 1

    for key in hist.keys():
        print(f"{key}: {hist[key]}")

    return


dim = 12

# generate random matrix
U = unitary_group.rvs(dim)#generate_random_unitary(dim)

shots = 100

with pq.Program() as program:
    pq.Q() | pq.StateVector(np.ones(dim))
    pq.Q() | pq.Interferometer(U)

    pq.Q() | pq.Sampling()

simulator = pqb.BoostedSamplingSimulator(d=dim)

t0 = time.time()

result = simulator.execute(program=program, shots=shots)
print("C++ time elapsed:", time.time() - t0, "s")

#print_histogram(result.samples)

