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

bool Graph::preprocessing() {
    int size = -1;
    bool isChanged = false;
    while (vertex.size() != size) {
        if (size != -1) {
            isChanged = true;
        }
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
            } else if (startFrom[i].size() == 1 && endTo[i].size() == 1) {
                // 若有点v入度初度均为1且无自环，假设两条边为(a,v)和(v,b)，则该点必然在拓扑序列中，可删去，之后添加边(a,b)
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
    return isChanged;
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


bool Graph::splitByScc() {
    vector<int>::iterator it;
    bool isChanged = false;
    for (int i : vertex) {
        for (auto it = startFrom[i].begin(); it != startFrom[i].end();) {
            int end = *it;
            if (sccIndex[i] != sccIndex[end]) {
                isChanged = true;
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
    return isChanged;
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

void Topo::init() {
    graph.getGraph();
    graph.getScc();
    graph.splitByScc();
    graph.preprocessing();
    bool isChanged = true;
    while (isChanged) {
        graph.getScc();
        isChanged = graph.splitByScc();
        if (isChanged) {
            isChanged = graph.preprocessing();
        }
    }
    clear();
    engine = default_random_engine(time(nullptr));
    distr = uniform_real_distribution<>(0.0, 1.0);
    statistic = vector<int>(3, 0);
}

void Topo::init(string filepath) {
    graph.getGraph(filepath);
    graph.getScc();
    graph.splitByScc();
    graph.preprocessing();
    bool isChanged = true;
    while (isChanged) {
        graph.getScc();
        isChanged = graph.splitByScc();
        if (isChanged) {
            isChanged = graph.preprocessing();
        }
    }
    clear();
    engine = default_random_engine(time(nullptr));
    distr = uniform_real_distribution<>(0.0, 1.0);
    statistic = vector<int>(3, 0);
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
        isInOrder[*iter] = IN;
    }
    for (int i : graph.vertex) {
        isOutdated[i] = true;
    }
}

void Topo::removeFromOrder(int v) {
    assert(pos[v].has_value());
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
        assert(pos[i].has_value());
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
    assert(pos[v].has_value());
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
            if (tle) {
                statistic[0] = nbLoop;
                statistic[1] = nbJump;
                statistic[2] = graph.vertex.size() - bestOrder.size() + graph.excludeVertex.size();
                setByOrder(bestOrder);
                return;
            }
        }
        temper *= temperScale;
    }
    setByOrder(bestOrder);
}

void Topo::cooling1(int maxFail, int scale, int maxMove, int time, volatile sig_atomic_t &tle) {
    auto start = system_clock::now();
    // time_t tt = system_clock::to_time_t(start);
    // cout << "now is " << ctime(&tt);
    list<int> bestOrder = order;
    int nbFail = 0;
    int nbLoop = 0, nbMove = 0;
    int cnt = 1;
    int v = 0, i = 0;
    Direction d = LEFT;
    while (true) {
        nbLoop++;
        chooseRandomMove(v, i, d);
        int delta = (d == LEFT) ? deltaLeft[v] : deltaRight[v];
        if (delta < 0) {
            insertOrder(v, i, d);
            nbFail = 0;
            nbMove++;
            if (order.size() > bestOrder.size()) {
                bestOrder = order;
            }
        } else {
            nbFail++;
            if (nbFail >= maxFail) {
                insertOrder(v, i, d);
                nbFail = 0;
                nbMove++;
            }
        }
        if (nbMove >= cnt * maxMove) {
            maxFail += scale;
            cnt++;
        }
        if (duration_cast<seconds>(system_clock::now() - start).count() >= time) {
            // tt = system_clock::to_time_t(system_clock::now());
            // cout << ctime(&tt);
            raise(SIGTERM);
        }
        if (tle) {
            statistic[0] = nbLoop;
            statistic[1] = nbMove;
            statistic[2] = graph.vertex.size() - bestOrder.size() + graph.excludeVertex.size();
            setByOrder(bestOrder);
            return;
        }
    }
}

void Topo::generateInitialOrder() {
    list<int> newOrder;
    Graph newGraph = graph;
    auto comp = [&newGraph](int x, int y){
        int sx = newGraph.endTo[x].size();
        int sy = newGraph.endTo[y].size();
        return ((sx != sy) ? (sx < sy) : (x < y));
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
