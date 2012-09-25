#include <bitset>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

#define BIT_COUNT 256
#define DOUBLE_BIT_COUNT BIT_COUNT*2
#define PATCH_SIZE 64

using namespace std;
using namespace cv;

typedef bitset<BIT_COUNT> BRIEF;

Point brief_array[DOUBLE_BIT_COUNT];

void init_brief(){
    srand ( time(NULL) );
    
    Mat test(Size(PATCH_SIZE, PATCH_SIZE), CV_8UC1);
    
    for(int i = 0; i<DOUBLE_BIT_COUNT; i+=2){
        brief_array[i] = Point(rand()%PATCH_SIZE, rand()%PATCH_SIZE);
        brief_array[i+1] = Point(rand()%PATCH_SIZE, rand()%PATCH_SIZE);
        line(test, brief_array[i], brief_array[i+1], Scalar(255, 255, 255));
    }
    
    imwrite("test.jpg", test);
}

int hamming_distance(BRIEF first, BRIEF second){
    return int((first^second).count());
}

BRIEF get_brief(Mat patch){   
    BRIEF brief(0ul);
    Point a, b;
    GaussianBlur(patch, patch, Size(9, 9), 2.0, 2.0);
    for(int i = 0, j = 0; i < DOUBLE_BIT_COUNT; i+=2, j++){
        a = brief_array[i];
        b = brief_array[i+1];
        
        if(patch.at<char>(a) > patch.at<char>(b)){
            brief.set(j);
        }
    }
    
    return brief;
}