#ifndef PARTICLE_H
#define PARTICLE_H

#include <opencv2/core/core.hpp>

#define HSV_VALUES 692
#define H_OFFSET 0
#define S_OFFSET 180
#define V_OFFSET 436

using namespace cv;

class Hist;

typedef struct _Particle {
  double w; //< Weight
  double cw; //< Cumulative weight
  int pos_x, pos_y; //< Position of particle
} Particle;

// H, for hue, is either 0-359 if the color is chromatic (not gray), or meaningless if it is gray. It represents degrees on the color wheel familiar to most people. Red is 0 (degrees), green is 120 and blue is 240.
// S, for saturation, is 0-255, and the bigger it is, the stronger the color is. Grayish colors have saturation near 0; very strong colors have saturation near 255.
// V, for value, is 0-255 and represents lightness or brightness of the color. 0 is black; 255 is as far from black as possible.

// Postup PF : 
// 0. inicializuj vsechny particly
// dokud je snimek
//   1. resample - vyber z n particlu novych n particlu (ty nejpravdepodobnejsi)
//   2. predict - posun je dle pohybu objektu + pridej sum
//   3. zmer nove pi_n
//      uprav kumulativni pi_n
//   4. vypocitej novy stav objektu x z oblasti oznacene particly.
// end
class Particles
{
  public:
    Particles(int n = 10, double dp = 1.0, int mp = 200);
    ~Particles();

    void init_samples(Mat &data, Rect &init_rect);
    void resample();
    void predict();
    void measure(Mat &data);
    void new_state(Mat &data);

    // particles related stuff
    Particle *particles;
    int pnum;
    double dynamic_param;
    int measure_param;
    double mean_pos_x;
    double mean_pos_y;

    // object related stuff
    Hist *object_hsv; //< State of object x, described by HSV histograms
    int width, height;

    void comp_hist(Mat &data, Hist &phist);
    double bhattacharya_metric(Hist &phist);
    double euclide_metric(Hist &phist);
  private:
    void normalize_weights();
    void comp_cumul();

    // other related stuff, like random numbers generators
    RNG uniform_gen; // potreba pro resampling
    RNG gauss_x_gen; // potreba pro predict (shifting)
    RNG gauss_y_gen; // potreba pro predict (shifting)

};

#endif
