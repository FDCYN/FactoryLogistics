#include <vector>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;

// Calculate the length of the route
double lenOfRoute_TSEA(vector<int> route) { // Calculate the length of the route
    double res = 0; for (int i = 0; i < route.size() - 1; i++) res += DM[stations[route[i]].id][stations[route[i+1]].id]; 
    return res;
}
// Calculate the objective function value of the route based on priority. Note: The default route is a loop (i.e., the first and last elements in the route are the same)
double visitOfRoute_TSEA(vector<int> route, vector<double> supply_normalized, vector<double> blockNAvail_normalized) {
    double res = 0; for (int i = 1; i < route.size() - 1; i++) res += i * ((1 - supply_normalized[route[i]]) + 1 * (1 - blockNAvail_normalized[route[i]])); 
    return res;
}

// The default input is a loop
// sol1 is the current individual, sol2 is the best solution
vector<int> Guided_Crossover(vector<int> sol1, vector<int> sol2)
{
    vector<int> sol1_tmp(sol1.begin(), sol1.end() - 1);
    vector<int> sol2_tmp(sol2.begin(), sol2.end() - 1);
    int cross = randareaInt(0, sol1_tmp.size() - 1); // Randomly generate the crossover point

    vector<int> offspring(sol1_tmp.begin(), sol1_tmp.begin() + cross + 1); // Generate offspring

    vector<int> retain(max(sol1_tmp) + 1, 0); // Record the elements retained from sol1_tmp
    for (int i = 0; i <= cross; i++) retain[sol1_tmp[i]] = 1;

    // Save fragments formed by removing retained elements from sol2_tmp
    vector<vector<int>> subparts;
    vector<int> subpart; 
    for (int i = 0; i < sol2_tmp.size(); i++) {
        if (retain[sol2_tmp[i]] != 1) subpart.push_back(sol2_tmp[i]); // Gather continuous elements to form fragments
        else {
            if (subpart.size() > 0) subparts.push_back(subpart); // Add the fragment to the collection
            subpart.clear(); // Clear the fragment to prepare for the next one
        }
    }
    if (subpart.size() > 0) subparts.push_back(subpart);

    // Insert fragments into offspring according to the nearest principle
    while (subparts.size() > 0) {
        int index; // Record the index of the fragment closest to the end of offspring
        double minDist = INFINITY; // Record the minimum distance
        int flag; // Record whether the head (0) or the tail (1) of the fragment is closest to the end

        // Find the fragment closest to the end of offspring and determine whether the head or tail is closer
        for (int i = 0; i < subparts.size(); i++) {
            if (DM[offspring[offspring.size() - 1]][subparts[i][0]] < minDist) 
                {flag = 0; minDist = DM[offspring[offspring.size() - 1]][subparts[i][0]]; index = i;}
            if (subparts[i].size() > 1 && DM[offspring[offspring.size() - 1]][subparts[i][subparts[i].size() - 1]] < minDist)
                {flag = 1; minDist = DM[offspring[offspring.size() - 1]][subparts[i][subparts[i].size() - 1]]; index = i;}
        }
        
        if (flag == 1) reverse(subparts[index].begin(), subparts[index].end());

        offspring.insert(offspring.end(), subparts[index].begin(), subparts[index].end());

        subparts.erase(subparts.begin() + index);
    }   

    // Convert offspring into a loop
    offspring.push_back(offspring[0]);
    
    return offspring;
}

