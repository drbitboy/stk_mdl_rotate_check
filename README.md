# stk_mdl_rotate_check
Trying to understand why a [Rotate ax ay az] statement in an AGI/STK MDL file is not behaving as expected

In the end, the problem was that the J2000-relative MDL needed its attitude set relative to the [Inertial fixed] reference in STK setup.

Brian T. Carcich  2017-11-10
