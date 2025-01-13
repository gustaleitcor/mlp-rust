#include <iostream>
#include <cstdint>
#include <cstring>
#include <cfloat>
#include <new>
#include <chrono>
#include <fstream>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "readData.h"
#include "Data.hpp"

#define REINSERTION 1
#define OR_OPT_2 	2
#define OR_OPT_3 	3
#define SWAP 		4
#define TWO_OPT		5
#define TABLE_SZ    26
#define DBL_SZ      8
#define INT_SZ      4

#define MATRIX

using std::chrono::high_resolution_clock;

typedef unsigned uint;

// 3D array
typedef std::vector<std::vector<std::vector<double>>> tSubseq;

typedef struct tInfo {
    double ** cost;
    int dimen;
    uint T;
    uint C;
    uint W;
    vector<int> rnd;
    uint rnd_index;
} tInfo;

typedef struct tSolution {
    std::vector<int> s;
    //tSeqInfo ** seq;
    double *** seq;
    double cost;
} tSolution;

tSolution Solution_init(tInfo info) {
    tSolution solut;
    solut.s = vector<int>(info.dimen+1);

    solut.seq = new double ** [info.dimen+1];
    for (int i = 0; i < info.dimen+1; i++) {
        solut.seq[i] = new double * [info.dimen+1];
        for (int j = 0; j < info.dimen+1; j++) {
            solut.seq[i][j] = new double [3];
        }
    }

    /*
    solut.seq = new tSeqInfo * [info.dimen+1];
    for (int i = 0; i < info.dimen+1; i++) {
        solut.seq[i] = new tSeqInfo [info.dimen+1];
        memset(solut.seq[i], 0.0, (info.dimen+1)*sizeof(tSeqInfo));
    }
    */

    solut.cost = DBL_MAX;

    return solut;
}

inline void Solution_cpy( tSolution & src, tSolution & tgt, const tInfo & info) {

    tgt.s = src.s;
    tgt.cost = src.cost;

    /*
    for (int i = 0; i < info.dimen+1; i++) {
        for (int j = 0; j < info.dimen+1; j++) {
            //memcpy(tgt.seq[i][j], src.seq[i][j], 3 * sizeof(double));
            std::copy(src.seq[i][j], src.seq[i][j] + 3, tgt.seq[i][j]);
        }
    }
    */

}

double R_table(int i){
    static const double table[] = {0.00, 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12, 
                                    0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20, 0.21, 0.22, 0.23, 0.24, 0.25};
    return table[i];
}

void print_s(std::vector<int> s) {

    for (int i = 0; i < s.size(); i++)
        std::cout << s[i]+1 << " ";
    std::cout << std::endl;
}

int partition(std::vector<int>& arr, int left, int right, const tInfo& info, int r) {
    int pivot = arr[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (info.cost[r][arr[j]] < info.cost[r][pivot]) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[right]);
    return i + 1;
}

void quicksort(std::vector<int>& arr, int left, int right, const tInfo& info, int r) {
    if (left < right) {
        int pivot = partition(arr, left, right, info, r);
        quicksort(arr, left, pivot - 1, info, r);
        quicksort(arr, pivot + 1, right, info, r);
    }
}

void sort(std::vector<int>& arr, int r, const tInfo& info) {
    quicksort(arr, 0, arr.size() - 1, info, r);
}

std::vector<int> construct(const double alpha, tInfo & info){

    std::vector<int> s = {0};
    s.reserve(info.dimen+1);
    std::vector<int> cL(info.dimen-1);
    for(int i = 1; i < info.dimen; ++i){
        cL[i-1] = i;
    }

    int r = 0;
    while (!cL.empty()) {
        sort(cL, r, info);

        int index = info.rnd[info.rnd_index++];
        int c = cL[index];
        s.push_back(c);
        r = c;
        cL.erase(cL.begin() + index);
    }

    s.push_back(0);

    return s;
}	

inline void swap(std::vector<int> &vec, int i, int j){
    std::iter_swap(vec.begin() + i, vec.begin() + j);
}

inline void reverse(std::vector<int> &vec, int i, int j){
    std::reverse(vec.begin() + i, vec.begin() + j+1);
}

inline void reinsert(std::vector<int> &vec, int i, int j, int pos){
    std::vector<int> seq (vec.begin() + i, vec.begin() +j+1);
    if(pos < i){
        vec.erase(vec.begin() + i, vec.begin() + j+1);
        vec.insert(vec.begin() + pos, seq.begin(), seq.end());
    }else{
        vec.insert(vec.begin() + pos, seq.begin(), seq.end());
        vec.erase(vec.begin() + i, vec.begin() + j+1);
    }

}



