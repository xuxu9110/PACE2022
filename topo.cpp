#define NDEBUG

#include "topo.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <assert.h>

using namespace std;

void f() {
    optional<int> i;
}

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
            cout << s << endl;
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
}

void Graph::getGraph(string filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "could not open the file" << endl;
    }
    istream& is = file;
    getGraph(is);
}

void Graph::showGraph() {
    cout << "n: " << n << endl;
    for (int i = 1; i <= n; ++i) {
        cout << "[" << i << "]: ";
        for (auto val : startFrom[i]) {
            cout << val << " ";
        }
        cout << endl;
    }
}

void Topo::clear() {
    order.clear();
    pos = vector<optional<Iter>>(graph.n + 1, nullopt);
    score = vector<Sc>(graph.n + 1, INVALID);
    outdatedVertex.clear();
    vertexNotInOrder.clear();
    for (int i = 1; i <= graph.n; ++i) {
        vertexNotInOrder.insert(i);
    }
    vLeft = vector<int>(graph.n + 1, -1);
    vRight = vector<int>(graph.n + 1, -1);
    deltaLeft = vector<int>(graph.n + 1, -1);
    deltaRight = vector<int>(graph.n + 1, -1);
    k = ceil(graph.n / log2(graph.n));
}

void Topo::init() {
    graph.getGraph();
    clear();
    engine = default_random_engine(time(nullptr));
    distr = uniform_real_distribution<>(0.0, 1.0);
    statistic = vector<vector<int>>(3, vector<int>(0));
}

void Topo::init(string filepath) {
    graph.getGraph(filepath);
    clear();
    engine = default_random_engine(time(nullptr));
    distr = uniform_real_distribution<>(0.0, 1.0);
    statistic = vector<vector<int>>(3, vector<int>(0));
}

void Topo::showOrder() {
    cout << "order: ";
    for (auto iter = order.begin(); iter != order.end(); ++iter) {
        cout << *iter << " ";
    }
    cout << endl;
}

void Topo::showScore() {
    cout << "score: ";
    for (auto val : score) {
        if (val == INVALID) {
            cout << "INV" << " ";
        } else {
            cout << val << " ";
        }
    }
    cout << endl;
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
    // sample(vertexNotInOrder.begin(), vertexNotInOrder.end(), &v, 1, engine);
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
    Sc res = l / 2 + r / 2 + (l & r & 1);
    if ((l == res) || (res == r)) {
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

void Topo::cooling(double initTemper, double temperScale, int maxMove, int maxFail) {
    clear();
    double temper = initTemper;
    int nbFail = 0;
    list<int> bestOrder;
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
        }
        statistic[0].push_back(nbLoop);
        statistic[1].push_back(nbJump);
        statistic[2].push_back(graph.n - bestOrder.size());
        if (isFailed) {
            nbFail++;
        } else {
            nbFail = 0;
        }
        temper *= temperScale;
    }
    setByOrder(bestOrder);
}