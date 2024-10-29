#include <random>
#include <vector>
#include <algorithm>
#include <iostream>
#include <time.h>
using namespace std;

// Static parameters
static default_random_engine e(0); // Engine

// Calculate the average of elements in a list
template <typename T>
double average(vector<T> lst){double res = 0; for (int i = 0; i < lst.size(); i++) res += double(lst[i]); return res / lst.size();}

// Calculate the Euclidean distance between two lists as coordinates
template <typename T>
double distance(vector<T> lst1, vector<T> lst2)
{double res = 0; for (int i = 0; i < lst1.size(); i++) res += pow(double(lst1[i]) - double(lst2[i]), 2); return pow(res, 0.5);}

// Throw an exception
void Error(string s) {throw std::logic_error(s);}

// Get the index of a certain element in the list
template <typename T>
int index(vector<T> lst, T x) {for (int i = 0; i < lst.size(); i++) if (lst[i] == x) return i; return -1;}

// Get the maximum value in the list
template <typename T>
T max(vector<T> lst){T M = lst[0]; for (int i = 1; i < lst.size(); i++) if (lst[i] > M) M = lst[i]; return M;}

// Get the minimum value in the list
template <typename T>
T min(vector<T> lst){T m = lst[0]; for (int i = 1; i < lst.size(); i++) if (lst[i] < m) m = lst[i]; return m;}

// Calculate the magnitude of a vector
template <typename T>
double magnitude(vector<T> v) {double res = 0; for (int i = 0; i < v.size(); i++) res += pow(v[i], 2); return pow(res, 0.5); }

// Output a normally distributed random number (mean and variance must be specified)
double norm(double miu, double sigma) { std::normal_distribution<double> n(miu, sigma); return n(e); }

// Output content
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
vector<int> randperm(int L, int U) // Random permutation between [L, U]
{
    vector<int> res, tmp; for (int i = L; i <= U; i++) tmp.push_back(i);
    for (int i = 0; i < U - L + 1; i++) {int r = randareaInt(0, U - L - i); res.push_back(tmp[r]); tmp[r] = tmp[U - L - i];}
    return res;
}
vector<int> randAreaPerm(int L, int U, int n) // Random arrangement of n numbers between [L, U] (in ascending order)
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

// Calculate the sum of elements in a list
template <typename T>
double sum(vector<T> lst){double res = 0; for (int i = 0; i < lst.size(); i++) res += lst[i]; return res;}

// Roulette wheel selection (input list, output the index of the selected element)
template <typename T>
int Roullete(vector<T> lst){
    double sel = randarea(0, sum(lst)), cul = 0;
    for (int i = 0; i < lst.size(); i++)
    { cul += lst[i]; if (sel <= cul) return i; }    
    return randareaInt(0, lst.size() - 1);
}

// Get the indices of the first n smallest elements in an array (e.g., for 2, 3, 1, 5, 4, the indices of the first 3 elements are: 2, 0, 1)
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

// Calculate the sum of two vectors (element-wise addition)
template <typename T>
vector<T> vPlus(vector<T> a, vector<T> b) { vector<T> res; for (int i = 0; i < a.size(); i++) res.push_back(a[i] + b[i]); return res; }
// Calculate the scalar multiplication of a vector (each element multiplied by a number)
template <typename T1, typename T2>
vector<T1> vMulN(vector<T1> a, T2 n) {vector<T1> res; for (int i = 0; i < a.size(); i++) res.push_back(a[i] * n); return res;}
template <typename T1, typename T2>
vector<T1> vMulN(T2 n, vector<T1> a) {vector<T1> res; for (int i = 0; i < a.size(); i++) res.push_back(a[i] * n); return res;}

// Calculate the variance of the vector
template <typename T>
double variance(vector<T> lst) {
    double avr = average(lst), res = 0; 
    for (int i = 0; i < lst.size(); i++) res += pow(lst[i] - avr, 2);
    return res / lst.size();
}
