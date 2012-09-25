#include "lbp.h"
#include "hist.h"

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>

#define PI 3.141592654

//LBP fluctuation
#define FLUCTUATION (-0)

//#define LBP_TYPE uniform_lbp
//#define LBP_SIZE uniform_lbp_hist_size
#define LBP_TYPE rotation_invariant_lbp
#define LBP_SIZE rotation_invariant_lbp_hist_size



Point *lbp_points;
int *rotation_invariant_lbp;
int *uniform_lbp;
int uniform_lbp_hist_size;
int rotation_invariant_lbp_hist_size;

Hist *int_hist;

//initialization of LBP points and rotation invariation
void init_lbp_points()
{
    int_hist = new Hist(59);

    lbp_points = (Point*)malloc(bits*sizeof(Point));
    double angle_change = (2*PI) / bits;
    double angle = 0;

    unsigned int max_values = pow(2.0,bits);
    unsigned int test_val = max_values / 2;
    rotation_invariant_lbp = (int*)malloc(max_values*sizeof(int));
    uniform_lbp = (int*)malloc(max_values*sizeof(int));

    for(int i = 0; i<bits; i++){
        lbp_points[i].x = floor(cos(angle)*radius + 0.5);
        lbp_points[i].y = floor(sin(angle)*radius + 0.5);
        angle += angle_change;
    }

    //rotation invariant and uniform lbp
    int val, min;
    int i, carry, transitions;
    map< int, vector<int> > support_rotation_invariant, support_uniform;
    for(i = 0; i < max_values; i++){
        min = i;
        val = i;

        transitions = 0;
        carry = (val&1) << bits-1;
        for(int v = 1; v <= bits; v++){
            val = carry | val >> 1;
            if(val < min)
                min = val;

            if( (test_val & carry) ^ (test_val & (val << bits-1)) )
                transitions++;

            carry = (val&1) << bits-1;
        }
        if(transitions < 3){
            support_uniform[i].push_back(i);
        }
        else{
            support_uniform[max_values].push_back(i);
        }
        support_rotation_invariant[min].push_back(i);
    }

    rotation_invariant_lbp_hist_size = support_rotation_invariant.size();
    uniform_lbp_hist_size = support_uniform.size();

    //rotation invariant
    map< int, vector<int> >::iterator m_iter;
    vector<int>::iterator v_iter;
    i = 0; //bin value
    for(m_iter = support_rotation_invariant.begin(); m_iter != support_rotation_invariant.end(); m_iter++){
        for(v_iter = m_iter->second.begin(); v_iter != m_iter->second.end(); v_iter++){
            rotation_invariant_lbp[*v_iter] = i;
        }
        i++;
    }

    //lbp
    i = 0; //bin value
    for(m_iter = support_uniform.begin(); m_iter != support_uniform.end(); m_iter++){
        for(v_iter = m_iter->second.begin(); v_iter != m_iter->second.end(); v_iter++){
            uniform_lbp[*v_iter] = i;
        }
        i++;
    }
}


//returns LBP histogram from already counter LBP pic
Hist lbp_feature_counted(const Mat *pic)
{
  int row, col, val;
  Hist temp(LBP_SIZE);

  for(row = 0; row < pic->rows; row++){
    for(col = 0; col < pic->cols; col++){
        val = pic->at<unsigned char>(row, col);
        temp.at[val] += 1.0;
    }
  }

  return temp;
}


//returns LBP histogram and counts lbp picture
void lbp_feature_pic(const Mat *pic, Mat *lbp)
{
  int row, col, val, val2;

  for(row = 0; row < pic->rows; row++){
    for(col = 0; col < pic->cols; col++){
        val = calculate_value(col, row, *pic);
        lbp->at<unsigned char>(row, col) = (unsigned char)LBP_TYPE[val];
    }
  }
}


int calculate_value(int centerX, int centerY, Mat pic)
{
  int value = 0, diff = 0;

  //calculates LBP
  for(int i = 0; i<bits; i++){
    value = value << 1;
    diff = (int)pic.at<unsigned char>(centerY, centerX) - (int)pic.at<unsigned char>(centerY+lbp_points[i].y, centerX+lbp_points[i].x) + FLUCTUATION;
    if(diff < 0){
      value += 1.0;
    }
  };

  return value;
}
Hist lbp_feature(Mat pic)
{
  int row, col, val;
  Hist temp(LBP_SIZE);

  for(row = 0; row < pic.rows; row++){
    for(col = 0; col < pic.cols; col++){
        val = calculate_value(col, row, pic);
        temp.at[LBP_TYPE[val]]++;
    }
  }
  return temp;
}
