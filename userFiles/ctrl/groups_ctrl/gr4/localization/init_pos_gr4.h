/*! 
 * \author Group 4
 * \file init_pos_gr4.h
 * \brief initial position of the robot
 */

#ifndef _INIT_POS_GR4_H_
#define _INIT_POS_GR4_H_ 
 
#include "namespace_ctrl.h"
#include "CtrlStruct_gr4.h"

NAMESPACE_INIT(ctrlGr4);

/// robot position
typedef struct RobotPosition
{
	double x; ///< x position [m]
	double y; ///< y position [m]
	double theta; ///< robot orientation [rad]

	double last_t; ///< last time position was updated

} RobotPosition;

void set_init_position(int robot_id, RobotPosition *rob_pos);
void set_triang_init_position(int robot_id, RobotPosition* triang_pos);


NAMESPACE_CLOSE();

#endif
