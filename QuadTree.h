#ifndef QUADTREE_QUADTREE_H
#define QUADTREE_QUADTREE_H

#include "Input.h"
#include <bits/stdc++.h>
#include "def.h"

using namespace std;

class QuadTreeNode{
public :
    vector<QuadTreeNode*> children = vector<QuadTreeNode*>(4);
    Input data;
    int level;
    vector<float> box;

    QuadTreeNode(vector<float> boundary, int level);
    void insert(Record);
    bool intersects(Record r);
    void rangeQuery(Record q, vector<float> &results, map<string, double> &map);
    void kNNQuery(array<float, 2> p, map<string, double> &stats, int k);
    void deleteTree();
    void calculateSize(int &);
    void getTreeHeight(int &);
    void snapshot();
    double minSqrDist(array<float, 4> r) const;
    void packing(Input &R);
    void packing();
    void divide();
    void count(int &, int &, int &, int &);
    bool isLeaf();
    void getStatistics();
    ~QuadTreeNode();
};

#endif //QUADTREE_QUADTREE_H
