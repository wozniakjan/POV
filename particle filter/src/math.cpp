#include "hist.h"
#include <math.h>
#include <cmath>

using namespace std;

double chi_squared_metric(Hist object_hsv, Hist phist)
{
    double chi = 0.0;
    double val = 0.0;
    for(int i=0; i<phist.bins; i++){
        val = pow(object_hsv.at[i] - phist.at[i], 2)/(object_hsv.at[i] + phist.at[i]);
        chi += pow(object_hsv.at[i] - phist.at[i], 2)/(object_hsv.at[i] + phist.at[i]);
    }
    return chi;
}

double L1_metric(Hist object_hsv, Hist phist){
    double l1 = 0.0;
    for(int i=0; i<phist.bins; i++){
        l1 += abs(object_hsv.at[i] - phist.at[i]);
    }
    return l1;
}

double euclide_metric(Hist object_hsv, Hist phist)
{
    double res = 0.0;
    for (int i=0; i<phist.bins; i++){
        res += pow(object_hsv.at[i] - phist.at[i], 2);
    }
    return sqrt(res);
}

double bhattacharya_metric(Hist object_hsv, Hist phist)
{
  // pri predpokladu, ze mame distribucni rozlozeni p,q (coz histogramy jsou),
  // muzeme vypocitat vzdalenost
  // (http://en.wikipedia.org/wiki/Bhattacharyya_distance) mezi nimi.
  double bc = 0.0;
  for (int i=0 ; i<phist.bins ; i++) {
    bc += sqrt(object_hsv.at[i] * phist.at[i]);
  }
  return -log(bc);
}

