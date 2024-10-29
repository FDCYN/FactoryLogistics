#include <vector>
#include <algorithm>
#include "TSEA_operators.h"
using namespace std;

int N = 10;
int iterN = 10;
double P1 = 0.9;
double P2 = 0.1;


struct Sol
{
    vector<int> sol;
    int flag; // 0: optimize only the first objective function; 1: optimize only the second objective function; 2: optimize both objective functions simultaneously
    vector<double> obVal = {-1, -1};

    // Check if the current individual is dominated by another individual
    bool isDominatedBy(const Sol& other) const {
        bool isDominated = false;
        bool isEqual = true;
        for (int i = 0; i < obVal.size(); ++i) {
            if (obVal[i] > other.obVal[i]) isDominated = true;
            else if (obVal[i] < other.obVal[i]) isEqual = false;
        }
        return isDominated && !isEqual;
    }
};

// Select best solution set, which contains the indices of individuals in the population
// obNum <= 1: single objective; obNum = 2: multiple objectives
vector<Sol> chooseBestSolutions(vector<Sol> population, int flag)
{
    vector<Sol> bestSolutoins;

    if (flag <= 1) {
        // Generate the current population sorted by the objective function values from smallest to largest
        vector<Sol> sortedPop = population;
        sort(sortedPop.begin(), sortedPop.end(), [](const Sol& s1, const Sol& s2) {return s1.obVal[s1.flag] < s2.obVal[s2.flag];});
        // Take the top 1/10 of individuals as the best solution set
        bestSolutoins.insert(bestSolutoins.end(), sortedPop.begin(), sortedPop.begin() + sortedPop.size() / 10);
    }
    else {
        vector<int> is_unDominated(population.size(), 1); // Record whether each individual is not dominated by others
        for (int i = 0; i < population.size(); i++)
            if (is_unDominated[i] == 1) 
                for (int j = 0; j < population.size(); j++)
                    if (is_unDominated[j] == 1) {
                        // Check if there is a domination relationship between the two individuals
                        if (population[i].isDominatedBy(population[j])) {is_unDominated[i] = 0; break;}
                        else if (population[j].isDominatedBy(population[i])) {is_unDominated[j] = 0;}
                    }

        for (int i = 0; i < population.size(); i++)
            if (is_unDominated[i])
                bestSolutoins.push_back(population[i]);
    }

    return bestSolutoins;
}

// Divide the population into multiple Pareto fronts
vector<vector<Sol>> ParetoFronts(vector<Sol> population) 
{
    vector<vector<Sol>> fronts;
    vector<Sol> pop = population;

    while (pop.size() > 0) {
        vector<Sol> front; // Current round's front
        vector<int> is_unDominated(pop.size(), 1); // Record whether each individual is not dominated by others
        // Find non-dominated solutions among the remaining individuals
        for (int i = 0; i < pop.size(); i++)
            if (is_unDominated[i] == 1) 
                for (int j = 0; j < pop.size(); j++)
                    if (is_unDominated[j] == 1) {
                        // Check if there is a domination relationship between the two individuals
                        if (pop[i].isDominatedBy(pop[j])) {is_unDominated[i] = 0; break;}
                        else if (pop[j].isDominatedBy(pop[i])) {is_unDominated[j] = 0;}
                    }
        // Add these non-dominated solutions to the front and remove these individuals
        for (int i = pop.size() - 1; i >= 0; i--)
            if (is_unDominated[i])
                {front.push_back(pop[i]); pop.erase(pop.begin() + i);}
        // Add front to fronts
        fronts.push_back(front);
    }

    return fronts;
}

// Based on the mixed population of parents and offspring, select a new population
vector<Sol> selection(vector<Sol> allPop, int flag) 
{
    vector<Sol> newPop;
    if (flag <= 1) {
        sort(allPop.begin(), allPop.end(), [](const Sol& s1, const Sol& s2) {return s1.obVal[s1.flag] < s2.obVal[s2.flag];});
        newPop.insert(newPop.end(), allPop.begin(), allPop.begin() + N);
    }
    else {
        int c = 0;
        vector<vector<Sol>> fronts = ParetoFronts(allPop);
        for (int f = 0; f < fronts.size(); f++)
            for (int i = 0; i < fronts[f].size(); i++) {
                newPop.push_back(fronts[f][i]); c++;
                if (c >= N) return newPop;
            }
    }
    return newPop;
}

