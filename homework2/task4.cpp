#include <iostream>
#include <cmath>

int main() {
  const double PI = 3.14159265;
    double x;
    double eps;
    std::cout << "x = ";
    std::cin >> x;
    std::cout << "eps = ";
    std::cin >> eps;
    double sum = 0.0;
    double val = 1.0;
    int counter = 1;
    int N = 0;
    while (std::abs(val)>=eps) {
      sum += val;
      val *= x/counter++;
      ++N;
    }
    std::cout << sum << '\n';
    std::cout << std::exp(x) << '\n';
    std::cout << "N = " << N << '\n';
}
