#ifndef _FIR_FILTER_H
#define _FIR_FILTER_H

#include <ap_fixed.h>

#define TAPS 66

typedef ap_fixed<16,1> sig_t;
typedef ap_fixed<16,1> tap_t;
typedef ap_fixed<17,2> out_t;

void fir_double (double x, double h[TAPS], double *y);
void fir_fixed(sig_t x, tap_t h[TAPS], out_t *y);
#endif
