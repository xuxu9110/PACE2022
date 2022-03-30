#include "topo.h"
#include <chrono>
#include <iostream>

using namespace chrono;

int main(int argc, char *argv[]) {
    auto start = system_clock::now();
    Topo topo;
    if (argc >= 2) {
        topo.init(argv[1]);
    } else {
        topo.init();
    }
    auto end = system_clock::now();
    cout << "init done, using time: " << duration_cast<microseconds>(end - start).count() * 1.0 / 1e6 << " s" << endl;
    // topo.graph.showGraph();
    start = system_clock::now();
    topo.cooling(0.6, 0.99, 5 * topo.graph.n, 50);
    end = system_clock::now();
    cout << "cooling done, using time: " << duration_cast<microseconds>(end - start).count() * 1.0 / 1e6 << " s" << endl;
    vector<int> res;
    for (int i = 1; i <= topo.graph.n; ++i) {
        if (!topo.pos[i]) {
            res.push_back(i);
        }
    }
    cout << "feedback set: ";
    for (int val : res) {
        cout << val << " ";
    }
    cout << endl << "feedback set size: " << res.size() << endl;
    string statisticName[] = {"number of loop", "number of Jump", "size of best feedback set"};
    for (int i = 0; i < topo.statistic.size(); ++i) {
        cout << statisticName[i] << ": ";
        for (int val : topo.statistic[i]) {
            cout << val << " ";
        }
        cout << endl;
    }
    return 0;
}