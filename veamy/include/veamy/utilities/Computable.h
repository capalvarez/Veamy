#ifndef VEAMY_COMPUTABLE_H
#define VEAMY_COMPUTABLE_H

class Computable {
public:
    virtual double apply(double x, double y, int index) = 0;
};

#endif