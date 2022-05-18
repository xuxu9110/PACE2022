#define _XOPEN_SOURCE

#include <vector>
#include <unordered_set>
#include <set>
#include <list>
#include <stack>
#include <random>
#include <chrono>
#include <csignal>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <ctime>

using namespace std;
using namespace chrono;

typedef unordered_set<int> intSet;

volatile sig_atomic_t tle = 0;

void term(int signum)
{
    // cout << "catch SIGTERM!" << endl;
    tle = 1;
}


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

class Graph {
public:
    int n = 0;
    vector<vector<int>> startFrom;
    vector<vector<int>> endTo;

    // 通过预处理得知必然在反馈点集里的点
    intSet excludeVertex;
    // 通过预处理得知必然在拓扑排序里的点
    intSet includeVertex;
    // 预处理后图中还有的其他点
    intSet vertex;

    void getGraph();
    void getGraph(istream& is);
    void deleteVertex(int v);
    // 预处理
    void preprocessing();
    
    vector<vector<int>> scc;
    int sccNum;
    stack<int> s, t;
    vector<bool> isInStack;
    vector<int> timestamp;
    vector<int> sccIndex;
    int index;
    void getScc();
    void gabow(int i);
    void splitByScc();
    void splitByScc(Graph& graph1);

    void PIE();
    void CORE();
    void DOME();
};

class Topo {
public:
    Graph graph;
    // 拓扑排序
    list<int> order;
    typedef list<int>::iterator Iter;
    vector<optional<Iter>> pos;

    typedef int Sc;
    vector<Sc> score;
    const Sc INVALID = numeric_limits<Sc>::min();
    const Sc scoreRange[2] = {numeric_limits<Sc>::min(), numeric_limits<Sc>::max()};

    vector<bool> isOutdated;
    enum State {IGNORED, IN, OUT};
    vector<State> isInOrder;
    Treap vertexNotInOrder;
    enum Direction {LEFT, RIGHT};
    // vLeft[i]表示以点i为终点的边起点中拓扑排序最后的点编号
    vector<int> vLeft;
    // vRight[i]表示以点i为起点的边终点中拓扑排序最前的点编号
    vector<int> vRight;
    // deltaLeft[i]表示将点i接在vLeft[i]后面时反馈集大小的变化
    vector<int> deltaLeft;
    // deltaRight[i]表示将点i接在vRight[i]前面时反馈集大小的变化
    vector<int> deltaRight;

    // [0]表示循环次数，[1]表示移动步数，[2]表示最佳反馈集大小
    vector<int> statistic;
    // 优化随机算法用
    int k;
    
    // 生成随机数用
    default_random_engine engine;
    uniform_real_distribution<> distr;

    void preprocessing();
    void init();
    void clear();
    // 重新调整score以保证相邻点的分数差距
    void modifyScore();
    // 计算插入点的分数并赋予score[v]
    void insertScore(int v);
    // 根据newOrder调整参数
    void setByOrder(list<int> newOrder);
    // 从拓扑排序中移除点v
    void removeFromOrder(int v);
    // 插入点v以生成新的拓扑排序，direc表示点i是点v的vLeft/vRight，若i为-1则为插入开头或结尾
    void insertOrder(int v, int i, Direction direc);
    // 随机选择一个不在order里的点、插入位置与其对应L/R
    void chooseRandomMove(int& v, int& i, Direction& direc);
    // 更新点v的vL/R和deltaL/R
    void updateVertex(int v);
    // 用退火算法寻找最长拓扑排序
    void cooling(double initTemper, double temperScale, int maxMove, int time, volatile sig_atomic_t &tle);
    // 生成初始解
    void generateInitialOrder();
};

void Graph::getGraph() {
    getGraph(cin);
}

