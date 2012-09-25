#include "tracker.h"
#include "particle.h"
#include "lbp.h"
#include "hist.h"

#include <vector>

//******************************************************************

void Tracker::realOnMouse(int currentEvent, int x, int y, int flags)
{
	if(currentEvent == CV_EVENT_LBUTTONDOWN){
		filling = true;
		px1 = x;
		py1 = y;
		px2 = x;
		py2 = y;
	} else if(currentEvent == CV_EVENT_MOUSEMOVE && filling){
		filling = true;
		px2 = x;
		py2 = y;
	} else if(currentEvent == CV_EVENT_LBUTTONUP && filling){
		filled = true;
		px2 = x; 
		py2 = y;
	}
}

// This is a function, not a class method
void wrappedOnMouse(int currentEvent, int x, int y, int flags, void* ptr)
{
    Tracker* mcPtr = (Tracker*)ptr;
    if(mcPtr != NULL)
        mcPtr->realOnMouse(currentEvent, x, y, flags);
}

//*******************************************************************


Tracker::Tracker(string in, string out){
    outputVideoName = out;
    visualOutput = 1;
    numberOfPastPositions = 0;
	px1 = py1 = px2 = py2 = 0;
    
    init_lbp_points();
    
    capture = new VideoCapture(in);
    if( !capture->isOpened() ){
        cerr << "Vstupni video nelze otevrit" << endl;
        return;
    }
    capture->set(CV_CAP_PROP_CONVERT_RGB, 1.0);
    inputVideoSize.width = capture->get(CV_CAP_PROP_FRAME_WIDTH);
    inputVideoSize.height = capture->get(CV_CAP_PROP_FRAME_HEIGHT);
    
    //otevre se stejnymi parametry jako je input
    writer = new VideoWriter( out, capture->get(CV_CAP_PROP_FOURCC), capture->get(CV_CAP_PROP_FPS), Size(capture->get(CV_CAP_PROP_FRAME_WIDTH),capture->get(CV_CAP_PROP_FRAME_HEIGHT)));
    if( !writer->isOpened() ){
        cerr << "Nemozno otevrit vystup" << endl;
        return;
    }
}

Tracker::~Tracker(){
    if(writer != NULL) delete(writer);
    if(capture != NULL) delete(capture);
}

void Tracker::track(int nsamples, double dynamicp)
{
	Mat frame;
	Mat hsvFrame;
	bool finishInnerLoop = false;
	Particles pf(nsamples, dynamicp);
	bool wasInit = false;
  
	namedWindow("fr", CV_WINDOW_KEEPRATIO);
	createTrackbar("kapa", "fr", &(pf.measure_param), 1000, NULL);
	setMouseCallback("fr", wrappedOnMouse, (void*)this);

	do{
		(*capture) >> frame;

		if(!frame.empty()){

			if(wasInit){
				cvtColor(frame, hsvFrame , CV_RGB2HSV);
				pf.resample();
				pf.predict();
				pf.measure(hsvFrame);
				pf.new_state(hsvFrame);

				for(int i=0 ; i<pf.pnum ; i++) {
					circle(frame, Point(pf.particles[i].pos_x, pf.particles[i].pos_y), 5, 
					       Scalar(0,0,255));
					circle(frame, Point((int)pf.mean_pos_x, (int)pf.mean_pos_y), 5, 
						   Scalar(255,0,0), -1);
					// rectangle(frame, Point(pf.particles[i].pos_x + (pf.width>>1), pf.particles[i].pos_y + (pf.height>>1)),
					//           Point(pf.particles[i].pos_x - (pf.width>>1), pf.particles[i].pos_y - (pf.height>>1)),
					//           Scalar(0,255,0));
				}
			}

			imshow("fr", frame);

			finishInnerLoop = false;
			switch(waitKey(2) & 255){
				case 't': // zastaveni prehravani a moznost oznacit objekt mysi

					filling = false;
					filled = false;
						
					while(!finishInnerLoop){
						Mat frameCopy = frame.clone();

						// vykresleni obdelniku, pokud tahnu mysi
						if(filling)
							rectangle(frameCopy, Point(px1, py1), Point(px2, py2), Scalar(255), 2);

						if(filled){
							filling = false;
							filled = false;
						}

						imshow("fr", frameCopy);

						switch(waitKey(2) & 255){
							case 't':
							case ' ':
								finishInnerLoop = true;
								Rect rct(Point(px1,py1), Point(px2,py2));
								if(rct.width <= 0 || rct.height <= 0)
									break;
								cvtColor(frame, hsvFrame , CV_RGB2HSV);
								pf.init_samples(hsvFrame, rct);
								wasInit = true;
								break;
						}
					}
					break;
			}

			writer->write(frame);
		}
	} while( !frame.empty() );
} 

