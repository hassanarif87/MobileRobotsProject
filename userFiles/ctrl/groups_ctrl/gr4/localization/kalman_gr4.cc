#include "kalman_gr4.h"
#include "odometry_gr4.h"
#include "triangulation_gr4.h"
#include "useful_gr4.h"
#include "config_file.h"
#include "config_file_gr4.h"

NAMESPACE_INIT(ctrlGr4);

/*! \brief follow a given path
 * 
 * \param[in,out] cvs controller main structure
 */
void kalman(CtrlStruct *cvs)
{
	// variable declaration
    RobotPosition *rob_pos;
    RobotPosition *triang_pos;
    KalmanStruct *kalman_pos;

    // variables initialization
    rob_pos = cvs->rob_pos;
    triang_pos = cvs->triang_pos;
    kalman_pos = cvs->kalman;

    // init loop
    if (!kalman_pos->init)
    {
        kalman_pos->initEst(cvs);
        kalman_pos->init = true;
    }

    // prediction step if odometry is available
    if (kalman_pos->odo_meas.odoFlag)
    {
        // buffers
        double pxx = kalman_pos->pEst.xx;
        double pxy = kalman_pos->pEst.xy;
        double pxtheta = kalman_pos->pEst.xtheta;
        double pyy = kalman_pos->pEst.yy;
        double pytheta = kalman_pos->pEst.ytheta;
        double pthetatheta = kalman_pos->pEst.thetatheta;

        // for readability
        double deltaS = kalman_pos->odo_meas.dS;
        double deltaTheta = kalman_pos->odo_meas.dTheta;
        double theta = rob_pos->theta;

        // mean update
        kalman_pos->xEst += kalman_pos->odo_meas.dS * cos(theta + 0.5*kalman_pos->odo_meas.dTheta);
        kalman_pos->yEst += kalman_pos->odo_meas.dS * sin(theta + 0.5*kalman_pos->odo_meas.dTheta);
        kalman_pos->thetaEst += kalman_pos->odo_meas.dTheta;

        // updating cov matrix expression taken from wxmaxima file
        // obtained via wxmaxima - formal calculation software.
        kalman_pos->pEst.xx = (pow(deltaS,2)*pow(sin(deltaTheta/2+theta),2)*pow(RobotGeometry::ENC_RES,2)*pow(RobotGeometry::WHEEL_RADIUS,2)*pow(RobotGeometry::WHEEL_BASE,-2))/24+(pow(cos(deltaTheta/2+theta),2)*pow(RobotGeometry::ENC_RES,2)*pow(RobotGeometry::WHEEL_RADIUS,2))/24+2*RobotGeometry::KS*pxx*abs(cos(theta))*abs(deltaS)+pxx;
        kalman_pos->pEst.xy = -(pow(deltaS,2)*cos(deltaTheta/2+theta)*sin(deltaTheta/2+theta)*pow(RobotGeometry::ENC_RES,2)*pow(RobotGeometry::WHEEL_RADIUS,2)*pow(RobotGeometry::WHEEL_BASE,-2))/24+(cos(deltaTheta/2+theta)*sin(deltaTheta/2+theta)*pow(RobotGeometry::ENC_RES,2)*pow(RobotGeometry::WHEEL_RADIUS,2))/24+pxy;
        kalman_pos->pEst.xtheta = (deltaS*sin(deltaTheta/2+theta)*pow(RobotGeometry::ENC_RES,2)*pow(RobotGeometry::WHEEL_RADIUS,2)*pow(RobotGeometry::WHEEL_BASE,-2))/12+pxtheta;
        kalman_pos->pEst.yy = (pow(deltaS,2)*pow(cos(deltaTheta/2+theta),2)*pow(RobotGeometry::ENC_RES,2)*pow(RobotGeometry::WHEEL_RADIUS,2)*pow(RobotGeometry::WHEEL_BASE,-2))/24+(pow(sin(deltaTheta/2+theta),2)*pow(RobotGeometry::ENC_RES,2)*pow(RobotGeometry::WHEEL_RADIUS,2))/24+2*RobotGeometry::KS*pyy*abs(sin(theta))*abs(deltaS)+pyy;
        kalman_pos->pEst.ytheta = pytheta-(deltaS*cos(deltaTheta/2+theta)*pow(RobotGeometry::ENC_RES,2)*pow(RobotGeometry::WHEEL_RADIUS,2)*pow(RobotGeometry::WHEEL_BASE,-2))/12;
        kalman_pos->pEst.thetatheta = (pow(RobotGeometry::ENC_RES,2)*pow(RobotGeometry::WHEEL_RADIUS,2)*pow(RobotGeometry::WHEEL_BASE,-2))/6+2*RobotGeometry::KTHETA*pthetatheta*abs(deltaTheta)+pthetatheta;

        kalman_pos->odo_meas.odoFlag = false;
    }

    // update step using triangularisation
    if ( kalman_pos->iter ==0 )
        // the iter condition is to reduce the innovation frequency, which is too high (cov. matrix is already small - in terms of eigenvalues norms.)

        if (kalman_pos->triang_flag = true && cvs->inputs->t > -10 )
        {// the t condition is so that we are sure triangulation readings are ok
            // buffers
            double pxx = kalman_pos->pEst.xx;
            double pxy = kalman_pos->pEst.xy;
            double pxtheta = kalman_pos->pEst.xtheta;
            double pyy = kalman_pos->pEst.yy;
            double pytheta = kalman_pos->pEst.ytheta;
            double pthetatheta = kalman_pos->pEst.thetatheta;
            double xEstBuf = kalman_pos->xEst;
            double yEstBuf = kalman_pos->yEst;
            double thetaEstBuf = kalman_pos->thetaEst;
            double xTriang = triang_pos->x;
            double yTriang = triang_pos->y;
            double thetaTriang = triang_pos->theta;

            double det = (pxx+RobotGeometry::OBS_VAR_X)*((RobotGeometry::OBS_VAR_THETA+pthetatheta)*(RobotGeometry::OBS_VAR_Y+pyy)-pytheta*pytheta)+pxtheta*(pxy*pytheta-pxtheta*(pyy+RobotGeometry::OBS_VAR_Y))+pxy*(pxtheta*pytheta-pxy*(pthetatheta+RobotGeometry::OBS_VAR_THETA));
            double kxx(0);
            double kxy(0);
            double kxtheta(0);
            double kyx(0);
            double kyy(0);
            double kytheta(0);
            double kthetax(0);
            double kthetay(0);
            double kthetatheta(0);

            // computing Kalman gain
            if (det>0)
            { // innovation matrix is invertible and really likely to be definite positive
                // obtained via wxmaxima - formal calculation software.

                kxx         = (1./det)*(-pthetatheta*pxy*pxy+2*pxtheta*pxy*pytheta-pxx*pytheta*pytheta+(pthetatheta*pxx-pxtheta*pxtheta)*pyy+(pxx*pyy-pxy*pxy)*RobotGeometry::OBS_VAR_THETA+(pxx*RobotGeometry::OBS_VAR_THETA+pthetatheta*pxx-pxtheta*pxtheta)*RobotGeometry::OBS_VAR_Y );
                kxy         = (1./det)*( (pthetatheta*pxy-pxtheta*pytheta+pxy*RobotGeometry::OBS_VAR_THETA)*RobotGeometry::OBS_VAR_X );
                kxtheta     = (1./det)*( (pxtheta*pyy-pxy*pytheta)*RobotGeometry::OBS_VAR_X+pxtheta*RobotGeometry::OBS_VAR_X*RobotGeometry::OBS_VAR_Y );
                kyx         = (1./det)*( (pthetatheta*pxy-pxtheta*pytheta+pxy*RobotGeometry::OBS_VAR_THETA)*RobotGeometry::OBS_VAR_Y );
                kyy         = (1./det)*( -pthetatheta*pxy*pxy+2*pxtheta*pxy*pytheta-pxx*pytheta*pytheta+(pthetatheta*pxx-pxtheta*pxtheta)*pyy+(pxx*pyy-pxy*pxy)*RobotGeometry::OBS_VAR_THETA+(pyy*RobotGeometry::OBS_VAR_THETA+pthetatheta*pyy-pytheta*pytheta)*RobotGeometry::OBS_VAR_X );
                kytheta     = (1./det)*( (-pxtheta*pxy+pxx*pytheta+pytheta*RobotGeometry::OBS_VAR_X)*RobotGeometry::OBS_VAR_Y );
                kthetax     = (1./det)*( (pxtheta*pyy-pxy*pytheta)*RobotGeometry::OBS_VAR_THETA+pxtheta*RobotGeometry::OBS_VAR_THETA*RobotGeometry::OBS_VAR_Y );
                kthetay     = (1./det)*( (pxx*pytheta-pxtheta*pxy)*RobotGeometry::OBS_VAR_THETA+pytheta*RobotGeometry::OBS_VAR_THETA*RobotGeometry::OBS_VAR_X );
                kthetatheta = (1./det)*( -pthetatheta*pxy*pxy+2*pxtheta*pxy*pytheta-pxx*pytheta*pytheta+(pthetatheta*pxx-pxtheta*pxtheta)*pyy+(pthetatheta*pyy-pytheta*pytheta)*RobotGeometry::OBS_VAR_X+(pthetatheta*RobotGeometry::OBS_VAR_X+pthetatheta*pxx-pxtheta*pxtheta)*RobotGeometry::OBS_VAR_Y );
            }
            else
            {
                printf("WARNING : State cov. matrix not definite positive\n");
                cvs->kalman->initEst(cvs); // cov. matrix back to initialization
            }

            // mean update
            kalman_pos->xEst = kxy*(yTriang-yEstBuf)+kxx*(xTriang-xEstBuf)+xEstBuf+kxtheta*(thetaTriang-thetaEstBuf);
            kalman_pos->yEst = kyy*(yTriang-yEstBuf)+yEstBuf+kyx*(xTriang-xEstBuf)+kytheta*(thetaTriang-thetaEstBuf);
            kalman_pos->thetaEst = kthetay*(yTriang-yEstBuf)+kthetax*(xTriang-xEstBuf)+kthetatheta*(thetaTriang-thetaEstBuf)+thetaEstBuf;

            // covariance update, with Joseph's form to stay in Sn++
            // obtained via wxmaxima - formal calculation software
            kalman_pos->pEst.xx = pow(kxy,2)*RobotGeometry::OBS_VAR_Y+pow(kxx,2)*RobotGeometry::OBS_VAR_X+pow(kxtheta,2)*RobotGeometry::OBS_VAR_THETA+pow(kxy,2)*pyy+2*kxtheta*kxy*pytheta+(2*kxx-2)*kxy*pxy+
                    (1-2*kxx+pow(kxx,2))*pxx+(2*kxtheta*kxx-2*kxtheta)*pxtheta+pow(kxtheta,2)*pthetatheta;
            kalman_pos->pEst.xy = kxy*kyy*RobotGeometry::OBS_VAR_Y+kxx*kyx*RobotGeometry::OBS_VAR_X+kxtheta*kytheta*RobotGeometry::OBS_VAR_THETA+(kxy*kyy-kxy)*pyy+(-kxtheta+kxy*kytheta+kxtheta*kyy)*pytheta+
                    (1-kxx+kxy*kyx+(kxx-1)*kyy)*pxy+(kxx-1)*kyx*pxx+((kxx-1)*kytheta+kxtheta*kyx)*pxtheta+kxtheta*kytheta*pthetatheta;
            kalman_pos->pEst.xtheta = kthetay*kxy*RobotGeometry::OBS_VAR_Y+kthetax*kxx*RobotGeometry::OBS_VAR_X+kxtheta*kthetatheta*RobotGeometry::OBS_VAR_THETA+kthetay*kxy*pyy+(kxtheta*kthetay+(kthetatheta-1)*kxy)*pytheta+
                    (-kthetay+kthetay*kxx+kthetax*kxy)*pxy+(kthetax*kxx-kthetax)*pxx+(1-kthetatheta+kxtheta*kthetax+(kthetatheta-1)*kxx)*pxtheta+
                    (kxtheta*kthetatheta-kxtheta)*pthetatheta;
            kalman_pos->pEst.yy = pow(kyy,2)*RobotGeometry::OBS_VAR_Y+pow(kyx,2)*RobotGeometry::OBS_VAR_X+pow(kytheta,2)*RobotGeometry::OBS_VAR_THETA+(1-2*kyy+pow(kyy,2))*pyy+(2*kytheta*kyy-2*kytheta)*pytheta+
                    (2*kyx*kyy-2*kyx)*pxy+pow(kyx,2)*pxx+2*kytheta*kyx*pxtheta+pow(kytheta,2)*pthetatheta;
            kalman_pos->pEst.ytheta = kthetay*kyy*RobotGeometry::OBS_VAR_Y+kthetax*kyx*RobotGeometry::OBS_VAR_X+kthetatheta*kytheta*RobotGeometry::OBS_VAR_THETA+(kthetay*kyy-kthetay)*pyy+
                    (1-kthetatheta+kthetay*kytheta+(kthetatheta-1)*kyy)*pytheta+(-kthetax+kthetay*kyx+kthetax*kyy)*pxy+kthetax*kyx*pxx+
                    (kthetax*kytheta+(kthetatheta-1)*kyx)*pxtheta+(kthetatheta-1)*kytheta*pthetatheta;
            kalman_pos->pEst.thetatheta = pow(kthetay,2)*RobotGeometry::OBS_VAR_Y+pow(kthetax,2)*RobotGeometry::OBS_VAR_X+pow(kthetatheta,2)*RobotGeometry::OBS_VAR_THETA+pow(kthetay,2)*pyy+(2*kthetatheta-2)*kthetay*pytheta+2*
                    kthetax*kthetay*pxy+pow(kthetax,2)*pxx+(2*kthetatheta-2)*kthetax*pxtheta+(1-2*kthetatheta+pow(kthetatheta,2))*pthetatheta;

            // flag back to false
            kalman_pos->triang_flag = false;

        }

    // taking triangulation values every 20 iterations - odometry can dilate a bit the state's error covariance matrix
    kalman_pos->iter = (kalman_pos->iter+1)%20;

    // Updating robot position
    rob_pos->x = kalman_pos->xEst;
    rob_pos->y = kalman_pos->yEst;
    rob_pos->theta = kalman_pos->thetaEst;

}

KalmanStruct::KalmanStruct() : init(false), xEst(0), yEst(0), thetaEst(0), triang_flag(false)
{
}

void KalmanStruct::initEst(CtrlStruct *cvs)
{
    xEst = cvs->rob_pos->x;
    yEst = cvs->rob_pos->y;
    thetaEst = cvs->rob_pos->theta;

    // diagonal initial cov matrix
    pEst.xx = T1_UNCERT*T1_UNCERT;
    pEst.yy = T2_UNCERT*T2_UNCERT;
    pEst.thetatheta = R3_UNCERT*R3_UNCERT;
}

NAMESPACE_CLOSE();