inline void subseq_load(tSolution & solut, const tInfo & info, int index = 0){
    alignas(INT_SZ) int i, j, j_prev, k;
    alignas(INT_SZ) int from = index;
    alignas(1) bool t;
    for (i = 0; i < info.dimen+1; i++) {
        k = 1 - i - (!i);
        t = i == from;

        solut.seq[i][i][info.T] = 0.0;
        solut.seq[i][i][info.C] = 0.0;
        solut.seq[i][i][info.W] = (double) !(i == 0);

        for (j = i+1; j < info.dimen+1; j++) {
            j_prev = j-1;
            
            solut.seq[i][j][info.T] = info.cost[solut.s[j_prev]][solut.s[j]] + solut.seq[i][j_prev][info.T];
            solut.seq[i][j][info.C] = solut.seq[i][j][info.T] + solut.seq[i][j_prev][info.C];
            solut.seq[i][j][info.W] = j + k;

        }
        from += t;
    }

    solut.cost = solut.seq[0][info.dimen][info.C];
}

inline bool search_swap(tSolution & solut, const tInfo & info) {
    alignas(DBL_SZ) double cost_new, 
        cost_concat_1, cost_concat_2, cost_concat_3, cost_concat_4;
    alignas(DBL_SZ) double cost_best = DBL_MAX;
    alignas(INT_SZ) int i, j, j_prev, j_next, i_prev, i_next;
    alignas(INT_SZ) int I;
    alignas(INT_SZ) int J;

    for (i = 1; i < info.dimen-1; ++i) {
        i_prev = i - 1;
        i_next = i + 1;

        //consecutive nodes
        cost_concat_1 =                 solut.seq[0][i_prev][info.T] + info.cost[solut.s[i_prev]][solut.s[i_next]];
        cost_concat_2 = cost_concat_1 + solut.seq[i][i_next][info.T]  + info.cost[solut.s[i]][solut.s[i_next+1]];

        cost_new = solut.seq[0][i_prev][info.C]                                                    +           //       1st subseq
        solut.seq[i][i_next][info.W]               * (cost_concat_1) + info.cost[solut.s[i_next]][solut.s[i]]  +           // concat 2nd subseq
        solut.seq[i_next+1][info.dimen][info.W]   * (cost_concat_2) + solut.seq[i_next+1][info.dimen][info.C];   // concat 3rd subseq

        if (cost_new < cost_best) {
            cost_best = cost_new - DBL_EPSILON;
            I = i;
            J = i_next;
        }

        for (j = i_next+1; j < info.dimen; ++j) {
            j_next = j + 1;
            j_prev = j - 1;

            cost_concat_1 =                 solut.seq[0][i_prev][info.T]       + info.cost[solut.s[i_prev]][solut.s[j]];
            cost_concat_2 = cost_concat_1                           + info.cost[solut.s[j]][solut.s[i_next]];
            cost_concat_3 = cost_concat_2 + solut.seq[i_next][j_prev][info.T]  + info.cost[solut.s[j_prev]][solut.s[i]];
            cost_concat_4 = cost_concat_3                           + info.cost[solut.s[i]][solut.s[j_next]];

            cost_new = solut.seq[0][i_prev][info.C]                                                 +      // 1st subseq
            cost_concat_1 +                                                             // concat 2nd subseq (single node)
            solut.seq[i_next][j_prev][info.W]      * cost_concat_2 + solut.seq[i_next][j_prev][info.C] +      // concat 3rd subseq
            cost_concat_3 +                                                             // concat 4th subseq (single node)
            solut.seq[j_next][info.dimen][info.W] * cost_concat_4 + solut.seq[j_next][info.dimen][info.C];   // concat 5th subseq

            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
            }
        }
    }

    if (cost_best < solut.cost - DBL_EPSILON) {
        swap(solut.s, I, J);
        subseq_load(solut, info, I);
        return true;
    }

    return false;
}

