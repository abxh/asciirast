# Matrices --- their role in graphics

A matrix encodes information. Relevant to graphics, it encodes linear transformations.
 
Example: A linear transformation:
``` math
\begin{aligned}
    x' &= Ax + By \\
    y' &= Cx + Dy
\end{aligned}
```
The matrix encoding:
``` math
\begin{bmatrix}
    x' \\
    y'
\end{bmatrix}
=
\begin{bmatrix}
    A & B \\
    C & D
\end{bmatrix}
\begin{bmatrix}
    x \\
    y
\end{bmatrix}
```

A notational trick? Not quite. The power of matrices come from the plethora of
tricks in linear algebra. One simple fact that defines the structure of the
graphics render pipelines is --- **matricies ability to be composed and the
rules thereof**.
 
If you can compose matrices, you can compose linear transformations that
have geometric meaning. This is powerful for several reasons:

- Since matrices are usually not applied to a single point, but several millions
  of points at each frame in gpus. By pre-composing matrices, instead of doing transformation
  step-by-step, you do the transformation at once. (shh. the way I made this ascii renderer initially)

- If you wish to change the uniform transformation done to the points, you simply recalculate the matrix.
  You can subdivide the parts of the matrix, and recalculate the changed subdivisions. You can use tricks
  involving inverse matrices to revert transformations.

- If you use 3d matrices for 2d, you have an encoding for most desired units of transformations
 (translation, rotation, scaling, sheering). You cannot translate 2d vectors with a 2d matrix. And similarly
  using 4d vectors for 3d. In practice, the same operation is done, and the hardware is built to perform 
  matrix multiplication really quickly using simd, pipelining and other fancy tricks.
