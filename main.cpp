#define _XOPEN_SOURCE
#include "topo.h"
#include <iostream>
#include <fstream>
#include <unistd.h>

volatile sig_atomic_t tle = 0;

void term(int signum)
{
    // cout << "catch SIGTERM!" << endl;
    tle = 1;
}

int main(int argc, char *argv[]) {

    signal(SIGTERM, term);

    auto start = system_clock::now();
    Topo topo;
    if (argc >= 2) {
        topo.init(argv[1]);
    } else {
        topo.init();
    }
    auto end = system_clock::now();
    
    string resultPath = "result/";
    string statisticPath = "statistic/";
    string resultFileName = (argc >= 3) ? argv[2] : "result";
    string statisticFileName = (argc >= 3) ? argv[2] : "statistic";
    ofstream statisticFile(statisticPath + statisticFileName);

    statisticFile << "{" << endl << "  \"init_time\": " << duration_cast<microseconds>(end - start).count() << "," << endl;
    // topo.graph.showGraph();
    start = system_clock::now();
    topo.cooling(0.6, 0.99, 5 * topo.graph.n, 50, tle);
    end = system_clock::now();
    statisticFile << "  \"cooling_time\": " << duration_cast<microseconds>(end - start).count() << "," << endl;
    vector<int> res;
    for (int i = 1; i <= topo.graph.n; ++i) {
        if (!topo.pos[i]) {
            res.push_back(i);
        }
    }
    ofstream resultFile(resultPath + resultFileName);
    for (int val : res) {
        resultFile << val << endl;
    }
    resultFile.close();
    statisticFile << "  \"feedback_set_size\": " << res.size() << "," << endl;
    string statisticName[] = {"number_of_loop", "number_of_jump", "size_of_set"};
    for (int i = 0; i < topo.statistic.size(); ++i) {
        statisticFile << "  \"" << statisticName[i] << "\": [";
        bool isFirst = true;
        for (int val : topo.statistic[i]) {
            if (!isFirst) {
                statisticFile << ", ";
            } else {
                isFirst = false;
            }
            statisticFile << val;
        }
        statisticFile << (i == 2 ? "]" : "],") << endl;
    }
    statisticFile << "}";
    statisticFile.close();
    return 0;
}