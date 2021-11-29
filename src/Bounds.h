#pragma once
#ifndef Bounds_H
#define Bounds_H

struct Bounds {
    float maxX;
    float maxY;
    float maxZ;
    float minX;
    float minY;
    float minZ;
    Bounds() {};
    Bounds(float aX, float aY, float aZ, float iX, float iY, float iZ) :
        maxX(aX), maxY(aY), maxZ(aZ),
        minX(iX), minY(iY), minZ(iZ) {
    };
};

#endif