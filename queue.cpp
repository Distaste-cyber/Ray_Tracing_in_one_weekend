#include <iostream>
using namespace std;

class Queue {
private:
  int n = 50;
  int arr[50];
  int front;
  int rear;

public:
  Queue(int f = -1, int r = -1) {
    this->front = f;
    this->rear = r;
  }

  void qinsert(int val) {
    if (front == -1) {
      front = 0;
      rear = 0;
      arr[rear] = val;
    } else if (rear == n - 1) {
      cout << "Queue Overflow" << endl;
      return;
    } else {
      rear = rear + 1;
      arr[rear] = val;
    }
  }

  int qdelete() {
    if (front == -1) {
      cout << "Queue Underflow" << endl;
      return -1;
    } else if (front == rear) {
      int val = arr[front];
      front = -1;
      rear = -1;
      return val;
    } else {
      int val = arr[front];
      front = front + 1;
      return val;
    }
  }
};

int main() {
  Queue queue = Queue();

  queue.qinsert(4);
  queue.qinsert(7);
  queue.qinsert(8);

  cout << queue.qdelete() << endl;
  cout << queue.qdelete() << endl;

  return 0;
}