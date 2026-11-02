// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_GAUSS_LEGENDRE_H_
#define KINGFISHER_QUADRATURE_GAUSS_LEGENDRE_H_

#include <mfem/linalg/vector.hpp>

#include "quadrature/quadrature.h"

namespace kingfisher {
/**
 * @brief Create the 1D Gauss-Legendre quadrature on the intervale \f$\xi\in
 * [-1,1]\f$.
 *
 * @param n_points Number of points
 * @return QuadSet1D
 */
QuadSet1D GaussLegendre(const int n_points);

/**
 * @brief Compute the Gauss-Legendre nodes on the standard interval(\f$\xi\in
 * [-1,1]\f$). Note that Gauss-Legendre nodes form an *open* quadrature set.
 *
 * @param n_points Number of points desired.
 * @return mfem::Vector
 */
mfem::Vector GaussLegendreNodes(const int n_points);

/**
 * @brief Compute the Gauss-Legendre quadrature weights corresponding to the
 * given nodes on the standard interval (\f$\xi\in
 * [-1,1]\f$).
 *
 * @param nodes Gauss-Legendre quadrature nodes.
 * @return mfem::Vector
 */
mfem::Vector GaussLegendreWeights(const mfem::Vector& nodes);

}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_GAUSS_LEGENDRE_H_
