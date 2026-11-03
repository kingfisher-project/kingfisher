// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/quadrature.h"

#include "quadrature/gauss_legendre.h"
#include "quadrature/gauss_lobatto_legendre.h"
#include "quadrature/trapezoid.h"
#include "utils/errors.h"
#include "utils/input_parameters.h"
#include "utils/numerics.h"

namespace kingfisher {
QuadSet1D Create1DQuadSet(const QuadSetParams& quad_set_params) {
  switch (quad_set_params.type) {
    case QuadSetType::Trapezoid:
      return Trapezoid(static_cast<int>(quad_set_params.n_points),
                       quad_set_params.min, quad_set_params.max);
    case QuadSetType::GaussLegendre:
      KingfisherError(!(DoubleEqual(quad_set_params.min, -1.0) &&
                        DoubleEqual(quad_set_params.max, 1.0)),
                      "GaussLegendre quadrature set can only be created on the "
                      "unit interval");
      return GaussLegendre(static_cast<int>(quad_set_params.n_points));
    case QuadSetType::GaussLobattoLegendre:
      KingfisherError(!(DoubleEqual(quad_set_params.min, -1.0) &&
                        DoubleEqual(quad_set_params.max, 1.0)),
                      "GaussLobattoLegendre quadrature set can only be created "
                      "on the unit interval");
      return GaussLobattoLegendre(static_cast<int>(quad_set_params.n_points));
    default:
      KingfisherError("QuadSetType passed to Create1DQuadSet not supported.");
  }
}

QuadSet2D Create2DQuadSet(const QuadSet1D& first_axis,
                          const QuadSet1D& second_axis) {
  QuadSet2D quad_set;
  quad_set.weights.SetSize(first_axis.weights.Size() *
                           second_axis.weights.Size());
  for (auto j = 0; j < second_axis.weights.Size(); j++) {
    for (auto i = 0; i < first_axis.weights.Size(); i++) {
      quad_set.weights[j * first_axis.weights.Size() + i] =
          first_axis.weights[i] * second_axis.weights[j];
    }
  }
  quad_set.first_axis_nodes = first_axis.nodes;
  quad_set.second_axis_nodes = second_axis.nodes;

  return quad_set;
}

QuadSet2D SphericalQuadSet(const AngularParams& angular_params) {
  switch (angular_params.quad_type) {
    case SphericalQuadType::TensorProduct: {
      auto azim_quad_set = Create1DQuadSet(angular_params.azimuthal_quad_set);
      auto polar_quad_set = Create1DQuadSet(angular_params.polar_quad_set);
      return Create2DQuadSet(azim_quad_set, polar_quad_set);
    }

    default:
      KingfisherError("Angular parameters quadrature type not supported.");
  }
}

double Integrate(const mfem::Vector& grid_function,
                 const mfem::Vector& weights) {
  KINGFISHER_ASSERT(grid_function.Size() == weights.Size(),
                    "Sizes of grid_function and weights passed to Integrate "
                    "are not the same.");
  return InnerProduct(grid_function, weights);
}
}  // namespace kingfisher
