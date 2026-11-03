// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_QUADRATURE_H_
#define KINGFISHER_QUADRATURE_QUADRATURE_H_

#include <mfem.hpp>

#include "quadrature/quad_set_params.h"

namespace kingfisher {

struct AngularParams;

/**
 * @brief Defines a quadrature set in 1D.
 *
 */
struct QuadSet1D {
  /// @brief Quadrature weights
  mfem::Vector weights;

  /// @brief Quadrature nodes
  mfem::Vector nodes;
};

/**
 * @brief Create a 1D quadrature set given a set of parameters.
 *
 * @param quad_set_params Quadrature parameters
 * @return QuadSet1D
 */
QuadSet1D Create1DQuadSet(const QuadSetParams& quad_set_params);

/**
 * @brief Defines a quadrature set in 2D.
 *
 */
struct QuadSet2D {
  /// @brief Quadrature weights
  mfem::Vector weights;

  /// @brief First axis' nodes
  mfem::Vector first_axis_nodes;

  /// @brief Second axis' nodes
  mfem::Vector second_axis_nodes;
};

/**
 * @brief Creates a 2D quadrature by taking a tensor product of two 1D
 * quadrature sets. `first_axis` is treated as the "fast" axis (the inner loop
 * of the tensor product). If x is the first axis, and y is second axis, then:
 *
 * \f[
 * \int_{-1}^1 \int_{-1}^1 f(x,y)dx dy\approx
 * \sum_{j=0}^{N_y}\sum_{i=0}^{N_x}w_{ij}f(x_i,y_j)
 * \f]
 *
 * @param first_axis First axis quadrature set.
 * @param second_axis Second axis quadrature set.
 * @return QuadSet2D
 */
QuadSet2D Create2DQuadSet(const QuadSet1D& first_axis,
                          const QuadSet1D& second_axis);

/**
 * @brief Creates a quadrature set on the unit sphere. Specifically, we
 * approximate the integral:
 *
 * \f[
 * \int_{4\pi}f(\hat{\Omega})d\hat{\Omega}=\int_{0}^{\pi}\int_{0}^{2\pi}f(\theta,\varphi)
 * \sin\varphi d\theta d\varphi=\int_{-1}^1\int_{0}^{2\pi}f(\theta,\mu)d\theta
 * d\mu \f]
 *
 * where \f$\mu\equiv \cos\varphi\f$ for \f$\varphi\in [0,\pi]\f$ and
 * \f$\theta\in [0,2\pi]\f$, with a summation:
 *
 * \f[
 * \int_{4\pi}f(\hat{\Omega})d\hat{\Omega} \approx \sum_{n=0}^N w_n
 * f(\hat{\Omega}_n). \f]
 *
 * If the quadrature set is specified to be approximated with a tensor product
 * along the azimuthal and polar axes, then we have:
 *
 * \f[
 * \int_{-1}^1\int_{0}^{2\pi}f(\theta,\mu)d\theta d\mu \approx
 * \sum_{i=1}^{N_{\varphi}}\sum_{j=1}^{N_{\theta}} w_{ij} f(\theta_i,\mu_j).
 * \f]
 *
 * @note For a tensor product, we demand the change of variables occur on
 * the polar angle to remove the Jacobian weighting. Additionally, the azimuthal
 * angle is "fast" index.
 *
 * @param angular_params Angular quadrature parameters
 * @return QuadSet1D
 * @throws KingfisherError The polar quadrature set of a spherical quadrature
 * must be on [-1,1].
 * @throws KingfisherError The azimuthal quadrature set of a spherical
 * quadrature set must be on [0,2\f$pi\f$].
 */
QuadSet2D SphericalQuadSet(const AngularParams& angular_params);

/**
 * @brief Integrates a grid function that was evaluated on a quadrature set's
 * nodes using the quadrature's weights. Weights and grid function values are
 * passed as flattened vectors.
 *
 * @param grid_function Grid function values
 * @param weights Quadrature weights
 * @return double
 */
double Integrate(const mfem::Vector& grid_function,
                 const mfem::Vector& weights);
}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_QUADRATURE_H_
