#ifndef TRIANGLE_H
#define TRIANGLE_H


typedef struct Triangle {
        Triangle(int v0=-1,int v1=-2,int v2=-3): v0(v0), v1(v1), v2(v2) {}
        Triangle(const Triangle &t): v0(t.v0), v1(t.v1), v2(t.v2) {}
        Triangle& operator=(const Triangle& other) = default;
        int v0, v1, v2;
} Triangle;


#endif
