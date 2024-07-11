#include <vector>
#include <bits/stdc++.h>
#include <algorithm>
using namespace std;

// 将站点均等分成k个扇区，返回的k个列表，每个列表为各个扇区的站点id
bool sortBySec(const tuple<int, double>& a, const tuple<int, double>& b) {return (get<1>(a) < get<1>(b));}
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


// // 按照站点缺货程度均匀性原则进行分配，是每辆车负责的站点缺货程度尽可能均匀
// vector<vector<int>> divideInSectionByCargo(vector<Station> stations, int k)
// {
//     // 先将站点按照其坐标与原点的连线和x轴正坐标之间的夹角进行排序  
//     vector<tuple<int, double>> part1, part2;
//     for (int i = 0; i < stations.size(); i++)
//     {
//         Station s = stations[i];
//         double angle = acos(s.pos[0] / pow(pow(s.pos[0], 2) + pow(s.pos[1], 2), 0.5));
//         if (s.pos[1] > 0) part1.push_back(make_tuple(s.id, angle));
//         else part2.push_back(make_tuple(s.id, angle));
//     }
    
//     sort(part1.begin(), part1.end(), sortBySec); reverse(part1.begin(), part1.end());
//     sort(part2.begin(), part2.end(), sortBySec); 
//     part1.insert(part1.end(), part2.begin(), part2.end());

//     // 求所有站点缺货程度之和，再除以k求出平均每辆车负责的辖区内站点缺货程度
//     double avr = 0;
//     for (int i = 0; i < stations.size(); i++) avr += 100 - stations[i].cargo;
//     avr /= k;

//     // 按照平均的缺货程度来分配各车负责的辖区
//     vector<vector<int>> res;
//     for (int i = 0; i < k; i++) res.push_back({});
//     double acc = 0;
//     int clusterInd = 0;
//     for (int i = 0; i < stations.size(); i++)
//     {
//         res[clusterInd].push_back(get<0>(part1[i])); 
//         if (clusterInd == k - 1 || acc < avr) 
//             acc += 100 - stations[get<0>(part1[i])].cargo; 
//         else {acc = 0; clusterInd++; }
//     }

//     // 按照各车辆负责的辖区之间缺货程度进行再均匀
//     vector<double> shortages;
//     for (int i = 0; i < cars.size(); i++) 
//     {
//         double tmp = 0;
//         for (int j = 0; j < res[i].size(); j++) tmp += 100 - stations[res[i][j]].cargo;
//         shortages.push_back(tmp);
//     }
//     int i = k - 1;
//     while (i > 0)
//     {
//         double before = abs(shortages[i] - shortages[i - 1]);
//         double after = abs(shortages[i - 1] - (100 - stations[res[i - 1][res[i - 1].size() - 1]].cargo) - 
//                           (shortages[i] + (100 - stations[res[i - 1][res[i - 1].size() - 1]].cargo))); 
//         if (before > after)
//         {
//             shortages[i - 1] -= 100 - stations[res[i - 1][res[i - 1].size() - 1]].cargo;
//             shortages[i] += 100 - stations[res[i - 1][res[i - 1].size() - 1]].cargo;
//             res[i].push_back(res[i - 1][res[i - 1].size() - 1]);
//             res[i - 1].pop_back();
//         }     
//         else
//         i--;    
//     }
    
//     return res;
// }