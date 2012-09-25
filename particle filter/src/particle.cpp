#include "particle.h"
#include "hist.h"
#include "lbp.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;

Particles::Particles(int n, double dp, int mp) 
{
  particles = (Particle*)malloc( n * sizeof(Particle) );
  pnum = n;
  dynamic_param = dp;
  measure_param = mp;
   
  object_hsv = new Hist(HSV_VALUES);

  uniform_gen = RNG(1685); // TODO : chtelo by to dle casu, at je to aspon trochu pseudo nahodne
  gauss_x_gen = RNG(1486984); // ---||---
  gauss_y_gen = RNG(954245); // ---||---
}

Particles::~Particles()
{
  free(particles);
}

void Particles::init_samples(Mat &data, Rect &init_rect)
{
  double new_weight = 1.0 / pnum;
  double cumul = 0;
  int x = init_rect.x + (init_rect.width / 2);
  int y = init_rect.y + (init_rect.height / 2);

  // init all samples to ekvivalent values
  for (int i=0 ; i<pnum ; i++) {
    cumul += new_weight;
    particles[i].w = new_weight;
    particles[i].cw = cumul;
    particles[i].pos_x = x;
    particles[i].pos_y = y;
  }

  // init object x - set HSV + size
  width = init_rect.width;
  height = init_rect.height;
  Mat refMat = data(init_rect);
  Vec3s value;
  double bit = 1.0/(refMat.rows * refMat.cols);
  for(int row=0 ; row<refMat.rows ; row++) {
    for(int col=0 ; col<refMat.cols ; col++) {
      value = refMat.at<Vec3b>(row, col);
      object_hsv->at[value[0]+H_OFFSET] += bit; // hue
      object_hsv->at[value[1]+S_OFFSET] += bit; // saturation
      object_hsv->at[value[2]+V_OFFSET] += bit; // value
    }
  }

  mean_pos_x = x ;
  mean_pos_y = y ;
}

void Particles::resample()
{
  Particle *new_part = (Particle*)malloc(pnum * sizeof(Particle));

  for(int i=0 ; i<pnum ; i++) {
    // vygeneruj si cislo od 0 do 1
    double r = uniform_gen.uniform(0.0, 1.0);
    // najdi nejmensi index (j)
    int j = 0;
    while(j < pnum && particles[j].cw < r) // TODO : udelat to binarnim vyhledavanim
      j++;
    // prirad particlu s indexem (j) na pozici particly s indexem (i)
    new_part[i] = particles[j];
  }

  free(particles);
  particles = new_part;
}

void Particles::predict()
{
  // shiftnuti dle pohybu - TODO : zatim nepouzivam
  // shiftnuti dle diffuze - nejaky Gaussovky pohyb
  for(int i=0 ; i<pnum ; i++) {
    // v podstate zasumeni pozic
    particles[i].pos_x += gauss_x_gen.gaussian(dynamic_param);
    particles[i].pos_y += gauss_y_gen.gaussian(dynamic_param);
  }
}

void Particles::measure(Mat &data)
{
  // zmereni novych vah castic
  // -- b = Bhattacharya distance
  // -- exp^(-1 * measure_param * b)
  double b = 0.0;
  Hist phist(HSV_VALUES);
  Mat refMat;
  Rect refRect;
  for (int i=0 ; i<pnum ; i++) {
    phist.clear_values();
    refRect = Rect(particles[i].pos_x - (width>>1), particles[i].pos_y - (height>>1), width, height);

	if(refRect.width + refRect.x >= data.cols || refRect.height + refRect.y >= data.rows)
		continue;

	if(refRect.x < 1 || refRect.y < 1)
		continue;

    refMat = data(refRect);

    Mat gray;     //prevod do grayscale pro lbp;
    cvtColor(refMat, gray, CV_RGB2GRAY);
    comp_hist(refMat, phist); //phist = lbp_feature(gray); //
    b = euclide_metric(phist); //b = euclide_metric(phist); //
    particles[i].w = exp(-measure_param * b);
  }
  // normalizace vah castic
  normalize_weights();
  // vypocitani novych kumulativnich vah
  comp_cumul();
}

