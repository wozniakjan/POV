#ifndef TRACKER_H
#define TRACKER_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <string>
#include <iostream>

#include "location_area.h"
#include "object_stack.h"

using namespace std;
using namespace cv;

class Tracker{
public:    
    Tracker(string inputVideoName, string outputVideoName);
    ~Tracker();
    int visualOutput;
    
    void trackRedBall();
    int detectRedBall(Mat frame);
    void trackRedBallPF(Rect starting_position, int nsamples, double dynamicp, double measurep);
	void track(int nsamples = 100, double dynamicp = 15);
	void realOnMouse(int currentEvent, int x, int y, int flags);    
    
private:
    VideoCapture *capture;
    VideoWriter *writer;
    
    ObjectStack<LocationArea> pastPosition;
    int numberOfPastPositions;
    void correctOffBounds(Point *check, Point limit);
    LocationArea findRedBall(Mat frame);
    
    string outputVideoName;
    Size inputVideoSize;

	bool filled;
	bool filling;
	int px1, py1, px2, py2;
	int lambda;
};

#endif //TRACKER_H
