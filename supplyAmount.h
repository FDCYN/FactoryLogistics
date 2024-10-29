#include <vector>
#include <cmath>
using namespace std;

vector<vector<Task>> supply_eachStation(vector<int> stationIndexs, vector<Station> stations, int carLoad)
{
    vector<Station> stations_tmp; 
    for (int i = 0; i < stationIndexs.size(); i++) stations_tmp.push_back(stations[stationIndexs[i]]);

    // Calculate the maximum ID among all machines
    int maxInd = max(stationIndexs) + 1;

    // Calculate the total number of blocks needed for all orders across all machines
    int allBlock = 0; 
    for (int s = 0; s < stations_tmp.size(); s++) 
        if (stations_tmp[s].blockNAvail < stations_tmp[s].blockNLeft) 
            allBlock += stations_tmp[s].blockNLeft - stations_tmp[s].blockNAvail;
    
    // Calculate how many trips are needed to satisfy all orders for all machines
    int rideNum = allBlock / carLoad;
      
    // Determine which orders remain after using the current available blocks for each machine
    for (int s = 0; s < stations_tmp.size(); s++) 
        while (stations_tmp[s].blockNAvail > 0 && stations_tmp[s].tasks.size() > 0) {
            int blockN = stations_tmp[s].tasks[0].blockNLeft; // Total blocks required for the current order
            if (stations_tmp[s].blockNAvail > blockN) { // If available blocks are enough to fulfill the current order
                stations_tmp[s].blockNAvail -= blockN; // Deduct the required blocks from available blocks
                stations_tmp[s].blockNLeft -= blockN;  // Deduct from the remaining blocks needed for processing
                stations_tmp[s].tasks.erase(stations_tmp[s].tasks.begin()); // Remove the current order
            }
            else { // If available blocks cannot fulfill the current order
                stations_tmp[s].tasks[0].blockNLeft -= stations_tmp[s].blockNAvail; // Use remaining blocks for the current order
                break;
            }
        }        
    
    // Convert all orders from all machines into {machine id, required blocks for the order} format and sort by block count in ascending order
    vector<vector<Task>> supply(maxInd); // Record the orders covered by the supply of each machine
    vector<Station> stations_tmp2 = stations_tmp;
    int finishNum = 0; // Number of machines where all orders have been placed in the sorted list
    for (int s = 0; s < stations_tmp2.size(); s++) 
        if (stations_tmp2[s].tasks.size() <= 0)
            finishNum++;
    
    vector<int> taskLeft(stations_tmp2.size(), 0); // Record the number of remaining orders for each machine after covering with supply
    for (int s = 0; s < stations_tmp2.size(); s++) taskLeft[s] = stations_tmp2[s].tasks.size();
    vector<int> candidation(stations_tmp2.size(), INFINITY); // Record the required block count for the first order of each machine
    int flag = 0; // 1: at least one machine has orders
    for (int s = 0; s < stations_tmp2.size(); s++) 
        if (stations_tmp2[s].tasks.size() > 0) {
            candidation[s] = stations_tmp2[s].tasks[0].blockNLeft;
            flag = 1;
        }
    
    int accumulation = 0; // Total accumulated supply

    if (flag)
        while (finishNum < stations_tmp2.size() && accumulation < 5 * carLoad) {
            int s;
            if (randarea(0, 1) < 0.8)
                s = index(candidation, min(candidation)); // Find the machine with the least required blocks for the first order
            else
                s = index(taskLeft, max(taskLeft));
            if (stations_tmp2[s].tasks.size() > 0) {
                supply[stations_tmp2[s].id].push_back(stations_tmp2[s].tasks[0]); 
                accumulation += stations_tmp2[s].tasks[0].blockNLeft;
                stations_tmp2[s].tasks.erase(stations_tmp2[s].tasks.begin()); // Remove the order
                taskLeft[s]--;
                if (stations_tmp2[s].tasks.size() <= 0) {
                    finishNum++; // If this machine has no more orders, increase the counter
                    candidation[s] = INFINITY; // Set the required blocks for the first order to infinity
                }
                else
                    candidation[s] = stations_tmp2[s].tasks[0].blockNLeft; // Update the required blocks for the first order
            }
        }

    // If any machines have not been allocated supply, allocate the required amount for their first order
    for (int s = 0; s < stations_tmp.size(); s++)
        if (supply[stations_tmp[s].id].size() <= 0 && stations_tmp[s].tasks.size() > 0) {
            supply[stations_tmp[s].id].push_back(stations_tmp[s].tasks[0]);
            accumulation += stations_tmp[s].tasks[0].blockNLeft;
        }
    // printf("accumulation:%d\n", accumulation);

    return supply;
}
