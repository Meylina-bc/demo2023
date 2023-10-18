#include <iostream>
#include <cmath>

int main() {
  double R;
  double r;
  double x;
  double y;
  std::cout << "R = ";
  std::cin >> R;
  std::cout << "r = ";
  std::cin >> r;
  std::cout << "x = ";
  std::cin >> x;
  std::cout << "y = ";
  std::cin >> y;
  double koor = x * x + y * y;
  if (koor <= R * R && koor >= r * r) {
    std::cout << "YES" << '\n';
  }
  else {
    std::cout << "NO" << '\n';
  }
}