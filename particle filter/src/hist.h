#ifndef HIST_H
#define HIST_H

#include <iostream>

/**
 * \brief Class for histogram, implements all the important features and operations
 * with histogram.
 *
 * \class Hist
 */
class Hist
{
public:
    Hist( unsigned int bin = 0);
    Hist( double arr[]);
    ~Hist();
    Hist & operator=(const Hist &rhs);
    Hist & operator=(const  double arr[]);
    Hist operator+(const Hist &rhs);
    Hist operator-(const Hist &rhs);
    Hist operator*(const double n);

    //void printHist(ofstream *log);
    //void printHist(ostream *log);
    
    void clear_values(void);
    void substract( unsigned int bin, double val=1.0);
    void normalize();
    void sum();

    void add( unsigned int bin, double val=1.0);
    
    double *at; ///< Array with histogram values
    unsigned int bins; ///< Number of bins
    
    double suma;

    void show(const std::string& window_name, unsigned int pixels_per_bin, unsigned int hheight);
};

#endif //HIST_H
