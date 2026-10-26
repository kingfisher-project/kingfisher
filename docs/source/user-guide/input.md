# Input Files

Input files for Kingfisher are constructed using `yaml`, and includes the follow (required) blocks:
1. version
2. problem
3. mesh
4. boundary_conditions
5. materials
6. sources
7. regions
8. angular
9. spatial
10. solver
11. output

These are all discussed in detail below.

## Version
The `version` block sets the schema version to check the input file against.
For now, only version 0 is available.

## Problem
The `problem` block holds simulation-wide settings.
It includes:
1. `name` - The user-set name of the simulation.
2. `mode` - The type of simulation that can be run. 

:::{note}
This page is a placeholder. It will document every input block, with defaults and units.
:::
