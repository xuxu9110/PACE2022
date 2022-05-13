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
    statisticFile << "  \"size_after_preprocessing\": " << topo.graph.vertex.size() << "," << endl;
    statisticFile << "  \"include_size\": " << topo.graph.includeVertex.size() << "," << endl;
    statisticFile << "  \"exclude_size\": " << topo.graph.excludeVertex.size() << "," << endl;
    // topo.graph.showGraph();
    start = system_clock::now();
    topo.generateInitialOrder();
    end = system_clock::now();
    statisticFile << "  \"generate_initial_order_time\": " << duration_cast<microseconds>(end - start).count() << "," << endl;
    statisticFile << "  \"initial_order_size\": " << topo.order.size() << "," << endl;
    start = system_clock::now();
    topo.cooling(0.6, 0.99, 5 * topo.graph.n, 595, tle);
    //topo.cooling1(5, 1, 5 * topo.graph.vertex.size(), 595, tle);
    end = system_clock::now();
    statisticFile << "  \"cooling_time\": " << duration_cast<microseconds>(end - start).count() << "," << endl;

    vector<int> res(topo.graph.excludeVertex.begin(), topo.graph.excludeVertex.end());
    for (int i : topo.graph.vertex) {
        if (!topo.pos[i].has_value()) {
            res.push_back(i);
        }
    }
    sort(res.begin(), res.end());
    ofstream resultFile(resultPath + resultFileName);
    for (int val : res) {
        resultFile << val << endl;
    }
    resultFile.close();
    statisticFile << "  \"feedback_set_size\": " << res.size() << "," << endl;
    statisticFile << "  \"number_of_loop\": " << topo.statistic[0] << "," << endl;
    statisticFile << "  \"number_of_move\": " << topo.statistic[1] << "," << endl;
    statisticFile << "  \"size_of_set\": " << topo.statistic[2] << endl;
    statisticFile << "}";
    statisticFile.close();
    return 0;
}