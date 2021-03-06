#ifndef __HELPER_H__
#define __HELPER_H__

// helper.h contains all the helper functions
// related to mathematics, strings
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include "ex.h"

// For stringing the value of macros
#define STR(X) #X
#define SSTR(X) STR(X)

// To delete a certain line
#define DELETE "\033[A\033[2K"

namespace Math {

    const float PRECISION = 0.0000005;

    // Value of pi
    const float pi = 3.14159;

    // function to round a floating number
    inline int round(float a){
        return a>=0 ? a+0.5 : a-0.5;
    }

    // function to determine if two floats are
    // equal with in a certain range
    inline bool equal(float a, float b){
        return  std::fabs(a-b) <= PRECISION;
    }

    inline bool equalPlus(float a, float b){
        return  std::fabs(a-b) <= PRECISION && a>=b;
    }

    inline bool equalMinus(float a, float b){
        return  std::fabs(a-b) <= PRECISION && b>=a;
    }

    // Convert degree into radian
    inline float toRadian(float degree){
        return degree/180*pi;
    }

    // Convert radian into degree
    inline float toDegree(float radian){
        return radian/pi*180;
    }

    // Return the absolute value
    template <class T>
        inline T abs(T a){
            return ((a<0)?-a:a);
        }

    // Return the maximum of two values
    template <class T>
        inline T max(T a, T b){
            return (a>b)?a:b;
        }

    // Return the minimum of two values
    template <class T>
        inline T min(T a, T b){
            return (a<b)?a:b;
        }
};

// Swap values to two similar types
template <class T>
inline void swap(T& a,T& b) {
    T t = a;
    a = b;
    b = t;
}

// trim from end
inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// split strings
inline std::vector<std::string>& splitr(const std::string &s,
        char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss,item,delim)) {
        elems.push_back(item);
    }
    return elems;
}

inline std::vector<std::string> split(const std::string &s,char delim) {
    std::vector<std::string> elems;
    splitr(s,delim,elems);
    return elems;
}

#endif