inline bool search_two_opt(tSolution & solut, const tInfo & info) {
    alignas(DBL_SZ) double cost_new, 
        cost_concat_1, cost_concat_2;
    alignas(DBL_SZ) double cost_best = DBL_MAX;// cost_l1, cost_l2;
    alignas(DBL_SZ) double rev_seq_cost;
    alignas(INT_SZ) int i, j, i_prev, j_next;
    alignas(INT_SZ) int I;
    alignas(INT_SZ) int J;

    for (i = 1; i < info.dimen-1; ++i) {
        i_prev = i - 1;

        rev_seq_cost = solut.seq[i][i+1][info.T];
        for (j = i + 2; j < info.dimen; ++j) {
            j_next = j + 1;


          rev_seq_cost += info.cost[solut.s[j-1]][solut.s[j]] * (solut.seq[i][j][info.W]-1.0);

          cost_concat_1 =                 solut.seq[0][i_prev][info.T]   + info.cost[solut.s[j]][solut.s[i_prev]];
          cost_concat_2 = cost_concat_1 + solut.seq[i][j][info.T]        + info.cost[solut.s[j_next]][solut.s[i]];

          cost_new = solut.seq[0][i_prev][info.C]                                                        +   //  1st subseq
              solut.seq[i][j][info.W]                * cost_concat_1 + rev_seq_cost                  +   // concat 2nd subseq (reversed seq)
              solut.seq[j_next][info.dimen][info.W] * cost_concat_2 + solut.seq[j_next][info.dimen][info.C];      // concat 3rd subseq

            
            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
            }
        }
    }

    if (cost_best < solut.cost - DBL_EPSILON) {
        reverse(solut.s, I, J);
        subseq_load(solut, info);
        return true;
    }

    return false;
}

inline bool search_reinsertion(tSolution & solut, const tInfo & info, const int opt) {
    alignas(DBL_SZ) double cost_new, cost_concat_1, cost_concat_2, cost_concat_3;
    alignas(DBL_SZ) double cost_best = DBL_MAX;//, cost_l1, cost_l2, cost_l3;
    alignas(INT_SZ) int i, j, k, k_next, i_prev, j_next;
    alignas(INT_SZ) int I;
    alignas(INT_SZ) int J;
    alignas(INT_SZ) int POS;

    for (i = 1, j = opt +i-1; i < info.dimen-opt+1; ++i, ++j) {
        j_next = j + 1;
        i_prev = i - 1;

        //k -> edges 
        for(k = 0; k < i_prev; ++k){
            k_next = k+1;

          cost_concat_1 =                 solut.seq[0][k][info.T]            + info.cost[solut.s[k]][solut.s[i]];
          cost_concat_2 = cost_concat_1 + solut.seq[i][j][info.T]            + info.cost[solut.s[j]][solut.s[k_next]];
          cost_concat_3 = cost_concat_2 + solut.seq[k_next][i_prev][info.T]  + info.cost[solut.s[i_prev]][solut.s[j_next]];

          cost_new = solut.seq[0][k][info.C]                                                                   +   //       1st subseq
              solut.seq[i][j][info.W]               * cost_concat_1 + solut.seq[i][j][info.C]                  +   //  concat 2nd subseq (reinserted seq)
              solut.seq[k_next][i_prev][info.W]     * cost_concat_2 + solut.seq[k_next][i_prev][info.C]        +   //  concat 3rd subseq
              solut.seq[j_next][info.dimen][info.W] * cost_concat_3 + solut.seq[j_next][info.dimen][info.C];       // concat 4th subseq

            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
                POS = k;
            }
        }

        for (k = i + opt; k < info.dimen; ++k) {
            k_next = k + 1;

          cost_concat_1 =                 solut.seq[0][i_prev][info.T]  + info.cost[solut.s[i_prev]][solut.s[j_next]];
          cost_concat_2 = cost_concat_1 + solut.seq[j_next][k][info.T]  + info.cost[solut.s[k]][solut.s[i]];
          cost_concat_3 = cost_concat_2 + solut.seq[i][j][info.T]       + info.cost[solut.s[j]][solut.s[k_next]];

          cost_new = solut.seq[0][i_prev][info.C]                                                                  +   //       1st subseq
                  solut.seq[j_next][k][info.W]          * cost_concat_1 + solut.seq[j_next][k][info.C]             +   // concat 2nd subseq
                  solut.seq[i][j][info.W]               * cost_concat_2 + solut.seq[i][j][info.C]                  +   // concat 3rd subseq (reinserted seq)
                  solut.seq[k_next][info.dimen][info.W] * cost_concat_3 + solut.seq[k_next][info.dimen][info.C];       // concat 4th subseq
          
            if (cost_new < cost_best) {
                cost_best = cost_new - DBL_EPSILON;
                I = i;
                J = j;
                POS = k;
            }
        }
    }

    if (cost_best < solut.cost - DBL_EPSILON) {
        reinsert(solut.s, I, J, POS+1);
        subseq_load(solut, info, I < POS+1 ? I : POS+1);
        return true;
    }

    return false;
}


