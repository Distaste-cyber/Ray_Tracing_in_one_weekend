#include <iostream>

void insertion_sort(int arr[], int n);
void selection_sort(int arr[], int n);
void bubble_sort(int arr[], int n);
void printArray(int arr[], int n);
void merge(int arr[], int l, int m, int r);
void merge_sort(int arr[], int l, int r);

int main() {

  int n;
  int arr[30];

  std::cout << "How many elements do you want in the array \n";
  std::cin >> n;

  std::cout << "Enter the elements of the array \n";

  for (int i = 0; i < n; i++) {
    std::cin >> arr[i];
  }

  merge_sort(arr, 0, n - 1);
  printArray(arr, n);

  return 0;
}

void insertion_sort(int arr[], int n) {

  for (int i = 1; i < n; i++) {
    int temp = arr[i];
    int j = i - 1;

    while ((j >= 0) && (arr[j] > temp)) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = temp;
  }
}

void printArray(int arr[], int n) {
  for (int i = 0; i < n; i++) {
    std::cout << arr[i] << " ";
  }
  std::cout << std::endl;
}

void selection_sort(int arr[], int n) {
  for (int i = 0; i < n; i++) {
    int minIndex = i;

    for (int j = i + 1; j < n; j++) {
      if (arr[j] < arr[minIndex]) {
        minIndex = j;
      }
    }

    if (minIndex != i) {
      int temp = arr[i];
      arr[i] = arr[minIndex];
      arr[minIndex] = temp;
    }
  }
}

void bubble_sort(int arr[], int n) {
  bool swapped;

  for (int i = 0; i < n - 1; i++) {
    swapped = false;
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j + 1] < arr[j]) {
        int temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
        swapped = true;
      }
    }

    if (!swapped) {
      break;
    }
  }
}

void merge(int arr[], int l, int m, int r) {
  int i, j, k;

  int n1 = m - l + 1;
  int n2 = r - m;

  int *L = new int[n1];
  int *R = new int[n2];

  for (i = 0; i < n1; i++) {
    L[i] = arr[l + i];
  }

  for (j = 0; j < n2; j++) {
    R[j] = arr[m + 1 + j];
  }

  i = 0;
  j = 0;
  k = l;
  while (i < n1 && j < n2) {
    if (L[i] <= R[j]) {
      arr[k] = L[i];
      i++;
    } else {
      arr[k] = R[j];
      j++;
    }
    k++;
  }
  while (i < n1) {
    arr[k] = L[i];
    i++;
    k++;
  }
  while (j < n2) {
    arr[k] = R[j];
    j++;
    k++;
  }
}

void merge_sort(int arr[], int l, int r) {
  if (l < r) {
    int m = l + (r - l) / 2;

    merge_sort(arr, l, m);
    merge_sort(arr, m + 1, r);

    merge(arr, l, m, r);
  }
}
