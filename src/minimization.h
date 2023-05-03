// minimizeIntersections - is a function based on the Klay Layered Algorithm,
// the main concept of which is to go through the net forward and backward,
// changing the position of the nodes in the layer
// link: https://rtsys.informatik.uni-kiel.de/~biblio/downloads/papers/jvlc13.pdf

#ifndef LSVIS_MINIMIZATION_HPP
#define LSVIS_MINIMIZATION_HPP

#include "layout.h"
void minimizeIntersections(Net &net);

#endif //LSVIS_MINIMIZATION_HPP
