#define DEBUG

#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <list>
#include <random>
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
    const Sc INVALID = numeric_limits<Sc>::min();
    const Sc scoreRange[2] = {numeric_limits<Sc>::min(), numeric_limits<Sc>::max()};

    typedef unordered_set<int> intSet;
    intSet outdatedVertex;
    // TODO: 为了方便randomChooseMove弄的，不确定与randomChooseMove中单独生成的情况比哪个更快
    intSet vertexNotInOrder;
    enum Direction {LEFT, RIGHT};
    // vLeft[i]表示以点i为终点的边起点中拓扑排序最后的点编号
    vector<int> vLeft;
    // vRight[i]表示以点i为起点的边终点中拓扑排序最前的点编号
    vector<int> vRight;
    // deltaLeft[i]表示将点i接在vLeft[i]后面时反馈集大小的变化
    vector<int> deltaLeft;
    // deltaRight[i]表示将点i接在vRight[i]前面时反馈集大小的变化
    vector<int> deltaRight;
    
    // 生成随机数用
    default_random_engine engine;
    uniform_real_distribution<> distr;

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
    void cooling(double initTemper, double temperScale, int maxMove, int maxFail);
};

void Topo::clear() {
    order.clear();
    pos = vector<optional<Iter>>(graph.n + 1, nullopt);
    score = vector<Sc>(graph.n + 1, INVALID);
    outdatedVertex.clear();
    vertexNotInOrder.clear();
    generate_n(inserter(vertexNotInOrder, vertexNotInOrder.end()), graph.n, []{static int x = 1; return x++;});
    vLeft = vector<int>(graph.n + 1, -1);
    vRight = vector<int>(graph.n + 1, -1);
    deltaLeft = vector<int>(graph.n + 1, -1);
    deltaRight = vector<int>(graph.n + 1, -1);
}

void Topo::init() {
    graph.getGraph();
    clear();
    engine = default_random_engine(time(nullptr));
    distr = uniform_real_distribution<>(0.0, 1.0);
}

void Topo::modifyScore() {
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
    }
    for (int i = 1; i <= graph.n; ++i) {
        updateVertex(i);
    }
}

void Topo::removeFromOrder(int v) {
    assert(pos[v].has_value());
    Iter iter = pos[v].value();
    order.erase(iter);
    pos[v] = nullopt;
    score[v] = INVALID;
    vertexNotInOrder.insert(v);
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
        assert(pos[i].has_value());
        Iter iter = pos[i].value();
        if (direc == LEFT) {
            iter++;
            pos[v] = order.insert(iter, v);
            
        } else {
            pos[v] = order.insert(iter, v);
        }
    }
    insertScore(v);
    vertexNotInOrder.erase(v);
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
        outdatedVertex.insert(graph.startFrom[setv].begin(), graph.startFrom[setv].end());
        outdatedVertex.insert(graph.endTo[setv].begin(), graph.endTo[setv].end());
    }
    // TODO: 不知道在用到对应数据时才更新是否会更快
    for (int odVertex : outdatedVertex) {
        updateVertex(odVertex);
    }
    outdatedVertex.clear();
}

void Topo::chooseRandomMove(int& v, int& i, Topo::Direction &direc) {
    sample(vertexNotInOrder.begin(), vertexNotInOrder.end(), &v, 1, engine);
    direc = (distr(engine) < 0.5) ? LEFT : RIGHT;
    i = (direc == LEFT) ? vLeft[v] : vRight[v];
}

void Topo::insertScore(int v) {
    Iter iter = pos[v].value();
    int l = scoreRange[0], r = scoreRange[1];
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
        modifyScore();
    } else {
        score[v] = res;
    }
}

void Topo::updateVertex(int v) {
    Sc maxScore = scoreRange[0];
    Sc minScore = scoreRange[1];
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

void Topo::cooling(double initTemper, double temperScale, int maxMove, int maxFail) {
    clear();
    double temper = initTemper;
    int nbFail = 0;
    list<int> bestOrder;
    while (nbFail < maxFail) {
        int nbMove = 0;
        bool isFailed = true;
        while (nbMove < maxMove) {
            int v, i;
            Direction d;
            chooseRandomMove(v, i, d);
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