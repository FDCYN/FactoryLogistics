#include <vector>
#include <bits/stdc++.h>
#include <algorithm>
using namespace std;

// Divide the stations evenly into k sections, returning k lists, each containing the station IDs of the respective section
vector<vector<int>> divideInSection(vector<Station> stations, int k)
{
    vector<vector<int>> res;
    for (int i = 0; i < k; i++) res.push_back({});
    int c = 0;
    for (int i = 0; i < stations.size(); i++) {
        res[c].push_back(i);
        if (res[c].size() >= stations.size() / k && c < k) c++;
    }
    return res;
}

// Distribute the stations based on the uniformity principle of shortage levels, aiming for each vehicle to handle a similar shortage level across its assigned stations
vector<vector<int>> divideInSectionByCargo(vector<Station> stations, int carNum)
{
    // Calculate the total shortage level across all stations, then divide by k to find the average shortage level each vehicle should manage
    double avr = 0;
    vector<int> burden_eachMachine; // Record the task load for each machine
    for (int i = 0; i < stations.size(); i++) {
        int burden_thisMachine = 0;
        for (int t = 0; t < stations[i].tasks.size(); t++) // Sum the remaining block counts for all orders of this machine
            burden_thisMachine += stations[i].tasks[t].blockNLeft;
        if (burden_thisMachine <= stations[i].blockNAvail) burden_thisMachine = 0;
        else burden_thisMachine -= stations[i].blockNAvail;
        burden_eachMachine.push_back(burden_thisMachine);
        avr += burden_thisMachine;
    }
    avr /= carNum;

    // Distribute the sections each vehicle is responsible for based on the average shortage level
    vector<vector<int>> res;
    for (int i = 0; i < carNum; i++) res.push_back({});
    double acc = 0;
    int clusterInd = 0;
    for (int i = 0; i < stations.size(); i++)
    {
        res[clusterInd].push_back(i); 
        if (clusterInd == carNum - 1 || acc < avr) 
            acc += burden_eachMachine[i]; 
        else { acc = 0; clusterInd++; }
    }

    // Further equalize the shortage levels between the sections assigned to each vehicle
    vector<double> shortages;
    for (int i = 0; i < carNum; i++) 
    {
        double tmp = 0;
        for (int j = 0; j < res[i].size(); j++) tmp += burden_eachMachine[res[i][j]];
        shortages.push_back(tmp);
    }
    int i = carNum - 1;
    while (i > 0)
    {
        double before = abs(shortages[i] - shortages[i - 1]);
        double after = abs((shortages[i - 1] - burden_eachMachine[res[i-1][res[i-1].size() - 1]]) - 
                          (shortages[i] + burden_eachMachine[res[i-1][res[i-1].size() - 1]])); 
        if (before > after)
        {
            shortages[i - 1] -= burden_eachMachine[res[i-1][res[i-1].size() - 1]];
            shortages[i] += burden_eachMachine[res[i-1][res[i-1].size() - 1]];
            res[i].push_back(res[i - 1][res[i - 1].size() - 1]);
            res[i - 1].pop_back();
        }     
        else
            i--;    
    }
    
    return res;
}
