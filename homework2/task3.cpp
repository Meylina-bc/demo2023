#include <iostream>
#include <cmath>

#define PI 3.14159265

int main() {
    int n;
    double x;
    std::cout << "n = ";
    std::cin >> n;
    std::cout << "x = ";
    std::cin >> x;
    double sum = 0;
    for (int i = 1; i <= n; ++i) {
      x = sin(x);
      sum += x;
    }
    std::cout << sum << '\n';
}
