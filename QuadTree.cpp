#include "QuadTree.h"

#define all(c) c.begin(), c.end()
#define dist(x1, y1, x2, y2) (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)

QuadTreeNode::QuadTreeNode(vector<float> boundary, int capacity, int level){
    this->capacity = capacity;
    this->box = boundary;
    this->level = level;
}

bool QuadTreeNode::isLeaf(){
    if (this->children[NW] == NULL) return true;
    return false;
}


void QuadTreeNode::packing(Input &R) {
    for (auto r: R) {
        if (inBoundary(r))
            this->data.push_back(r);
    }
    this->packing();
}

void QuadTreeNode::packing() {
    if (this->data.size() > this->capacity){
        if (this->isLeaf()) this->divide();
        for (auto r: this->data) {
            for (auto c: children)
                if (c->inBoundary(r)) c->data.push_back(r);
        }
        this->data.clear();
        for (auto c: children) c->packing();
    }
}

void QuadTreeNode::insert(Record r){
    if (!inBoundary(r)) return;
    if (this->data.size() < this->capacity) this->data.push_back(r);
    else{
        if (this->isLeaf()) this->divide();

        for (auto rec: this->data){
            for (auto c: children)
                c->insert(rec);
        }
        this->data.clear();
        for (auto c: children) c->insert(r);
    }
}

void QuadTreeNode::divide(){

    vector<float> box = this->box;
    float xMid, yMid;
    if (POINT_SPLIT) {
        this->data.sortData();
        auto median = this->data[this->data.size() / 2];
        xMid = median.box[XLOW];
        yMid = median.box[YLOW];
    }
    else{
        xMid = (box[XHIGH] + box[XLOW]) / 2.0;
        yMid = (box[YHIGH] + box[YLOW]) / 2.0;
    }

    vector<float> northWest = {box[XLOW], yMid, xMid, box[YHIGH]};
    this->children[NW] = new QuadTreeNode(northWest, this->capacity,this->level + 1);

    vector<float> northEast = {xMid, yMid, box[XHIGH], box[YHIGH]};
    this->children[NE] = new QuadTreeNode(northEast, this->capacity, this->level + 1);

    vector<float> southWest = {box[XLOW], box[YLOW], xMid, yMid};
    this->children[SW] = new QuadTreeNode(southWest, this->capacity, this->level+1);

    vector<float> southEast = {xMid, box[YLOW], box[XHIGH], yMid};
    this->children[SE] = new QuadTreeNode(southEast, this->capacity, this->level+1);
}

bool QuadTreeNode::inBoundary(Record r){
    return !(this->box[XLOW] >= r.box[XHIGH] || this->box[XHIGH] < r.box[XLOW]
            || this->box[YLOW] >= r.box[YHIGH] || this->box[YHIGH] < r.box[YLOW]);
}

void QuadTreeNode::rangeQuery(Record q, vector<float> &resultItemsIds, map<string, double> &stats) {
    if (!inBoundary(q)) return;
    if (this->isLeaf()) {
        if (q.box[XHIGH] > this->box[XLOW] || q.box[XLOW] < this->box[XHIGH] || q.box[YHIGH] > this->box[YLOW] || q.box[YLOW] < this->box[YHIGH] ){
            stats["leaf"]++;
            for (auto r: this->data){
                if (!r.inBoundary(q)) continue;
                else{
                    //cout << r.id << endl;
                    resultItemsIds.push_back(r.id);
                }
            }
        }
        return;
    }
    stats["directory"]++;
    for (auto c: children) c-> rangeQuery(q, resultItemsIds, stats);
}

typedef struct knnPoint {
    array<float, 2> pt;
    double dist = FLT_MAX;
    int id;
    bool operator<(const knnPoint &second) const { return dist < second.dist; }
} knnPoint;

typedef struct knnNode {
    QuadTreeNode *sn;
    double dist = FLT_MAX;
    bool operator<(const knnNode &second) const { return dist > second.dist; }
} knnNode;

double QuadTreeNode::minSqrDist(array<float, 4> r) const {
    auto b = this->box;
    bool left = r[2] < b[XLOW];
    bool right = b[XHIGH] < r[0];
    bool bottom = r[3] < b[YLOW];
    bool top = b[YHIGH] < r[1];
    if (top) {
        if (left) return dist(b[XLOW], b[YHIGH], r[2], r[1]);
        if (right) return dist(b[XHIGH], b[YHIGH], r[0], r[1]);
        return (r[1] - b[YHIGH]) * (r[1] - b[YHIGH]);
    }
    if (bottom) {
        if (left) return dist(b[XLOW], b[YLOW], r[2], r[3]);
        if (right) return dist(b[XHIGH], b[YLOW], r[0], r[3]);
        return (b[YLOW] - r[3]) * (b[YLOW] - r[3]);
    }
    if (left) return (b[XLOW] - r[2]) * (b[XLOW] - r[2]);
    if (right) return (r[0] - b[XHIGH]) * (r[0] - b[XHIGH]);
    return 0;
}

