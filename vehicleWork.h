#include <vector>
using namespace std;

struct CarAndTimeLeft 
{
    Car car;
    double timeLeft;
};

// 前往下一个站点
CarAndTimeLeft toNextStation(Car car, double timeLeft)
{
    Station z = stations[car.route[0]];
    double d = distance(car.pos, z.pos);
    // 如果在剩余时间内该车到不了当前目的站点
    if (d > carSpeed * timeLeft)
    {
        car.onTheWay += timeLeft;
        car.pos[0] -= (car.pos[0] - z.pos[0]) * (carSpeed * timeLeft / d);
        car.pos[1] -= (car.pos[1] - z.pos[1]) * (carSpeed * timeLeft / d);
        timeLeft = 0;
    }
    // 如果在该单位时间内能够到达当前目的地
    else
    {
        stations[z.id].time = 0; // 该站点未被访问时间置零
        car.stopStation = z.id;   
        car.pos[0] = z.pos[0];
        car.pos[1] = z.pos[1];  // 到达当前目的站点
        timeLeft -= d / carSpeed;  // 剩余时间扣除最后达到目的站点的路程时间
        car.onTheWay += d / carSpeed; // 增加车辆在路上的时间
        car.status = 2;  // 将该车辆的状态转化为卸货状态
        car.unloadLeft = 50 * ((100 - z.cargo) / 100);  // 计算该车在当前站点卸货量
        if (car.unloadLeft > car.cargo)  // 如果计算出的某类货物卸货量小于目前小车搭载的该类货物量, 则将该类货物全部卸掉
            car.unloadLeft = car.cargo;
    }

    CarAndTimeLeft CT = {car, timeLeft};
    return CT;
}

// 卸货
CarAndTimeLeft unload(Car car, double timeLeft)
{
    Station z = stations[car.stopStation];
    double unLoadAmount = unloadSpeed * timeLeft;  // 剩余时间内可以卸货的量
    // 剩余时间内无法全部卸完货
    if (unLoadAmount < car.unloadLeft)
    {
        car.unloadTime += timeLeft;
        car.cargo -= unLoadAmount; // 更新车辆货物剩余量
        car.unloadLeft -= unLoadAmount; // 更新剩余卸货量
        stations[z.id].cargo += unLoadAmount; // 更新站点货物剩余
        timeLeft = 0;
    }
    // 剩余时间能够卸完货
    else
    {
        car.route = modifyRoute(car.route);
        // 将当前站点移出并放于访问序列末尾
        car.route.erase(car.route.begin());
        car.route.push_back(car.route[0]);

        car.unloadTime += car.unloadLeft / unloadSpeed;
        timeLeft -= car.unloadLeft / unloadSpeed; // 剩余时间减去卸货所用时间
        car.cargo -= car.unloadLeft; // 更新车辆货物剩余量
        stations[z.id].cargo += car.unloadLeft;  // 更新站点货物剩余
        car.unloadLeft = 0;  // 更新剩余卸货量
        car.stopStation = -1;  // 车辆离站
        if (car.cargo < 5)  // 如果车辆当前载货中的各类货物最多不超过一定额度时便开始返程
        {
            car.status = 3;
            car.backCount += 1;
        }
        else  // 否则车辆朝着下一个目的站点驶去
            car.status = 1;
    }

    CarAndTimeLeft CT = {car, timeLeft};
    return CT;
}

// 返回仓库
CarAndTimeLeft backToDepot(Car car, double timeLeft)
{
    double d = distance(car.pos, depot);  // 计算当前车辆和总厂之间的距离
    // 如果在剩余时间内该车无法完成返程
    if (d > carSpeed * timeLeft)  
    {
        car.pos[0] -= (car.pos[0] - depot[0]) * (carSpeed * timeLeft / d);
        car.pos[1] -= (car.pos[1] - depot[1]) * (carSpeed * timeLeft / d);
        car.onTheWay += timeLeft;
        timeLeft = 0;
    }
    // 如果在剩余时间内车辆能够完成返程
    else  
    {
        timeLeft -= d / carSpeed;  // 剩余时间减去回到总厂的路上用时
        car.onTheWay += d / carSpeed;
        car.pos = depot;  // 车辆到达仓库
        car.status = 4;  // 车辆状态为装货
    }

    CarAndTimeLeft CT = {car, timeLeft};
    return CT;
}

// 重装货物
CarAndTimeLeft reload(Car car, double timeLeft)
{
    double reloadAmount = reloadSpeed * timeLeft;
    if (car.cargo + reloadAmount < carLoad) // 如果剩余时间内无法完成装货
    {
        car.reloadTime += timeLeft;
        car.cargo += reloadAmount;
        timeLeft = 0;
    }
    // 剩余时间能够完成装货
    else
    {
        car.reloadTime += (carLoad - car.cargo) / reloadSpeed;
        timeLeft -= (carLoad - car.cargo) / reloadSpeed; // 剩余时间减去装货时间
        car.cargo = carLoad;
        car.status = 1; // 车辆开始继续出发
    }
    
    CarAndTimeLeft CT = {car, timeLeft};
    return CT;
}