void RVND(tSolution & solut, tInfo & info) {

    alignas(alignof(std::vector<int>)) std::vector<int> neighbd_list = {SWAP, TWO_OPT, REINSERTION, OR_OPT_2, OR_OPT_3};
    alignas(INT_SZ) uint index;
    alignas(INT_SZ) int neighbd;
    bool improve_flag;

    while (!neighbd_list.empty()) {

        index = info.rnd[info.rnd_index++];
        neighbd = neighbd_list[index];

        improve_flag = false;

        switch(neighbd){
            case REINSERTION:
                improve_flag = search_reinsertion(solut, info, REINSERTION);
                break;				
            case OR_OPT_2:
                improve_flag = search_reinsertion(solut, info, OR_OPT_2);
                break;				
            case OR_OPT_3:
                improve_flag = search_reinsertion(solut, info, OR_OPT_3);
                break;				
            case SWAP:
                improve_flag = search_swap(solut, info);
                break;
            case TWO_OPT:
                improve_flag = search_two_opt(solut, info);
                break;				
        }

        if (improve_flag) {
            neighbd_list = {SWAP, TWO_OPT, REINSERTION, OR_OPT_2, OR_OPT_3};
        } else {
            neighbd_list.erase(neighbd_list.begin() + index);
        }
    }

}

std::vector<int> perturb(tSolution * solut, tInfo & info) {
    auto s = solut->s;
    int A_start = 1;
    int A_end = 1;
    int B_start = 1;
    int B_end = 1;

    while ((A_start <= B_start && B_start <= A_end) || (B_start <= A_start && A_start <= B_end)) {
        A_start = info.rnd[info.rnd_index++];
        A_end = A_start + info.rnd[info.rnd_index++];

        B_start = info.rnd[info.rnd_index++];
        B_end = B_start + info.rnd[info.rnd_index++];
    }
    
    if (A_start < B_start) {
        reinsert(s, B_start, B_end-1, A_end);
        reinsert(s, A_start, A_end-1, B_end);
    } else {
        reinsert(s, A_start, A_end-1, B_end);
        reinsert(s, B_start, B_end-1, A_end);
    }

    return s;
}


void GILS_RVND(int Imax, int Iils, tInfo & info) {

    tSolution solut_partial = Solution_init(info);
    tSolution solut_crnt = Solution_init(info);
    tSolution solut_best = Solution_init(info);

    for(int i = 0; i < Imax; ++i){
        int aux = info.rnd[info.rnd_index++];

        double alpha = R_table(aux);

        printf("[+] Search %d\n", i+1);
        printf("\t[+] Constructing..\n");	


        solut_crnt.s = construct(alpha, info);
        subseq_load(solut_crnt, info);

        Solution_cpy(solut_crnt, solut_partial, info);
        printf("\t[+] Looking for the best Neighbor..\n");
        printf("\t    Construction Cost: %.3lf\n", solut_partial.cost);	

        int iterILS = 0;
        while (iterILS < Iils) {
            RVND(solut_crnt, info);
            if(solut_crnt.cost < solut_partial.cost - DBL_EPSILON){
                Solution_cpy(solut_crnt, solut_partial, info);
                iterILS = 0;
            }

            solut_crnt.s = perturb(&solut_partial, info);
            subseq_load(solut_crnt, info);
            iterILS++;
        }

        if (solut_partial.cost < solut_best.cost - DBL_EPSILON) {
            Solution_cpy(solut_partial, solut_best, info);
        }

        std::cout << "\tCurrent best cost: "<< solut_best.cost << std::endl;

        std::cout << "SOLUCAO: ";
        for(int i = 0; i < solut_best.s.size(); i++){
            std::cout << solut_best.s[i] << " ";
        }
        std::cout << std::endl;

    }
    printf("COST: %.2lf\n", solut_best.cost);
}

int main(int argc, char **argv){
    int Imax = 10;
    int Iils;

    tInfo info = {};
    info.T = 0;
    info.W = 1;
    info.C = 2;

    std::vector<int> rnd;

    info.dimen = loadData(&info.cost, rnd);
    info.rnd = rnd;
    info.rnd_index = 0;

    tSolution solut = Solution_init(info);

    Iils = info.dimen < 100 ? info.dimen : 100;
    auto t1 = high_resolution_clock::now();
    GILS_RVND(Imax, Iils, info);
    auto t2 = high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();

    double res = (double)duration / 10e2;
    std::cout << "TIME: " << res << std::endl;

    return 0;
}

