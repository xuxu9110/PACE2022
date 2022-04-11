#ifndef TREAP_H
#define TREAP_H

#include <random>
#include <ctime>
using namespace std;

struct Node {
    int data;
    Node *left;
    Node *right;
    int priority;
    int size; // 当前子树的节点个数
    Node(int value, int level) : data(value), left(nullptr), right(nullptr), priority(level), size(1) {}
    void modifySize();
    void clear();
};

class Treap {
private:
    Node *root;
    mt19937 rng;

    void leftRotate(Node* &p);
    void rightRotate(Node* &p);
    void insert(Node* &p, int value);
    void erase(Node* &p, int value);
    int kth(Node* &p, int k);
public:
    Treap();
    int size();
    void insert(int value);
    void erase(int x);
    Node* find(int x);
    int kth(int k);
    void clear();
};

#endif