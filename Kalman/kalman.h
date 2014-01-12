#ifndef _KALMAN_H_
#define _KALMAN_H_

#include <stdint.h>
#include "matrix.h"

/*!
* \def EXTERN_INLINE_KALMAN Helper inline to switch from local inline to extern inline
*/
#ifndef EXTERN_INLINE_KALMAN
#define EXTERN_INLINE_KALMAN EXTERN_INLINE
#endif

/*!
* \brief Kalman Filter structure
* \see kalman_measurement_t
*/
typedef struct
{
    /*!
    * \brief State vector
    */
    matrix_t x;

    /*!
    * \brief System matrix
    * \see P
    */
    matrix_t A;

    /*!
    * \brief System covariance matrix
    * \see A
    */
    matrix_t P;
    
    /*!
    * \brief Input vector
    */
    matrix_t u;

    /*!
    * \brief Input matrix
    * \see Q
    */
    matrix_t B;

    /*!
    * \brief Input covariance/uncertainty matrix
    * \see B
    */
    matrix_t Q;

    /*!
    * \brief Temporary variables.
    */
    struct 
    {
        /*!
        * \brief Auxiliary array for matrix multiplication, needs to be MAX(num states, num inputs)
        *
        * This auxiliary field can also be used as a backing field for the predicted x vector, however
        * it MUST NOT be aliased with either temporary P or temporary BQ.
        */
        matrix_data_t *aux;

        /*!
        * \brief x-sized temporary vector
        * \see x
        */
        matrix_t predicted_x;

        /*!
        * \brief P-Sized temporary matrix  (number of states x number of states)
        *
        * The backing field for this temporary MAY be aliased with temporary BQ.
        *
        * \see P
        */
        matrix_t P;

        /*!
        * \brief BxQ-sized temporary matrix (number of states x number of inputs)
        *
        * The backing field for this temporary MAY be aliased with temporary P.
        *
        * \see B
        * \see Q
        */
        matrix_t BQ;

    } temporary;

} kalman_t;

/*!
* \brief Kalman Filter measurement structure
* \see kalman_t
*/
typedef struct
{
    /*!
    * \brief Measurement vector
    */
    matrix_t z;

    /*!
    * \brief Measurement transformation matrix
    * \see R
    */
    matrix_t H;

    /*!
    * \brief Process noise covariance matrix
    * \see H
    */
    matrix_t R;

    /*!
    * \brief Innovation vector
    */
    matrix_t y;

    /*!
    * \brief Residual covariance matrix
    * \see A
    */
    matrix_t S;

    /*!
    * \brief Kalman gain matrix
    */
    matrix_t K;

    /*!
    * \brief Temporary variables.
    */
    struct
    {
        /*!
        * \brief Auxiliary array for matrix multiplication, needs to be MAX(num states, num measurements)
        *
        * This auxiliary field MUST NOT be aliased with either temporary HP, KHP, HPt or S_inverted.
        */
        matrix_data_t *aux;
        
        /*!
        * \brief S-Sized temporary matrix  (number of measurements x number of measurements)
        *
        * The backing field for this temporary MAY be aliased with any other temporaries except for aux.
        *
        * \see S
        */
        matrix_t S_inv;

        /*!
        * \brief H-Sized temporary matrix  (number of measurements x number of states)
        *
        * The backing field for this temporary MAY be aliased with temporary temp_PHt.
        * The backing field for this temporary MUST NOT be aliased with temporary temp_KHP.
        */
        matrix_t temp_HP;

        /*!
        * \brief P-Sized temporary matrix  (number of states x number of states)
        *
        * The backing field for this temporary MAY be aliased with temporary temp_PHt.
        * The backing field for this temporary MUST NOT be aliased with temporary temp_HP.
        */
        matrix_t temp_KHP;

        /*!
        * \brief PxH'-Sized (H'-Sized) temporary matrix  (number of states x number of measurements)
        *
        * The backing field for this temporary MAY be aliased with temporary temp_HP.
        * The backing field for this temporary MAY be aliased with temporary temp_KHP.
        */
        matrix_t temp_PHt;

    } temporary;

} kalman_measurement_t;

/*!
* \brief Initializes the Kalman Filter
* \param[in] kf The Kalman Filter structure to initialize
* \param[in] num_states The number of state variables
* \param[in] num_inputs The number of input variables
* \param[in] A The state transition matrix ({\ref num_states} x {\ref num_states})
* \param[in] x The state vector ({\ref num_states} x \c 1)
* \param[in] B The input transition matrix ({\ref num_states} x {\ref num_inputs})
* \param[in] u The input vector ({\ref num_inputs} x \c 1)
* \param[in] P The state covariance matrix ({\ref num_states} x {\ref num_states})
* \param[in] Q The input covariance matrix ({\ref num_inputs} x {\ref num_inputs})
* \param[in] aux The auxiliary buffer (length {\ref num_states} or {\ref num_inputs}, whichever is greater)
* \param[in] predictedX The temporary vector for predicted X ({\ref num_states} x \c 1)
* \param[in] temp_P The temporary matrix for P calculation ({\ref num_states} x {\ref num_states})
* \param[in] temp_BQ The temporary matrix for BQ calculation ({\ref num_states} x {\ref num_inputs})
*/
void kalman_filter_initialize(kalman_t *kf, uint_fast8_t num_states, uint_fast8_t num_inputs, matrix_data_t *A, matrix_data_t *x, 
                              matrix_data_t *B, matrix_data_t *u, matrix_data_t *P, matrix_data_t *Q,
                              matrix_data_t *aux, matrix_data_t *predictedX, matrix_data_t *temp_P, matrix_data_t *temp_BQ);

