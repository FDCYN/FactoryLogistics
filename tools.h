#include <random>
#include <vector>
#include <algorithm>
#include <iostream>
#include <time.h>
using namespace std;

// 静态参数
static default_random_engine e(0); //引擎

// 求列表元素平均值
template <typename T>
double average(vector<T> lst){double res = 0; for (int i = 0; i < lst.size(); i++) res += double(lst[i]); return res / lst.size();}

// 求两个列表作为坐标之间的欧式距离
template <typename T>
double distance(vector<T> lst1, vector<T> lst2)
{double res = 0; for (int i = 0; i < lst1.size(); i++) res += pow(double(lst1[i]) - double(lst2[i]), 2); return pow(res, 0.5);}

// 抛出异常
void Error(string s) {throw std::logic_error(s);}

// 求列表中某元素下标
template <typename T>
int index(vector<T> lst, T x) {for (int i = 0; i < lst.size(); i++) if (lst[i] == x) return i; return -1;}

// 求列表中的最大值
template <typename T>
T max(vector<T> lst){T M = lst[0]; for (int i = 1; i < lst.size(); i++) if (lst[i] > M) M = lst[i]; return M;}

// 求列表中的最小值
template <typename T>
T min(vector<T> lst){T m = lst[0]; for (int i = 1; i < lst.size(); i++) if (lst[i] < m) m = lst[i]; return m;}

// 求向量模长
template <typename T>
double magnitude(vector<T> v) {double res = 0; for (int i = 0; i < v.size(); i++) res += pow(v[i], 2); return pow(res, 0.5); }

// 输出一个正态分布随机数（均值、方差须指定）
double norm(double miu, double sigma) { std::normal_distribution<double> n(miu, sigma); return n(e); }

// 输出内容
template <typename T>
void print(T* lst, int len){ for (int i = 0; i < len; i++) cout<<lst[i]<<' '; cout<<'\n'; }
template <typename T>
void print(T lst, int len){ for (int i = 0; i < len; i++) cout<<lst[i]<<' '; cout<<'\n'; }
template <typename T>
void print(vector<T> lst){ for (int i = 0; i < lst.size(); i++) cout<<lst[i]<<' '; cout<<'\n'; }
template <typename T>
void print(T a){cout<<a<<' ';}

double randarea(double L, double U) {return (double)rand()/(double)RAND_MAX * (U - L) + L;}
int randareaInt(int L, int U) {
    int res = (int)((double)rand()/(double)RAND_MAX * (U - L + 1)) + L; 
    if (res < L) res = L; if(res > U) res = U; return res;} // [L, U]  
vector<int> randperm(int L, int U) // [L, U]间的随机排列
{
    vector<int> res, tmp; for (int i = L; i <= U; i++) tmp.push_back(i);
    for (int i = 0; i < U - L + 1; i++) {int r = randareaInt(0, U - L - i); res.push_back(tmp[r]); tmp[r] = tmp[U - L - i];}
    return res;
}
vector<int> randAreaPerm(int L, int U, int n) // [L, U]间n个随机数排列（从小到大）
{
    vector<int> res;
    for (int i = 0; i < n; )
    {
        int r = randareaInt(L, U), j = 0, f = 1;
        for (; j < res.size(); j++)
        {
            if (res[j] == r) {f = 0; break;}
            if (res[j] > r) break;
        }
        if (f) { res.insert(res.begin() + j, r); i++; }
    }
    return res;
}

// 求列表元素之和
template <typename T>
double sum(vector<T> lst){double res = 0; for (int i = 0; i < lst.size(); i++) res += lst[i]; return res;}

// 轮盘赌(输入列表, 输出被选元素的下标)
template <typename T>
int Roullete(vector<T> lst){
    double sel = randarea(0, sum(lst)), cul = 0;
    for (int i = 0; i < lst.size(); i++)
    { cul += lst[i]; if (sel <= cul) return i; }    
    return randareaInt(0, lst.size() - 1);
}

// 求一个数组前n个元素的下标（例：2、3、1、5、4 前3个元素下标为：2、0、1）
template <typename T>
vector<int> smallest_n_ind(vector<T> lst, int n)
{
    vector<int> res;
    for(int i = 0; i < n; i++)
    {
        int smallest = -1;
        for (int j = 0; j < lst.size(); j++)
            if(index(res, j) < 0 && smallest == -1 || index(res, j) < 0 && lst[j] <= lst[smallest])
                smallest = j;
        res.push_back(smallest);
    }
    return res;
}

// 求向量之和（对应元素相加）
template <typename T>
vector<T> vPlus(vector<T> a, vector<T> b) { vector<T> res; for (int i = 0; i < a.size(); i++) res.push_back(a[i] + b[i]); return res; }
// 求向量数乘运算（各元素与数字相乘）
template <typename T1, typename T2>
vector<T1> vMulN(vector<T1> a, T2 n) {vector<T1> res; for (int i = 0; i < a.size(); i++) res.push_back(a[i] * n); return res;}
template <typename T1, typename T2>
vector<T1> vMulN(T2 n, vector<T1> a) {vector<T1> res; for (int i = 0; i < a.size(); i++) res.push_back(a[i] * n); return res;}

// 求方差
template <typename T>
double variance(vector<T> lst) {
    double avr = average(lst), res = 0; 
    for (int i = 0; i < lst.size(); i++) res += pow(lst[i] - avr, 2);
    return res / lst.size();
}