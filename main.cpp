#include "Input.h"
#include "QuadTree.h"
#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

int main(int argc, char **argv){

    if (argc != 5){
        cout << "Usage: ./quadTree capacity dataFile limit queryFile" << endl;
        exit(1);
    }

    int capacity = atoi(argv[1]);
	int limit = atoi(argv[3]);

    vector<float> boundary = {-180.0, -90.0, 180.0, 90.0};

    Input dataset, queries;
    dataset.loadData(argv[2], limit);
    queries.loadQueries(argv[4]);

    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    QuadTreeNode* tree = new QuadTreeNode(boundary, capacity, 0);
    tree->packing(dataset);
    double time = duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
    cout << "Index creation time: " << time << endl;

    map<string, double> rangeLog, knnLog, inLog;
    for (auto q: queries){
        if (q.type == 'r') {
            vector<float> results;
            map<string, double> stats;
            startTime = high_resolution_clock::now();
            tree->rangeQuery(q, results, stats);
            cout << "results count: " << results.size() << endl;

            rangeLog["time " + to_string(q.id)] += duration_cast<microseconds>(
                    high_resolution_clock::now() - startTime).count();
            rangeLog["count " + to_string(q.id)]++;
            rangeLog["nodes " + to_string(q.id)] += stats["leaf"] + stats["directory"];
            rangeLog["leaf " + to_string(q.id)] += stats["leaf"];
            rangeLog["directories " + to_string(q.id)] += stats["directory"];
        }
        else if (q.type == 'k') {
            map<string, double> stats;
            auto kNNPoint = q.toKNNPoint();
			cout << "knn" << endl;
            startTime = high_resolution_clock::now();
            tree->kNNQuery(kNNPoint, stats, q.id);
            knnLog["time " + to_string(q.id)] += duration_cast<microseconds>(
                    high_resolution_clock::now() - startTime).count();
            knnLog["count " + to_string(q.id)]++;
            knnLog["nodes " + to_string(q.id)] += stats["leaf"] + stats["directory"];
            knnLog["leaf " + to_string(q.id)] += stats["leaf"];
            knnLog["directories " + to_string(q.id)] += stats["directory"];
        }
        else if (q.type == 'i'){
            startTime = high_resolution_clock::now();
            tree->insert(q);
            inLog["time"] += duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
            inLog["count"]++;
        }
    }
    cout << "---Insertions---" << endl;
    for (auto it = inLog.cbegin(); it != inLog.cend(); ++it)
        cout << it->first << ": " << it->second << endl;

    cout << "---Range---" << endl;
    for (auto it = rangeLog.cbegin(); it != rangeLog.cend(); ++it)
        cout<< it->first << ": " << it->second << endl;

    cout << "---KNN---" << endl;
    for (auto it = knnLog.cbegin(); it != knnLog.cend(); ++it)
        cout<< it->first << ": " << it->second << endl;

    cout << "---Quad-Tree Statistics---" << endl;
    tree->getStatistics();
    tree->deleteTree();
    return 0;
}

