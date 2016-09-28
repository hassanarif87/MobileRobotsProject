/* --------------------------------------------------------
 * This code interface mbs data struct with an arbitrary user model 
 * MBsysC modules are distributed as part of the ROBOTRAN 
 * software. They provides functionalities for dealing with
 * symbolic equations generated by ROBOTRAN. 
 *
 * More info on www.robotran.be 
 *
 * Universite catholique de Louvain, Belgium 
 *
 * Last update : Fri Apr 10 11:43:58 2015
 * --------------------------------------------------------
 *
 */
#ifndef MBS_USER_INTERFACE_h 
#define MBS_USER_INTERFACE_h

#include "project_userfct_export.h"

// ============================================================ //


typedef struct UserModel  UserModel;

#ifdef __cplusplus
extern "C" {
#endif
	PROJECT_USERFCT_EXPORT UserModel* mbs_new_user_model();
	PROJECT_USERFCT_EXPORT void mbs_delete_user_model(UserModel* ums);
#ifdef __cplusplus
}
#endif
// ============================================================ //

/**
 * Contains information about UserIO.
 * This structure is intended to automate the treatment
 * of UserIO, in partiucalar for copying that from/to
 * input/output port of Simulink S-function block.
 */

typedef struct{
    int n_in;    ///< number of user input port
    int n_out;   ///< number of user output port
    int *size_in;  ///< size_in[i] = size of i-th user input
    int *size_out; ///< size_out[i] = size of i-th user output
    double **ptr_in;  /// ptr_in[i][0]  = pointer to the 1st element of the i-th input must be copied
    double **ptr_out; /// ptr_out[i][0] = pointer to the 1st element of the i-th output must be copied
} UserIoInfo;


typedef struct UserIO UserIO;

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * Initialize the UserIO structure and bind the ioInfo pointer
 * to the allocated memory
 * (project specific -> see definition in project/userfctR/user_IO.c)
 */ 
    PROJECT_USERFCT_EXPORT UserIO * mbs_new_user_IO(UserIoInfo* ioInfo);
/**
 * Free the memory associated to the given UserIO structure.
 * (project specific -> see definition in project/userfctR/user_IO.c)
 */ 
    PROJECT_USERFCT_EXPORT void mbs_delete_user_IO(UserIO *uvs);
    
    
/**
 * Set all the user_IO to the given value
 * (generic code)
 */ 
    PROJECT_USERFCT_EXPORT void mbs_set_user_IO(UserIoInfo* ioInfo, double val);
#ifdef __cplusplus
}
#endif
 
# endif