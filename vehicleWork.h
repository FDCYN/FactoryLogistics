#include <vector>
using namespace std;

struct CarAndTimeLeft 
{
    Car car;
    double timeLeft;
};

int replanFlag = 1;

// Move to the next station
CarAndTimeLeft toNextStation(Car car, double timeLeft)
{
    Station z = stations[car.route[0]];
    // If the car cannot reach the current destination station within the remaining time
    if (car.timeLeft > timeLeft)
    {
        car.onTheWay += timeLeft;
        car.timeLeft -= timeLeft;
        timeLeft = 0;
    }
    // If the car can reach the current destination within this unit of time
    else
    {
        timeLeft -= car.timeLeft;  // Subtract the travel time to reach the current destination from the remaining time
        // Update some parameters of the car
        car.onTheWay += car.timeLeft;
        car.stopStation = z.id;   
        car.status = 2;  // Change the car's status to unloading
        int unloadAmount = supplyAmount_eachMachine[z.id];
        car.cargo -= unloadAmount; // Deduct the unloaded amount from the car's remaining cargo
        car.timeLeft = unloadAmount * unloadT_eachBlock; // Total time required for unloading
        // Update other parameters
        stations[z.id].time = 0; // Reset the time the station has not been visited
        stations[z.id].blockNAvail += unloadAmount;
        stations[z.id].visitCount++;
    }

    CarAndTimeLeft CT = {car, timeLeft};
    return CT;
}

// Unloading
CarAndTimeLeft unload(Car car, double timeLeft)
{
    Station z = stations[car.stopStation];
    // Cannot completely unload within the remaining time
    if (car.timeLeft > timeLeft)
    {
        car.unloadTime += timeLeft;
        car.timeLeft -= timeLeft;
        timeLeft = 0;
    }
    // Can unload completely within the remaining time
    else
    {   
        // Dynamic programming for a new path
        vector<vector<Task>> supply_thisCar = supply_eachStation(car.scope, stations, carLoad);
        for (int i = 0; i < car.scope.size(); i++) { // Update information
            vector<Task> taskCovered = supply_thisCar[car.scope[i]];
            supply_eachMachine[car.scope[i]] = taskCovered;
            int sup = 0; for (int j = 0; j < taskCovered.size(); j++) sup += taskCovered[j].blockNLeft;
            supplyAmount_eachMachine[car.scope[i]] = sup;
        }
        if (replanFlag) car.route = TSEA_main(car.route, stations, supply_eachMachine); replan_count++;

        // Remove the current station and append it to the end of the visit sequence
        car.route.erase(car.route.begin());
        car.route.push_back(car.route[0]);

        // Update some parameters
        car.unloadTime += car.timeLeft;
        timeLeft -= car.timeLeft; // Subtract the time used for unloading from the remaining time
        car.stopStation = -1;  // The car leaves the station
        if (car.cargo <= 0) { // If the car's current cargo is within a certain limit, it starts returning
            car.status = 3;
            car.backCount += 1;
            car.timeLeft = Manhattan_dist(stations[z.id].pos, depot) / carSpeed;
        }
        else { // Otherwise, the car heads towards the next destination station
            car.status = 1;
            car.timeLeft = Manhattan_dist(stations[z.id].pos, stations[car.route[0]].pos) / carSpeed;
        }
    }

    CarAndTimeLeft CT = {car, timeLeft};
    return CT;
}

// Return to depot
CarAndTimeLeft backToDepot(Car car, double timeLeft)
{
    // If the car cannot complete the return trip within the remaining time
    if (car.timeLeft > timeLeft)  
    {
        car.timeLeft -= timeLeft;
        car.onTheWay += timeLeft;
        timeLeft = 0;
    }
    // If the car can complete the return trip within the remaining time
    else  
    {
        car.onTheWay += car.timeLeft; 
        timeLeft -= car.timeLeft;  // Subtract the time taken to return to the depot from the remaining time
        car.stopStation = -2; // Station -2 indicates the car is at the depot
        car.status = 4;  // The car's status is reloading

        // Calculate how much cargo needs to be replenished after returning to the depot
        int reloadAmount = carLoad;
        car.timeLeft = reloadAmount * reloadT_eachBlock; // Calculate the time needed to reload cargo
        car.cargo = reloadAmount;
    }

    CarAndTimeLeft CT = {car, timeLeft};
    return CT;
}

// Reload cargo
CarAndTimeLeft reload(Car car, double timeLeft)
{
    if (car.timeLeft > timeLeft) // If it cannot finish loading within the remaining time
    {
        car.reloadTime += timeLeft;
        car.timeLeft -= timeLeft;
        timeLeft = 0;
    }
    // Remaining time can complete loading
    else
    {
        if (replanFlag) car.route = TSEA_main(car.route, stations, supply_eachMachine); replan_count++;
        car.reloadTime += car.timeLeft;
        timeLeft -= car.timeLeft; // Subtract the loading time from the remaining time
        car.status = 1; // The car is ready to depart again
        car.timeLeft = Manhattan_dist(depot, stations[car.route[0]].pos) / carSpeed;
    }
    
    CarAndTimeLeft CT = {car, timeLeft};
    return CT;
}