// Initialization
// route: loop
vector<Sol> TSEA_init(vector<int> route, int N, int flag, vector<double> supply_normalized, vector<double> leftTime_normalized)
{
    vector<Sol> population;
    Sol s = {route, flag, {lenOfRoute_TSEA(route), visitOfRoute_TSEA(route, supply_normalized, leftTime_normalized)}};
    population.push_back(s);
    for (int i = 0; i < N - 1; i++) {
        vector<int> s_e = randAreaPerm(1, route.size() - 2, 2);
        vector<int> newRoute = route;
        random_shuffle(newRoute.begin() + s_e[0], newRoute.begin() + s_e[1]);
        Sol s = {route, flag, {lenOfRoute_TSEA(route), visitOfRoute_TSEA(route, supply_normalized, leftTime_normalized)}};
        population.push_back(s);
    }
    return population;
}

// TSEA optimization process
Sol TSEA(vector<Sol> pop, int flag, vector<double> supply_normalized, vector<double> leftTime_normalized)
{
    vector<Sol> population = pop;
    int ns1 = 0, ns1_nf1 = 0, ns2 = 0, ns2_nf2 = 0; // Used to calculate the selection probability of two crossover operators

    for (int iter = 0; iter < iterN; iter++)
    {
        vector<Sol> bestSols = chooseBestSolutions(population, flag); // Select part of the best solutions
        /////// Crossover stage
        vector<int> which_operator; // Record which operator generated the new solution in the second phase of EA
        for (int i = 0; i < N; i++)
            if (randarea(0, 1) < P1) {
                int k = randareaInt(0, bestSols.size() - 1);
                
                vector<int> offspring_route;
                if (flag == 0) offspring_route = Guided_Crossover(population[i].sol, bestSols[k].sol);
                else if (flag == 1) offspring_route = Guided_Crossover_2(population[i].sol, bestSols[k].sol, supply_normalized, leftTime_normalized);
                else {
                    // Select crossover operator based on probability for crossover operation
                    double rd = randarea(0, 1);
                    double P = 0.5; 
                    if (ns2*ns1_nf1 + ns1*ns2_nf2 > 0) P = ns1*ns2_nf2 / (ns2*ns1_nf1 + ns1*ns2_nf2);

                    if (rd < P) {offspring_route = Guided_Crossover(population[i].sol, bestSols[k].sol); ns1_nf1++; which_operator.push_back(1);}
                    else        {offspring_route = Guided_Crossover_2(population[i].sol, bestSols[k].sol, supply_normalized, leftTime_normalized); ns2_nf2++; which_operator.push_back(2);}
                }
                
                Sol s = {offspring_route, flag, {lenOfRoute_TSEA(offspring_route), visitOfRoute_TSEA(offspring_route, supply_normalized, leftTime_normalized)}};
                population.push_back(s);
            }
        /////// Mutation stage
        if (flag == 2) {
            // Find all non-dominated solutions
            vector<int> is_unDominated(population.size(), 1); // Record whether each individual is not dominated by others
            for (int i = 0; i < population.size(); i++)
                if (is_unDominated[i] == 1) 
                    for (int j = 0; j < population.size(); j++)
                        if (is_unDominated[j] == 1) {
                            // Check if there is a dominance relationship between two individuals
                            if (population[i].isDominatedBy(population[j])) {is_unDominated[i] = 0; break;}
                            else if (population[j].isDominatedBy(population[i])) {is_unDominated[j] = 0;}
                        }

            vector<tuple<Sol, double>> tmp;
            for (int i = population.size() - 1; i >= 0; i--) if (is_unDominated[i]) tmp.push_back(make_tuple(population[i], population[i].obVal[1])); 
            sort(tmp.begin(), tmp.end(), [](const tuple<Sol, double>& a, const tuple<Sol, double>& b) {return get<1> (a) < get<1> (b);});
            vector<tuple<Sol, double>> front;
            if (tmp.size() <= 2) front.push_back(tmp[0]);
            else 
                for (int i = 1; i < tmp.size() - 1; i++) front.push_back(make_tuple(get<0>(tmp[i]), 
                                                                        abs(get<0>(tmp[i-1]).obVal[0] - get<0>(tmp[i]).obVal[0]) * 
                                                                        abs(get<0>(tmp[i]).obVal[1] - get<0>(tmp[i+1]).obVal[1])));
            sort(tmp.begin(), tmp.end(), [](const tuple<Sol, double>& a, const tuple<Sol, double>& b) {return get<1> (a) > get<1> (b);});

            vector<int> r1 = twoOpt_Mutation(get<0>(front[0]).sol, supply_normalized, leftTime_normalized);
            Sol s1 = {r1, flag, {lenOfRoute_TSEA(r1), visitOfRoute_TSEA(r1, supply_normalized, leftTime_normalized)}};
            population.push_back(s1);

            // Simultaneously update the scores of the two operators
            for (int i = N; i < is_unDominated.size(); i++)
                if (is_unDominated[i]) {
                    if (which_operator[i - N] == 1) ns1++;
                    else ns2++;
                }
        }
        /////// Selection stage
        population = selection(population, flag);
    }
    
    Sol s;
    if (flag <= 1) {
        s = population[0];
        for (int i = 1; i < population.size(); i++) if (s.obVal[flag] > population[i].obVal[flag]) s = population[i];
    }
    else {
        sort(population.begin(), population.end(), [](const Sol& s1, const Sol& s2) {return s1.obVal[0] * s1.obVal[1] < s2.obVal[0] * s2.obVal[1];});
        s = population[0];
    }
    return s;
}



