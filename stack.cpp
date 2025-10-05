#include <algorithm>
#include <iostream>
#include <string>
using namespace std;

class Stack {
private:
  static const int n = 50;
  char arr[n];
  int top;

public:
  Stack(int t = -1) { this->top = t; }

  void push(char val) {
    if (top == n - 1) {
      cout << "Stack Overflow!" << endl;
    } else {
      arr[++top] = val;
    }
  }

  char pop() {
    if (top == -1) {
      cout << "Stack Underflow!" << endl;
      return '\0';
    }
    return arr[top--];
  }

  char get_top() {
    if (top == -1)
      return '\0';
    return arr[top];
  }

  bool is_empty() { return top == -1; }
};

bool is_operator(char c) {
  return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

int precedence(char c) {
  if (c == '^')
    return 3;
  if (c == '*' || c == '/')
    return 2;
  if (c == '+' || c == '-')
    return 1;
  return 0;
}

string infix_to_postfix(string infix) {
  Stack stk;
  string postfix = "";

  for (char c : infix) {
    if (isalnum(c)) {
      // operand (A, B, C, 1, 2, etc.)
      postfix += c;
    } else if (c == '(') {
      stk.push(c);
    } else if (c == ')') {
      while (!stk.is_empty() && stk.get_top() != '(') {
        postfix += stk.pop();
      }
      stk.pop(); // remove '('
    } else if (is_operator(c)) {
      while (!stk.is_empty() && precedence(stk.get_top()) >= precedence(c)) {
        postfix += stk.pop();
      }
      stk.push(c);
    }
  }

  // pop remaining operators
  while (!stk.is_empty()) {
    postfix += stk.pop();
  }

  return postfix;
}

string prefix_converter(string infix) {
  reverse(infix.begin(), infix.end());

  for (char &c : infix) {
    if (c == '(')
      c = ')';
    else if (c == ')')
      c = '(';
  }

  string postfix = infix_to_postfix(infix);

  reverse(postfix.begin(), postfix.end());

  return postfix;
}

int main() {
  string expr;
  cout << "Enter infix expression: ";
  cin >> expr;

  cout << "Prefix: " << prefix_converter(expr) << endl;
  return 0;
}
