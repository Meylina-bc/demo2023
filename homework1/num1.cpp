#include <iostream>

int main() {
  double a;
  double b;
  double c;
  double d;
  double x;
  std::cout << "A[a] = ";
  std::cin >> a;
  std::cout << "A[b] = ";  
  std::cin >> b;
  std::cout << "B[c] = ";  
  std::cin >> c;
  std::cout << "B[d] = ";  
  std::cin >> d;
  std::cout << "x = ";  
  std::cin >> x;
  double answer = c + (d-c)/(b-a)*(x-a);
  std::cout << "y = " << answer << '\n';
}