/*!
* \brief Sets the measurement vector
* \param[in] kfm The Kalman Filter measurement structure to initialize
* \param[in] num_states The number of states
* \param[in] num_measurements The number of measurements
* \param[in] H The measurement transformation matrix ({\ref num_measurements} x {\ref num_states})
* \param[in] z The measurement vector ({\ref num_measurements} x \c 1)
* \param[in] R The process noise / measurement uncertainty ({\ref num_measurements} x {\ref num_measurements})
* \param[in] y The innovation ({\ref num_measurements} x \c 1)
* \param[in] S The residual covariance ({\ref num_measurements} x {\ref num_measurements})
* \param[in] K The Kalman gain ({\ref num_states} x {\ref num_measurements})
*/
void kalman_measurement_initialize(kalman_measurement_t *kfm, uint_fast8_t num_states, uint_fast8_t num_measurements, matrix_data_t *H, matrix_data_t *z, matrix_data_t *R, 
                                   matrix_data_t *y, matrix_data_t *S, matrix_data_t *K);

/*!
* \brief Performs the time update / prediction step.
* \param[in] kf The Kalman Filter structure to predict with.
* \param[in] lambda Lambda factor (\c 0 < {\ref lambda} <= \c 1) to forcibly reduce prediction certainty. Smaller values mean larger uncertainty.
* 
* This call assumes that the input covariance and variables are already set in the filter structure.
*/
void kalman_predict(kalman_t *kf, matrix_data_t lambda) HOT PURE;

/*!
* \brief Performs the measurement update step.
* \param[in] kf The Kalman Filter structure to correct.
*/
void kalman_correct(kalman_t *kf, kalman_measurement_t *kfm) HOT PURE;

/*!
* \brief Gets a pointer to the state vector x.
* \param[in] kf The Kalman Filter structure
* \return The state vector x.
*/
EXTERN_INLINE_KALMAN matrix_t* kalman_get_state_vector(kalman_t *kf) HOT PURE
{
    return &(kf->x);
}

/*!
* \brief Gets a pointer to the state transition matrix A.
* \param[in] kf The Kalman Filter structure
* \return The state transition matrix A.
*/
EXTERN_INLINE_KALMAN matrix_t* kalman_get_state_transition(kalman_t *kf) HOT PURE
{
    return &(kf->A);
}

/*!
* \brief Gets a pointer to the system covariance matrix P.
* \param[in] kf The Kalman Filter structure
* \return The system covariance matrix.
*/
EXTERN_INLINE_KALMAN matrix_t* kalman_get_system_covariance(kalman_t *kf)
{
    return &(kf->P);
}

/*!
* \brief Gets a pointer to the input vector u.
* \param[in] kf The Kalman Filter structure
* \return The input vector u.
*/
EXTERN_INLINE_KALMAN matrix_t* kalman_get_input_vector(kalman_t *kf) HOT PURE
{
    return &(kf->u);
}

/*!
* \brief Gets a pointer to the input transition matrix B.
* \param[in] kf The Kalman Filter structure
* \return The input transition matrix B.
*/
EXTERN_INLINE_KALMAN matrix_t* kalman_get_input_transition(kalman_t *kf) HOT PURE
{
    return &(kf->B);
}

/*!
* \brief Gets a pointer to the input covariance matrix P.
* \param[in] kf The Kalman Filter structure
* \return The input covariance matrix.
*/
EXTERN_INLINE_KALMAN matrix_t* kalman_get_input_covariance(kalman_t *kf) HOT PURE
{
    return &(kf->Q);
}

/*!
* \brief Gets a pointer to the measurement vector z.
* \param[in] kfm The Kalman Filter measurement structure.
* \return The measurement vector z.
*/
EXTERN_INLINE_KALMAN matrix_t* kalman_get_measurement_vector(kalman_measurement_t *kfm) HOT PURE
{
    return &(kfm->z);
}

/*!
* \brief Gets a pointer to the measurement transformation matrix H.
* \param[in] kfm The Kalman Filter measurement structure.
* \return The measurement transformation matrix H.
*/
EXTERN_INLINE_KALMAN matrix_t* kalman_get_measurement_transformation(kalman_measurement_t *kfm) HOT PURE
{
    return &(kfm->H);
}

/*!
* \brief Gets a pointer to the process noise matrix R.
* \param[in] kfm The Kalman Filter measurement structure.
* \return The process noise matrix R.
*/
EXTERN_INLINE_KALMAN matrix_t* kalman_get_process_noise(kalman_measurement_t *kfm) HOT PURE
{
    return &(kfm->R);
}

#undef EXTERN_INLINE_KALMAN
#endif