/*
void Tracker::trackRedBallPF(Rect starting_position, int nsamples, double dynamicp, double measurep) {
  Mat frame;
  Mat hsvFrame;
  Particles pf(nsamples, dynamicp, measurep);

  // potrebuju jeden frame k inicializaci PF
  (*capture) >> frame;
  if(!frame.empty()) {
    cvtColor(frame, hsvFrame , CV_RGB2HSV);
    pf.init_samples(hsvFrame, starting_position);
  }
  
  do{
    (*capture) >> frame;
    if( !frame.empty() ){
      cvtColor(frame, hsvFrame , CV_RGB2HSV);
      pf.resample();
      pf.predict();
      pf.measure(hsvFrame);
      pf.new_state(hsvFrame);
      for(int i=0 ; i<pf.pnum ; i++) {
        circle(frame, Point(pf.particles[i].pos_x, pf.particles[i].pos_y), 5, 
               Scalar(0,0,255));
        circle(frame, Point((int)pf.mean_pos_x, (int)pf.mean_pos_y), 5, 
               Scalar(255,0,0), -1);
        rectangle(frame, Point(pf.particles[i].pos_x + (pf.width>>1), pf.particles[i].pos_y + (pf.height>>1)),
                  Point(pf.particles[i].pos_x - (pf.width>>1), pf.particles[i].pos_y - (pf.height>>1)),
                  Scalar(0,255,0));
      }
      if(VIDEO_OUTPUT){
        namedWindow("hist", CV_WINDOW_KEEPRATIO);
        pf.object_hsv->show("hist",2,300);
        namedWindow("fr", CV_WINDOW_KEEPRATIO);
        imshow("fr", frame); waitKey(38);
        writer->write( frame);
      }
    }
  } while( !frame.empty() );
}
*/

void Tracker::correctOffBounds(Point *check, Point limit){
    if(check->x < 0) check->x = 0;
    if(check->y < 0) check->y = 0;
    if(check->x > limit.x) check->x = limit.x;
    if(check->y > limit.y) check->y = limit.y;
}

int Tracker::detectRedBall(Mat frame){
    Mat roiFrame;
    Size size = Size(0, 0);
    Vec3b red1(55, 55, 200);
    Point upCorner(0, 0);
    int foundObjectsCount = 0;
    
    if(pastPosition.count) {
        LocationArea lp = pastPosition[0];
        size.width = lp.width * 8;
        size.height = lp.height * 8;
        
        upCorner = lp.tl() - Point(size);
        Point downCorner = lp.br() + Point(size);
        Point limit(frame.cols, frame.rows);
        
        correctOffBounds(&upCorner, limit);
        correctOffBounds(&downCorner, limit);
        
        roiFrame = Mat(frame, Rect(upCorner, downCorner));
    }
    else{
        roiFrame = frame;
    }

    LocationArea found = findRedBall(roiFrame);// + upCorner;
    found.x += upCorner.x;
    found.y += upCorner.y;
    pastPosition.insert(found);
    
    return 1;
}

LocationArea Tracker::findRedBall(Mat roiFrame){
    vector<Point> points;
    Point average_point(-1,-1);
    int sumX = 0, sumY = 0;
    LocationArea ball_area;

    Vec3b red1(55, 55, 200);
    
    for(int r = 0; r < roiFrame.rows; r++){
        for(int c = 0; c < roiFrame.cols; c++){
            if(red1[0] > roiFrame.at<Vec3b>(r,c)[0] &&
               red1[1] > roiFrame.at<Vec3b>(r,c)[1] &&
               red1[2] < roiFrame.at<Vec3b>(r,c)[2]) {
                points.push_back(Point(c,r));
                sumX += c;
                sumY += r;
            }
        }
    }
    
    if(!points.empty()){
        int size = points.size();
        average_point.x = sumX / size;
        average_point.y = sumY / size;
        
        double varianceX = 0, varianceY = 0; 
        int minX = average_point.x, maxX = average_point.x, minY = average_point.y, maxY = average_point.y;
        
        for(vector<Point>::iterator i = points.begin(); i != points.end(); i++){
            varianceX += abs(i->x - average_point.x);
            varianceY += abs(i->y - average_point.y);
            if(maxX < i->x) maxX = i->x;
            if(minX > i->x) minX = i->x;
            if(maxY < i->y) maxY = i->y;
            if(minY > i->y) minY = i->y;//cout << *i << " variance: " << Point(varianceX, varianceY) << endl;
        }
        //cout << "avg" << average_point << " variance: " << Point(varianceX/size, varianceY/size) << size << endl;
        ball_area.x = minX;
        ball_area.y = minY;
        ball_area.width = maxX - minX;
        ball_area.height = maxY - minY;
    }
    
    return ball_area;
}
