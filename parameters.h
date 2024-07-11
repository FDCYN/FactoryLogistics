#include <vector>
#include <cmath>
using namespace std;

///////////////////////////////////////////// 参数 /////////////////////////////////////////////

// object definitions
struct Task
{
    int barNum; // 木条数
    int blockN_eachBar; // 每个木条所需木块数
    int barN_eachUnit; // 每个单位时间所能处理的木条数
    int blockNLeft; // 当前订单待处理的木块数量
    int blockN_eachUnit; // 每个单位时间所能处理的木块数
};

class Station
{
    public:
        int id;
        vector<double> pos;
        double time = 0;
        vector<Task> tasks;
        int blockNAvail; // 目前机器可使用的木块数量
        int blockNLeft; // 当前机器所有订单中，待拼接的木块数量
        int type; // 0: 主要接受小型订单，1：主要接受中型订单，2：主要接受大型订单
        
        // 一些用于评价算法性能的参数
        int task_finished = 0;
        double idle_time = 0; // 空闲时间
        int visitCount = 0; // 被访问次数
};
vector<Station> stations;

class Car
{
    public:
        double cargo;           // 当前小车载货量
        vector<double> pos;     // 当前小车的位置
        vector<int> route;      // 当前小车剩余的路径安排
        int status;             // status:1表示正在路上, 2表示正在卸货, 3表示正在返程, 4表示正在装货
        int stopStation = -1;   // 当前车辆停靠的站点id
        vector<int> scope;      // 该车负责的站点在stations中的下标
        double timeLeft;        // 车辆当前状态的剩余时间
        // 一些用于评价算法性能的参数
        int backCount = 0;      // 回仓次数
        double onTheWay = 0;    // 车辆在路上的时间
        double unloadTime = 0;  // 车辆总共卸货时间
        double reloadTime = 0;  // 车辆总共装货时间
};
vector<Car> cars;
vector<double> depot;

// parameter settings
int carNum = 3;
int stationNumEachEdge = 10; // 每辆车负责的机器阵列为正方形，此为正方形的边长
int stationNum = -1;
int iterNum = 500;
double carSpeed = 5;
double unloadT_eachBlock = 0.0001, reloadT_eachBlock = 0.0001; // 装货和卸货时，每装（卸）一个木块所花单位时间
double carLoad = 50000;
double task_prob = 0.05; // 每个单位时间，每台机器收到订单的概率

int barNum_L = 50, barNum_U = 250; // 每个订单木条数上下限

// 算法中要用的参数
vector<vector<double>> DM;  // 距离矩阵
vector<vector<Task>> supply_eachMachine; // 各个机器供给量覆盖的订单
vector<int> supplyAmount_eachMachine; // 各个机器供给量
int replan_count;

///////////////////////////////////////////// 函数 /////////////////////////////////////////////
// Build Distance Matrix
vector<vector<double>> Build_Distance_Matrix(vector<Station> stations)
{
    vector<vector<double>> res;
    // 开辟空间
    for (int i = 0; i < stations.size(); i++)
    {
        vector<double> tmp(stations.size());
        res.push_back(tmp);
    }
    // 赋值
    for (int i = 0; i < stations.size(); i++)
    {
        for (int j = i; j < stations.size(); j++)
            if (i == j) {res[stations[i].id][stations[j].id] = INFINITY; res[stations[j].id][stations[i].id] = INFINITY;}
            else {
                res[stations[i].id][stations[j].id] = abs(stations[i].pos[1] - stations[j].pos[1]);
                if (stations[i].pos[0] == stations[j].pos[0]) res[stations[i].id][stations[j].id] += 1;
                else res[stations[i].id][stations[j].id] += abs(stations[i].pos[0] - stations[j].pos[0]);;

                res[stations[j].id][stations[i].id] = res[stations[i].id][stations[j].id];
            }
    }
    return res;
}

// initialization of parameters
void initObjects(int stationNum, int carNum)
{
    stations.clear(); cars.clear(); DM.clear(); replan_count = 0;

    stationNum = stationNumEachEdge * stationNumEachEdge * carNum;
    for (int i = 0; i < stationNum; i++) {
        supply_eachMachine.push_back({});
        supplyAmount_eachMachine.push_back(-1);
    }

    // stations
    for (int i = 0; i < stationNum; i++)
    {
        Station s;
        s.id = i;
        // s.blockNAvail = randareaInt(0, 500);
        s.blockNAvail = 0;
        s.blockNLeft = 0;
        s.pos = {i / stationNumEachEdge + 1.0, double(stationNumEachEdge) - 1 - i % stationNumEachEdge};
        // s.type = randareaInt(0, 2);

        // 将同一类型的机器放在一起
        if (i % (stationNum / carNum) < stationNum / carNum * 0.33) s.type = 0;
        else if (i % (stationNum / carNum) < stationNum / carNum * 0.66) s.type = 1;
        else s.type = 2;

        // printf("%d, %d\n", i, s.type);

        // 每台机器初始时指派初始订单
        for (int j = 0; j < 10; j++) {
            // 根据机器的类型生成订单的木条数量和木条中的木块数
            int barNum, blockN;
            double sigma = 25;
            switch (s.type) {
                case 0: barNum = round(norm(barNum_L, sigma)); blockN = round(norm(5, sigma));
                    if (barNum < barNum_L) barNum = barNum_L + (barNum_L - barNum); if (barNum > barNum_U) barNum = barNum_U;
                    if (blockN < 5) blockN = 5 + (5 - blockN); if (blockN > 25) blockN = 25;
                    break;
                case 1: barNum = round(norm((barNum_L + barNum_U) / 2, sigma)); blockN = round(norm(15, sigma));
                    if (barNum < barNum_L) barNum = barNum_L; if (barNum > barNum_U) barNum = barNum_U;
                    if (blockN < 5) blockN = 5; if (blockN > 25) blockN = 25;
                    break;
                case 2: barNum = round(norm(barNum_U, sigma)); blockN = round(norm(25, sigma));
                    if (barNum > barNum_U) barNum = barNum_U - (barNum - barNum_U); if (barNum < barNum_L) barNum = barNum_L;
                    if (blockN > 25) blockN = 25 - (blockN - 25); if (blockN < 5) blockN = 5;
                    break;
            }
            
            // 根据类型生成具体订单
            Task t;
            t.barNum = barNum;
            t.blockN_eachBar = blockN;
            t.barN_eachUnit = randareaInt(1, 3);
            t.blockNLeft = t.barNum * t.blockN_eachBar;
            t.blockN_eachUnit = t.barN_eachUnit * t.blockN_eachBar;
            s.tasks.push_back(t);
            s.blockNLeft += t.blockNLeft;
        }     
        
        stations.push_back(s);
    }
    // depot
    depot = {double(carNum * stationNumEachEdge / 2), -1};

    // vehicles
    for (int i = 0; i < carNum; i++)
    {
        Car c;
        c.pos = depot;
        c.cargo = carLoad;
        c.status = 1;
        cars.push_back(c);
    }
    // distance matrix
    DM = Build_Distance_Matrix(stations);
}
