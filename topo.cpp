#define NDEBUG

#include "topo.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <set>
#include <tuple>
#include <queue>
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

void nextCombination(vector<int>& pre, int size) {
    int n = pre.size();
    int ni = n - 1, maxNi = size - 1;
    while (pre[ni] + 1 > maxNi) {
        ni--;
        maxNi--;
        if (ni < 0) {
            pre[0] = -1;
            return;
        }
    }
    pre[ni]++;
    while (++ni < n) {
        pre[ni] = pre[ni - 1] + 1;
    }
}

void Graph::dealWithSmallSCC(int maxSize) {
    getScc();
    for (int i = 0; i < sccNum; ++i) {
        int size = scc[i].size();
        if (size <= maxSize) {
            vector<bool> inDAG(size, false);
            for (int curSize = 2; curSize <= size - 2; ++curSize) {
                vector<int> combination(curSize);
                for (int j = 0; j < curSize; ++j) {
                    combination[j] = j;
                }
                do {
                    vector<int> vertices;
                    for (int k : combination) {
                        vertices.push_back(scc[i][k]);
                    }
                    if (isDAG(vertices)) {
                        for (int k = 0; k < size; ++k) {
                            inDAG[k] = false;
                        }
                        for (int k : combination) {
                            inDAG[k] = true;
                        }
                        break;
                    }
                    nextCombination(combination, size);
                } while (combination[0] >= 0);
                if (combination[0] < 0) {
                    break;
                }
            }
            for (int j = 0; j < size; ++j) {
                int v = scc[i][j];
                if (inDAG[j]) {
                    includeVertex.insert(v);
                } else {
                    excludeVertex.insert(v);
                }
                vertex.erase(v);
                startFrom[v].clear();
                endTo[v].clear();
            }
        }
    }
}

bool Graph::isDAG(vector<int> vertices) {
    int size = vertices.size();
    auto sf = vector<vector<int>>(size);
    auto inDegrees = vector<int>(size, 0);
    auto isVisited = vector<bool>(size, false);
    for (int i = 0; i < size; ++i) {
        auto ends = startFrom[vertices[i]];
        for (auto end : ends) {
            int j = 0;
            while ((j < size) && (vertices[j] != end)) {
                j++;
            }
            if (j < size) {
                sf[i].push_back(j);
                inDegrees[j]++;
            }
        }
    }
    queue<int> q;
    for (int i = 0; i < size; ++i) {
        if (inDegrees[i] == 0) {
            q.push(i);
        }
    }
    while (!q.empty()) {
        int cur = q.front();
        q.pop();
        isVisited[cur] = true;
        for (int end : sf[cur]) {
            inDegrees[end]--;
            if (inDegrees[end] == 0) {
                q.push(end);
            }
        }
    }
    for (int j = 0; j < size; ++j) {
        if (!isVisited[j]) {
            return false;
        }
    }
    return true;
}

Topo Topo::copy() {
    Topo topo = Topo(*this);
    topo.vertexNotInOrder = vertexNotInOrder.copy();
    topo.pos = vector<optional<Iter>>(graph.n + 1, nullopt);
    for (auto iter = topo.order.begin(); iter != topo.order.end(); ++iter) {
        topo.pos[*iter] = iter;
    }
    return topo;
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
    timeInOrder = vector<int>(graph.n + 1, 0);
    lastTimeInsert = vector<int>(graph.n + 1, -1);
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
    graph.dealWithSmallSCC(9);
}

void Topo::init() {
    graph.getGraph();
    preprocessing();
    clear();
    engine = default_random_engine(time(nullptr));
    distr = uniform_real_distribution<>(0.0, 1.0);
    statistic = vector<int>(3, 0);
}

