#ifndef BSINC_TABLES_H
#define BSINC_TABLES_H

#include "bsinc_defs.h"


struct BSincTable {
    float scaleBase, scaleRange;
    unsigned int m[BSincScaleCount];
    unsigned int filterOffset[BSincScaleCount];
    const float *Tab;
};

extern const BSincTable bsinc12;
extern const BSincTable bsinc24;

#endif /* BSINC_TABLES_H */
