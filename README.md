# stk_mdl_rotate_check
Trying to understand why a [Rotate ax ay az] statement in an AGI/STK MDL file is not behaving as expected

In the end, the problem was that the J2000-relative MDL needed its attitude set relative to the [Inertial fixed] reference in STK setup.

This is in support of the New Horizons (NH) mission flyby of Kuiper Belt Object (KBO) MU69 ca. 2019-JAN-01.

### Manifest

mu69_alt_2sig.mdl - AGI/STK MDL file; shape model for visualization in AGI/STK application of uncertainties around MU69; ellipsoid-like surface of 2-sigma uncertainty volume for the NH flyby of Mu69.  +X axis is Time Of Flight uncertainty (TOF; downtrack); +Y axis is B-normal uncertainty (B-norm; normal to directory plane; cross-product [B-mag X TOF]); +Z axis is B-magnitude uncertainty (B-mag; in direction of MU69=>NH at Time of Closest Approach (TCA)) 

__smrc_000.c__ - source file to calculate geometry and orientation of MDL wrt J2000 reference frame; uses JPL/NAIF CSPICE library of SPICE toolkit cf. https://naif.jpl.nasa.gov/

__Makefile__ - generic 

__README.md__ - this file

__spice_kernels/__ - geometry of the flyby; SPICE kernels to support this work

__spice_kernels/mu69_2486958_offbodies_180s.bsp__ - SPICE SPK; Spacecraft and/or Planet kernel;  ephemerides of two virtual bodies fixed at +/- 180s uptrack and downtrack from MU69, used in STK to visualize target

__spice_kernels/naif0012.tls__ - SPICE LSK; Leapsecond Kernel

__spice_kernels/nh_v220.tf__ - SPICE FK; Frames Kernel

__spice_kernels/temporary_mu69_radii.tpc__ - SPICE PCK; Planetery Constants Kernel; temporary radii tri-axial ellpsoid model for MU69, related to uncertainty ellipsoid mu69_alt_2sig.mdl above.

__spice_kernels/nh_mu69_subset.bsp__ - SPICE SPK; ephemerides of several bodies (NH spacecraft, MU69, Earth, Moon, Sun, Earth-Moon barycenter, Pluto barycenter).

Brian T. Carcich  2017-11-10