void Particles::new_state(Mat &data)
{
  // TODO: predpoklad toho, ze velikost objektu se v case nemeni !! Velikost
  // oken se tedy poresi pozdeji
  double big = -0.1;
  int pos = -1;
  mean_pos_x = 0;
  mean_pos_y = 0;
  for(int i=0 ; i<pnum ; i++) {
    // novy stred objektu vezmeme jako soucet vahovanych stredu castic
    if(particles[i].w > big) {
      big = particles[i].w;
      pos = i;
    }
    mean_pos_x += particles[i].w * particles[i].pos_x;
    mean_pos_y += particles[i].w * particles[i].pos_y;
  }
  // mean_pos_x += width;
  // mean_pos_y += height;

  int new_pos_x=0, new_pos_y=0;
  new_pos_x = particles[pos].pos_x;
  new_pos_y = particles[pos].pos_y;
}

void Particles::normalize_weights() {
  double sum = 0.0;

  for (int i=0 ; i<pnum ; i++)
    sum += particles[i].w;

  for (int i=0 ; i<pnum ; i++)
    particles[i].w /= sum;
}

void Particles::comp_cumul() {
  double cumul = 0.0;

  for (int i=0 ; i<pnum ; i++) {
    cumul += particles[i].w;
    particles[i].cw = cumul;
  }
}

double Particles::euclide_metric(Hist &phist)
{
    double res = 0.0;
    for (int i=0; i<phist.bins; i++){
        res += pow(object_hsv->at[i] - phist.at[i], 2);
    }
    return sqrt(res);
}

double Particles::bhattacharya_metric(Hist &phist)
{
  // pri predpokladu, ze mame distribucni rozlozeni p,q (coz histogramy jsou),
  // muzeme vypocitat vzdalenost
  // (http://en.wikipedia.org/wiki/Bhattacharyya_distance) mezi nimi.
  double bc = 0.0;
  for (int i=0 ; i<phist.bins ; i++) {
    bc += sqrt(object_hsv->at[i] * phist.at[i]);
  }
  return -log(bc);
}


void Particles::comp_hist(Mat &data, Hist &phist) {
  Vec3b value;
  double bit = 1.0/(data.rows * data.cols);
  for(int row=0 ; row<data.rows ; row++) {
    for(int col=0 ; col<data.cols ; col++) {
      value = data.at<Vec3b>(row, col);
      phist.at[value[0]+H_OFFSET] += bit; // hue
      phist.at[value[1]+S_OFFSET] += bit; // saturation
      phist.at[value[2]+V_OFFSET] += bit; // value
    }
  }
}

#if 0
#include "particle.h"
#include "hist.h"
#include "lbp.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;

Particles::Particles(int n, double dp, double mp) 
{
  particles = (Particle*)malloc( n * sizeof(Particle) );
  pnum = n;
  dynamic_param = dp;
  measure_param = mp;
   
  object_hsv = new Hist(HSV_VALUES);

  uniform_gen = RNG(1685); // TODO : chtelo by to dle casu, at je to aspon trochu pseudo nahodne
  gauss_x_gen = RNG(1486984); // ---||---
  gauss_y_gen = RNG(954245); // ---||---
}

Particles::~Particles()
{
  free(particles);
}

void Particles::init_samples(Mat &data, Rect &init_rect)
{
  double new_weight = 1.0 / pnum;
  double cumul = 0;
  int x = init_rect.x + (init_rect.width / 2);
  int y = init_rect.y + (init_rect.height / 2);

  // init all samples to ekvivalent values
  for (int i=0 ; i<pnum ; i++) {
    cumul += new_weight;
    particles[i].w = new_weight;
    particles[i].cw = cumul;
    particles[i].pos_x = x;
    particles[i].pos_y = y;
  }

  // init object x - set HSV + size
  width = init_rect.width;
  height = init_rect.height;
  Mat refMat = data(init_rect);
  Vec3s value;
  double bit = 1.0/(refMat.rows * refMat.cols);
  for(int row=0 ; row<refMat.rows ; row++) {
    for(int col=0 ; col<refMat.cols ; col++) {
      value = refMat.at<Vec3b>(row, col);
      object_hsv->at[value[0]+H_OFFSET] += bit; // hue
      object_hsv->at[value[1]+S_OFFSET] += bit; // saturation
      object_hsv->at[value[2]+V_OFFSET] += bit; // value
    }
  }
}

void Particles::resample()
{
  Particle *new_part = (Particle*)malloc(pnum * sizeof(Particle));

  for(int i=0 ; i<pnum ; i++) {
    // vygeneruj si cislo od 0 do 1
    double r = uniform_gen.uniform(0.0, 1.0);
    // najdi nejmensi index (j)
    int j = 0;
    while(j < pnum && particles[j].cw < r) // TODO : udelat to binarnim vyhledavanim
      j++;
    // prirad particlu s indexem (j) na pozici particly s indexem (i)
    new_part[i] = particles[j];
  }

  free(particles);
  particles = new_part;
}

