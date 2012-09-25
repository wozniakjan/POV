//#ifndef LBP_H
//#define LBP_H


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <fstream>

/*!
    Defines actual LBP radius
*/
#define radius 1

/*!
    Defines actual LBP bit count
*/
#define bits 8

class Hist;

using namespace cv;
using namespace std;


extern int uniform_lbp_hist_size;               ///< Size of uniform lbp histogram
extern int rotation_invariant_lbp_hist_size;    ///< Size of rotation invariant lbp histogram
extern int *rotation_invariant_lbp;             ///< Pre counted rotation invariant LBP array
extern int *uniform_lbp;                        ///< Pre counted uniform LBP array
extern Hist *int_hist;                          ///< used histogram

/**
* \brief Initializes all important LBP arrays and variables, precounts LBP value arrays
**/
void init_lbp_points();

/**
* \brief Calculates LBP value of target pixel in target position
*
* \centerX X coordinates of center
* \centerY Y coordinates of center
* \pic Matrix in which LBP are counted
*
* \returns LBP value
**/
int calculate_value(int centerX, int centerY, Mat pic);


/**
* \brief Precalculates LBP picture
*
* \pic Matrix in which LBP are counted
* \lbp Matrix with LBP values
**/
void lbp_feature_pic(const Mat *pic, Mat *lbp);

/**
* \brief Calculates LBP histogram from target picture
*
* \original picture from which LBP features are counted
*
* \returns LBP histogram
**/
Hist lbp_feature(Mat original);

/**
* \brief Calculates LBP histogram from pre counted LBP picture
*
* \pic LBP picture
*
* \returns LBP histogram
**/
Hist lbp_feature_counted(const Mat *pic);

//#endif //LBP_H