void Graph::getGraph(istream& is) {
    string s;
    bool isFirstLine = true;
    int cnt = 1;
    while (getline(is, s)) {
        if (!s.empty() && s[0] == '%') {
            continue;
        }
        stringstream sin(s);
        if (isFirstLine) {
            sin >> n;
            startFrom = vector<vector<int>>(n + 1);
            endTo = vector<vector<int>>(n + 1);
            isFirstLine = false;
        } else {
            int val;
            while(sin >> val) {
                startFrom[cnt].push_back(val);
                endTo[val].push_back(cnt);
            }
            if (cnt >= n) {
                break;
            }
            cnt++;
        }
    }
    vertex.clear();
    for (int i = 1; i <= n; ++i) {
        vertex.insert(i);
    }
}

void Graph::deleteVertex(int v) {
    for (int j : startFrom[v]) {
        auto it = find(endTo[j].begin(), endTo[j].end(), v);
        if (it != endTo[j].end()) {
            endTo[j].erase(it);
        }
    }
    for (int j : endTo[v]) {
        auto it = find(startFrom[j].begin(), startFrom[j].end(), v);
        if (it != startFrom[j].end()) {
            startFrom[j].erase(it);
        }
    }
    startFrom[v].clear();
    endTo[v].clear();
    vertex.erase(v);
}

void Graph::preprocessing() {
    int size = -1;
    while (vertex.size() != size) {
        size = vertex.size();
        for (auto it = vertex.begin(); it != vertex.end(); ) {
            int i = *it;
            it++;
            if (startFrom[i].empty() || endTo[i].empty()) {
                // 若有点v入度或出度为0，则点v必然在拓扑序列中，可删去
                includeVertex.insert(i);
                deleteVertex(i);
            } else if (find(startFrom[i].begin(), startFrom[i].end(), i) != startFrom[i].end()) {
                // 若有点出现自环，则该点必然在反馈集中，可删去
                excludeVertex.insert(i);
                deleteVertex(i);
            } else if (startFrom[i].size() == 1) {
                // 若有点v出度均为1且无自环，假设该边为(v,b)，则该点v必然在拓扑序列中，可删去，b继承v所有的入点
                int b = startFrom[i].at(0);
                includeVertex.insert(i);
                auto ins = endTo[i];
                deleteVertex(i);
                for (int j : ins) {
                    auto it1 = find(endTo[b].begin(), endTo[b].end(), j);
                    if (it1 == endTo[b].end()) {
                        endTo[b].push_back(j);
                        startFrom[j].push_back(b);
                    }
                }
            } else if (endTo[i].size() == 1) {
                // 若有点v入度均为1且无自环，假设该边为(a,v)，则该点v必然在拓扑序列中，可删去，a继承v所有的出点
                int a = endTo[i].at(0);
                includeVertex.insert(i);
                auto outs = startFrom[i];
                deleteVertex(i);
                for (int j : outs) {
                    auto it1 = find(startFrom[a].begin(), startFrom[a].end(), j);
                    if (it1 == startFrom[a].end()) {
                        startFrom[a].push_back(j);
                        endTo[j].push_back(a);
                    }
                }
            }
        }
    }
}

void Graph::getScc() {
    sccNum = 0;
    index = 0;
    scc = vector<vector<int>>(n + 1);
    isInStack = vector<bool>(n + 1, false);
    timestamp = vector<int>(n + 1, 0);
    sccIndex = vector<int>(n + 1, -1);
    for (int i : vertex) {
        if (!timestamp[i]) {
            gabow(i);
        }
    }
}

void Graph::gabow(int i) {
    typedef pair<int, bool> P;
    stack<P> stk;
    stk.push(make_pair(i, false));
    while (!stk.empty()) {
        P now = stk.top();
        int j = now.first;
        if (now.second) {
            while (timestamp[t.top()] > timestamp[j]) {
                t.pop();
            }
            stk.pop();
        }
        else if (!timestamp[j]) {
            s.push(j);
            t.push(j);
            isInStack[j] = true;
            timestamp[j] = ++index;
            for (auto it = startFrom[j].begin(); it != startFrom[j].end(); it++) {
                if (!timestamp[*it]) {
                    stk.push(make_pair(*it, false));
                } else if (isInStack[*it]) {
                    stk.push(make_pair(*it, true));
                }
            }
        } else {
            if (j == t.top()) {
                t.pop();
                while (s.top() != j) {
                    scc[sccNum].push_back(s.top());
                    sccIndex[s.top()] = sccNum;
                    isInStack[s.top()] = false;
                    s.pop();
                }
                scc[sccNum].push_back(j);
                sccIndex[j] = sccNum;
                isInStack[j] = false;
                s.pop();
                sccNum++;
            }
            stk.pop();
        }
    }
}

