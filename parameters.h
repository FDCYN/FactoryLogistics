#include <vector>
#include <cmath>
using namespace std;

///////////////////////////////////////////// 参数 /////////////////////////////////////////////

// object definitions
struct Task
{
    int barNum; // Number of bars
    int blockN_eachBar; // Number of blocks required for each bar
    int barN_eachUnit; // Number of bars that can be processed per unit time
    int blockNLeft; // Current order's number of blocks to be processed
    int blockN_eachUnit; // Number of blocks that can be processed per unit time
};

class Station
{
    public:
        int id;
        vector<double> pos;
        double time = 0;
        vector<Task> tasks;
        int blockNAvail; // Number of blocks currently available for the machine
        int blockNLeft; // Number of blocks to be assembled from all current orders in the machine
        int type; // 0: mainly accepts small orders, 1: mainly accepts medium orders, 2: mainly accepts large orders, 3: uniformly accepts any orders
        
        // Some parameters for evaluating algorithm performance
        int task_finished = 0;
        double idle_time = 0; // Idle time
        int visitCount = 0; // Number of visits
};
vector<Station> stations;

class Car
{
    public:
        double cargo;           // Current cargo weight of the car
        vector<double> pos;     // Current position of the car
        vector<int> route;      // Remaining path arrangements for the car
        int status;             // status: 1 means on the way, 2 means unloading, 3 means returning, 4 means loading
        int stopStation = -1;   // ID of the current stop station for the vehicle
        vector<int> scope;      // Indices of the stations that the car is responsible for in 'stations'
        double timeLeft;        // Remaining time for the vehicle's current status
        // Some parameters for evaluating algorithm performance
        int backCount = 0;      // Number of returns to the warehouse
        double onTheWay = 0;    // Time the vehicle has been on the road
        double unloadTime = 0;  // Total unloading time of the vehicle
        double reloadTime = 0;  // Total loading time of the vehicle
};
vector<Car> cars;
vector<double> depot;

// parameter settings
int carNum = 3;
int length = 30, heighth = 10; // Length and height of the rectangular layout of all machines
// int stationNumEachEdge = 10; // Each vehicle is responsible for a square array of machines, this is the length of the square's side
int stationNum = -1;
int iterNum = 500;
double carSpeed = 3;
double unloadT_eachBlock = 0.0001, reloadT_eachBlock = 0.0001; // Time taken to load (unload) each block
double carLoad = 50000;
double task_prob = 0.05; // Probability of each machine receiving an order per unit time

int barNum_L = 50, barNum_U = 250; // Upper and lower limits for the number of bars in each order

int isBalanced = 0;

// Parameters used in the algorithm
vector<vector<double>> DM;  // Distance matrix
vector<vector<Task>> supply_eachMachine; // Orders covered by the supply quantity of each machine
vector<int> supplyAmount_eachMachine; // Supply quantity of each machine
int replan_count;
int reAllocation_count;

///////////////////////////////////////////// Functions /////////////////////////////////////////////
// Build Distance Matrix
vector<vector<double>> Build_Distance_Matrix(vector<Station> stations)
{
    vector<vector<double>> res;
    // Allocate space
    for (int i = 0; i < stations.size(); i++)
    {
        vector<double> tmp(stations.size());
        res.push_back(tmp);
    }
    // Assign values
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
void initObjects(int stationNum, int carNum, int length, int height)
{
    stations.clear(); cars.clear(); DM.clear(); replan_count = 0; reAllocation_count = 0;

    stationNum = length * height;
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
        s.pos = {i / height + 1.0, double(height) - 1 - i % height};
        // s.type = randareaInt(0, 2);

        // Group machines of the same type together
        if (i % (stationNum / carNum) < stationNum / carNum * 0.33) s.type = 0;
        else if (i % (stationNum / carNum) < stationNum / carNum * 0.66) s.type = 1;
        else s.type = 2;

        // With this line, all machines become the fourth type
        if (isBalanced)
            s.type = 3;

        // Set all machines of the first vehicle to accept small orders
        if (i < stationNum / carNum) s.type = 0;

        // printf("%d, %d\n", i, s.type);

        int taskNum;
        switch (s.type) {
            case 0: taskNum = 8; break;
            case 1: taskNum = 6; break;
            case 2: taskNum = 4; break;
        } 

        for (int j = 0; j < taskNum; j++) {
            // Generate the number of bars and blocks in each order based on the machine type
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
                case 3: barNum = randareaInt(barNum_L, barNum_U); blockN = randareaInt(5, 25);
                    break;
            }
            
            
            // Generate specific orders based on the type
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
    depot = {double(length / 2), -1};

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