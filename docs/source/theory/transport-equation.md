# The Transport Equation

Kingfisher solves the steady-state linear Boltzmann transport equation for the angular flux $\angflux(\vr, \vOmega, E)$:

$$
\vOmega \cdot \nabla \angflux(\vr, \vOmega, E) + \sigt(\vr, E)\, \angflux(\vr, \vOmega, E)
= \int_0^\infty \! \int_{4\pi} \sigs(\vr, \vOmega' \cdot \vOmega, E' \to E)\,
  \angflux(\vr, \vOmega', E') \, d\Omega' \, dE' + q(\vr, \vOmega, E)
$$ (eq-transport)

where $\sigt$ is the total macroscopic cross section, $\sigs$ is the differential scattering cross section, and $q$ is the source. The scalar flux is the angular integral of the angular flux,

$$
\sclflux(\vr, E) = \int_{4\pi} \angflux(\vr, \vOmega, E) \, d\Omega .
$$

:::{note}
This page is a placeholder beyond the statement of {eq}`eq-transport`. It will cover the energy discretization, boundary conditions, and the form of the equation that Kingfisher discretizes.
:::

Symbols such as `\angflux`, `\vOmega`, and `\sigt` are shared macros defined in `docs/source/conf.py`; see [](../developer-guide/documentation.md).