void Graph::splitByScc() {
    vector<int>::iterator it;
    for (int i : vertex) {
        for (auto it = startFrom[i].begin(); it != startFrom[i].end();) {
            int end = *it;
            if (sccIndex[i] != sccIndex[end]) {
                startFrom[i].erase(it);
                auto it1 = find(endTo[end].begin(), endTo[end].end(), i);
                if (it1 != endTo[end].end()) {
                    endTo[end].erase(it1);
                }
            } else {
                it++;
            }
        }
    }
}

void Graph::splitByScc(Graph& graph1) {
    vector<int>::iterator it;
    for (int i : graph1.vertex) {
        for (int j : graph1.startFrom[i]) {
            if (graph1.sccIndex[i] != graph1.sccIndex[j]) {
                auto it = find(startFrom[i].begin(), startFrom[i].end(), j);
                if (it != startFrom[i].end()) {
                    startFrom[i].erase(it);
                }
                it = find(endTo[j].begin(), endTo[j].end(), i);
                if (it != endTo[j].end()) {
                    endTo[j].erase(it);
                }
            }
        }
    }
}

void Graph::PIE() {
    Graph graph1;
    graph1.n = n;
    graph1.startFrom = vector<vector<int>>(n + 1);
    graph1.endTo = vector<vector<int>>(n + 1);
    graph1.vertex = vertex;
    for (int i : vertex) {
        for (int j : startFrom[i]) {
            if (find(startFrom[j].begin(), startFrom[j].end(), i) == startFrom[j].end()) {
                graph1.startFrom[i].push_back(j);
                graph1.endTo[j].push_back(i);
            }
        }
    }
    graph1.getScc();
    splitByScc(graph1);
}

void Graph::CORE() {
    vector<bool> valid = vector<bool>(n + 1, true);
    vector<int> PIV;
    for (int i : vertex) {
        bool isIIvertex = true;
        if (startFrom[i].empty() || (startFrom[i].size() != endTo[i].size())) {
            isIIvertex = false;
        } else {
            for (int j : startFrom[i]) {
                if (find(startFrom[j].begin(), startFrom[j].end(), i) == startFrom[j].end()) {
                    isIIvertex = false;
                    break;
                }
            }
        }
        if (isIIvertex) {
            PIV.push_back(i);
        }
    }
    auto comp = [this](int x, int y){
        int sx = startFrom[x].size();
        int sy = startFrom[y].size();
        return (sx != sy) ? (sx < sy) : (x < y);
    };
    sort(PIV.begin(), PIV.end(), comp);
    for (int i : PIV) {
        if (valid[i]) {
            auto neighbour = startFrom[i];
            int start = 0, end = 0;
            bool isClique = true;
            if (neighbour.size() > 1) {
                for (int m = 0; m < neighbour.size() - 1; m++) {
                    start = neighbour[m];
                    for (int n = m + 1; n < neighbour.size(); n++) {
                        end = neighbour[n];
                        if ((find(startFrom[start].begin(), startFrom[start].end(), end) == startFrom[start].end()) || 
                        (find(startFrom[end].begin(), startFrom[end].end(), start) == startFrom[end].end())) {
                            isClique = false;
                            break;
                        }
                    }
                }
            }
            if (isClique) {
                includeVertex.insert(i);
                excludeVertex.insert(neighbour.begin(), neighbour.end());
                deleteVertex(i);
                for (int j : neighbour) {
                    deleteVertex(j);
                }
            }
            valid[i] = false;
            for (int j : neighbour) {
                valid[j] = false;
            }
        }
    }
}

