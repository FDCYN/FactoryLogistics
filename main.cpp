#include <cstdio>
#include <vector>
#include <algorithm>
#include "tools.h"
#include "parameters.h"
#include "stationAssignment.h"
#include "pathPlanning.h"
#include "vehicleWork.h"
using namespace std;


int main()
{

    /////////////// initialization ///////////////
    initObjects(stationNum, carNum);
    vector<vector<int>> clusters = divideInSection(stations, carNum);
    for (int i = 0; i < cars.size(); i++) { cars[i].scope = clusters[i]; cars[i].route = genInitRoute(cars[i], stations);}
        

    /////////////// main loop ///////////////
    double allDecay = 0;
    int iter = 1;
    while (iter <= iterNum)
    {
        // 机器材料消耗
        for (int i = 0; i < stations.size(); i++)
        {
            // 按照概率收到新的订单
            if (randarea(0, 1) < task_prob) {
                Task t;
                t.barNum = randareaInt(5, 15);
                t.blockN_eachBar = randareaInt(5, 15);
                t.barN_eachUnit = randareaInt(2, 4);
                t.blockNLeft = t.barNum * t.blockN_eachBar;
                t.blockN_eachUnit = t.barN_eachUnit * t.blockN_eachBar;
                stations[i].tasks.push_back(t);
                stations[i].blockNLeft += t.blockNLeft;              
            }
            // 每个单位的原材料消耗
            int assumption = stations[i].tasks[0].blockN_eachUnit;
            if (stations[i].blockNAvail >= assumption) { // 只有在当前可用木块数足够时进行下面操作
                stations[i].blockNAvail -= assumption; // 当前机器所能用的木块减少
                stations[i].blockNLeft -= assumption; // 当前机器待处理的木块减少
                stations[i].tasks[0].blockNLeft -= assumption; // 当前机器所处理的订单中待处理的木块减少
                if (stations[i].tasks[0].blockNLeft <= 0) { // 若当所处理的订单已完成
                    stations[i].task_finished ++; // 本台机器完成的订单数+1
                    stations[i].tasks.erase(stations[i].tasks.begin()); // 删除当前订单
                }
            }
            else {
                stations[i].idle_time ++; // 当前机器空闲时间增加
            }
            
            stations[i].time ++;
        }
            
        // 每辆车的工作
        for (int i = 0; i < cars.size(); i++)
        {
            double timeLeft = 1;
            while (timeLeft > 0)
            {
                if (cars[i].status == 1 && timeLeft > 0)
                {
                    CarAndTimeLeft ct = toNextStation(cars[i], timeLeft);
                    cars[i] = ct.car; timeLeft = ct.timeLeft;
                }
                
                if (cars[i].status == 2 && timeLeft > 0)
                {
                    CarAndTimeLeft ct = unload(cars[i], timeLeft);
                    cars[i] = ct.car; timeLeft = ct.timeLeft;
                }

                if (cars[i].status == 3 && timeLeft > 0)
                {
                    CarAndTimeLeft ct = backToDepot(cars[i], timeLeft);
                    cars[i] = ct.car; timeLeft = ct.timeLeft;
                }
                
                if (cars[i].status == 4 && timeLeft > 0)
                {
                    CarAndTimeLeft ct = reload(cars[i], timeLeft);
                    cars[i] = ct.car; timeLeft = ct.timeLeft;
                }
            }
        }

        // 机器重分配
        vector<double> shortageEachCar;
        for (int i = 0; i < cars.size(); i++)
        {
            double tmp = 0;
            for (int j = 0; j < cars[i].scope.size(); j++) tmp += 100 - stations[cars[i].scope[j]].cargo;
            shortageEachCar.push_back(tmp);
        }
        if (max(shortageEachCar) - min(shortageEachCar) > 400)
        {
            vector<vector<int>> clusters = divideInSectionByCargo(stations, carNum);
            for (int i = 0; i < cars.size(); i++) { cars[i].scope = clusters[i]; cars[i].route = genInitRoute(cars[i], stations); }
        }

        // 更新一些信息
        double allCargo = 0;
        for (int i = 0; i < stations.size(); i++)
        {
            stations[i].wait += (100 - stations[i].cargo) / 100;
            allCargo += stations[i].cargo;
        }
        iter++;

    }
    /////////////// 数据结算 ///////////////
    double wait = 0;
    for (int i = 0; i < stations.size(); i++) wait += stations[i].wait;
    double OTW = 0, ULT = 0, RLT = 0;
    for (int i = 0; i < cars.size(); i++)
    {
        ULT += cars[i].unloadTime;
        RLT += cars[i].reloadTime;
        OTW += cars[i].onTheWay;
    }
    
    printf("wait:%.2lf, OTW:%.2lf, alldecay:%.2lf, ULT:%.2lf, RLT:%.2lf\n", wait, OTW, allDecay/unloadSpeed, ULT, RLT);

    
}