void Particles::predict()
{
  // shiftnuti dle pohybu - TODO : zatim nepouzivam
  // shiftnuti dle diffuze - nejaky Gaussovky pohyb
  for(int i=0 ; i<pnum ; i++) {
    // v podstate zasumeni pozic
    particles[i].pos_x += gauss_x_gen.gaussian(dynamic_param);
    particles[i].pos_y += gauss_y_gen.gaussian(dynamic_param);
  }
}

void Particles::measure(Mat &data)
{
  // zmereni novych vah castic
  // -- b = Bhattacharya distance
  // -- exp^(-1 * measure_param * b)
  double b = 0.0;
  Hist phist(HSV_VALUES);
  Mat refMat;
  Rect refRect;
  for (int i=0 ; i<pnum ; i++) {
    phist.clear_values();
    refRect = Rect(particles[i].pos_x - (width>>1), particles[i].pos_y - (height>>1), width, height);
    refMat = data(refRect);

    //Mat gray;     //prevod do grayscale pro lbp;
    //cvtColor(refMat, gray, CV_RGB2GRAY);
    comp_hist(refMat, phist); //phist = lbp_feature(refMat); //
    bhattacharya_metric(phist); //b = euclide_metric(phist); //
    particles[i].w = exp(-measure_param * b);
  }
  // normalizace vah castic
  normalize_weights();
  // vypocitani novych kumulativnich vah
  comp_cumul();
}

void Particles::new_state(Mat &data)
{
  // TODO: predpoklad toho, ze velikost objektu se v case nemeni !! Velikost
  // oken se tedy poresi pozdeji
  double big = -0.1;
  int pos = -1;
  for(int i=0 ; i<pnum ; i++) {
    // novy stred objektu vezmeme jako soucet vahovanych stredu castic
    if(particles[i].w > big) {
      big = particles[i].w;
      pos = i;
    }
  }

  int new_pos_x=0, new_pos_y=0;
  new_pos_x = particles[pos].pos_x;
  new_pos_y = particles[pos].pos_y;

  // nyni vypocteme z dane oblasti novy object_hsv
  Vec3b value;
  object_hsv->clear_values();
  double bit = 1.0/(width*height);
  for(int i=new_pos_y - (height/2) ; i<new_pos_y+(height/2) ; i++) {
    for(int j=new_pos_x - (width/2) ; j<new_pos_x+(width/2) ; j++) {
      value = data.at<Vec3b>(i,j);
      object_hsv->at[value[0]+H_OFFSET] += bit; // hue
      object_hsv->at[value[1]+S_OFFSET] += bit; // saturation
      object_hsv->at[value[2]+V_OFFSET] += bit; // value
    }
  }

  // nyni mame objekt popsany novym histogramem, ktery byl vypocitan z castic a
  // cely dej se opakuje znova a znova ...
}

void Particles::normalize_weights() {
  double sum = 0.0;

  for (int i=0 ; i<pnum ; i++)
    sum += particles[i].w;

  for (int i=0 ; i<pnum ; i++)
    particles[i].w /= sum;
}

void Particles::comp_cumul() {
  double cumul = 0.0;

  for (int i=0 ; i<pnum ; i++) {
    cumul += particles[i].w;
    particles[i].cw = cumul;
  }
}

double Particles::euclide_metric(Hist &phist)
{
    double res = 0.0;
    for (int i=0; i<phist.bins; i++){
        res += pow(object_hsv->at[i] - phist.at[i], 2);
    }
    return sqrt(res);
}

double Particles::bhattacharya_metric(Hist &phist)
{
  // pri predpokladu, ze mame distribucni rozlozeni p,q (coz histogramy jsou),
  // muzeme vypocitat vzdalenost
  // (http://en.wikipedia.org/wiki/Bhattacharyya_distance) mezi nimi.
  double bc = 0.0;
  for (int i=0 ; i<HSV_VALUES ; i++) {
    bc += sqrt(object_hsv->at[i] * phist.at[i]);
  }
  return -log(bc);
}


void Particles::comp_hist(Mat &data, Hist &phist) {
  Vec3b value;
  double bit = 1.0/(data.rows * data.cols);
  for(int row=0 ; row<data.rows ; row++) {
    for(int col=0 ; col<data.cols ; col++) {
      value = data.at<Vec3b>(row, col);
      phist.at[value[0]+H_OFFSET] += bit; // hue
      phist.at[value[1]+S_OFFSET] += bit; // saturation
      phist.at[value[2]+V_OFFSET] += bit; // value
    }
  }
}

#endif
