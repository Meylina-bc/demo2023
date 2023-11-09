#include <iostream>

int main() {
    int n;
    std::cin >> n;
    double sum = 0;
    int count = 1;
    for (int i = 1; i <= n; ++i) {
      sum += 1.0/count;
      count *= 2;
    }
    std::cout << sum << '\n';
}
