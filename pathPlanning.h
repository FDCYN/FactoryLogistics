#include <vector>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;

vector<int> modifyRoute(vector<int> route);
vector<int> shortestRoute(vector<int> route);

// Specific Manhattan distance for this project
double Manhattan_dist(vector<double> pos1, vector<double> pos2) {
    double res = abs(pos1[1] - pos2[1]);
    if (abs(pos1[0] - pos2[0]) >= 0.5) res += abs(pos1[0] - pos2[0]);
    else res += 1 - abs(pos1[0] - pos2[0]);
    return res;
}

// Calculate the length of the route
double lenOfRoute(vector<int> route) { // Calculate the length of the path
    double res = 0;
    for (int i = 0; i < route.size() - 1; i++) res += DM[stations[route[i]].id][stations[route[i+1]].id]; 
    return res;
}

/////////////////////////// Generate Initial Route ///////////////////////////
vector<int> genInitRoute(Car car, vector<Station> stations)
{
    vector<int> route;
    vector<int> tmp = car.scope;

    // Find the closest station to the car among the stations it is responsible for
    int minInd = 0;
    double minDist = Manhattan_dist(stations[tmp[minInd]].pos, car.pos);
    for (int i = 1; i < car.scope.size(); i++)
    {
        double newDist = Manhattan_dist(stations[tmp[i]].pos, car.pos);
        if (newDist < minDist) { minInd = i; minDist = newDist; }
    }
    route.push_back(tmp[minInd]);
    
    // Randomly arrange the other points in route except for the closest one to the depot
    tmp.erase(tmp.begin() + minInd);
    random_shuffle(tmp.begin(), tmp.end());
    route.insert(route.end(), tmp.begin(), tmp.end());
    route.push_back(route[0]); // Form a loop

    // // Optimize the current path using HH
    // route = shortestRoute(route);
    
    return route;
}

// /////////////////////////// Optimize Path Using Pareto Front ///////////////////////////
bool sortByLength_Exp1(const tuple<vector<int>, double, double>& a, const tuple<vector<int>, double, double>& b) {
    return (get<1>(a) < get<1>(b));
}
bool sortByPriority_Exp1(const tuple<vector<int>, double, double>& a, const tuple<vector<int>, double, double>& b) {
    return (get<2>(a) < get<2>(b));
}

// Calculate the objective function value of the path according to priority
// Note: The default path is a loop (i.e., the first and last elements of route are the same)
double visitOfRoute_Exp1(vector<int> route, vector<double> supply_normalized, vector<double> blockNAvail_normalized) 
{
    double res = 0;
    for (int i = 1; i < route.size() - 1; i++) 
        res += i * ((1 - supply_normalized[route[i]]) + 1 * (1 - blockNAvail_normalized[route[i]])); 
    return res;
}
