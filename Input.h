#ifndef QUADTREE_INPUT_H
#define QUADTREE_INPUT_H

#include <bits/stdc++.h>
#include "def.h"

using namespace std;

class Record {
public:
    float id;
    char type;
    vector<float> box = vector<float>(4);

    Record();
    Record(float id, vector<float>); // for data
    Record(char type, vector<float> boundary, float info); // for query
	bool contains(Record);
    bool operator < (const Record& rhs) const;
    array<float, 2> toKNNPoint();
    ~Record();

    bool intersects(Record r);
};

class Input : public vector<Record>
{
public:
    Input();
    void loadData(const char *filename, int limit);
    void loadQueries(const char *filename);
    void sortData();
    ~Input();
};

#endif //QUADTREE_INPUT_H
