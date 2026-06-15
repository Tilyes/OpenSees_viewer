wipe
model basic -ndm 2 -ndf 3

node  1     0.0     0.0   -mass [expr $ele_mass/2]   [expr $ele_mass/2]   [expr $ele_mass/2]
node  2     0.0   100.0   -mass $ele_mass $ele_mass $ele_mass
node  3     0.0   200.0   -mass $ele_mass $ele_mass $ele_mass
node  4     0.0   300.0   -mass [expr $ele_mass/2]   [expr $ele_mass/2]   [expr $ele_mass/2]
node  5      0.0   500.0 

beamIntegration HingeRadau 101 1 0 1 0 2
element forceBeamColumn  1   1   2   1  101
element forceBeamColumn  2   2   3   1  101
element forceBeamColumn  3   3   4   1  101
element forceBeamColumn  4   4   5   1  101


