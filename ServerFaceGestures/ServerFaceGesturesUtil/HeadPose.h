#pragma once

#ifndef _HEADPOSE_H_
#define _HEADPOSE_H_

#include "FaceTrackerUtil.h"
#include <opencv/cv.h>

class FACE_API HeadPose
{
public:
    HeadPose    ( void );
    ~HeadPose   ( void );

    void EstimateHeadPose   ( void );
    void Dump               ( void );
    void CreateStat         ( IplImage* pFrame, CvRect* pFaceRect );

    // http://planning.cs.uiuc.edu/node103.html
    // https://code.ros.org/trac/opencv/browser/trunk/opencv/tests/cv/src/aposit.cpp?rev=3913
    // http://www.morethantechnical.com/2010/03/19/quick-and-easy-head-pose-estimation-with-opencv-w-code/
    int         Distance            (void) const;
    CvMatr32f   RotationMatrix      (void) const;
    float*      Angles              (void);      // a[0]: yaw, a[1]: pitch, a[2]: roll
    CvVect32f   TranslationVector   (void) const;
    void        ImagePoints         (vector<CvPoint2D32f> pVal);

private:
    void CreateModelPoints  ( void );
    void DrawXYPlane        ( IplImage* pFrame, CvRect* pFaceRect );
	void DrawXZPlane        ( IplImage* pFrame, CvRect* pFaceRect );
    void DisplayLine        ( IplImage* pFrame, CvPoint2D32f pPt1, CvPoint2D32f pPt2, CvScalar pRgb );

    vector<CvPoint2D32f>    _imagePoints;
    vector<CvPoint3D32f>    _modelPoints;        // Must be populated before calling POSIT
    int                     _distance;
    float                   _angles[3];

    CvPOSITObject*          _positObject;
    CvMatr32f               _rotationMatrix;     // POSIT-returned rotation matrix
    CvVect32f               _translationVector;  // t[0]: left/right, t[1]: up/down, t[2]: forward/backward
    CvTermCriteria          _criteria;           // Set POSIT termination criteria: 100 max iterations, convergence epsilon 1.0e-4f
};

#endif //_HEADPOSE_H_