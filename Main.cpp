#include <cstdio>
#include <vector>
#include <algorithm>
#include <cmath>
#include "tools.h"
#include "parameters.h"
#include "stationAssignment.h"
#include "supplyAmount.h"
#include "pathPlanning.h"
#include "multiOpt_TSEA.h"
#include "vehicleWork.h"
#pragma GCC optimize(1)
#pragma GCC optimize(2)
#pragma GCC optimize(3,"Ofast","inline")
using namespace std;


vector<vector<int>> squares = {{20, 8, 2, 500, 1}, {30, 8, 3, 500, 1}, {40, 8, 4, 500, 1}, 
                               {20, 10, 2, 300, 1}, {30, 10, 3, 300, 1}, {40, 10, 4, 300, 1}, 
                               {20, 10, 2, 500, 1}, {30, 10, 3, 500, 1}, {40, 10, 4, 500, 1},  
                               {20, 10, 2, 700, 1}, {30, 10, 3, 700, 1}, {40, 10, 4, 700, 1},
                               {20, 10, 2, 500, 0}, {30, 10, 3, 500, 0}, {40, 10, 4, 500, 0},  
                               {20, 12, 2, 500, 1}, {30, 12, 3, 500, 1}, {40, 12, 4, 500, 1}}; 
vector<vector<int>> flags = {{1, 1}};
vector<int> iterNums = {500};

