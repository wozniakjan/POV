#include <stdlib.h>
#include <iostream>
#include <assert.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "particle.h"
#include "hist.h"

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
  Mat img = imread("../data/apple_travel_island_full_hd_background.jpg", 1);
  Mat hsv;
  cvtColor(img, hsv, CV_BGR2HSV);

  // !!! constructor test !!!
  //
  cout << "CONSTRUCTOR TEST" << endl;
  Particles p(5, 5, 3.2);

  // should pnum = 20, dp = 5, mp = 3.2
  assert(p.pnum == 5);
  assert(p.dynamic_param == 5);
  assert(p.measure_param == 3.2);
  cout << "----------DONE-----------" << endl;

  // !!! init samples test !!!
  //
  cout << "INIT SAMPLES TEST" << endl;
  Rect refRect = Rect(400,400,30,30);
  p.init_samples(hsv, refRect);
  // should ...
  assert(p.particles[0].pos_x == 415);
  assert(p.particles[0].pos_y == 415);
  assert(p.particles[0].w = 0.20);
  assert(p.particles[0].cw = 0.20);
  assert(p.particles[1].cw = 0.40);
  assert(p.particles[4].cw = 1);
  // this histogram seems legit :)
  for(int i=0 ; p.object_hsv->at[i] != -1 ; i++) {
    cout << p.object_hsv->at[i] << " ";
  }
  cout << endl;
  cout << "----------DONE-----------" << endl;

  // !!! resample test !!!
  //
  cout << "RESAMPLING TEST" << endl;
  for(int i=0 ; i<p.pnum ; i++)
    cout << "  (" << p.particles[i].pos_x << "," << p.particles[i].pos_y << ")" << endl;
  p.resample();
  for(int i=0 ; i<p.pnum ; i++)
    cout << "  (" << p.particles[i].pos_x << "," << p.particles[i].pos_y << ")" << endl;
  cout << "----------DONE-----------" << endl;

  // !!! predict test !!!
  //
  cout << "PREDICTION TEST" << endl;
  for(int i=0 ; i<p.pnum ; i++)
    cout << "  (" << p.particles[i].pos_x << "," << p.particles[i].pos_y << ")" << endl;
  p.predict();
  for(int i=0 ; i<p.pnum ; i++)
    cout << "  (" << p.particles[i].pos_x << "," << p.particles[i].pos_y << ")" << endl;
  cout << "----------DONE-----------" << endl;

  // !!! measure test
  //
  cout << "MEASURE TEST" << endl;
  p.measure(hsv);
  for(int i=0 ; i<p.pnum ; i++)
    cout << "  (" << p.particles[i].pos_x << "," << p.particles[i].pos_y << ")  "  << p.particles[i].w << endl;
  cout << "----------DONE-----------" << endl;

  // !!! new state test
  //
  cout << "NEW STATE TEST" << endl;
  p.new_state(hsv);
  cout << p.width << " " << p.height << endl;
  cout << "----------DONE-----------" << endl;


  // !!! normalize weights test
  //
  cout << "NORMALIZE TEST" << endl;
  double sum = 0.0;
  for(int i=0 ; i<p.pnum ; i++) {
    sum += p.particles[i].w ;
  }
  assert(sum < 1.0001 && sum > 0.9999);
  cout << "----------DONE-----------" << endl;

  // !!! comp cumul test
  //
  cout << "COMP CUMUL TEST" << endl;
  double prev = 0.0;
  for(int i=0 ; i<p.pnum ; i++) {
    assert(prev <= p.particles[i].cw);
  }
  assert(p.particles[p.pnum -1].cw > 0.9999 && p.particles[p.pnum -1].cw< 1.0001);
  cout << "----------DONE-----------" << endl;


  // !!! comp hist test
  //
  cout << "COMP HIST TEST" << endl;
  Hist testHist(HSV_VALUES);
  Mat testData = hsv(Rect(700,700, 30,30));
  p.comp_hist(testData, testHist);
  for(int i=0 ; testHist.at[i] != -1 ; i++) {
    cout << testHist.at[i] << " ";
  }
  cout << endl;
  cout << "----------DONE-----------" << endl;

  // !!! bhattacharya distance test
  //
  cout << "BHATTACHARYA TEST" << endl;
  cout << p.bhattacharya_metric(testHist) << endl;
  cout << "----------DONE-----------" << endl;

  return EXIT_SUCCESS;
}
