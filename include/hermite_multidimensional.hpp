// Copyright 2019 Xanadu Quantum Technologies Inc.


// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/**
 * @file
 * Contains functions for calculating the multidimensional
 * Hermite polynomials, used for computation of batched hafnians.
 */

#pragma once
#include <stdafx.h>
#include <assert.h>

typedef unsigned long long int ullint;


/**
 * Returns the index of the one dimensional flattened vector corresponding to the multidimensional tensor
 *
 * @param pos
 * @param resolution
 *
 * @return index on flattened vector
 */
ullint vec2index(std::vector<int> &pos, int resolution) {
    int dim = pos.size();
    ullint nextCoordinate = 0;

    nextCoordinate = pos[0]-1;
    for(int ii = 0; ii < dim-1; ii++) {
        nextCoordinate = nextCoordinate*resolution + (pos[ii+1]-1);
    }

    return nextCoordinate;

}

/**
 * Updates the iterators needed for the calculation of the Hermite multidimensional functions
 *
 * @param nextPos a vector of integers
 * @param jumpFrom a vector of integers
 * @param jump integer specifying whether to jump to the next index
 * @param resolution integer specifying the cuotff
 * @dim dimension of the R matrix
 *
 * @k index necessary for knowing which elements are needed from the input vector y and matrix R
 */
int update_iterator(std::vector<int> &nextPos, std::vector<int> &jumpFrom, int &jump, const int &resolution, const int &dim) {
    if (jump > 0) {
        jumpFrom[jump] += 1;
        jump = 0;
    }
    for (int ii = 0; ii < dim; ii++) {
        std::vector<int> forwardStep(dim, 0);
        forwardStep[ii] = 1;

        if ( forwardStep[ii] + nextPos[ii] > resolution) {
            nextPos[ii] = 1;
            jumpFrom[ii] = 1;
            jump = ii+1;
        }
        else {
            jumpFrom[ii] = nextPos[ii];
            nextPos[ii] = nextPos[ii] + 1;
            break;
        }
    }
    int k=0;
    for(; k < dim; k++) {
        if(nextPos[k] != jumpFrom[k]) break;
    }
    return k;
}

