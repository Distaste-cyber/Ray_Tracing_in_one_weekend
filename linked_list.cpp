#include <iostream>
using namespace std;

struct ListNode {
  int data;
  ListNode *next;
  ListNode(int val) : data(val), next(nullptr) {}
};

class LinkedList {
private:
  ListNode *head;

public:
  LinkedList(ListNode *h = nullptr) { head = h; }

  void insert(int position, int data) {
    ListNode *newNode = new ListNode(data);

    if (position == 1) {
      newNode->next = head;
      head = newNode;
      return;
    }

    ListNode *curr = head;
    ListNode *temp = nullptr;
    int k = 1;

    while (k < position && curr != nullptr) {
      temp = curr;
      curr = curr->next;
      k++;
    }

    if (k != position) { // loop ended before reaching desired position
      cout << "Invalid position!" << endl;
      delete newNode;
      return;
    }

    temp->next = newNode;
    newNode->next = curr;
  }

  void deletion(int position) {
    struct ListNode *curr, *temp;
    int k = 1;
    if (head == nullptr) {
      cout << "List is Empty" << endl;
    }
    curr = head;
    if (position == 1) {
      head = head->next;
      free(curr);
      return;
    } else {
      while (k < position && curr != nullptr) {
        temp = curr;
        curr = curr->next;
        k++;
      }
      if (k != position) {
        cout << "Invalid Position" << endl;
        return;
      }
      temp->next = curr->next;
      free(curr);
    }
  }

  ListNode *getHead() { return head; }
};

void print_linked_list(ListNode *head) {
  ListNode *curr = head;
  while (curr != nullptr) {
    cout << curr->data << endl;
    curr = curr->next;
  }
}

int main() {
  ListNode *node = new ListNode(1);
  LinkedList l(node);
  l.insert(2, 3);
  l.insert(3, 0);
  l.insert(5, 99);
  l.deletion(2);
  print_linked_list(l.getHead());
  return 0;
}
