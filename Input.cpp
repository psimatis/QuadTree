#include "Input.h"

Record::Record() {}

Record::Record(float _id, vector<float> r) {
    id = _id;
    box[XLOW] = box[XHIGH] = r[XLOW];
    box[YLOW] = box[YHIGH] = r[YLOW];
}

Record::Record(char _type, vector<float> r, float _id) {
    type = _type;
    id = _id;
    box[XLOW] = r[XLOW];
    box[YLOW] = r[YLOW];
    box[XHIGH] = r[XHIGH];
    box[YHIGH] = r[YHIGH];
}

bool Record::intersects(Record r) {
    return !(box[XLOW] > r.box[XHIGH] || box[XHIGH] < r.box[XLOW] ||
             box[YLOW] > r.box[YHIGH] || box[YHIGH] < r.box[YLOW]);
}

bool Record::operator<(const Record &b) const {
    if (box[XLOW] != b.box[XLOW])
        return box[XLOW] < b.box[XLOW];
    return box[YLOW] < b.box[YLOW];
}

array<float, 2> Record::toKNNPoint() { return array<float, 2>({box[XLOW], box[YLOW]}); }

Record::~Record() {}

Input::Input() {}

void Input::loadData(const char *filename, int limit) {
    ifstream file(filename);
    if (!file) {
        cerr << "Cannot open the File : " << filename << endl;
        exit(1);
    }
    string line;
    int count = 0;
    while (getline(file, line)) {
        if (count == limit && limit != -1)
            break;
        istringstream buf(line);
        float x, y, id;
        buf >> id >> x >> y;
        emplace_back(id, vector<float>({x, y}));
        count++;
    }
    file.close();
}

void Input::loadQueries(const char *filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Cannot open the File : " << filename << endl;
        exit(1);
    }
    string line;
    while (getline(file, line)) {
        istringstream buf(line);
        char type;
        float xl, yl, xh, yh, id;
        buf >> type;
        if (type == 'r') {
            buf >> xl >> yl >> xh >> yh >> id;
            emplace_back(type, vector<float>({xl, yl, xh, yh}), id);
        } else {
            buf >> xl >> yl >> id;
            emplace_back(type, vector<float>({xl, yl, xl, yl}), id);
        }
    }
    file.close();
}

void Input::sortData() { sort(begin(), end()); }

Input::~Input() {}