void Graph::DOME() {
    for (int i : vertex) {
        for (auto it = startFrom[i].begin(); it != startFrom[i].end();) {
            int j = *it;
            bool isDominated = true;
            if (find(startFrom[j].begin(), startFrom[j].end(), i) != startFrom[j].end()) {
                isDominated = false;
            } else {
                bool tmp1 = true;
                for (int pre : endTo[i]) {
                    if (find(startFrom[i].begin(), startFrom[i].end(), pre) == startFrom[i].end()) {
                        if (find(endTo[j].begin(), endTo[j].end(), pre) == endTo[j].end()) {
                            tmp1 = false;
                            break;
                        }
                    }
                }
                if (!tmp1) {
                    bool tmp2 = true;
                    for (int suc : startFrom[j]) {
                        if (find(endTo[j].begin(), endTo[j].end(), suc) == endTo[j].end()) {
                            if (find(startFrom[i].begin(), startFrom[i].end(), suc) == startFrom[i].end()) {
                                tmp2 = false;
                                break;
                            }
                        }
                    }
                    if (!tmp2) {
                        isDominated = false;
                    }
                }
            }
            if (isDominated) {
                startFrom[i].erase(it);
                auto it1 = find(endTo[j].begin(), endTo[j].end(), i);
                if (it1 != endTo[j].end()) {
                    endTo[j].erase(it1);
                }
            } else {
                it++;
            }
        }
    }
}

void Topo::clear() {
    order.clear();
    pos = vector<optional<Iter>>(graph.n + 1, nullopt);
    score = vector<Sc>(graph.n + 1, INVALID);
    isOutdated = vector<bool>(graph.n + 1, false);
    isInOrder = vector<State>(graph.n + 1, IGNORED);
    vertexNotInOrder.clear();
    for (int i : graph.vertex) {
        vertexNotInOrder.insert(i);
        isInOrder[i] = OUT;
    }
    vLeft = vector<int>(graph.n + 1, -1);
    vRight = vector<int>(graph.n + 1, -1);
    deltaLeft = vector<int>(graph.n + 1, -1);
    deltaRight = vector<int>(graph.n + 1, -1);
    k = graph.n;
    for (int i = 0; i < 3; ++i) {
        k = graph.n / log2(k);
    }
}

void Topo::preprocessing() {
    int size = -1;
    while (size != graph.vertex.size()) {
        size = graph.vertex.size();
        graph.PIE();
        graph.CORE();
        graph.DOME();
        graph.preprocessing();
    }
}

void Topo::init() {
    graph.getGraph();
    preprocessing();
    clear();
    engine = default_random_engine(time(nullptr));
    distr = uniform_real_distribution<>(0.0, 1.0);
    statistic = vector<int>(3, 0);
}

void Topo::modifyScore() {
    score = vector<Sc>(graph.n + 1, INVALID);
    int size = order.size() + 1;
    int cnt = 1;
    for (auto iter = order.begin(); iter != order.end(); ++iter) {
        double scale = 1.0 * cnt / size;
        score[*iter] = (Sc) ((1 - scale) * scoreRange[0] + scale * scoreRange[1]);
        cnt++;
    }
}

void Topo::setByOrder(list<int> newOrder) {
    clear();
    order = newOrder;
    int size = order.size() + 1;
    int cnt = 1;
    for (auto iter = order.begin(); iter != order.end(); ++iter) {
        pos[*iter] = iter;
        double scale = 1.0 * cnt / size;
        score[*iter] = (Sc) ((1 - scale) * scoreRange[0] + scale * scoreRange[1]);
        cnt++;
        vertexNotInOrder.erase(*iter);
        isInOrder[*iter] = IN;
    }
    for (int i : graph.vertex) {
        isOutdated[i] = true;
    }
}