/////////// TSEA main function
vector<int> TSEA_main(vector<int> route, vector<Station> stations, vector<vector<Task>> supply)
{
    //////////////// Calculate the normalized ratio of supply/covered orders for each machine, and the time to deplete remaining blocks for each machine
    vector<double> supply_toBeNormalized(max(route) + 1, -1);
    vector<double> leftTime_toBeNormalized(max(route) + 1, 0);
    // Calculate the ratio of supply/covered orders for each machine
    vector<int> supplyAmount(max(route) + 1); // Record the supply of each machine
    for (int i = 0; i < route.size() - 1; i++) {
        // Calculate the supply and number of covered orders for each machine
        vector<Task> taskCovered = supply[route[i]];
        int sup = 0;
        for (int j = 0; j < taskCovered.size(); j++) sup += taskCovered[j].blockNLeft;
        supplyAmount[route[i]] = sup;
        // Calculate the ratio of supply/number of orders
        if (taskCovered.size() > 0)
            supply_toBeNormalized[route[i]] = double(sup)/taskCovered.size(); // The smaller this ratio, the better
        else
            supply_toBeNormalized[route[i]] = 0;
    }

    // Calculate the time to deplete the remaining blocks for each machine
    for (int i = 0; i < route.size() - 1; i++) {
        int bnAvail = stations[route[i]].blockNAvail; // Remaining number of blocks at the current machine
        for (int t = 0; t < stations[route[i]].tasks.size(); t++) {
            if (stations[route[i]].tasks[t].blockNLeft < bnAvail) { // Remaining blocks are sufficient to complete the current order
                leftTime_toBeNormalized[route[i]] += double(ceil(double(stations[route[i]].tasks[t].barNum) / stations[route[i]].tasks[t].barN_eachUnit));
                bnAvail -= stations[route[i]].tasks[t].blockNLeft;
            }
            else { // Remaining blocks are insufficient to complete the current order
                int barN = bnAvail / stations[route[i]].tasks[t].blockN_eachBar; // Remaining blocks can produce a limited number of bars for the current order
                leftTime_toBeNormalized[route[i]] += double(ceil(double(barN) / stations[route[i]].tasks[t].barN_eachUnit));
                bnAvail = 0;
                break;
            }
        }
        if (bnAvail > 0) // If there are remaining blocks after completing all orders, set the remaining depletion time to infinity
            leftTime_toBeNormalized[route[i]] = iterNum;
    }

    // Normalization operation
    double max_supply = max(supply_toBeNormalized), max_blockNAvail = max(leftTime_toBeNormalized);
    vector<double> supply_normalized(max(route) + 1, -1);
    vector<double> leftTime_normalized(max(route) + 1, 0);
    if (max_supply <= 0) max_supply = 1;
    if (max_blockNAvail <= 0) max_blockNAvail = 1;
    for (int i = 0; i < route.size() - 1; i++) {
        supply_normalized[route[i]] = supply_toBeNormalized[route[i]] / max_supply;
        leftTime_normalized[route[i]] = leftTime_toBeNormalized[route[i]] / max_blockNAvail;
    }

    ///////////////// Start optimization /////////////////
    vector<Sol> pop1 = TSEA_init(route, N, 0, supply_normalized, leftTime_normalized);
    Sol s1 = TSEA(pop1, 0, supply_normalized, leftTime_normalized);
    vector<Sol> pop2 = TSEA_init(route, N, 1, supply_normalized, leftTime_normalized);
    Sol s2 = TSEA(pop2, 1, supply_normalized, leftTime_normalized);

    vector<Sol> pop3;
    for (int i = 0; i < N / 2; i++) {pop3.push_back(s1); pop3.push_back(s2);}
    Sol s3 = TSEA(pop3, 2, supply_normalized, leftTime_normalized);
    
    return s3.sol;
}
