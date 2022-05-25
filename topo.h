#ifndef TOPO_H
#define TOPO_H

#include <vector>
#include <unordered_set>
#include <list>
#include <stack>
#include <random>
#include <chrono>
#include <csignal>
#include <algorithm>
#include <optional>
#include "treap.h"

using namespace std;
using namespace chrono;

typedef unordered_set<int> intSet;

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
    void getGraph(string filePath);
    void showGraph();
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

    void dealWithSmallSCC(int maxSize);
    bool isDAG(vector<int> vertices);
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
    // 每个点在拓扑排序中的时间
    vector<int> timeInOrder;
    // 每个点上次加入拓扑排序的时间
    vector<int> lastTimeInsert;
    
    // 生成随机数用
    default_random_engine engine;
    uniform_real_distribution<> distr;

    Topo copy();
    void preprocessing();
    void init();
    void init(string filpath);
    void clear();
    void showOrder();
    void showScore();
    // 重新调整score以保证相邻点的分数差距
    void modifyScore();
    // 计算插入点的分数并赋予score[v]
    void insertScore(int v);
    // 根据newOrder调整参数
    void setByOrder(list<int> newOrder);
    // 从拓扑排序中移除点v
    void removeFromOrder(int v);
    // 插入点v以生成新的拓扑排序，direc表示点i是点v的vLeft/vRight，若i为-1则为插入开头或结尾，返回被删除的点集
    intSet insertOrder(int v, int i, Direction direc);
    // 随机选择一个不在order里的点、插入位置与其对应L/R
    void chooseRandomMove(int& v, int& i, Direction& direc);
    // 更新点v的vL/R和deltaL/R
    void updateVertex(int v);
    // 用退火算法寻找最长拓扑排序
    void cooling(double initTemper, double temperScale, int maxMove, int maxFail, 
        system_clock::time_point start, int time, volatile sig_atomic_t &tle);
    // 目标函数，越大越好
    double objFunc(list<int> order);
    // 往M个方向走N步，持续记录移除的所有点，从移除的所有点中选择
    void search1(int M, int N, int numRand, int maxFail, int time, volatile sig_atomic_t &tle);
    // 往M个方向走N步，每一步所选取的点都来自前一步移除的点集
    void search2(int M, int N, int numRand, int maxFail, int time, volatile sig_atomic_t &tle);
    void search3(int M, int N, int numRand, int maxFail, int time, volatile sig_atomic_t &tle);
    // 生成初始解
    void generateInitialOrder();
};


#endif