# Kalman Filter C Implementation #

Microcontroller targeted naive Kalman filter implementation in pure C using code ported from the [Efficient Java Matrix Library](https://code.google.com/p/efficient-java-matrix-library).

## Implemented so far ##
* Memory-optimizing preprocessor based Kalman Filter factory
* Algorithmically optimized matrix/matrix and matrix/vector operations
* Matrix inverse using Cholesky decomposition

## Example filters ##
* Gravity constant estimation using only measured position