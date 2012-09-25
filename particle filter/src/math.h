#ifndef MATH_H
#define MATH_H

double chi_squared_metric(Hist object_hsv, Hist phist);
double euclide_metric(Hist object_hsv, Hist phist);
double bhattacharya_metric(Hist object_hsv, Hist phist);
double L1_metric(Hist object_hsv, Hist phist);

#endif