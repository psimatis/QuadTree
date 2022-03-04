#include "Input.h"
#include "QuadTree.h"
#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

int main(int argc, char **argv){

    if (argc != 4){
        cout << "Usage: ./quadTree dataFile limit queryFile" << endl;
        exit(1);
    }

	int limit = atoi(argv[2]);

    vector<float> boundary = {-180.0, -90.0, 180.0, 90.0};

    Input dataset, queries;
    dataset.loadData(argv[1], limit);
    queries.loadQueries(argv[3]);

    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    QuadTreeNode* tree = new QuadTreeNode(boundary, 0);
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
            //cout << "results count: " << results.size() << endl;
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
        else{
    		cout << "---Insertions---" << endl;
    		for (auto it = inLog.begin(); it != inLog.end(); ++it){
        		cout << it->first << ": " << it->second << endl;
				it->second = 0;
			}
    		cout << "---Range---" << endl;
    		for (auto it = rangeLog.begin(); it != rangeLog.end(); ++it){
        		cout<< it->first << ": " << it->second << endl;
        		it->second = 0;
        	}
    		cout << "---KNN---" << endl;
    		for (auto it = knnLog.begin(); it != knnLog.end(); ++it){
        		cout<< it->first << ": " << it->second << endl;
        		it->second = 0;
        	}
    		cout << "---Quad-Tree Statistics---" << endl;
    		tree->getStatistics();
    	}
    }
    return 0;
}