// Crossover operator based on Guided_Crossover (default input is a loop)
// Its purpose is to generate more paths that meet the priority
vector<int> Guided_Crossover_2(vector<int> sol1, vector<int> sol2, vector<double> supply_normalized, vector<double> leftTime_normalized)
{
    vector<int> sol1_tmp(sol1.begin(), sol1.end() - 1);
    vector<int> sol2_tmp(sol2.begin(), sol2.end() - 1);
    int cross = randareaInt(0, sol1_tmp.size() - 1); // Randomly generate the crossover point

    vector<int> offspring(sol1_tmp.begin(), sol1_tmp.begin() + cross + 1); // Generate offspring

    vector<int> retain(max(sol1_tmp) + 1, 0); // Record the elements retained from sol1_tmp
    for (int i = 0; i <= cross; i++) retain[sol1_tmp[i]] = 1;

    // Save fragments formed by removing retained elements from sol2_tmp
    vector<vector<int>> subparts;
    vector<int> subpart; 
    for (int i = 0; i < sol2_tmp.size(); i++) {
        if (retain[sol2_tmp[i]] != 1) subpart.push_back(sol2_tmp[i]); // Gather continuous elements to form fragments
        else {
            if (subpart.size() > 0) subparts.push_back(subpart); // Add the fragment to the collection
            subpart.clear(); // Clear the fragment to prepare for the next one
        }
    }
    if (subpart.size() > 0) subparts.push_back(subpart);

    // Calculate the weight of each fragment (i.e., the average weight of the elements in the fragment)
    vector<tuple<vector<int>, double>> subparts_withWeights;
    for (int i = 0; i < subparts.size(); i++) {
        double weight = 0;
        for (int j = 0; j < subparts[i].size(); j++) weight += (1 - supply_normalized[subparts[i][j]]) + 1 * (1 - leftTime_normalized[subparts[i][j]]);
        weight /= subparts[i].size();
        subparts_withWeights.push_back(make_tuple(subparts[i], weight));
    }
    // Sort the fragments in descending order by weight
    sort(subparts_withWeights.begin(), subparts_withWeights.end(), [](const tuple<vector<int>, double>& a, const tuple<vector<int>, double>& b) {return get<1>(a) > get<1>(b);});
    // Insert the fragments into offspring sequentially, reversing them if needed based on the weights of the elements within each fragment
    for (int i = 0; i < subparts.size(); i++) {
        // Determine whether reversal is needed, and reverse the fragment if necessary
        double w1 = 0, w2 = 0;
        for (int j = 0; j < subparts[i].size(); j++) {
            w1 += (j + 1) * ((1 - supply_normalized[subparts[i][j]]) + 1 * (1 - leftTime_normalized[subparts[i][j]]));
            w2 += (subparts[i].size() - j) * ((1 - supply_normalized[subparts[i][j]]) + 1 * (1 - leftTime_normalized[subparts[i][j]]));
        }
        if (w1 > w2) reverse(subparts[i].begin(), subparts[i].end());
        // Insert the fragment into offspring
        offspring.insert(offspring.end(), subparts[i].begin(), subparts[i].end());
    }

    // Convert offspring into a loop
    offspring.push_back(offspring[0]);
    
    return offspring;
}

vector<int> twoOpt_Mutation(vector<int> sol, vector<double> supply_normalized, vector<double> leftTime_normalized)
{
    vector<int> res = sol; double len = lenOfRoute_TSEA(res); double vis = visitOfRoute_TSEA(res, supply_normalized, leftTime_normalized);
    vector<tuple<vector<int>, double, double>> routes; // Tuple (route, length, priority of visit)
    routes.push_back(make_tuple(res, len, vis));

    int p = randareaInt(1, res.size() - 2);
        
    for (int j = 1; j < res.size() - 1; j++) // Iterate and reverse sub-segments
        if (j != p && j >= 1 && j < res.size() - 1)
        {
            // Calculate the route, length, and priority after reversing the sub-segment
            vector<int> tmp = res;
            if (j < p) reverse(tmp.begin() + j, tmp.begin() + p + 1);
            else reverse(tmp.begin() + p, tmp.begin() + j + 1);
            double l = lenOfRoute(tmp), v = visitOfRoute_Exp1(tmp, supply_normalized, leftTime_normalized);
            // In the routes vector, discard if an element dominates this route, replace if it dominates some element,
            // otherwise, insert in ascending order of length
            int flag = 1;
            for (int k = 0; k < routes.size(); k++)
                if (l >= get<1>(routes[k]) && v >= get<2>(routes[k])) { flag = 0; break;}
                else if (l <= get<1>(routes[k]) && v <= get<2>(routes[k])) { routes.erase(routes.begin() + k); k--;}
            if (flag) routes.insert(routes.end(), make_tuple(tmp, l, v));
        }

    sort(routes.begin(), routes.end(), sortByLength_Exp1);
    vector<double> tmp;
    for (int j = 0; j < routes.size(); j++)
        tmp.push_back(get<1>(routes[j]) * get<2>(routes[j]));
    res = get<0>(routes[index(tmp, min(tmp))]);

    return res;
}
