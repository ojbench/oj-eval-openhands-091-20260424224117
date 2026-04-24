#include <iostream>
#include <vector>
#include <algorithm>

template<class Key, class Compare = std::less<Key>>
class ESet {
public:
    enum Color { RED, BLACK };
    struct Node {
        Key key;
        Node *left, *right, *parent;
        Color color;
        int size;

        Node() : left(nullptr), right(nullptr), parent(nullptr), color(BLACK), size(0) {}
        Node(const Key& k, Node* nil) : key(k), left(nil), right(nil), parent(nil), color(RED), size(1) {}
    };

private:
    Node *root;
    Node *nil;
    Compare comp;
    int _size;

    void update_size(Node* x) {
        if (x != nil) {
            x->size = x->left->size + x->right->size + 1;
        }
    }

    void left_rotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != nil) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == nil) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
        update_size(x);
        update_size(y);
    }

    void right_rotate(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        if (x->right != nil) x->right->parent = y;
        x->parent = y->parent;
        if (y->parent == nil) root = x;
        else if (y == y->parent->left) y->parent->left = x;
        else y->parent->right = x;
        x->right = y;
        y->parent = x;
        update_size(y);
        update_size(x);
    }

    void insert_fixup(Node* z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        left_rotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    right_rotate(z->parent->parent);
                }
            } else {
                Node* y = z->parent->parent->left;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        right_rotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    left_rotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    void transplant(Node* u, Node* v) {
        if (u->parent == nil) root = v;
        else if (u == u->parent->left) u->parent->left = v;
        else u->parent->right = v;
        v->parent = u->parent;
    }

    void erase_fixup(Node* x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                Node* w = x->parent->right;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    left_rotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        w->left->color = BLACK;
                        w->color = RED;
                        right_rotate(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    left_rotate(x->parent);
                    x = root;
                }
            } else {
                Node* w = x->parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    right_rotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        left_rotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    right_rotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;
    }

    Node* copy(Node* x, Node* p, Node* other_nil) {
        if (x == other_nil) return nil;
        Node* y = new Node(x->key, nil);
        y->color = x->color;
        y->size = x->size;
        y->parent = p;
        y->left = copy(x->left, y, other_nil);
        y->right = copy(x->right, y, other_nil);
        return y;
    }

    void clear(Node* x) {
        if (x == nil) return;
        clear(x->left);
        clear(x->right);
        delete x;
    }

public:
    class iterator {
        friend class ESet;
        Node* node;
        const ESet* set;
    public:
        iterator(Node* n = nullptr, const ESet* s = nullptr) : node(n), set(s) {}
        const Key& operator*() const { return node->key; }
        bool operator==(const iterator& other) const { return node == other.node; }
        bool operator!=(const iterator& other) const { return node != other.node; }
        iterator& operator++() {
            if (node == set->nil) return *this;
            if (node->right != set->nil) {
                node = node->right;
                while (node->left != set->nil) node = node->left;
            } else {
                Node* y = node->parent;
                while (y != set->nil && node == y->right) {
                    node = y;
                    y = y->parent;
                }
                node = y;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        iterator& operator--() {
            if (node == set->nil) {
                node = set->root;
                if (node == set->nil) return *this;
                while (node->right != set->nil) node = node->right;
                return *this;
            }
            if (node->left != set->nil) {
                node = node->left;
                while (node->right != set->nil) node = node->right;
            } else {
                Node* y = node->parent;
                while (y != set->nil && node == y->left) {
                    node = y;
                    y = y->parent;
                }
                if (y == set->nil) return *this; // Already at begin
                node = y;
            }
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
    };

    ESet() : _size(0) {
        nil = new Node();
        nil->left = nil->right = nil->parent = nil;
        nil->size = 0;
        nil->color = BLACK;
        root = nil;
    }
    ~ESet() {
        clear(root);
        delete nil;
    }
    ESet(const ESet& other) : _size(other._size), comp(other.comp) {
        nil = new Node();
        nil->left = nil->right = nil->parent = nil;
        nil->size = 0;
        nil->color = BLACK;
        root = copy(other.root, nil, other.nil);
    }
    ESet& operator=(const ESet& other) {
        if (this != &other) {
            clear(root);
            _size = other._size;
            comp = other.comp;
            root = copy(other.root, nil, other.nil);
        }
        return *this;
    }
    ESet(ESet&& other) noexcept : root(other.root), nil(other.nil), comp(other.comp), _size(other._size) {
        other.root = other.nil = nullptr;
        other._size = 0;
    }
    ESet& operator=(ESet&& other) noexcept {
        if (this != &other) {
            clear(root);
            delete nil;
            root = other.root;
            nil = other.nil;
            comp = other.comp;
            _size = other._size;
            other.root = other.nil = nullptr;
            other._size = 0;
        }
        return *this;
    }

    std::pair<iterator, bool> emplace(const Key& key) {
        Node* y = nil;
        Node* x = root;
        while (x != nil) {
            y = x;
            if (comp(key, x->key)) x = x->left;
            else if (comp(x->key, key)) x = x->right;
            else return {iterator(x, this), false};
        }
        Node* z = new Node(key, nil);
        z->parent = y;
        if (y == nil) root = z;
        else if (comp(z->key, y->key)) y->left = z;
        else y->right = z;
        z->left = z->right = nil;
        z->color = RED;
        
        // Update sizes up to root
        Node* temp = z;
        while (temp != nil) {
            update_size(temp);
            temp = temp->parent;
        }

        insert_fixup(z);
        _size++;
        return {iterator(z, this), true};
    }

    size_t erase(const Key& key) {
        Node* z = root;
        while (z != nil) {
            if (comp(key, z->key)) z = z->left;
            else if (comp(z->key, key)) z = z->right;
            else break;
        }
        if (z == nil) return 0;

        Node* y = z;
        Node* x;
        Color y_original_color = y->color;
        if (z->left == nil) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = z->right;
            while (y->left != nil) y = y->left;
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z) x->parent = y;
            else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        // Update sizes from x's parent up to root
        Node* temp = x->parent;
        while (temp != nil) {
            update_size(temp);
            temp = temp->parent;
        }

        if (y_original_color == BLACK) erase_fixup(x);
        delete z;
        _size--;
        return 1;
    }

    iterator find(const Key& key) const {
        Node* x = root;
        while (x != nil) {
            if (comp(key, x->key)) x = x->left;
            else if (comp(x->key, key)) x = x->right;
            else return iterator(x, this);
        }
        return end();
    }

    int count_less(const Key& key, bool inclusive) const {
        int count = 0;
        Node* x = root;
        while (x != nil) {
            if (comp(x->key, key)) {
                count += x->left->size + 1;
                x = x->right;
            } else if (comp(key, x->key)) {
                x = x->left;
            } else {
                if (inclusive) count += x->left->size + 1;
                else count += x->left->size;
                break;
            }
        }
        return count;
    }

    size_t range(const Key& l, const Key& r) const {
        if (comp(r, l)) return 0;
        return count_less(r, true) - count_less(l, false);
    }

    size_t size() const noexcept { return _size; }
    iterator begin() const noexcept {
        Node* x = root;
        if (x == nil) return end();
        while (x->left != nil) x = x->left;
        return iterator(x, this);
    }
    iterator end() const noexcept { return iterator(nil, this); }
};

int main() {
    ESet<long long> s[25];
    ESet<long long>::iterator it;
    int op, lst = 0, it_a = -1, valid = 0;
    while (scanf("%d", &op) != EOF) {
        long long a, b, c;
        switch (op) {
            case 0: {
                scanf("%lld%lld", &a, &b);
                auto p = s[a].emplace(b);
                if (p.second) {
                    it_a = (int)a;
                    it = p.first;
                    valid = 1;
                }
                break;
            }
            case 1:
                scanf("%lld%lld", &a, &b);
                if (valid && it_a == a && *it == b) valid = 0;
                s[a].erase(b);
                break;
            case 2:
                scanf("%lld", &a);
                s[++lst] = s[a];
                break;
            case 3: {
                scanf("%lld%lld", &a, &b);
                auto it2 = s[a].find(b);
                if (it2 != s[a].end()) {
                    printf("true\n");
                    it_a = (int)a;
                    it = it2;
                    valid = 1;
                } else
                    printf("false\n");
                break;
            }
            case 4:
                scanf("%lld%lld%lld", &a, &b, &c);
                printf("%zu\n", s[a].range(b, c));
                break;
            case 5:
                if (valid) {
                    auto it_prev = it;
                    if (it == s[it_a].begin()) valid = 0;
                    else --it;
                }
                if (valid)
                    printf("%lld\n", *it);
                else
                    printf("-1\n");
                break;
            case 6:
                if (valid) {
                    ++it;
                    if (it == s[it_a].end()) valid = 0;
                }
                if (valid)
                    printf("%lld\n", *it);
                else
                    printf("-1\n");
                break;
        }
    }
    return 0;
}
