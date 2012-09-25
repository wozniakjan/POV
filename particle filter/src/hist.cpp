#include "hist.h"

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

/*!
    Defines maximum number of bins
*/
#define MAX_BINS 65536

//constructor
Hist::Hist(unsigned int b)
{
    bins = b;
    
    at = ( double*)malloc((b+1) * sizeof( double));

    for(int i = 0; i<b; i++){
        at[i] = 0;
    }

    at[b] = -1;
}

//constructor
Hist::Hist( double arr[])
{
    bins = 0;
    at = NULL;

    if(arr != NULL){
        (*this) = arr;
    }
//pruchod a spocitat bins
}

//destructor
Hist::~Hist()
{
    if(at != NULL);
        free(at);
}

//operators
Hist& Hist::operator=(const Hist &rhs)
{
    if(this->bins < rhs.bins){
        this->bins = rhs.bins;

        if(this->at != NULL)
            free(this->at);

        this->at = ( double*)malloc((bins+2) * sizeof( double));
    }
    else this->bins = rhs.bins;

    for(int i = 0; i < bins; i++){
        this->at[i] = rhs.at[i];
    }

    at[bins] = -1;

    return *this;
}
Hist& Hist::operator=(const double arr[])
{
    int i = 0;
    int size_changed = 0;

    while(arr[i] != -1 && i < MAX_BINS){
        if(this->bins <= i){
            size_changed = 1;
            this->bins += 10;
            this->at = ( double*)realloc(at, (bins+2) * sizeof( double));
        }

        this->at[i] = arr[i];
        i++;
    }
    this->bins = i;

    at[bins] = -1;

    return *this;
}
Hist Hist::operator+(const Hist &rhs)
{
    int min_bins;
    (this->bins < rhs.bins) ? min_bins = this->bins : min_bins = rhs.bins;

    Hist result(min_bins);

    for(int i = 0; i < min_bins; i++){
        result.at[i] = this->at[i] + rhs.at[i];
    }

    result.at[result.bins] = -1.0;
    return result;
}
Hist Hist::operator-(const Hist &rhs)
{
    int min_bins;
    (this->bins < rhs.bins) ? min_bins = this->bins : min_bins = rhs.bins;

    Hist result(min_bins);

    for(int i = 0; i < min_bins; i++){
        result.at[i] = this->at[i] - rhs.at[i];
    }

    result.at[result.bins] = -1.0;
    return result;
}
Hist Hist::operator*(const double n)
{
    Hist result(this->bins);

    for(int i = 0; i < bins; i++){
        result.at[i] = this->at[i] * n;
    }

    return result;
}

//clear val
void Hist::clear_values(void)
{
    for(int i = 0; i < bins; i++){
        at[i] = 0.0;
    }
    at[bins] = -1.0;
}

//substracts target value from target bin
void Hist::substract( unsigned int bin, double val)
{
    at[bin] = at[bin]-val;
    if(at[bin] < 0.0) at[bin] = 0.0;
}
void Hist::add( unsigned int bin, double val)
{
    at[bin] = at[bin]+val;
    if(at[bin] < 0.0) at[bin] = 0.0;
}

//nurmalizuje histogram aby integral pres biny byl roven 1
void Hist::normalize(void)
{
    sum();
    for(int i = 0; i < bins; i++){
        at[i] /= suma;
    }
    suma = 1.0;
}

//clear val
void Hist::sum(void)
{
    suma = 0.0;
    for(int i = 0; i < bins; i++){
        suma += at[i];
    }
}

void Hist::show(const std::string& window_name, unsigned int pixels_per_bin, unsigned int hheight) {
  cv::Mat visual(hheight,pixels_per_bin * bins, CV_8U);
  visual.setTo(255);

  // nalezeni nejvetsi hodnoty, aby to potom bylo nejak normalizovane
  double biggest_value = .0 ;
  for(int i = 0 ; i < bins ; i++) {
    if(at[i] > biggest_value)
      biggest_value = at[i];
  }

  int pix_height = hheight / biggest_value;

  cv::Rect fill_rect;
  for(int x = 0 ; x < bins ; x += pixels_per_bin) {
    cv::rectangle(visual, cv::Point(x, hheight-pix_height*at[(int)(x/pixels_per_bin)]),
                  cv::Point(x+pixels_per_bin, hheight), cv::Scalar(0), -1);
  }

  cv::imshow(window_name, visual);
}
