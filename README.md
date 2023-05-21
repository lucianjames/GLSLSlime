# GLSLSlime
Rewriting my simulation of slime mold. Inspired by that one video I saw on youtube that one time

Old version: https://github.com/LJ3D/glsl_cellular_automaton_linux

This program in action: https://www.youtube.com/watch?v=uFs06r8yxRc

# Compiling
Compiling is simple, just run the basic steps you would for building any cmake project.
It may take a while to run `cmake ..` at first, as dependencies are fetched.

!!! Added some opencv stuff, so you now need to have opencv installed.
NOTE: i had to install the fmt package manually to get opencv to compile for whatever reason

NOTE: you may have to manually install some dependencies of GLFW, you should get an error message telling you what package is missing from your system.
