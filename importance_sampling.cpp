#include "constants.h"

#include <iostream>
#include <iomanip>

double icd(double d) {
    return 2.0 * std::pow(d, 1.0/3.0);
}

double pdf(double x) {
    return (3.0/8.0) * x*x;
}

int main() {
    int N = 10;
    auto sum = 0.0;

    for (int i = 0; i < N; i++) {
        auto z = random_double();
        if (z == 0)
            continue;
        
        auto x = icd(z);
        auto x_sq = x*x / pdf(x);
        std::cout << "x: " << x << '\n';
        std::cout << "x2: " << x_sq << '\n';
        sum += x_sq;
    }
    std::cout << std::fixed << std::setprecision(12);
    std::cout << "I = " << (sum / N) << '\n';
}
