#define _XOPEN_SOURCE

#include <vector>
#include <unordered_set>
#include <set>
#include <list>
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

    intSet outdatedVertex;
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

    // [0]表示每轮迭代次数，[1]表示每轮delta>0的步数，[2]表示每轮的最佳反馈集大小
    vector<vector<int>> statistic;
    // 优化随机算法用
    int k;
    
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
    void cooling(double initTemper, double temperScale, int maxMove, int maxFail, volatile sig_atomic_t &tle);
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
                includeVertex.insert(i);
                deleteVertex(i);
            } else if (find(startFrom[i].begin(), startFrom[i].end(), i) != startFrom[i].end()) {
                excludeVertex.insert(i);
                deleteVertex(i);
            } else if (startFrom[i].size() == 1 && endTo[i].size() == 1) {
                int a = endTo[i].at(0), b = startFrom[i].at(0);
                includeVertex.insert(i);
                deleteVertex(i);
                auto it1 = find(startFrom[a].begin(), startFrom[a].end(), b);
                if (it1 == startFrom[a].end()) {
                    startFrom[a].push_back(b);
                    endTo[b].push_back(a);
                }
            }
        }
    }
}

void Topo::clear() {
    order.clear();
    pos = vector<optional<Iter>>(graph.n + 1, nullopt);
    score = vector<Sc>(graph.n + 1, INVALID);
    outdatedVertex.clear();
    vertexNotInOrder = graph.vertex;
    vLeft = vector<int>(graph.n + 1, -1);
    vRight = vector<int>(graph.n + 1, -1);
    deltaLeft = vector<int>(graph.n + 1, -1);
    deltaRight = vector<int>(graph.n + 1, -1);
    k = sqrt(graph.n);
}

void Topo::init() {
    graph.getGraph();
    graph.preprocessing();
    clear();
    engine = default_random_engine(time(nullptr));
    distr = uniform_real_distribution<>(0.0, 1.0);
    statistic = vector<vector<int>>(3, vector<int>(0));
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
    }
    outdatedVertex.insert(graph.vertex.begin(), graph.vertex.end());
}

void Topo::removeFromOrder(int v) {
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
        Iter iter = pos[i].value();
        if (direc == LEFT) {
            iter++;
        }
        pos[v] = order.insert(iter, v);
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
    outdatedVertex.insert(rmVertex.begin(), rmVertex.end());
    for (int setv : rmVertex) {
        outdatedVertex.insert(graph.startFrom[setv].begin(), graph.startFrom[setv].end());
        outdatedVertex.insert(graph.endTo[setv].begin(), graph.endTo[setv].end());
    }
    // TODO: 不知道在用到对应数据时才更新是否会更快
    /* for (int odVertex : outdatedVertex) {
        updateVertex(odVertex);
    }
    outdatedVertex.clear(); */
}

void Topo::chooseRandomMove(int& v, int& i, Topo::Direction &direc) {
    /*
    int k = (int) (distr(engine) * vertexNotInOrder.size()) + 1;
    v = vertexNotInOrder.kth(k);
    */
    if (vertexNotInOrder.size() <= k) {
        sample(vertexNotInOrder.begin(), vertexNotInOrder.end(), &v, 1, engine);
    } else {
        while (true) {
            v = (int)(distr(engine) * graph.n + 1);
            if (vertexNotInOrder.find(v) != vertexNotInOrder.end()) {
                break;
            }
        }
    }
    if (outdatedVertex.find(v) != outdatedVertex.end()) {
        updateVertex(v);
        outdatedVertex.erase(v);
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

void Topo::cooling(double initTemper, double temperScale, int maxMove, int maxFail, volatile sig_atomic_t &tle) {
    auto start = system_clock::now();
    double temper = initTemper;
    int nbFail = 0;
    list<int> bestOrder = order;
    while (nbFail < maxFail) {
        int nbMove = 0;
        bool isFailed = true;
        int nbLoop = 0, nbJump = 0;
        while (nbMove < maxMove) {
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
                    isFailed = false;
                }
            }
            if (duration_cast<seconds>(system_clock::now() - start).count() >= 590) {
                raise(SIGTERM);
            }
            if (tle) {
                statistic[0].push_back(nbLoop);
                statistic[1].push_back(nbJump);
                statistic[2].push_back(graph.vertex.size() - bestOrder.size() + graph.excludeVertex.size());
                setByOrder(bestOrder);
                return;
            }
        }
        statistic[0].push_back(nbLoop);
        statistic[1].push_back(nbJump);
        statistic[2].push_back(graph.vertex.size() - bestOrder.size() + graph.excludeVertex.size());
        if (isFailed) {
            nbFail++;
        } else {
            nbFail = 0;
        }
        temper *= temperScale;
    }
    setByOrder(bestOrder);
}

void Topo::generateInitialOrder() {
    list<int> newOrder;
    Graph newGraph = graph;
    auto comp = [&newGraph](int x, int y){
        int sx = newGraph.endTo[x].size();
        int sy = newGraph.endTo[y].size();
        return ((sx != sy) ? (sx < sy) : (x < y));
    };
    auto orderSet = set<int, decltype(comp)>(comp);
    for (int i : newGraph.vertex) {
        orderSet.insert(i);
    }
    while (!orderSet.empty()) {
        auto it = orderSet.begin();
        int v = *it;
        newOrder.push_back(v);
        orderSet.erase(it);
        intSet ends = intSet(newGraph.startFrom[v].begin(), newGraph.startFrom[v].end());
        auto starts = newGraph.endTo[v] ;
        for (int i : starts) {
            ends.insert(newGraph.startFrom[i].begin(), newGraph.startFrom[i].end());
            orderSet.erase(i);
        }
        for (int i : starts) {
            ends.erase(i);
        }
        ends.erase(v);
        for (int i : ends) {
            orderSet.erase(i);
        }
        for (int i : starts) {
            newGraph.deleteVertex(i);
        }
        newGraph.deleteVertex(v);
        for (int i : ends) {
            orderSet.insert(i);
        }
    }
    setByOrder(newOrder);
}

int main() {
    signal(SIGTERM, term);

    auto start = system_clock::now();
    Topo topo;
    topo.init();
    auto end = system_clock::now();

    topo.generateInitialOrder();
    topo.cooling(0.6, 0.99, 5 * topo.graph.n, 50, tle);
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