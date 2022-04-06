#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <chrono>
using namespace std;

struct FP_Node {
    FP_Node() {
        father = nullptr;
        bros = nullptr;
        frequency = 1;
    }
    FP_Node(int item_num) {
        this->item_num = item_num;
        frequency = 1;
        father = nullptr;
        bros = nullptr;
    }
    int item_num;
    int frequency;
    vector<FP_Node*> next;
    FP_Node* father;
    FP_Node* bros;
};

struct Header_node {
    Header_node(int item_num, int frequency) {
        this->item_num = item_num;
        this->frequency = frequency;
        this->head = nullptr;
    }
    int item_num;
    int frequency;
    FP_Node* head;
};

double Minimum_confidence = 0.001;
int MINIMUM;    // 最小置信度下每条记录需要大于的个数
int cnt = 0;    // 频繁项集的个数
unordered_map<int, int> mp; // 计算每个数据出现的次数

bool cmp(Header_node* p1, Header_node* p2) {
    if (p1->frequency == p2->frequency) {
        return p1->item_num < p2->item_num;
    }
    return p1->frequency > p2->frequency;
}

bool cmp1 (int a1, int a2) {
    if (mp[a1] > mp[a2]) {
        return true;
    }
    return false;
}

int print(vector<vector<int> > a) {
    for (int i = 0; i < (int)a.size(); i++) {
        for (int j = 0; j < a[i].size(); j++) {
            cout << a[i][j] << ' ';
        }
        cout << endl;
    }
    return 0;
}

vector<vector<int> > generate_patterns(FP_Node* FP_tree, int item_num, unordered_map<int, FP_Node*> entre) {
    vector<vector<int> > patterns;  // 条件模式基
    FP_Node* it = entre[item_num];
    while (it->bros != nullptr) {
        it = it->bros;
        vector<int> path;
        FP_Node *up = it;
        while (up->father != FP_tree) {
            up = up->father;
            path.push_back(up->item_num);
        }
        path.push_back(-1 * it->frequency); // 最后一个数字用复数来表示该条记录出现了几次
        patterns.push_back(path);
    }
    return patterns;
}

int fp_growth(vector<vector<int> >data) {
    // 根据数据库构建头表和FP_Tree
    FP_Node* FP_Tree = new FP_Node();
    FP_Tree->item_num = -1;
    FP_Tree->father = nullptr;
    vector<Header_node*> Header_Table;
    unordered_map<int, FP_Node*> entre; // 头表的head节点入口
    mp.clear(); // 清空hashmap为递归做准备
    for (int i = 0; i < data.size(); i++) {
        int freq = 1;   // 每次需要加几个
        int to_iter = data[i].size();
        if (data[i].back() < 0) {   // 如果这个数据库最后是有标识的
            freq = -1 * data[i].back();
            to_iter -= 1;   // 最后一个次数不做扫描
        }
        for (int j = 0; j < to_iter; j++) {
            mp[data[i][j]] += freq;
        }
    }
    for (unordered_map<int, int>::iterator it = mp.begin(); it != mp.end(); it++) {
        if (it->second >= MINIMUM) {
            Header_node* line = new Header_node(it->first, it->second);
            line->head = new FP_Node();
            entre[it->first] = line->head;
            Header_Table.push_back(line);
        }
    }   // 构建头表
    sort(Header_Table.begin(), Header_Table.end(), cmp);
    bool single_path = true;    // 该树是否是单一路径的呢
    for (int i = 0; i < (int)data.size(); i++) {
        vector<int> temp;
        FP_Node* p = FP_Tree;
        int to_add = 1;
        if (data[i].back() < 0) {   // 获取要加的次数
            to_add = -1 * data[i].back();
            data[i].pop_back();
        }
        for (int j = 0; j < (int)data[i].size(); j++) {
            if (mp[data[i][j]] >= MINIMUM) {    // 去除非频繁的元素
                temp.push_back(data[i][j]);
            }
        }
        sort(temp.begin(), temp.end(), cmp1);   // 按出现频率排序
        for (int i = 0; i < (int)temp.size(); i++) {
            bool found = false;
            for (int j = 0; j < (int)p->next.size(); j++) { // 在下一步中寻找那个元素
                if (p->next[j]->item_num == temp[i]) {
                    found = true;
                    p = p->next[j];
                    break;
                }
            }
            if (found) {
                p->frequency += to_add;
                continue;   //结束了
            }
            // 没找到
            FP_Node* it = entre[temp[i]];
            while (it->bros != nullptr) {
                it = it->bros;
            }
            FP_Node* t = new FP_Node(temp[i]);
            t->frequency = to_add;
            t->father = p;
            if (p->next.size() > 0) {   // next节点中有分枝，不是单一路径的树
                single_path = false;
            }
            p->next.push_back(t);
            it->bros = t;
            p = t;
        }
    }
    int n = Header_Table.size();
    if (single_path) {  // 只含有一条路径
        cnt += pow(2, n) - 1;
        return 0;
    }
    for (int i = n - 1; i >= 0; i--) {  // 逆序遍历头表，生成条件模式基，递归
        cnt += 1;
        vector<vector<int> > patterns = generate_patterns(FP_Tree, Header_Table[i]->item_num, entre);
        fp_growth(patterns); 
    }
    return 0;
}

int main() {
    auto start = std::chrono::steady_clock::now();
    ifstream ifs("retail.dat");
    if (!ifs) {
        cout << "Can't open the file." << endl;
        return 0;
    }
    string line;
    int line_num = 0;
    vector<vector<int> > data;  // 存放初始数据
    while (getline(ifs, line)) {    // 处理文件输入
        int last = 0;
        vector<int> temp;
        line_num += 1;
        for (int i = 0; i <= (int)line.size(); i++) {
            if (*(line.end() - 1)== ' ') {
                line.pop_back();
            }
            if (i == line.size() || line[i] == ' ') {
                int num = stoi(line.substr(last, i - last));
                temp.push_back(num);
                mp[num] += 1;
                last = i + 1;
            }
        }
        data.push_back(temp);
    }
    MINIMUM = ceil(line_num * Minimum_confidence);
    fp_growth(data);
    cout << cnt << endl;
    ifs.close();
    cout << "finished" << endl;
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::micro> elapsed = end - start; // std::micro 表示以微秒为时间单位
    std::cout<< "time: "  << elapsed.count() << "us" << std::endl;
    return 0;
}