void Topo::removeFromOrder(int v) {
    Iter iter = pos[v].value();
    order.erase(iter);
    pos[v] = nullopt;
    score[v] = INVALID;
    vertexNotInOrder.insert(v);
    isInOrder[v] = OUT;
}

void Topo::insertOrder(int v, int i, Direction direc) {
    // 插入点v
    if (i == -1) {
        if (direc == LEFT) {
            order.push_front(v);
            pos[v] = order.begin();
        } else {
            order.push_back(v);
            pos[v] = --order.end();
        }
    } else {
        Iter iter = pos[i].value();
        if (direc == LEFT) {
            iter++;
        }
        pos[v] = order.insert(iter, v);
    }
    insertScore(v);
    vertexNotInOrder.erase(v);
    isInOrder[v] = IN;
    // 移除与点v方向相反的点
    intSet rmVertex;
    for (int vertex : graph.endTo[v]) {
        if (score[vertex] != INVALID && score[vertex] > score[v]) {
            rmVertex.insert(vertex);
        }
    }
    for (int vertex : graph.startFrom[v]) {
        if (score[vertex] != INVALID && score[vertex] < score[v]) {
            rmVertex.insert(vertex);
        }
    }
    for (int vertex : rmVertex) {
        removeFromOrder(vertex);
    }
    // S={点v、移除点}，将S与S的相邻点update
    rmVertex.insert(v);
    for (int setv : rmVertex) {
        isOutdated[setv] = true;
        for (int j : graph.startFrom[setv]) {
            isOutdated[j] = true;
        }
        for (int j : graph.endTo[setv]) {
            isOutdated[j] = true;
        }
    }
}

void Topo::chooseRandomMove(int& v, int& i, Topo::Direction &direc) {
    if (vertexNotInOrder.size() <= k) {
        int k = (int) (distr(engine) * vertexNotInOrder.size()) + 1;
        v = vertexNotInOrder.kth(k);
        // sample(vertexNotInOrder.begin(), vertexNotInOrder.end(), &v, 1, engine);
        // 抛弃数组使用平衡树维护
    } else {
        while (true) {
            v = (int)(distr(engine) * graph.n + 1);
            if (isInOrder[v] == OUT) {
                break;
            }
            // 可改进为维护一个bool的数组以代替这个find的过程
        }
    }
    if (isOutdated[v]) {
        updateVertex(v);
        isOutdated[v] = false;
    }
    direc = (distr(engine) < 0.5) ? LEFT : RIGHT;
    i = (direc == LEFT) ? vLeft[v] : vRight[v];
    // cout << "v: " << v << " i: " << i << " d: " << direc << endl;
}

void Topo::insertScore(int v) {
    Iter iter = pos[v].value();
    Sc l = scoreRange[0], r = scoreRange[1];
    if (iter != order.begin()) {
        l = score[*(--iter)];
        ++iter;
    }
    if (iter != --order.end()) {
        r = score[*(++iter)];
        --iter;
    }
    Sc res = (l >> 1) + (r >> 1) + (l & r & 1);
    if ((l >= res) || (res >= r)) {
        modifyScore();
    } else {
        score[v] = res;
    }
}

void Topo::updateVertex(int v) {
    Sc maxScore = scoreRange[0];
    Sc minScore = scoreRange[1];
    vLeft[v] = -1;
    vRight[v] = -1;
    deltaLeft[v] = -1;
    deltaRight[v] = -1;
    for (int startV : graph.endTo[v]) {
        if (score[startV] == INVALID) {
            continue;
        }
        if (score[startV] > maxScore) {
            vLeft[v] = startV;
            maxScore = score[startV];
        }
    };
    for (int endV : graph.startFrom[v]) {
        if (score[endV] == INVALID) {
            continue;
        }
        if (score[endV] < minScore) {
            vRight[v] = endV;
            minScore = score[endV];
        }
        if (score[endV] <= maxScore) {
            deltaLeft[v]++;
        }
    };
    for (int startV : graph.endTo[v]) {
        if (score[startV] == INVALID) {
            continue;
        }
        if (score[startV] >= minScore) {
            deltaRight[v]++;
        }
    };
}

