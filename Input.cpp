#include "Input.h"

Record::Record(){}

Record::Record(float id, vector<float>d){
    this->id = id;
    this->box[XLOW] = this->box[XHIGH] = d[XLOW];
    this->box[YLOW] = this->box[YHIGH] = d[YLOW];
}

Record::Record(char type, vector<float> q, float id){
    this->type = type;
    this->id = id;
    this->box[XLOW] = q[XLOW];
    this->box[YLOW] = q[YLOW];
    this->box[XHIGH] = q[XHIGH];
    this->box[YHIGH] = q[YHIGH];
}

bool Record::inBoundary(Record r){
    return !(this->box[XLOW] > r.box[XHIGH] || this->box[XHIGH] < r.box[XLOW]
             || this->box[YLOW] > r.box[YHIGH] || this->box[YHIGH] < r.box[YLOW]);
}

array<float, 2> Record::toKNNPoint(){
    return array<float, 2>({box[XLOW], box[YLOW]});
}

Record::~Record(){}

Input::Input(){}

void Input::loadData(const char *filename, int limit){
    ifstream file(filename);
    if(!file){
        cerr << "Cannot open the File : " << filename << endl;
        exit(1);
    }
    string line;
    int count = 0;
    while (getline( file, line ) ){
        istringstream buf(line);
        float x, y, id;
        buf >> id >> x >> y;
        this->emplace_back(id, vector<float>({x,y}));
        if (count == limit && limit != -1) break;
        count++;
    }
    file.close();
}

void Input::loadQueries(const char *filename){
    ifstream file( filename );
    if(!file){
        cerr << "Cannot open the File : " << filename << endl;
        exit(1);
    }
    string line;
    while (getline(file, line)){
        istringstream buf(line);
        char type;
        float xl, yl, xh, yh, id;
        buf >> type;
        if (type == 'r') {
            buf >> xl >> yl >> xh >> yh >> id;
            this->emplace_back(type, vector<float>({xl, yl, xh, yh}), id);
        }
        else{
            buf >> xl >> yl >> id;
            this->emplace_back(type, vector<float>({xl, yl, xl, yl}), id);
        }
    }
    file.close();
}

bool Record::operator < (const Record& b) const {
    if (this->box[XLOW] != b.box[XLOW]) return this->box[XLOW] < b.box[XLOW];
    return this->box[YLOW] < b.box[YLOW];
}

void Input::sortData(){
    sort(this->begin(), this->end());
}

Input::~Input(){}
