#include "QuadTree.h"

struct Stats {
    struct StatType {
        long count = 0;
        long io = 0;
        // long time = 0;
    };

    StatType del;
    StatType insert;
    map<int, StatType> knn;
    map<float, StatType> range;
    StatType reload;
};

void createQuerySet(string fileName, vector<tuple<char, vector<float>, float>> &queryArray) {
    cout << "Begin query creation for QuadTree" << endl;
    string line;

    ifstream file(fileName);
    if (file.is_open()) {
        // getline(file, line); // Skip the header line
        while (getline(file, line)) {
            char type = line[line.find_first_not_of(" ")];
            vector<float> q;
            if (type == 'l') {
                queryArray.emplace_back(make_tuple(type, q, 0));
            } else {
                line = line.substr(line.find_first_of(type) + 1);
                const char *cs = line.c_str();
                char *end;
                int params = (type == 'r') ? 4 : 2;
                for (uint d = 0; d < params; d++) {
                    q.emplace_back(strtof(cs, &end));
                    cs = end;
                }
                float info = strtof(cs, &end);
                queryArray.emplace_back(make_tuple(type, q, info));
            }
        }
        file.close();
    }
    cout << "Finish query creation for QuadTree" << endl;
}


void knnQuery(tuple<char, vector<float>, float> q, QuadTreeNode *index, Stats &stats) {
    array<float, 2> p;
    for (uint i = 0; i < p.size(); i++)
        p[i] = get<1>(q)[i];
    int k = get<2>(q);
    map<string, double> info;
    index->kNNQuery(p, info, k);        
    
    // cerr << "Points: " << p[0] << " | " << p[1] << endl;

    // high_resolution_clock::time_point startTime = high_resolution_clock::now();
    //Info info = index->kNNQuery(p, k);
    /* stats.knn[k].time +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count(); */
    // stats.knn[k].io += info.reads;
    stats.knn[k].io += info["leaf"];
    stats.knn[k].count++;
}

void rangeQuery(tuple<char, vector<float>, float> q, QuadTreeNode *index, Stats &stats) {
    vector<float> query(4);
    for (uint i = 0; i < query.size(); i++)
        query[i] = get<1>(q)[i];
    float rs = get<2>(q);
    Record rec = Record('r', query, rs); 
    vector<float> results;
    map<string, double> info;

    index->rangeQuery(rec, results, info);

    // high_resolution_clock::time_point startTime = high_resolution_clock::now();
    // Info info = index->rangeQuery(query);
    /* stats.range[rs].time +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count(); */
    // stats.range[rs].io += info.reads;
    stats.range[rs].io += info["leaf"];
    stats.range[rs].count++;
}

void insertQuery(tuple<char, vector<float>, float> q, QuadTreeNode *index, Stats &stats) {
    Record rec;
    for (uint i = 0; i < rec.box.size() / 2; i++){
        rec.box[i] = get<1>(q)[i];
        rec.box[i+2] = get<1>(q)[i];
    }
    
    //cout << rec.box[0] << ","<< rec.box[1] << ","<< rec.box[2] << ","<< rec.box[3] << endl;
    rec.id = get<2>(q);
    rec.type = get<0>(q);
    map<string, double> info;

    index->insert(rec, info);

    // high_resolution_clock::time_point startTime = high_resolution_clock::now();
    // Info info = index->insertQuery(p);
    /* stats.insert.time +=
        duration_cast<microseconds>(high_resolution_clock::now() - startTime).count(); */
    // stats.insert.io += info.writes;
    stats.insert.io += info["IO"];
    stats.insert.count++;
}

void deleteQuery(tuple<char, vector<float>, float> q, QuadTreeNode *index, Stats &stats) {
    /*
    Record p;
    for (uint i = 0; i < p.data.size(); i++)
        p.data[i] = get<1>(q)[i];
    p.id = get<2>(q);

    // high_resolution_clock::time_point startTime = high_resolution_clock::now();
    Info info = index->deleteQuery(p);
    // stats.del.time += duration_cast<microseconds>(high_resolution_clock::now() -
    // startTime).count();
    stats.del.count++;
    */
}


