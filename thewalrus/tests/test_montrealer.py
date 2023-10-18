# Copyright 2021 Xanadu Quantum Technologies Inc.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain adj copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Montrealer tests"""

import pytest
import numpy as np
from thewalrus import mtl, lmtl
from thewalrus.reference import rspm, rpmp, mtl as mtl_symb
from scipy.special import factorial2
from scipy.stats import unitary_group


@pytest.mark.parametrize("n", range(1, 8))
def test_montrealer_all_ones(n):
    """Test that the Montrealer of a matrix of ones gives (2n-2)!!"""
    adj = np.ones([2 * n, 2 * n])
    mtl_val = mtl(adj)
    mtl_expect = factorial2(2 * n - 2)
    assert np.allclose(mtl_val, mtl_expect)


@pytest.mark.parametrize("n", range(1, 8))
def test_loop_montrealer_all_ones(n):
    """Test that the Montrealer of a matrix of ones gives (n+1)(2n-2)!!"""
    adj = np.ones([2 * n, 2 * n])
    lmtl_val = lmtl(adj, zeta = np.diag(adj))
    lmtl_expect = (n+1)*factorial2(2 * n - 2)
    assert np.allclose(lmtl_val, lmtl_expect)


@pytest.mark.parametrize("n", range(1, 8))
def test_size_of_pmpr(n):
    """rpmp(2n) should have (2n-2)!! elements"""
    terms_rpmp = sum(1 for _ in rpmp(range(2 * n)))
    terms_theo = factorial2(2 * n - 2)
    assert terms_rpmp == terms_theo


@pytest.mark.parametrize("n", range(1, 8))
def test_size_of_rspm(n):
    """rspm(2n) should have (n+1)(2n-2)!! elements"""
    terms_rspm = sum(1 for _ in rspm(range(2 * n)))
    terms_theo = (n + 1) * factorial2(2 * n - 2)
    assert terms_rspm == terms_theo


@pytest.mark.parametrize("n", range(2, 8))
def test_size_of_rpmp(n):
    """The rpmp must form a Y-alternating walk without loops"""
    test = True
    for perfect in rpmp(range(1, 2 * n + 1)):
        last = perfect[0][1]  # starting point
        reduced_last = last - n if last > n else last
        # different mode in every tuple
        if reduced_last == 1:
            test = False

        for i in perfect[1:]:
            reduced = i[0] - n if i[0] > n else i[0], i[1] - n if i[1] > n else i[1]
            # different mode in every tuple
            if reduced[0] == reduced[1]:
                test = False
            # consecutive tuple contain the same mode
            if reduced_last not in reduced:
                test = False

            last = i[0] if reduced[1] == reduced_last else i[1]
            reduced_last = last - n if last > n else last

        # last mode most coincide with the first one
        if reduced_last != 1:
            test = False

    assert test


@pytest.mark.parametrize("n", range(2, 8))
def test_mtl_functions_agree(n):
    """Make sure both mtl functions agree with one another"""
    u_n = unitary_group.rvs(n)
    u_n = u_n + u_n.conj().T
    u_m = unitary_group.rvs(n)
    u_m = u_m + u_m.T
    adj = np.block([[u_m.conj(), u_n], [u_n.T, u_m]])
    assert np.allclose(mtl_symb(adj), mtl(adj))


@pytest.mark.parametrize("n", range(2, 8))
def test_lmtl_functions_agree(n):
    """Make sure both lmtl functions agree with one another"""
    u_n = unitary_group.rvs(n)
    u_n = u_n + u_n.conj().T
    u_m = unitary_group.rvs(n)
    u_m = u_m + u_m.T
    adj = np.block([[u_m.conj(), u_n], [u_n.T, u_m]])
    zeta = np.diag(adj).conj()
    assert np.allclose(lmtl(adj, zeta), mtl_symb(adj, loop=True))


@pytest.mark.parametrize("n", range(2, 8))
def test_mtl_lmtl_agree(n):
    """Make sure mtl and lmtl give the same result if zeta = 0"""
    u_n = unitary_group.rvs(n)
    u_n = u_n + u_n.conj().T
    u_m = unitary_group.rvs(n)
    u_m = u_m + u_m.T
    adj = np.block([[u_m.conj(), u_n], [u_n.T, u_m]])
    zeta = np.zeros(2 * n, dtype=np.complex128)
    assert np.allclose(lmtl(adj, zeta), lmtl(adj, zeta))


@pytest.mark.parametrize("n", range(2, 8))
def test_mtl_lmtl_reference_agree(n):
    """Make sure mtl and lmtl from .reference give the same result if zeta = 0"""
    u_n = unitary_group.rvs(n)
    u_n = u_n + u_n.conj().T
    u_m = unitary_group.rvs(n)
    u_m = u_m + u_m.T
    adj = np.block([[u_m.conj(), u_n], [u_n.T, u_m]])
    zeta = np.zeros(2 * n, dtype=np.complex128)
    np.fill_diagonal(adj, zeta)
    assert np.allclose(mtl_symb(adj, loop=True), mtl_symb(adj))


@pytest.mark.parametrize("n", range(2, 8))
def test_mtl_permutation(n):
    """Make sure the mtl is invariant under permutation"""


@pytest.mark.parametrize("n", range(2,5))
def test_mtl_associated_adjacency(n):
    """Make sure the mtl of the associated adjacency matrix is 0"""
    u_zero = np.zeros((n,n), dtype = np.complex128)

    u_n1 = unitary_group.rvs(n)
    u_n2 = unitary_group.rvs(n)
    u_n = np.block([[u_n1, u_zero], [u_zero, u_n2]])
    u_n = u_n + u_n.conj().T

    u_m1 = unitary_group.rvs(n)
    u_m2 = unitary_group.rvs(n)
    u_m = np.block([[u_m1, u_zero], [u_zero, u_m2]])
    u_m_r = u_m + u_m.T

    u_m3 = unitary_group.rvs(n)
    u_m4= unitary_group.rvs(n)
    u_m = np.block([[u_m3, u_zero], [u_zero, u_m4]])
    u_m_l = u_m + u_m.T

    adj = np.block([[u_m_r, u_n], [u_n.T, u_m_l]])

    assert np.allclose(mtl(adj), 0)


@pytest.mark.parametrize("n", range(2, 5))
def test_mtl_diagonal_trace(n):
    """Make sure the mtl of A time a diagonal matrix gives the trace times the mtl of A"""
    gamma = np.random.uniform(-1, 1, n) + 1.j * np.random.uniform(-1, 1, n)
    product = np.prod([abs(i)**2 for i in gamma])
    gamma = np.diag(np.concatenate((gamma, gamma.conj())))

    u_n = unitary_group.rvs(n)
    u_n = u_n + u_n.conj().T
    u_m = unitary_group.rvs(n)
    u_m = u_m + u_m.T
    adj = np.block([[u_m.conj(), u_n], [u_n.T, u_m]])

    adj_gamma = gamma @ adj @ gamma.conj

    assert np.allclose(mtl(ajd_gamma), product * mtl(adj))


