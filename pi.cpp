#include "constants.h"

#include <iostream>
#include <iomanip>
#include <fstream>

int main() {
    std::cout << std::fixed << std::setprecision(12);

    int inside_circle = 0;
    int inside_circle_stratified = 0;
    int sqrt_N = 1000;
    std::ofstream csv_file("pi_estimates.csv");

    if (csv_file.is_open()) {
        csv_file << "regular,stratified\n";
    }

    for (int i = 0; i < sqrt_N; i++) {
        for (int j = 0; j < sqrt_N; j++){
            auto x = random_double(-1,1);
            auto y = random_double(-1,1);
            if (x*x + y*y < 1)
                inside_circle++;

            x = 2 * ((i + random_double()) / sqrt_N) - 1;
            y = 2 * ((j + random_double()) / sqrt_N) - 1;
            if (x*x + y*y < 1)
                inside_circle_stratified++;

            if (j % 100 == 0) {
                int total_points = (i * sqrt_N) + j + 1; // Total points sampled so far
                csv_file << (4.0 * inside_circle) / total_points << ","
                         << (4.0 * inside_circle_stratified) / total_points << "\n";
            }
        }
    }

    csv_file.close();

    std::cout << "Regular estimate of pi: " << (4.0 * inside_circle) / (sqrt_N*sqrt_N) << std::endl;
    std::cout << "Stratified estimate of pi: " << (4.0 * inside_circle_stratified) / (sqrt_N*sqrt_N) << std::endl;
}