//void evaluate(MPT *index, string queryFile, string logFile) {
void evaluate(QuadTreeNode *index, string queryFile, string logFile) {
    Stats stats;
    auto roundit = [](float val, int d = 2) { return round(val * pow(10, d)) / pow(10, d); };
    bool canQuery = false;

    cout << "Begin Querying " << queryFile << endl;
    string line;
    ifstream file(queryFile);
    if (file.is_open()) {
        // getline(file, line); // Skip the header line
        while (getline(file, line)) {
            char type = line[line.find_first_not_of(" ")];
            tuple<char, vector<float>, float> q;
            vector<float> pts;
            if (type == 'l') {
                q = make_tuple(type, pts, 0);
            } else {
                line = line.substr(line.find_first_of(type) + 1);
                const char *cs = line.c_str();
                char *end;
                int params = (type == 'r') ? 4 : 2;
                for (uint d = 0; d < params; d++) {
                    pts.emplace_back(strtof(cs, &end));
                    cs = end;
                }
                float info = strtof(cs, &end);
                q = make_tuple(type, pts, info);
            }
            if (get<0>(q) == 'k') {
	    	if (canQuery) knnQuery(q, index, stats);
            } else if (get<0>(q) == 'r') {
                if (canQuery) rangeQuery(q, index, stats);
            } else if (get<0>(q) == 'i') {
                insertQuery(q, index, stats);
            } else if (get<0>(q) == 'd') {
                deleteQuery(q, index, stats);
            } else if (get<0>(q) == 'z') {
                stats = Stats();
		canQuery = true;
            } else if (get<0>(q) == 'l') {
                ofstream log;
                log.open(logFile, ios_base::app);
                if (!log.is_open())
                    cerr << "Unable to open log.txt";

                log << "------------------Results-------------------" << endl << endl;

                log << "------------------Range Queries-------------------" << endl;
                log << setw(8) << "Size" << setw(8) << "Count" << setw(8) << "I/O" << setw(8)
                    << "Time" << endl;
                for (auto &l : stats.range) {
                    log << setw(8) << l.first << setw(8) << l.second.count << setw(8)
                        << roundit(l.second.io / double(l.second.count)) << endl;
                    // << roundit(l.second.time / double(l.second.count)) << endl;
                }

                log << endl << "------------------KNN Queries-------------------" << endl;
                log << setw(8) << "k" << setw(8) << "Count" << setw(8) << "I/O" << setw(8) << "Time"
                    << endl;
                for (auto &l : stats.knn) {
                    log << setw(8) << l.first << setw(8) << l.second.count << setw(8)
                        << roundit(l.second.io / double(l.second.count)) << endl;
                    // << roundit(l.second.time / double(l.second.count)) << endl;
                }

                log << endl << "------------------Insert Queries-------------------" << endl;
                log << "Count:\t" << stats.insert.count << endl;
                log << "I/O:\t" << stats.insert.io / double(stats.insert.count) << endl;
                // log << "Time: \t" << stats.insert.time / double(stats.insert.count) << endl <<
                // endl;

                log << endl << "------------------ Reloading -------------------" << endl;
                log << "Count:\t" << stats.reload.count << endl;
                log << "I/O (overall):\t" << stats.reload.io << endl << endl;


                map<string, float> info = index->getStatistics();  
                //map<string, double> info;
                //float indexSize = index->size(info);
                log << "QuadTree size in MB: " << info["size"] << endl;
                log << "No. of pages: " << info["pages"] << endl;
                log << "No. of directories: " << info["directories"] << endl;

                log << endl << "************************************************" << endl;

                log.close();
            } else
                cerr << "Invalid Query!!!" << endl;
            // cerr << endl;
        }
        file.close();
    } else cerr << "Error with query file! Path: " << queryFile << endl;
    cout << "Finish Querying..." << endl;
}

int main(int argCount, char **args) {
    map<string, string> config;
    string projectPath = string(args[1]);
    string queryType = string(args[2]);
    int directoryCap = CAPACITY;
    int pageCap = CAPACITY;
    long limit = 1e7;
    string sign = "-" + to_string(directoryCap);
    // sign += "-T" + to_string(int(100 * TOLERANCE));

	
    string expPath = projectPath + "/Experiments/";
    string prefix = expPath + queryType + "/";
    string queryFile = projectPath + "/Queries/" + queryType + ".txt";
    /* string queryFile = projectPath + "/data/OSM-USA/" + queryType;
    string dataFile = projectPath + "/data/OSM-USA/osm-usa-10mil"; */
    /* string queryFile = projectPath + "/data/NewYorkTaxi/" + queryType;
    string dataFile = projectPath + "/data/NewYorkTaxi/taxiNY"; */
    int offset = 0;

    cout << "---Generation--- " << endl;


    cout << "DEBUG: " << prefix + "log" + sign + ".txt" << endl;

    string logFile = prefix + "log" + sign + ".txt";
    ofstream log(logFile);
    if (!log.is_open())
        cout << "Unable to open log.txt";
    
    vector<float> boundary = {-180.0, -90.0, 180.0, 90.0};
    // high_resolution_clock::time_point start = high_resolution_clock::now();
    cout << "Defining QuadTree..." << endl;
    QuadTreeNode* tree = new QuadTreeNode(boundary, 0);
    //MPT index = MPT(directoryCap, pageCap);
    /* cout << "Bulkloading MPT..." << endl;
    index.bulkload(dataFile, limit);
    double hTreeCreationTime =
        duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    log << "MPT Creation Time: " << hTreeCreationTime << endl; */
    log << "Directory Capacity: " << directoryCap << endl;
    log << "Page Capacity: " << pageCap << endl;
    /* map<string, double> info;
    float indexSize = index.size(info);
    log << "MPT size in MB: " << float(indexSize / 1e6) << endl;
    log << "No. of pages: " << info["pages"] << endl;
    log << "No. of directories: " << info["directories"] << endl; */

    /* cout << "---Creating query set---" << endl;
    vector<tuple<char, vector<float>, float>> queryArray;
    createQuerySet(queryFile, queryArray); */

    cout << "---Evaluation--- " << endl;
    //evaluate(&index, queryFile, logFile);
    evaluate(tree, queryFile, logFile);
    //index.snapshot(prefix);
    return 0;
}

