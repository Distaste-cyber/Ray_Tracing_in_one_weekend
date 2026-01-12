#include <iostream>
#include <string>

struct Vector3 {
    float x, y, z;
};

int main() {

  int value = 5;
  int array[5];
  Vector3 vector;

  int *harray = new int[5];
  Vector3* hvector = new Vector3();

  int *hvalue = new int;
  *hvalue = 5;
  return 0;
}