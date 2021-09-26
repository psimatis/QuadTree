#include "QuadTree.h"

#define all(c) c.begin(), c.end()
#define dist(x1, y1, x2, y2) (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)

QuadTreeNode::QuadTreeNode(vector<float> _boundary, int _level) {
    box = _boundary;
    level = _level;
}

bool QuadTreeNode::isLeaf() {
    if (children[NW] == NULL)
        return true;
    return false;
}

void QuadTreeNode::packing(Input &R) {
    for (auto r : R)
        data.push_back(r);
    packing();
}

void QuadTreeNode::packing() {
    if (data.size() > CAPACITY) {
        if (isLeaf())
            divide();
        for (auto r : data) {
            for (auto c : children) {
                if (c->intersects(r)) {
                    c->data.push_back(r);
                    break;
                }
            }
        }
        data.clear();
        for (auto c : children)
            c->packing();
    }
}

void QuadTreeNode::insert(Record r) {
    if (isLeaf()) {
        data.push_back(r);
        if (data.size() > CAPACITY) {
            divide();
            for (auto rec : data) {
                auto c = children.begin();
                while (!(*c)->intersects(rec))
                    c++;
                (*c)->data.push_back(rec);
            }
            data.clear();
        }
    } else {
        auto c = children.begin();
        while (!(*c)->intersects(r))
            c++;
        (*c)->insert(r);
    }
}

void QuadTreeNode::divide() {

    float xMid, yMid;
    if (POINT_SPLIT) {
        data.sortData();
        auto median = data[data.size() / 2];
        xMid = median.box[XLOW];
        yMid = median.box[YLOW];
    } else {
        xMid = (box[XHIGH] + box[XLOW]) / 2.0;
        yMid = (box[YHIGH] + box[YLOW]) / 2.0;
    }

    vector<float> northWest = {box[XLOW], yMid, xMid, box[YHIGH]};
    children[NW] = new QuadTreeNode(northWest, level + 1);

    vector<float> northEast = {xMid, yMid, box[XHIGH], box[YHIGH]};
    children[NE] = new QuadTreeNode(northEast, level + 1);

    vector<float> southWest = {box[XLOW], box[YLOW], xMid, yMid};
    children[SW] = new QuadTreeNode(southWest, level + 1);

    vector<float> southEast = {xMid, box[YLOW], box[XHIGH], yMid};
    children[SE] = new QuadTreeNode(southEast, level + 1);
}

bool QuadTreeNode::intersects(Record q) {
    return !(box[XLOW] > q.box[XHIGH] || q.box[XLOW] > box[XHIGH] || box[YLOW] > q.box[YHIGH] ||
             q.box[YLOW] > box[YHIGH]);
}

void QuadTreeNode::rangeQuery(Record q, vector<float> &resultItemsIds, map<string, double> &stats) {
    if (isLeaf()) {
        if (intersects(q)) {
            stats["leaf"]++;
            for (auto r : data) {
                if (q.intersects(r)) {
                    //cout << r.id << " " << r.box[XLOW] << " " << r.box[YLOW] << endl;
                    resultItemsIds.push_back(r.id);
                }
            }
        }
        return;
    } else {
        stats["directory"]++;
        for (auto c : children) {
            if (c->intersects(q))
                c->rangeQuery(q, resultItemsIds, stats);
        }
    }
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
        if (left)
            return dist(b[XLOW], b[YHIGH], r[2], r[1]);
        if (right)
            return dist(b[XHIGH], b[YHIGH], r[0], r[1]);
        return (r[1] - b[YHIGH]) * (r[1] - b[YHIGH]);
    }
    if (bottom) {
        if (left)
            return dist(b[XLOW], b[YLOW], r[2], r[3]);
        if (right)
            return dist(b[XHIGH], b[YLOW], r[0], r[3]);
        return (b[YLOW] - r[3]) * (b[YLOW] - r[3]);
    }
    if (left)
        return (b[XLOW] - r[2]) * (b[XLOW] - r[2]);
    if (right)
        return (r[0] - b[XHIGH]) * (r[0] - b[XHIGH]);
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
    unseenNodes.emplace(knnNode{this, minSqrDist(query)});
    double dist, minDist;
    QuadTreeNode *node;

    while (!unseenNodes.empty()) {
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnPts.top().dist;
        if (dist < minDist) {
            if (node->isLeaf()) {
                for (auto p : node->data) {
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
                for (auto c : node->children) {
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
        } else
            break;
    }
	// prints the results
    /*while (!knnPts.empty()) {
        cout << knnPts.top().pt[0] << " " << knnPts.top().pt[1] << " dist: " << knnPts.top().dist
             << " id:" << knnPts.top().id << endl;
        knnPts.pop();
    }*/
}

void QuadTreeNode::snapshot() {
    ofstream log("QuadTree.csv", ios_base::app);
    log << level << "," << isLeaf() << "," << data.size() << "," << box[XLOW] << "," << box[YLOW]
        << "," << box[XHIGH] << "," << box[YHIGH] << endl;
    log.close();

    if (!isLeaf()) {
        for (auto c : children)
            c->snapshot();
    }
}

void QuadTreeNode::count(int &p, int &d, int &dpc, int &pc) {
    if (isLeaf()) {
        p++;
        dpc += data.size();
        return;
    }
    d++;
    pc += children.size();

    for (auto c : children)
        c->count(p, d, dpc, pc);
}

void QuadTreeNode::calculateSize(int &s) {
    s += sizeof(int) + sizeof(float) * 4; // height  and rectangle
    if (isLeaf())
        return;
    else
        s += 4 * 8; // pointer size

    for (auto c : children)
        c->calculateSize(s);
}

void QuadTreeNode::getTreeHeight(int &h) {
    if (isLeaf()) {
        if (level > h)
            h = level;
        return;
    }

    for (auto c : children)
        c->getTreeHeight(h);
}

void QuadTreeNode::deleteTree() {
    if (isLeaf()) {
        for (auto c : children)
            c->deleteTree();
    }
    delete this;
}

void QuadTreeNode::getStatistics() {
    int size = 0, height = 0, pages = 0, directories = 0, dataPoints = 0, pointers = 0;
    calculateSize(size);
    getTreeHeight(height);
    count(pages, directories, dataPoints, pointers);
    if (POINT_SPLIT)
        cout << "Strategy: Optimized Point-Quad-Tree" << endl;
    else
        cout << "Strategy: Point-Region-Quad-Tree" << endl;
    cout << "Capacity: " << CAPACITY << endl;
    cout << "Size in MB: " << size / float(1e6) << endl;
    cout << "Height: " << height << endl;
    cout << "Pages: " << pages << endl;
    cout << "Directories: " << directories << endl;
    cout << "Data points: " << dataPoints << endl;
    cout << "Internal pointers: " << pointers << endl;
    // snapshot();
}

QuadTreeNode::~QuadTreeNode() {}