void Topo::cooling(double initTemper, double temperScale, int maxMove, int time, volatile sig_atomic_t &tle) {
    auto start = system_clock::now();
    // time_t tt = system_clock::to_time_t(start);
    // cout << "now is " << ctime(&tt);
    double temper = initTemper;
    list<int> bestOrder = order;
    int nbLoop = 0, nbJump = 0;
    while (true) {
        int nbMove = 0;
        while (nbMove < maxMove) {
            if (tle || vertexNotInOrder.size() == 0) {
                statistic[0] = nbLoop;
                statistic[1] = nbJump;
                statistic[2] = graph.vertex.size() - bestOrder.size() + graph.excludeVertex.size();
                setByOrder(bestOrder);
                return;
            }
            nbLoop++;
            int v = 0, i = 0;
            Direction d = LEFT;
            chooseRandomMove(v, i, d);
            int delta = (d == LEFT) ? deltaLeft[v] : deltaRight[v];
            if ((delta <= 0) || (exp(-delta * 1.0 / temper) >= distr(engine))) {
                if (delta > 0) {
                    nbJump++;
                }
                insertOrder(v, i, d);
                nbMove++;
                if (order.size() > bestOrder.size()) {
                    bestOrder = order;
                }
            }
            if (duration_cast<seconds>(system_clock::now() - start).count() >= time) {
                raise(SIGTERM);
            }
        }
        temper *= temperScale;
    }
}

void Topo::generateInitialOrder() {
    list<int> newOrder;
    Graph newGraph = graph;
    auto comp = [&newGraph](int x, int y){
        int sx = newGraph.endTo[x].size();
        int sy = newGraph.endTo[y].size();
        if (sx != sy) {
            return (sx < sy);
        } else {
            int sumx = newGraph.startFrom[x].size();
            int sumy = newGraph.startFrom[y].size();
            for (int i : newGraph.endTo[x]) {
                sumx += newGraph.startFrom[i].size();
            }
            for (int i : newGraph.endTo[y]) {
                sumy += newGraph.startFrom[i].size();
            }
            return (sumx != sumy) ? (sumx > sumy) : (x < y);
        }
    };
    auto orderSet = set<int, decltype(comp)>(comp);
    for (int i : newGraph.vertex) {
        orderSet.insert(i);
    }
    while (!orderSet.empty()) {
        auto it = orderSet.begin();
        int v = *it;
        newOrder.push_back(v);
        auto deleteSet = newGraph.endTo[v];
        deleteSet.push_back(v);
        intSet involvedSet;
        for (int i : deleteSet) {
            involvedSet.insert(newGraph.startFrom[i].begin(), newGraph.startFrom[i].end());
            involvedSet.insert(newGraph.endTo[i].begin(), newGraph.endTo[i].end());
            for (int j : newGraph.endTo[i]) {
                involvedSet.insert(newGraph.startFrom[j].begin(), newGraph.startFrom[j].end());
            }
        }
        for (int i : deleteSet) {
            orderSet.erase(i);
            involvedSet.erase(i);
        }
        for (int i : involvedSet) {
            orderSet.erase(i);
        }
        for (int i : deleteSet) {
            newGraph.deleteVertex(i);
        }
        for (int i : involvedSet) {
            orderSet.insert(i);
        }
    }
    setByOrder(newOrder);
}


int main() {
    signal(SIGTERM, term);

    Topo topo;
    topo.init();
    topo.generateInitialOrder();
    topo.cooling(0.6, 0.99, 5 * topo.graph.n, 600, tle);
    vector<int> res(topo.graph.excludeVertex.begin(), topo.graph.excludeVertex.end());
    for (int i : topo.graph.vertex) {
        if (!topo.pos[i].has_value()) {
            res.push_back(i);
        }
    }
    sort(res.begin(), res.end());
    for (int val : res) {
        cout << val << endl;
    }
    return 0;
}