namespace libwalrus {

/**
 * Returns the multidimensional Hermite polynomials \f$H_k^{(R)}(y)\f$.
 *
 * This implementation is based on the MATLAB code available at
 * https://github.com/clementsw/gaussian-optics
 *
 * @param R a flattened vector of size \f$n^2\f$, representing a
 *       \f$n\times n\f$ symmetric matrix.
 * @param y a flattened vector of size \f$n\f$.
 * @param resolution highest number of photons to be resolved.
 *
 */
template <typename T>
inline T* hermite_multidimensional_cpp(const std::vector<T> &R, const std::vector<T> &y, const int &resolution) {
    int dim = std::sqrt(static_cast<double>(R.size()));

    ullint Hdim = pow(resolution, dim);
    T *H;
    H = (T*) malloc(sizeof(T)*Hdim);
    T val = 0;
    memset(H,sizeof(val),sizeof(H));
    H[0] = 1;

    std::vector<int> nextPos(dim, 1);
    std::vector<int> jumpFrom(dim, 1);
    int jump = 0;
    int k;
    ullint nextCoordinate, fromCoordinate;

    for (ullint jj = 0; jj < Hdim-1; jj++) {

        k = update_iterator(nextPos, jumpFrom, jump, resolution, dim);

        nextCoordinate = vec2index(nextPos, resolution);
        fromCoordinate = vec2index(jumpFrom, resolution);

        H[nextCoordinate] = H[fromCoordinate] * y[k];

        std::vector<int> tmpjump(dim, 0);

        for (int ii = 0; ii < dim; ii++) {
            if (jumpFrom[ii] > 1) {
                std::vector<int> prevJump(dim, 0);
                prevJump[ii] = 1;
                std::transform(jumpFrom.begin(), jumpFrom.end(), prevJump.begin(), tmpjump.begin(), std::minus<int>());
                ullint prevCoordinate = vec2index(tmpjump, resolution);
                H[nextCoordinate] = H[nextCoordinate] - (static_cast<T>(jumpFrom[ii]-1))*(R[dim*k+ii])*H[prevCoordinate];

            }
        }

    }
    return H;

}



/**
 * Returns the normalized multidimensional Hermite polynomials \f$\tilde{H}_k^{(R)}(y)\f$.
 *
 * This implementation is based on the MATLAB code available at
 * https://github.com/clementsw/gaussian-optics
 *
 * @param R a flattened vector of size \f$n^2\f$, representing a
 *       \f$n\times n\f$ symmetric matrix.
 * @param y a flattened vector of size \f$n\f$.
 * @param resolution highest number of photons to be resolved.
 *
 */
template <typename T>
inline T*  renorm_hermite_multidimensional_cpp(const std::vector<T> &R, const std::vector<T> &y, const int &resolution) {
    int dim = std::sqrt(static_cast<double>(R.size()));

    ullint Hdim = pow(resolution, dim);
    T *H;
    H = (T*) malloc(sizeof(T)*Hdim);
    T val = 0;
    memset(H,sizeof(val),sizeof(H));	//std::cout<<H[0];
    //std::cout<<H[1];
    //std::cout<<H[2];
    
    H[0] = 1;
    std::vector<double> intsqrt(resolution+1, 0);
    for (int ii = 0; ii<=resolution; ii++) {
        intsqrt[ii] = std::sqrt((static_cast<double>(ii)));
    }
    std::vector<int> nextPos(dim, 1);
    std::vector<int> jumpFrom(dim, 1);
    int jump = 0;
    int k;
    ullint nextCoordinate, fromCoordinate;
    for (ullint jj = 0; jj < Hdim-1; jj++) {
        k = update_iterator(nextPos, jumpFrom, jump, resolution, dim);

        nextCoordinate = vec2index(nextPos, resolution);
        fromCoordinate = vec2index(jumpFrom, resolution);

        H[nextCoordinate] = H[fromCoordinate] * y[k]/(intsqrt[nextPos[k]-1]);

        std::vector<int> tmpjump(dim, 0);

        for (int ii = 0; ii < dim; ii++) {
            if (jumpFrom[ii] > 1) {
                std::vector<int> prevJump(dim, 0);
                prevJump[ii] = 1;
                std::transform(jumpFrom.begin(), jumpFrom.end(), prevJump.begin(), tmpjump.begin(), std::minus<int>());
                ullint prevCoordinate = vec2index(tmpjump, resolution);
                H[nextCoordinate] = H[nextCoordinate] - (intsqrt[jumpFrom[ii]-1]/intsqrt[nextPos[k]-1])*(R[k*dim+ii])*H[prevCoordinate];
            }
        }
    }
    return H;
}

/**
 * Returns the matrix elements of an interferometer parametrized in terms of its R matrix
 *
 * @param R a flattened vector of size \f$n^2\f$, representing a
 *       \f$n\times n\f$ symmetric matrix.
 * @param resolution highest number of photons to be resolved.
 *
 */
template <typename T>
inline T* interferometer_cpp(const std::vector<T> &R, const int &resolution) {

    int dim = std::sqrt(static_cast<double>(R.size()));
    assert(dim % 2 == 0);
	int num_modes = dim/2;
    ullint Hdim = pow(resolution, dim);
    T *H;
    H = (T*) malloc(sizeof(T)*Hdim);
    T val = 0;
    memset(H,sizeof(val),sizeof(H));
    H[0] = 1;
    std::vector<double> intsqrt(resolution+1, 0);
    for (int ii = 0; ii<=resolution; ii++) {
        intsqrt[ii] = std::sqrt((static_cast<double>(ii)));
    }
    std::vector<int> nextPos(dim, 1);
    std::vector<int> jumpFrom(dim, 1);
    int jump = 0;
    int k;
    ullint nextCoordinate, fromCoordinate;

    for (ullint jj = 0; jj < Hdim-1; jj++) {
        k = update_iterator(nextPos, jumpFrom, jump, resolution, dim);
        int bran = 0;
        for (int ii=0; ii < num_modes; ii++) {
            bran += nextPos[ii];
        }

        int ketn = 0;
        for (int ii=num_modes; ii < dim; ii++) {
            ketn += nextPos[ii];
        }
        if (bran == ketn) {
            nextCoordinate = vec2index(nextPos, resolution);
            fromCoordinate = vec2index(jumpFrom, resolution);
            std::vector<int> tmpjump(dim, 0);
            int low_lim;
            int high_lim;

            if (k > num_modes) {
                low_lim = 0;
                high_lim = num_modes;
            }
            else {
                low_lim = num_modes;
                high_lim = dim;
            }

            for (int ii = low_lim; ii < high_lim; ii++) {
                if (jumpFrom[ii] > 1) {
                    std::vector<int> prevJump(dim, 0);
                    prevJump[ii] = 1;
                    std::transform(jumpFrom.begin(), jumpFrom.end(), prevJump.begin(), tmpjump.begin(), std::minus<int>());
                    ullint prevCoordinate = vec2index(tmpjump, resolution);
                    H[nextCoordinate] = H[nextCoordinate] - (intsqrt[jumpFrom[ii]-1]/intsqrt[nextPos[k]-1])*(R[k*dim+ii])*H[prevCoordinate];
                }
            }
        }
    }
    return H;
}

/**
 * Returns the matrix elements of a single mode squeezing operation parametrized in terms of its R matrix
 *
 * @param R a flattened vector of size 4, representing a
 *       \f$2\times 2\f$ symmetric matrix.
 * @param resolution highest number of photons to be resolved.
 *
 */
template <typename T>
inline T* squeezing_cpp(const std::vector<T> &R, const int &resolution) {
    int dim = std::sqrt(static_cast<double>(R.size()));

    ullint Hdim = pow(resolution, dim);
    T *H;
    H = (T*) malloc(sizeof(T)*Hdim);
    T val = 0;
    memset(H,sizeof(val),sizeof(H));
    H[0] = std::sqrt(-R[1]);
    std::vector<double> intsqrt(resolution+1, 0);
    for (int ii = 0; ii<=resolution; ii++) {
        intsqrt[ii] = std::sqrt((static_cast<double>(ii)));
    }
    assert(dim == 2);
    int num_modes = dim/2;

    std::vector<int> nextPos(dim, 1);
    std::vector<int> jumpFrom(dim, 1);
	int jump = 0;
    int k;
    ullint nextCoordinate, fromCoordinate;
    for (ullint jj = 0; jj < Hdim-1; jj++) {
        k = update_iterator(nextPos, jumpFrom, jump, resolution, dim);

        int bran = nextPos[0];
        int ketn = nextPos[1];
        if (bran % 2 == ketn % 2) {
            ullint nextCoordinate = vec2index(nextPos, resolution);
            ullint fromCoordinate = vec2index(jumpFrom, resolution);

            std::vector<int> tmpjump(dim, 0);
            for (int ii = 0; ii < dim; ii++) {
                if (jumpFrom[ii] > 1) {
                    std::vector<int> prevJump(dim, 0);
                    prevJump[ii] = 1;
                    std::transform(jumpFrom.begin(), jumpFrom.end(), prevJump.begin(), tmpjump.begin(), std::minus<int>());
                    ullint prevCoordinate = vec2index(tmpjump, resolution);
                    H[nextCoordinate] = H[nextCoordinate] - (intsqrt[jumpFrom[ii]-1]/intsqrt[nextPos[k]-1])*(R[k*dim+ii])*H[prevCoordinate];

                }
            }
        }
    }
    return H;
}



/**
 * Returns the matrix elements of a displacement operation parametrized in terms of its double vector y
 *
 * @param y a flattened vector of size \f$2\f$, represeting the displacement via \f$\alpha, \alpha^*\f$
 * @param resolution highest number of photons to be resolved.
 *
 */
template <typename T>
inline T* displacement_cpp(const std::vector<T> &y, const int &resolution) {
    int dim = 2;

    ullint Hdim = pow(resolution, dim);
    T *H;
    H = (T*) malloc(sizeof(T)*Hdim);
    T val = 0;
    memset(H,sizeof(val),sizeof(H));
    H[0] = std::exp(0.5*y[0]*y[1]);
    std::vector<double> intsqrt(resolution+1, 0);
    for (int ii = 0; ii<=resolution; ii++) {
        intsqrt[ii] = std::sqrt((static_cast<double>(ii)));
    }
    std::vector<int> nextPos(dim, 1);
    std::vector<int> jumpFrom(dim, 1);
    int jump = 0;
    int k;
    ullint nextCoordinate, fromCoordinate;
    for (ullint jj = 0; jj < Hdim-1; jj++) {
        k = update_iterator(nextPos, jumpFrom, jump, resolution, dim);

        nextCoordinate = vec2index(nextPos, resolution);
        fromCoordinate = vec2index(jumpFrom, resolution);

        H[nextCoordinate] = H[fromCoordinate] * y[k]/(intsqrt[nextPos[k]-1]);

        std::vector<int> tmpjump(dim, 0);


        int ii = 0;
        if(k==0){
        	ii = 1;
        }

        if (jumpFrom[ii] > 1) {
            std::vector<int> prevJump(dim, 0);
            prevJump[ii] = 1;
            std::transform(jumpFrom.begin(), jumpFrom.end(), prevJump.begin(), tmpjump.begin(), std::minus<int>());
            ullint prevCoordinate = vec2index(tmpjump, resolution);
            H[nextCoordinate] = H[nextCoordinate] - (intsqrt[jumpFrom[ii]-1]/intsqrt[nextPos[k]-1])*H[prevCoordinate];

	    }

    }
    return H;

}

/**
 * Returns the matrix elements of a two-mode squeezer parametrized in terms of its R matrix
 *
 * @param R a flattened vector of size \f$n^2\f$, representing a
 *       \f$n\times n\f$ symmetric matrix.
 * @param resolution highest number of photons to be resolved.
 *
 */

template <typename T>
inline T* two_mode_squeezing_cpp(const std::vector<T> &R, const int &resolution) {
    int dim = std::sqrt(static_cast<double>(R.size()));
    assert(dim == 4);

    ullint Hdim = pow(resolution, dim);
    T *H;
    H = (T*) malloc(sizeof(T)*Hdim);
    T val = 0;
    memset(H,sizeof(val),sizeof(H));
    //memset(H, 0x00, sizeof(H));
    for (ullint jj = 0; jj < Hdim; jj++){
    	H[jj] = 0.0;
    	std::cout << H[jj];
    }

    H[0] = -R[2];
    std::vector<double> intsqrt(resolution+1, 0);
    for (int ii = 0; ii<=resolution; ii++) {
        intsqrt[ii] = std::sqrt((static_cast<double>(ii)));
    }
    std::vector<int> nextPos(dim, 1);
    std::vector<int> jumpFrom(dim, 1);
    int jump = 0;
    int k;
    ullint nextCoordinate, fromCoordinate;
    for (ullint jj = 0; jj < Hdim-1; jj++) {
    	k = update_iterator(nextPos, jumpFrom, jump, resolution, dim);
    	int bran = nextPos[0]-nextPos[1];
        int ketn = nextPos[2]-nextPos[3];
        if (bran == ketn) {
            ullint nextCoordinate = vec2index(nextPos, resolution);
            ullint fromCoordinate = vec2index(jumpFrom, resolution);

            std::vector<int> tmpjump(dim, 0);

            for (int ii = 0; ii < dim; ii++) {
                if (jumpFrom[ii] > 1) {
                    std::vector<int> prevJump(dim, 0);
                    prevJump[ii] = 1;
                    std::transform(jumpFrom.begin(), jumpFrom.end(), prevJump.begin(), tmpjump.begin(), std::minus<int>());
                    ullint prevCoordinate = vec2index(tmpjump, resolution);
                    H[nextCoordinate] = H[nextCoordinate] - (intsqrt[jumpFrom[ii]-1]/intsqrt[nextPos[k]-1])*(R[k*dim+ii])*H[prevCoordinate];

                }
            }
        }
    }
    return H;
}





}