void QuadTreeNode::kNNQuery(array<float, 2> q, map<string, double> &stats, int k) {
    auto calcSqrDist = [](array<float, 4> x, array<float, 2> y) {
        return pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2);
    };

    vector<knnPoint> tempPts(k);
    array<float, 4> query{q[0], q[1], q[0], q[1]};
    priority_queue<knnPoint, vector<knnPoint>> knnPts(all(tempPts));
    priority_queue<knnNode, vector<knnNode>> unseenNodes;
    unseenNodes.emplace(knnNode{this, this->minSqrDist(query)});
    double dist, minDist;
    QuadTreeNode *node;

    while (!unseenNodes.empty()) {
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnPts.top().dist;
        if (dist < minDist) {
            if (node->isLeaf()) {
                for (auto p : node->data){
                    minDist = knnPts.top().dist;
                    dist = calcSqrDist(query, p.toKNNPoint());
                    if (dist < minDist) {
                        knnPoint kPt;
                        kPt.pt = p.toKNNPoint();
                        kPt.dist = dist;
                        kPt.id = p.id;
                        knnPts.pop();
                        knnPts.push(kPt);
                    }
                }
                stats["leaf"]++;
            } else {
                minDist = knnPts.top().dist;
                for (auto c: node->children) {
                    dist = c->minSqrDist(query);
                    if (dist < minDist) {
                        knnNode kn;
                        kn.sn = c;
                        kn.dist = dist;
                        unseenNodes.push(kn);
                    }
                }
                stats["directory"]++;
            }
        } else break;
    }

    while (!knnPts.empty()) {
        cout << knnPts.top().pt[0] << " "  << knnPts.top().pt[1] << " dist: " << knnPts.top().dist << " id:" << knnPts.top().id << endl;
        knnPts.pop();
    }
}

void QuadTreeNode::snapshot() {
    ofstream log("QuadTree.csv", ios_base::app);
    log << this->level << "," << this->data.size() << "," << this->box[XLOW] << ","
        	<< this->box[YLOW] << "," << this->box[XHIGH] << "," << this->box[YHIGH] << endl;
    log.close();

    if (!this->isLeaf()) {
    	for (auto c: children) c->snapshot();
	}
}

void QuadTreeNode::count(int &p, int &d, int &dpc, int &pc) {
    if (this->isLeaf()) {
        p++;
        dpc += this->data.size();
        return;
    } else {
        d++;
        pc += this->children.size();
    }

    for (auto c: children) c->count(p, d, dpc, pc);
}

void QuadTreeNode::calculateSize(int &s){
    s += sizeof(int)  * 2 // height and capacity
         + sizeof(float) * 4; // rectangle
    if (this->isLeaf()) return;
    else s += 4 * 8; // pointer size

    for (auto c: children) c->calculateSize(s);
}

void QuadTreeNode::getTreeHeight(int &h){
    if (this->isLeaf()) {
        if (this->level > h) h = this->level;
        return;
    }

    for (auto c: children) c->getTreeHeight(h);
}

void QuadTreeNode::deleteTree(){
    if ( this->isLeaf() ) {
        for (auto c: children) c->deleteTree();
    }
    delete this;
}

void QuadTreeNode::getStatistics() {
    int size = 0, height = 0, pages = 0, directories = 0, dataPoints = 0, pointers = 0;
    this->calculateSize(size);
    this->getTreeHeight(height);
    this->count(pages, directories, dataPoints, pointers);
    if (POINT_SPLIT) cout << "Strategy: Optimized Point-Quad-Tree" << endl;
    else cout << "Strategy: Point-Region-Quad-Tree" << endl;
    cout << "Capacity: " << capacity << endl;
    cout << "Size in MB: " << size / float(1e6) << endl;
    cout << "Height: " << height << endl;
    cout << "Pages: " << pages << endl;
    cout << "Directories: " << directories << endl;
    cout << "Data points: " << dataPoints << endl;
    cout << "Internal pointers: " << pointers << endl;
    this->snapshot();
}

QuadTreeNode::~QuadTreeNode(){}


