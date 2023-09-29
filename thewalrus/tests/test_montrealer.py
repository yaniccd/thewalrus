import pytest
import numpy as np
from thewalrus import mtl, lmtl
from thewalrus._montrealer import montrealer, lmontrealer
from thewalrus.reference import rspm, rpmp
#from thewalrus.quantum import
from scipy.special import factorial2

@pytest.mark.parametrize("n", list(range(1,6)))
def test_montrealer_all_ones(n):
    """Test that the Montrealer of a matrix of ones gives (2n-2)!!"""
    A = np.ones([2*n, 2*n])
    mtl_val = mtl(A)
    mtl_expect = factorial2(2*n-2)
    assert np.allclose(mtl_val, mtl_expect)