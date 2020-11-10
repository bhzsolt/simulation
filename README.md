# Molecular Dynamics / Brownian Dynamics

Pedestrian crossing simulation

## System configuration

| Key | Value |
| --- | --- |
| particle/cell_size | 0.(2) |
| min(particles) | 100 |
| max(particles) | 1000 |
| dt | 0.001 |
| F | 0.5 |
| r₀ | 4.0 |
| rₘᵢₙ| 0.2 |

## Building & Running

use `make` in the root directory to build the project.
`cd build/bin`
`./simulation` to run the simulation. it should produce a .mvi file and a .times file
`./drawing [0/1]` to run the test-plotter. 0 for windowed, 1 for fullscreen.

## Plotter modules
| Module | Info |
| --- | --- |
| colors | module containing color codes and color name defines |
| shaders | module concerning shader loading and compiling |
| model | module abstracting away creation of models |
| drawing | window creation and handling + a dump of test code |