int main()
{

    for (int sq = 0; sq < squares.size(); sq++) {
        for (int it = 0; it < iterNums.size(); it++) {
            for (int fl = 0; fl < flags.size(); fl++) {
                ////////////////// Control variables
                isBalanced = squares[sq][4];

                length = squares[sq][0], heighth = squares[sq][1];  // Length and height of machine distribution
                carNum = squares[sq][2];
                iterNum = squares[sq][3];
                replanFlag = flags[fl][0];             // 1: dynamic path planning present, 0: no dynamic path planning
                int reAllocationFlag = flags[fl][1];   // 1: dynamic machine assignment present, 0: no dynamic machine assignment


                // 创建文件
                string isB = "unBalanced"; if (isBalanced) isB = "Balanced";
                string file = isB + "_" + to_string(length)+"&"+to_string(heighth) +"&"+ to_string(carNum) + "_" +  to_string(iterNum) + "iters_Plan("+to_string(replanFlag)+")_Alloc("+to_string(reAllocationFlag)+").txt";
                const char* filename = file.c_str();
                FILE *fp = fopen(filename, "w");
                fclose(fp);


                ////////////////// 
                // Variables to save runtime data
                vector<vector<double>> taskChange_eachRun; // Change in completed orders for each run
                vector<double> OTW_eachRun;                // On-the-way time for all vehicles in each run
                vector<double> ULT_eachRun;                // Unloading time for all vehicles in each run
                vector<double> RLT_eachRun;                // Reloading time for all vehicles in each run
                vector<double> taskFinished_eachRun;       // Number of orders completed by all machines in each run
                vector<double> idleTime_eachRun;           // Total idle time of all machines in each run
                vector<double> reAllocCount_eachRun;       // Number of reallocations in each run
                fp = fopen(filename, "w");
                fprintf(fp, "OTW, ULT, RLT, taskFinished, idleTime, reAllocationCount\n");
                fclose(fp);

                // Multiple runs
                for (int run = 0; run < 20; run++)
                {
                    /////////////// initialization ///////////////
                    initObjects(stationNum, carNum, length, heighth);
                    vector<vector<int>> clusters = divideInSection(stations, carNum);
                    for (int i = 0; i < cars.size(); i++) { cars[i].scope = clusters[i]; cars[i].route = genInitRoute(cars[i], stations); }

                    vector<double> taskChange_thisRun;
                    int iter = 1;

                    while (iter <= iterNum)
                    {
                        ///////// Machine material consumption
                        for (int i = 0; i < stations.size(); i++) {

                            if (stations[i].type == 0) task_prob = 0.01;
                            else if (stations[i].type == 1) task_prob = 0.002;
                            else task_prob = 0.001;

                            // Receive new orders according to probability
                            if (randarea(0, 1) < task_prob) {
                                int barNum, blockN;
                                double sigma = 25;
                                switch (stations[i].type) {
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
                                    case 3: barNum = randareaInt(barNum_L, barNum_U); blockN = randareaInt(5, 25);
                                        break;
                                }
                                Task t;
                                t.barNum = barNum;
                                t.blockN_eachBar = blockN;
                                t.barN_eachUnit = randareaInt(1, 3);
                                t.blockNLeft = t.barNum * t.blockN_eachBar;
                                t.blockN_eachUnit = t.barN_eachUnit * t.blockN_eachBar;
                                stations[i].tasks.push_back(t);
                                stations[i].blockNLeft += t.blockNLeft;              
                            }
                            // Raw material consumption for each unit of time
                            if (stations[i].tasks.size() > 0) {
                                int assumption = stations[i].tasks[0].blockN_eachUnit;
                                if (stations[i].blockNAvail >= assumption) { // Only proceed if current available block count is sufficient
                                    stations[i].blockNAvail -= assumption; // Reduce available blocks for the current machine
                                    stations[i].blockNLeft -= assumption; // Reduce pending blocks for the current machine
                                    stations[i].tasks[0].blockNLeft -= assumption; // Reduce pending blocks for the current order being processed
                                    if (stations[i].tasks[0].blockNLeft <= 0) { // If the current order is completed
                                        stations[i].task_finished ++; // Increment completed order count for the machine
                                        stations[i].tasks.erase(stations[i].tasks.begin()); // Remove current order
                                    }
                                }
                                else stations[i].idle_time ++; // Increment idle time for the current machine
                            }
                            
                            stations[i].time ++;
                        }

                        ///////// Work of each vehicle
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

                        // Calculate the burden for each vehicle
                        vector<int> burden_eachCar(cars.size(), 0);
                        for (int c = 0; c < cars.size(); c++) 
                            for (int s = 0; s < cars[c].scope.size(); s++) {
                                int burden_thisMachine = 0;
                                for (int t = 0; t < stations[cars[c].scope[s]].tasks.size(); t++) 
                                    burden_thisMachine += stations[cars[c].scope[s]].tasks[t].blockNLeft;
                                if (burden_thisMachine <= stations[cars[c].scope[s]].blockNAvail) burden_thisMachine = 0;
                                else burden_thisMachine -= stations[cars[c].scope[s]].blockNAvail;
                                burden_eachCar[c] += burden_thisMachine;
                            }

                        ///////// Dynamic Machine Reassigment 
                        if (reAllocationFlag && max(burden_eachCar) - min(burden_eachCar) > double(min(burden_eachCar)) * 0.4) {
                            printf("-------------------- ReAllocation --------------------\n");
                            vector<vector<int>> clusters = divideInSectionByCargo(stations, carNum);
                            for (int i = 0; i < cars.size(); i++) { cars[i].scope = clusters[i]; cars[i].route = genInitRoute(cars[i], stations); }
                            reAllocation_count++;
                        }

                        // Record the total number of completed orders by all machines up to the current time unit
                        int task_finished = 0;
                        for (int i = 0; i < stations.size(); i++) task_finished += stations[i].task_finished;
                        taskChange_thisRun.push_back(task_finished);
                        

                        printf("isBalanced:%d, run:%d, square:%d*%d*%d, iterNum:%d, replan:%d, reAlloc:%d, iter:%d\n", isBalanced, run, length, heighth, carNum, iterNum, replanFlag, reAllocationFlag, iter);

                        iter++;
                    }

                    /////////////// Data Settlement ///////////////
                    taskChange_eachRun.push_back(taskChange_thisRun);
                    // Print some summary information
                    double OTW = 0, ULT = 0, RLT = 0;
                    for (int i = 0; i < cars.size(); i++)
                    {
                        ULT += cars[i].unloadTime;
                        RLT += cars[i].reloadTime;
                        OTW += cars[i].onTheWay;
                    }
                    int taskFinishedNum = 0;
                    int idleTime = 0;
                    for (int i = 0; i < stations.size(); i++) {taskFinishedNum += stations[i].task_finished; idleTime += stations[i].idle_time;}
                    printf("OTW:%.2lf, ULT:%.2lf, RLT:%.2lf, taskFinishedNum:%d, idleTime:%d, reAllocation:%d\n", OTW, ULT, RLT, taskFinishedNum, idleTime, reAllocation_count);
                    printf("back to depot: "); for (int i = 0; i < cars.size(); i++) printf("%d ", cars[i].backCount); printf("\n");
                    // Print information for each vehicle
                    for (int c = 0; c < cars.size(); c++) {
                        int taskFinished_thisCar = 0;
                        for (int s = 0; s < cars[c].scope.size(); s++) taskFinished_thisCar += stations[cars[c].scope[s]].task_finished;
                        printf("car:%d, OTW:%.2f, ULT:%.2f, RLT:%.2f, task finishsed:%d\n", c, cars[c].onTheWay, cars[c].unloadTime, cars[c].reloadTime, taskFinished_thisCar);
                    }

                    /////////////// Data Output ///////////////
                    OTW_eachRun.push_back(double(OTW));
                    ULT_eachRun.push_back(double(ULT));
                    RLT_eachRun.push_back(double(RLT));
                    taskFinished_eachRun.push_back(double(taskFinishedNum));
                    idleTime_eachRun.push_back(double(idleTime));
                    reAllocCount_eachRun.push_back(double(reAllocation_count));

                    fp = fopen(filename, "a+");
                    fprintf(fp, "%lf  %lf  %lf  %d  %d  %d\n", OTW, ULT, RLT, taskFinishedNum, idleTime, reAllocation_count);
                    fclose(fp);
                }
                

                // average results after multiple runs
                vector<double> avr_taskChange;
                for (int i = 0; i < taskChange_eachRun[0].size(); i++) {
                    vector<double> tmp;
                    for (int j = 0; j < taskChange_eachRun.size(); j++) tmp.push_back(taskChange_eachRun[j][i]);
                    avr_taskChange.push_back(average(tmp));
                }
                fp = fopen(filename, "a+");
                fprintf(fp, "average:\n%lf  %lf  %lf  %lf  %lf  %lf\n", 
                        average(OTW_eachRun), average(ULT_eachRun), average(RLT_eachRun), average(taskFinished_eachRun), average(idleTime_eachRun),average(reAllocCount_eachRun));
                for (int i = 0; i < avr_taskChange.size(); i++) fprintf(fp, "%lf ", avr_taskChange[i]);
                fclose(fp);


            }
        }
    }
    

    printf("end");
}
