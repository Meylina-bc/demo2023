#include <iostream>
#include <cmath>

int main() {
  double a;
  double b;
  double c;
  std::cout << "a = ";
  std::cin >> a;
  std::cout << "b = ";
  std::cin >> b;
  std::cout << "c = ";
  std::cin >> c;
  double D = b * b - 4 * a * c;
  if (D < 0) {
    std::cout << "no roots" << '\n';
  }
  else if (D == 0) {
    std::cout << "x = " << (-b)/(2 * a) << '\n';
  }
  else {
    std::cout << "x1 = " << (-b + sqrt(D)) / (2 * a) << '\n' << "x2 = " << (-b - sqrt(D)) / (2 * a) << '\n';
  }
}