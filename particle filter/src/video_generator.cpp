#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#define FPS 25

#define VIDEO_OUTPUT 0

using namespace std; 
using namespace cv;

void printHelp() {
  cout << "Video generator" << endl;
  cout << "  Jednoduchy program pro generovani testovaciho \"blbeho\" videa s ohranicenym objektem (koleckem)" << endl << endl;
  cout << "OPTIONS" << endl;
  cout << "  -h ... tato napoveda" << endl <<
    "  -ot ... jmeno vystupniho textoveho souboru. Defaultne \"text.txt\"" << endl <<
    "  -ov ... jmeno vystupniho video souboru. Defaultne \"video.avi\"" << endl << 
    "  -nb ... nepouzivat obrazek pozadi, pozadi je nastaveno na cernou barvu" << endl;
}

int main( int argc, char* argv[])
{
  Mat bkgPicture;
  string outText = "data/text.txt";
  string outVideo = "data/video.avi";
  Scalar color(0,0,230);

  // zpracovani cmd
  bool useBkgPicture = true;
  for(int i =1 ; i < argc; i++) {
    if(string(argv[i]) == "-h") {
      printHelp();
      return EXIT_SUCCESS;
    }
    else if(string(argv[i]) == "-ot" && i+1 < argc) {
      outText = string(argv[++i]);
    }
    else if(string(argv[i]) == "-ov" && i+1 < argc) {
      outVideo = string(argv[++i]);
    }
    else if(string(argv[i]) == "-nb") {
      useBkgPicture = true;
    }
    else {
      cerr << "Nerozpoznana volba / spatne zadany parametr" << endl;
      return EXIT_FAILURE;
    }
  }

  if(useBkgPicture)
    bkgPicture = imread("data/apple_travel_island_full_hd_background.jpg", -1);
  else
    bkgPicture.setTo(0);

  if(bkgPicture.data == NULL) {
    cerr << "Nemohu otevrit obrazek pozadi" << endl;
    return EXIT_FAILURE;
  }

  namedWindow("video");

  int center_x = bkgPicture.cols/2, center_y = bkgPicture.rows/2;
  int add_x = 0, add_y = 0;
  int radius = 60;
  RNG angle(rand());
  RNG speed;
  RNG frequency(time(NULL));
  VideoWriter outVideoWriter(outVideo, CV_FOURCC('D','I','V','X'), FPS, bkgPicture.size(), true);

  fstream outFile(outText.c_str(), fstream::out);
  
  // pohyb objektem po scene
  for(;;) {
    Mat bkgCopy = bkgPicture.clone();
    circle(bkgCopy, Point(center_x, center_y), radius, color, -1);
    outFile << center_x-radius << " " <<  center_y-radius << " " << (radius<<2) << " " <<  (radius<<2) << endl;
    if(VIDEO_OUTPUT)
      imshow("video", bkgCopy);
    if(abs(frequency.gaussian(10)) > 20) {
      add_x = angle.gaussian(5);
      add_y = speed.gaussian(5);
    }

    if(center_x < 0 || center_x > bkgCopy.cols)
      add_x = -add_x;
    if(center_y < 0 || center_y > bkgCopy.cols)
      add_y = -add_y;

    center_x += add_x;
    center_y += add_y;

    outVideoWriter << bkgCopy;
    
    switch(waitKey(1) & 255) {
      case 'k': // konec
      case 'q': // quit
      case 0x1B: // ESC
        outFile.close();
        return EXIT_SUCCESS;
        break;
    }
  }
}
