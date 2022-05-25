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
    if (root != nullptr) {
        return root->size;
    } else {
        return 0;
    }
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
            if(p->left->priority > p->priority) {
                rightRotate(p);
            }
        } else {
            insert(p->right, value);
            if(p->right->priority < p->priority) {
                leftRotate(p);
            }
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

Treap Treap::copy() {
    Treap newTreap = Treap();
    newTreap.rng = rng;
    if (root == nullptr) return newTreap;
    newTreap.root = new Node(root->data, root->priority);
    copyTo(root, newTreap.root);
    return newTreap;
}

void Treap::copyTo(Node* &src, Node* &dest) {
    dest->size = src->size;
    if (src->left) {
        dest->left = new Node(src->left->data, src->left->priority);
        copyTo(src->left, dest->left);
    }
    if (src->right) {
        dest->right = new Node(src->right->data, src->right->priority);
        copyTo(src->right, dest->right);
    }
}