SetFactory("OpenCASCADE");

// create domain -- four rectangles used for uniform mesh
Rectangle(1) = {0, 0, 0, 1, 1, 0}; 
Rectangle(2) = {1, 1, 0, 1, 1, 0}; 
Rectangle(3) = {1, 0, 0, 1, 1, 0};
Rectangle(4) = {0, 1, 0, 1, 1, 0}; 

BooleanFragments{Surface{1, 2, 3, 4}; Delete;}{}

// boundary conditions
Physical Curve("boundary:reflective_wall", 13) = {12, 4, 1, 9};
Physical Curve("boundary:outer_wall", 14) = {11, 7, 6, 10};

// set material(s)
Physical Surface("material:lathrop_material", 15) = {4, 2, 3, 1};

// set sources
Physical Surface("source:square_source", 16) = {1};
