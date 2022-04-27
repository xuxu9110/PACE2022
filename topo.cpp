#define NDEBUG

#include "topo.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <set>
#include <assert.h>

using namespace std;

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
    for (int i : vertex) {
        cout << "[" << i << "]: ";
        for (auto val : startFrom[i]) {
            cout << val << " ";
        }
        cout << endl;
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
    /*
     1. 若有点v入度或出度为0，则点v必然在拓扑序列中，可删去；
     2. 若有点v入度初度均为1，假设两条边为(a,v)和(v,b)，则该点必然在拓扑序列中，可删去，之后添加边(a,b)；
     3. 若有点出现自环，则该点必然在反馈集中，可删去；
    */
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

void Topo::init(string filepath) {
    graph.getGraph(filepath);
    graph.preprocessing();
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
    // time_t tt = system_clock::to_time_t(start);
    // cout << "now is " << ctime(&tt);
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
            if (duration_cast<seconds>(system_clock::now() - start).count() >= 595) {
                // tt = system_clock::to_time_t(system_clock::now());
                // cout << ctime(&tt);
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