void Topo::init(string filepath) {
    graph.getGraph(filepath);
    preprocessing();
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

intSet Topo::insertOrder(int v, int i, Direction direc) {
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
    intSet updateSet = rmVertex;
    updateSet.insert(v);
    for (int setv : updateSet) {
        isOutdated[setv] = true;
        for (int j : graph.startFrom[setv]) {
            isOutdated[j] = true;
        }
        for (int j : graph.endTo[setv]) {
            isOutdated[j] = true;
        }
    }
    return rmVertex;
}

void Topo::chooseRandomMove(int& v, int& i, Topo::Direction &direc) {
    if (vertexNotInOrder.size() <= k) {
        int k = (int) (distr(engine) * vertexNotInOrder.size()) + 1;
        v = vertexNotInOrder.kth(k);
    } else {
        while (true) {
            v = (int)(distr(engine) * graph.n + 1);
            if (isInOrder[v] == OUT) {
                break;
            }
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

void Topo::cooling(double initTemper, double temperScale, int maxMove, int maxFail, 
                    system_clock::time_point start, int time, volatile sig_atomic_t &tle) {
    double temper = initTemper;
    double heatScale = 1.0 / pow(temperScale, maxFail);
    list<int> bestOrder = order;
    int nbLoop = 0, nbJump = 0, nbFail = 0;
    while (true) {
        int nbMove = 0;
        bool isFailed = true;
        while (nbMove < maxMove) {
            if (duration_cast<seconds>(system_clock::now() - start).count() >= time) {
                raise(SIGTERM);
            }
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
                    isFailed = false;
                }
            }
        }
        if (isFailed) {
            nbFail++;
            if (nbFail < maxFail) {
                temper *= temperScale;
            } else {
                nbFail = 0;
                temper *= heatScale;
                if (temper > initTemper) {
                    temper = initTemper;
                }
            }
        } else {
            nbFail = 0;
            temper *= temperScale;
        }
    }
}

double Topo::objFunc(list<int> order) {
    return order.size();
}

void Topo::search1(int M, int N, int numRand, int maxFail, int time, volatile sig_atomic_t &tle) {
    auto start = system_clock::now();
    list<int> bestOrder = order;
    int nbLoop = 0, nbMove = 0;
    while (true) {
        if (tle || vertexNotInOrder.size() == 0) {
            statistic[0] = nbLoop;
            statistic[1] = nbMove;
            statistic[2] = graph.vertex.size() - bestOrder.size() + graph.excludeVertex.size();
            setByOrder(bestOrder);
            return;
        }
        nbLoop++;
        // 往M个方向走N步，持续记录移除的所有点，后N-n步从移除的所有点中选择
        typedef tuple<int, int, Direction, double> State;
        auto state = vector<vector<State>>(M);
        pair<int, int> bestPos = make_pair(-1, -1);
        double bestFunc = numeric_limits<double>::min();
        for (int i = 0; i < M; ++i) {
            intSet removeSet;
            int fail = 0;
            double lastFunc = objFunc(order);
            Topo topo = copy();
            for (int j = 0; j < N; ++j) {
                int v = 0, pos = 0;
                Direction d = LEFT;
                if ((j < numRand) || (removeSet.size() == 0)) {
                    topo.chooseRandomMove(v, pos, d);
                } else {
                    int minDelta = INT_MAX;
                    for (int curV : removeSet) {
                        if (topo.isOutdated[curV]) {
                            topo.updateVertex(curV);
                            topo.isOutdated[curV] = false;
                        }
                        if (topo.deltaLeft[curV] < minDelta) {
                            minDelta = topo.deltaLeft[curV];
                            v = curV;
                            pos = vLeft[curV];
                            d = LEFT;
                        }
                        if (topo.deltaRight[curV] < minDelta) {
                            minDelta = topo.deltaRight[curV];
                            v = curV;
                            pos = vRight[curV];
                            d = RIGHT;
                        }
                    }
                }
                removeSet.erase(v);
                intSet rmSet = topo.insertOrder(v, pos, d);
                removeSet.insert(rmSet.begin(), rmSet.end());
                double curFunc = topo.objFunc(topo.order);
                state[i].push_back(make_tuple(v, pos, d, curFunc));
                if (curFunc > bestFunc) {
                    bestFunc = curFunc;
                    bestPos = make_pair(i, j);
                }
                if (curFunc <= lastFunc) {
                    fail++;
                } else {
                    fail = 0;
                }
                if (fail > maxFail) {
                    break;
                }
                lastFunc = curFunc;
            }
            topo.vertexNotInOrder.clear();
        }
        int i = bestPos.first;
        nbMove += bestPos.second + 1;
        for (int j = 0; j <= bestPos.second; ++j) {
            int v = get<0>(state[i][j]), pos = get<1>(state[i][j]);
            Direction d = get<2>(state[i][j]);
            insertOrder(v, pos, d);
            if (order.size() > bestOrder.size()) {
                bestOrder = order;
            }
        }
        if (duration_cast<seconds>(system_clock::now() - start).count() >= time) {
            raise(SIGTERM);
        }
    }
}

void Topo::search2(int M, int N, int numRand, int maxFail, int time, volatile sig_atomic_t &tle) {
    auto start = system_clock::now();
    list<int> bestOrder = order;
    int nbLoop = 0, nbMove = 0;
    typedef tuple<int, int, Direction, double> State;
    while (true) {
        if (tle || vertexNotInOrder.size() == 0) {
            statistic[0] = nbLoop;
            statistic[1] = nbMove;
            statistic[2] = graph.vertex.size() - bestOrder.size() + graph.excludeVertex.size();
            setByOrder(bestOrder);
            return;
        }
        nbLoop++;
        // 往M个方向走N步，持续记录移除的所有点，除第一步外每一步所选取的点都来自前一步移除的点集
        auto state = vector<vector<State>>(M);
        pair<int, int> bestPos = make_pair(-1, -1);
        double bestFunc = numeric_limits<double>::min();
        for (int i = 0; i < M; ++i) {
            intSet removeSet;
            int fail = 0;
            double lastFunc = objFunc(order);
            Topo topo = copy();
            for (int j = 0; j < N; ++j) {
                int v = 0, pos = 0;
                Direction d = LEFT;
                if (removeSet.size() == 0) {
                    topo.chooseRandomMove(v, pos, d);
                } else if (j < numRand) {
                    sample(removeSet.begin(), removeSet.end(), &v, 1, engine);
                    if (topo.isOutdated[v]) {
                        topo.updateVertex(v);
                        topo.isOutdated[v] = false;
                    }
                    d = (distr(engine) < 0.5) ? LEFT : RIGHT;
                    pos = (d == LEFT) ? vLeft[v] : vRight[v];
                } else {
                    int minDelta = INT_MAX;
                    for (int curV : removeSet) {
                        if (topo.isOutdated[curV]) {
                            topo.updateVertex(curV);
                            topo.isOutdated[curV] = false;
                        }
                        if (topo.deltaLeft[curV] < minDelta) {
                            minDelta = topo.deltaLeft[curV];
                            v = curV;
                            pos = vLeft[curV];
                            d = LEFT;
                        }
                        if (topo.deltaRight[curV] < minDelta) {
                            minDelta = topo.deltaRight[curV];
                            v = curV;
                            pos = vRight[curV];
                            d = RIGHT;
                        }
                    }
                }
                removeSet = topo.insertOrder(v, pos, d);
                double curFunc = topo.objFunc(topo.order);
                state[i].push_back(make_tuple(v, pos, d, curFunc));
                if (curFunc > bestFunc) {
                    bestFunc = curFunc;
                    bestPos = make_pair(i, j);
                }
                if (curFunc <= lastFunc) {
                    fail++;
                } else {
                    fail = 0;
                }
                if (fail > maxFail) {
                    break;
                }
                lastFunc = curFunc;
            }
            topo.vertexNotInOrder.clear();
        }
        int i = bestPos.first;
        nbMove += bestPos.second + 1;
        for (int j = 0; j <= bestPos.second; ++j) {
            int v = get<0>(state[i][j]), pos = get<1>(state[i][j]);
            Direction d = get<2>(state[i][j]);
            insertOrder(v, pos, d);
            if (order.size() > bestOrder.size()) {
                bestOrder = order;
            }
        }
        if (duration_cast<seconds>(system_clock::now() - start).count() >= time) {
            raise(SIGTERM);
        }
    }
}

void Topo::search3(int M, int N, int numRand, int maxFail, int time, volatile sig_atomic_t &tle) {
    auto start = system_clock::now();
    list<int> bestOrder = order;
    int nbLoop = 0, nbMove = 0;
    typedef tuple<int, int, Direction, double> State;
    while (true) {
        if (tle || vertexNotInOrder.size() == 0) {
            statistic[0] = nbLoop;
            statistic[1] = nbMove;
            statistic[2] = graph.vertex.size() - bestOrder.size() + graph.excludeVertex.size();
            setByOrder(bestOrder);
            return;
        }
        nbLoop++;
        // 往M个方向走N步，持续记录移除的所有点，除第一步外每一步所选取的点都来自前一步移除的点集
        auto state = vector<vector<State>>(M);
        pair<int, int> bestPos = make_pair(-1, -1);
        double bestFunc = numeric_limits<double>::min();
        for (int i = 0; i < M; ++i) {
            intSet removeSet;
            int fail = 0;
            double lastFunc = objFunc(order);
            Topo topo = copy();
            for (int j = 0; j < N; ++j) {
                int v = 0, pos = 0;
                Direction d = LEFT;
                if (removeSet.size() == 0) {
                    topo.chooseRandomMove(v, pos, d);
                } else if (j < numRand) {
                    sample(removeSet.begin(), removeSet.end(), &v, 1, engine);
                    if (topo.isOutdated[v]) {
                        topo.updateVertex(v);
                        topo.isOutdated[v] = false;
                    }
                    d = (distr(engine) < 0.5) ? LEFT : RIGHT;
                    pos = (d == LEFT) ? vLeft[v] : vRight[v];
                } else {
                    int minDelta = INT_MAX;
                    for (int curV : removeSet) {
                        if (topo.isOutdated[curV]) {
                            topo.updateVertex(curV);
                            topo.isOutdated[curV] = false;
                        }
                        if (topo.deltaLeft[curV] < minDelta) {
                            minDelta = topo.deltaLeft[curV];
                            v = curV;
                            pos = vLeft[curV];
                            d = LEFT;
                        }
                        if (topo.deltaRight[curV] < minDelta) {
                            minDelta = topo.deltaRight[curV];
                            v = curV;
                            pos = vRight[curV];
                            d = RIGHT;
                        }
                    }
                }
                removeSet.erase(v);
                intSet rmSet = topo.insertOrder(v, pos, d);
                removeSet.insert(rmSet.begin(), rmSet.end());
                //removeSet = topo.insertOrder(v, pos, d);
                double curFunc = topo.objFunc(topo.order);
                state[i].push_back(make_tuple(v, pos, d, curFunc));
                if (curFunc > bestFunc) {
                    bestFunc = curFunc;
                    bestPos = make_pair(i, j);
                }
                if (curFunc <= lastFunc) {
                    fail++;
                } else {
                    fail = 0;
                }
                if (fail > maxFail) {
                    break;
                }
                lastFunc = curFunc;
            }
            topo.vertexNotInOrder.clear();
        }
        int i = bestPos.first;
        nbMove += bestPos.second + 1;
        for (int j = 0; j <= bestPos.second; ++j) {
            int v = get<0>(state[i][j]), pos = get<1>(state[i][j]);
            Direction d = get<2>(state[i][j]);
            insertOrder(v, pos, d);
            if (order.size() > bestOrder.size()) {
                bestOrder = order;
            }
        }
        if (duration_cast<seconds>(system_clock::now() - start).count() >= time) {
            raise(SIGTERM);
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
