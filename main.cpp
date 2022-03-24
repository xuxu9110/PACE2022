#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <list>
#include <random>
#include <tuple>
#include <cmath>
#include <ctime>
#include <assert.h>
#include <limits>

using namespace std;

class Graph {
public:
    int n = 0;
    vector<vector<int>> startFrom;
    vector<vector<int>> endTo;

    void getGraph();
    void showGraph();
};

void Graph::getGraph() {
    // TODO
}

void Graph::showGraph() {
    // TODO
}

class Topo {
public:
    Graph graph;
    // 拓扑排序
    list<int> order;
    typedef list<int>::iterator Iter;
    vector<optional<Iter>> pos;
    typedef int Sc;
    vector<Sc> score;
    const Sc INVALID = -1;

    set<int> outdatedVertex;
    enum Direction {LEFT, RIGHT};
    // vLeft[i]表示以点i为终点的边起点中拓扑排序最后的点编号
    vector<int> vLeft;
    // vRight[i]表示以点i为起点的边终点中拓扑排序最前的点编号
    vector<int> vRight;
    // deltaLeft[i]表示将点i接在vLeft[i]后面时反馈集大小的变化
    vector<int> deltaLeft;
    // deltaRight[i]表示将点i接在vRight[i]前面时反馈集大小的变化
    vector<int> deltaRight;

    void init();
    void clear();
    // 重新调整score以保证相邻点的分数差距
    void modifyScore();
    // 计算插入点的分数
    Sc insertScore(int v, bool isRetry = false);
    // 根据newOrder调整参数
    void setByOrder(list<int> newOrder);
    // 随机选择一个不在order里的点、插入位置与其对应L/R
    tuple<int, int, Direction> chooseRandomMove();
    // 更新点v的vL/R和deltaL/R
    void updateVertex(int v);
    // 插入点v以生成新的拓扑排序，direc表示点i是点v的vLeft/vRight，若i为-1则为插入开头或结尾
    void insertOrder(int v, int i, Direction direc);
    // 用退火算法寻找最长拓扑排序
    void cooling(double initTemper, double temperScale, int maxMove, int maxFail);
};

void Topo::clear() {
    order.clear();
    pos = vector<optional<Iter>>(graph.n + 1, nullopt);
    score = vector<Sc>(graph.n + 1, INVALID);
    outdatedVertex.clear();
    vLeft = vector<int>(graph.n + 1, -1);
    vRight = vector<int>(graph.n + 1, -1);
    deltaLeft = vector<int>(graph.n + 1, -1);
    deltaRight = vector<int>(graph.n + 1, -1);
}

void Topo::init() {
    graph.getGraph();
    clear();
}

void Topo::modifyScore() {
    // TODO
}

void Topo::setByOrder(list<int> newOrder) {
    // TODO
}

tuple<int, int, Topo::Direction> Topo::chooseRandomMove() {
    // TODO
}

Topo::Sc Topo::insertScore(int v, bool isRetry) {
    Iter iter = pos[v].value();
    int l = numeric_limits<Sc>::min(), r = numeric_limits<Sc>::max();
    if (iter != order.begin()) {
        l = score[*(--iter)];
        ++iter;
    }
    if (iter != --order.end()) {
        r = score[*(++iter)];
        --iter;
    }
    Sc res = (l + r) / 2;
    if ((l == res) || (res == r)) {
        assert(!isRetry);
        modifyScore();
        return insertScore(v, true);
    }
    return res;
}

void Topo::updateVertex(int v) {
    // TODO
}

void Topo::insertOrder(int v, int i, Direction direc) {
    if (i == -1) {
        if (direc == LEFT) {
            order.push_front(v);
            pos[v] = order.begin();
        } else {
            order.push_back(v);
            pos[v] = --order.end();
        }
    } else {
        assert(pos[i].has_value());
        Iter iter = pos[i].value();
        if (direc == LEFT) {
            iter++;
            pos[v] = order.insert(iter, v);
            
        } else {
            pos[v] = order.insert(iter, v);
        }
    }
    score[v] = insertScore(v);
    // TODO: 移除与点v方向相反的点
    // TODO: S={点v、移除点}，将S与S的相邻点输入outdatedVertex
    // TODO: 将outdatedVertex里的点输入updateVertex()
}

void Topo::cooling(double initTemper, double temperScale, int maxMove, int maxFail) {
    clear();
    double temper = initTemper;
    int nbFail = 0;
    list<int> bestOrder;
    default_random_engine engine(time(nullptr));
    uniform_real_distribution<> distr(0.0, 1.0);
    while (nbFail < maxFail) {
        int nbMove = 0;
        bool isFailed = true;
        while (nbMove < maxMove) {
            int v, i;
            Direction d;
            tie(v, i, d) = chooseRandomMove();
            double delta = (d == LEFT) ? deltaLeft[v] : deltaRight[v];
            if ((delta <= 0.0) || (exp(-delta / temper) >= distr(engine))) {
                insertOrder(v, i, d);
                nbMove++;
                if (order.size() > bestOrder.size()) {
                    bestOrder = order;
                    isFailed = false;
                }
            }
        }
        if (isFailed) {
            nbFail++;
        } else {
            nbFail = 0;
        }
        temper *= temperScale;
    }
    setByOrder(bestOrder);
}

int main() {
    Topo topo;
    topo.init();
    topo.cooling(0.6, 0.99, 5 * topo.graph.n, 50);
    vector<int> res;
    for (int i = 1; i <= topo.graph.n; ++i) {
        if (!topo.pos[i]) {
            res.push_back(i);
        }
    }
    for (int val : res) {
        cout << val << " ";
    }
    return 0;
}