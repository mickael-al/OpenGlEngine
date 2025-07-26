#ifndef __SIMPLEX_NOISE__
#define __SIMPLEX_NOISE__

#include <cmath>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm> 

class SimplexNoise {
public:
    SimplexNoise(int seed = 0) {
        perm = std::vector<int>(512);
        for (int i = 0; i < 256; ++i) 
        {
            perm[i] = i;
        }
        std::shuffle(perm.begin(), perm.begin() + 256, std::default_random_engine(seed));
        for (int i = 0; i < 256; ++i) 
        {
            perm[256 + i] = perm[i];
        }
    }

    float noise2D(float x, float y) {
        float s = (x + y) * 0.5f * (sqrt(3.0f) - 1.0f);
        int i = floor(x + s);
        int j = floor(y + s);
        float t = (i + j) * 0.5f * (3.0f - sqrt(3.0f));
        float X0 = i - t, Y0 = j - t;
        float x0 = x - X0, y0 = y - Y0;

        int i1 = (x0 > y0) ? 1 : 0;
        int j1 = (x0 > y0) ? 0 : 1;

        float x1 = x0 - i1 + 0.5f, y1 = y0 - j1 + 0.5f;
        float x2 = x0 - 1.0f + 1.0f, y2 = y0 - 1.0f + 1.0f;

        int gi0 = perm[i + perm[j]] % 12;
        int gi1 = perm[i + i1 + perm[j + j1]] % 12;
        int gi2 = perm[i + 1 + perm[j + 1]] % 12;

        float n0 = grad(gi0, x0, y0);
        float n1 = grad(gi1, x1, y1);
        float n2 = grad(gi2, x2, y2);

        return 70.0f * (n0 + n1 + n2);
    }

    float noise3D(float x, float y, float z) {
        float s = (x + y + z) * (1.0f / 3.0f);
        int i = floor(x + s), j = floor(y + s), k = floor(z + s);
        float t = (i + j + k) * (1.0f / 6.0f);
        float X0 = i - t, Y0 = j - t, Z0 = k - t;
        float x0 = x - X0, y0 = y - Y0, z0 = z - Z0;

        int i1 = (x0 > y0) ? 1 : 0, j1 = (x0 > z0) ? 1 : 0, k1 = (y0 > z0) ? 1 : 0;
        int i2 = (x0 > y0) ? 0 : 1, j2 = (y0 > z0) ? 0 : 1, k2 = (z0 > y0) ? 0 : 1;

        float x1 = x0 - i1 + 0.5f, y1 = y0 - j1 + 0.5f, z1 = z0 - k1 + 0.5f;
        float x2 = x0 - i2 + 1.0f, y2 = y0 - j2 + 1.0f, z2 = z0 - k2 + 1.0f;
        float x3 = x0 - 1.0f + 1.5f, y3 = y0 - 1.0f + 1.5f, z3 = z0 - 1.0f + 1.5f;

        int gi0 = perm[i + perm[j + perm[k]]] % 12;
        int gi1 = perm[i + i1 + perm[j + j1 + perm[k + k1]]] % 12;
        int gi2 = perm[i + i2 + perm[j + j2 + perm[k + k2]]] % 12;
        int gi3 = perm[i + 1 + perm[j + 1 + perm[k + 1]]] % 12;

        float n0 = grad(gi0, x0, y0, z0);
        float n1 = grad(gi1, x1, y1, z1);
        float n2 = grad(gi2, x2, y2, z2);
        float n3 = grad(gi3, x3, y3, z3);

        return 32.0f * (n0 + n1 + n2 + n3);
    }

private:
    std::vector<int> perm;

    float grad(int hash, float x, float y) {
        int h = hash & 15;
        float u = h < 8 ? x : y, v = h < 4 ? y : (h == 12 || h == 14 ? x : 0);
        return ((h & 8) ? -1 : 1) * (u + v);
    }

    float grad(int hash, float x, float y, float z) {
        int h = hash & 15;
        float u = h < 8 ? x : y, v = h < 4 ? y : z, w = h < 4 ? z : 0;
        return ((h & 8) ? -1 : 1) * (u + v + w);
    }
};

#endif //!__SIMPLEX_NOISE__