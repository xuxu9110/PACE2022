#include "treap.h"

void Node::modifySize() {
    size = 1 + (left ? left->size : 0) + (right ? right->size : 0);
}

void Node::clear() {
    if (left) {
        left->clear();
        delete left;
        left = nullptr;
    }
    if (right) {
        right->clear();
        delete right;
        right = nullptr;
    }
}

Treap::Treap() {
    root = nullptr;
    rng = mt19937(time(nullptr));
}

int Treap::size() {
    return root->size;
}

void Treap::leftRotate(Node* &p) {
    Node *k = p->right;
    p->right = k->left;
    k->left = p;
    p->modifySize();
    k->modifySize();
    p = k;
}

void Treap::rightRotate(Node* &p) {
    Node *k = p->left;
    p->left = k->right;
    k->right = p;
    p->modifySize();
    k->modifySize();
    p = k;
}

void Treap::insert(int value) {
    insert(root, value);
}

void Treap::insert(Node* &p, int value) {
    if (p == nullptr) {
        p = new Node(value, rng());
    } else {
        if (value == p->data) {
            return;
        } else if (value < p->data) {
            insert(p->left, value);
        } else {
            insert(p->right, value);
        }

        if(p->left && p->left->priority > p->priority) {
            rightRotate(p);
        } else if(p->right && p->right->priority < p->priority) {
            leftRotate(p);
        }
    }
    p->modifySize();
}

void Treap::erase(int value) {
    erase(root, value);
}

void Treap::erase(Node* &p, int value) {
    if (p == nullptr) {
        return;
    }
    if (p->data == value) {
        if (!p->left) {
            Node* temp = p;
            p = p->right;
            delete temp;
        } else if (!p->right) {
            Node* temp = p;
            p = p->left;
            delete temp;
        } else {
            if (p->left->priority > p->right->priority) {
                rightRotate(p);
                erase(p->right, value);
            } else {
                leftRotate(p);
                erase(p->left, value);
            }
        }
    } else {
        if (value < p->data) {
            erase(p->left, value);
        } else {
            erase(p->right, value);
        }
    }
    if (p != nullptr) {
        p->modifySize();
    }
}

Node* Treap::find(int value) {
    Node *p = root;
    while (p) {
        if (p->data == value) {
            return p;
        } else {
            p = p->data < value ? p->right : p->left;
        }
    }
    return nullptr;
}

int Treap::kth(int k) {
    return kth(root, k);
}

int Treap::kth(Node* &p, int k) {
    int order = p->left ? p->left->size + 1 : 1;
    if (order == k) {
        return p->data;
    } else if (order > k) {
        return p->left ? kth(p->left, k) : -1;
    } else {
        return p->right ? kth(p->right, k - order) : -1;
    }
}

void Treap::clear() {
    if (root) {
        root->clear();
        delete root;
    }
    root = nullptr;
}