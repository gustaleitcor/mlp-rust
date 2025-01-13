import java.util.ArrayList;
import java.util.Random;
import java.util.Collections;
import java.lang.Math;

class GILS_RVND {

    // parameters
    private int                     Iils;
    private final int               Imax = 10;
    private static final double []  R = {0.0, 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12, 
                                        0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20, 0.21, 0.22, 0.23, 0.24, 0.25};
    private final int               R_size = 26;

    tInfo info;

    GILS_RVND(){
        Data data = new Data();
        data.loadData();

        int dimension = data.getDimension();
        double [][] c = new double [dimension][dimension];
        for(int i = 0; i < dimension; i++){
            for(int j = i; j < dimension; j++){
                c[i][j] = data.getDistance(i, j);
                c[j][i] = data.getDistance(i, j);
            }
        }
        int [] rnd = data.getRnd();
        info = new tInfo(dimension, c, rnd);

        Iils = (dimension < 100 ? dimension : 100);

    }

    private void subseq_load(tSolution solut, tInfo info){
        int dimension = info.getDimen();
        for(int i = 0; i < dimension+1; i++){
            int k = 1 - i - (i != 0 ? 0 : 1);

            solut.setSeq(i, i, info.T, 0.0);
            solut.setSeq(i, i, info.C, 0.0);
            solut.setSeq(i, i, info.W, (i != 0 ? 1.0 : 0.0));

            for(int j = i+1; j < dimension+1; j++){
                int j_prev = j-1;

                solut.setSeq(i, j, info.T, info.getCost(solut.getSolut().get(j_prev), solut.getSolut().get(j)) + solut.getSeq(i, j_prev, info.T));
                solut.setSeq(i, j, info.C, solut.getSeq(i, j, info.T) + solut.getSeq(i, j_prev, info.C));
                solut.setSeq(i, j, info.W, j + k);

            }
        }
        
        solut.setCost(solut.getSeq(0, info.getDimen(), info.C));
    }

    public void sort(ArrayList<Integer> arr, int r, tInfo info) {
        quicksort(arr, 0, arr.size() - 1, info, r);
    }

    private static void quicksort(ArrayList<Integer> arr, int left, int right, tInfo info, int r) {
        if (left < right) {
            int pivot = partition(arr, left, right, info, r);
            quicksort(arr, left, pivot - 1, info, r);
            quicksort(arr, pivot + 1, right, info, r);
        }
    }

    private static int partition(ArrayList<Integer> arr, int left, int right, tInfo info, int r) {
        int pivot = arr.get(right);
        int i = left - 1;
        for (int j = left; j < right; j++) {
            if (info.getCost(r, arr.get(j)) < info.getCost(r, pivot)) {
                i++;
                int temp = arr.get(i);
                arr.set(i, arr.get(j));
                arr.set(j, temp);
            }
        }
        int temp = arr.get(i + 1);
        arr.set(i + 1, arr.get(right));
        arr.set(right, temp);
        return i + 1;
    }

    private ArrayList<Integer> construction(double alpha, tInfo info){
        ArrayList<Integer> s = new ArrayList<Integer>();
        s.add(0);

        ArrayList<Integer> cList = new ArrayList<Integer>();
        int dimension = info.getDimen();
        for(int i = 1; i < dimension; i++){
            cList.add(i);
        }

        int r = 0;
        while(!cList.isEmpty()){
            sort(cList, r, info);

            int range = (int)(((double)cList.size()) * alpha)+1;

            int index = info.rndCrnt();
            int c = cList.get(index);

            s.add(c);
            cList.remove(Integer.valueOf(c));
            r = c;
        }

        s.add(0);
        return s;
    }

    private void swap(ArrayList<Integer> s, int i, int j){
        Collections.swap(s, i, j);
    }

    private void reverse(ArrayList<Integer> s, int i, int j){
        Collections.reverse(s.subList(i,j+1));
    }

    private void reinsert(ArrayList<Integer> s, int i, int j, int pos){
        if(i > pos){
            int sz = j-i+1;
            s.addAll(pos, s.subList(i, j+1));
            s.subList(i+sz, j+1+sz).clear();
        }else{
            s.addAll(pos, s.subList(i, j+1));
            s.subList(i, j+1).clear();
        }

    }

    private boolean search_swap(tSolution solut, tInfo info){
        double cost_best = Double.MAX_VALUE;
        double cost_new;
        double cost_concat_1;
        double cost_concat_2;
        double cost_concat_3;
        double cost_concat_4;
        int I = -1;
        int J = -1;

        int dimension = info.getDimen();
        
        for(int i = 1; i < dimension-1; i++){
            int i_prev = i - 1;
            int i_next = i + 1;
            int s_i_prev = solut.getSolut().get(i_prev);
            int s_i_next = solut.getSolut().get(i_next);
            int s_i = solut.getSolut().get(i);

            // immediate nodes case

            cost_concat_1 =                 solut.getSeq(0, i_prev, info.T) + info.getCost(s_i_prev, s_i_next);
            cost_concat_2 = cost_concat_1 + solut.getSeq(i, i_next, info.T) + info.getCost(s_i, solut.getSolut().get(i_next+1));

            cost_new = solut.getSeq(0, i_prev, info.C)                                                            // 1st subseq 
                    + solut.getSeq(i, i_next, info.W)           * (cost_concat_1) + info.getCost(s_i_next, s_i)              // concat 2nd subseq
                    + solut.getSeq(i_next+1, dimension , info.W) * (cost_concat_2) + solut.getSeq(i_next+1, dimension, info.C);  // concat 3rd subseq

            if(cost_new < cost_best){
                cost_best = cost_new - info.EPSILON;
                I = i;
                J = i_next;
            }

            for(int j = i_next+1; j < dimension; j++){
                int j_next = j+1;
                int j_prev = j-1;
                int s_j = solut.getSolut().get(j);
                int s_j_prev = solut.getSolut().get(j_prev);
                int s_j_next = solut.getSolut().get(j_next);

                cost_concat_1 = solut.getSeq(0, i_prev, info.T) + info.getCost(s_i_prev, s_j);
                cost_concat_2 = cost_concat_1 + info.getCost(s_j, s_i_next);
                cost_concat_3 = cost_concat_2 + solut.getSeq(i_next, j_prev, info.T) + info.getCost(s_j_prev, s_i);
                cost_concat_4 = cost_concat_3 + info.getCost(s_i, s_j_next);

                cost_new = solut.getSeq(0, i_prev, info.C)                                                        /* first subseq */
                        + cost_concat_1                                                             /* concatenate second subseq (single node) */
                        + solut.getSeq(i_next, j_prev, info.W) * cost_concat_2 + solut.getSeq(i_next, j_prev, info.C)           /* concatenate third subseq */
                        + cost_concat_3                                                             /* concatenate fourth subseq (single node) */
                        + solut.getSeq(j_next, dimension, info.W) * cost_concat_4 + solut.getSeq(j_next, dimension, info.C);    /* concatenate fifth subseq */

                if(cost_new < cost_best){
                    cost_best = cost_new - info.EPSILON;
                    I = i;
                    J = j;
                }
            }
        }

        if(cost_best < solut.getCost()){
            swap(solut.getSolut(), I, J);
            subseq_load(solut, info);
            return true;
        }

        return false;
    } 

    private boolean search_two_opt(tSolution solut, tInfo info){
        int I = -1;
        int J = -1;
        double cost_best = Double.MAX_VALUE;
        double cost_new;
        double cost_concat_1;
        double cost_concat_2;
        int dimension = info.getDimen();

        for(int i = 1; i < dimension-1; i++){
            int i_prev = i -1;
            int s_i = solut.getSolut().get(i);
            int s_i_prev = solut.getSolut().get(i_prev);
            double rev_seq_cost = solut.getSeq(i, i+1, info.T);

            for(int j = i+2; j < dimension; j++){
                int j_next = j+1;
                int s_j_next = solut.getSolut().get(j_next);
                int s_j_prev = solut.getSolut().get(j-1);
                int s_j = solut.getSolut().get(j);

                rev_seq_cost += info.getCost(s_j_prev, s_j) * (solut.getSeq(i, j, info.W)-1);

                cost_concat_1 =                 solut.getSeq(0, i_prev, info.T) + info.getCost(s_j, s_i_prev);
                cost_concat_2 = cost_concat_1 + solut.getSeq(i, j, info.T) + info.getCost(s_j_next, s_i);

                cost_new = solut.getSeq(0, i_prev, info.C)                                                        /*        1st subseq */
                        + solut.getSeq(i, j, info.W)              * cost_concat_1 + rev_seq_cost                  /* concat 2nd subseq (reversed seq) */
                        + solut.getSeq(j_next, dimension, info.W) * cost_concat_2 + solut.getSeq(j_next, dimension, info.C);    /* concat 3rd subseq */

                if(cost_new < cost_best){
                    cost_best = cost_new - info.EPSILON;
                    I = i;
                    J = j;
                }
            }
        }

        if(cost_best < solut.getCost() - info.EPSILON){
            reverse(solut.getSolut(), I, J);
            subseq_load(solut, info);
            return true;
        }

        return false;
    } 

    private boolean search_reinsertion(tSolution solut, tInfo info, int opt){
        double cost_best = Double.MAX_VALUE;
        double cost_new;
        double cost_concat_1;
        double cost_concat_2;
        double cost_concat_3;
        int I = -1;
        int J = -1;
        int POS = -1;
        int dimension = info.getDimen();

        for(int i = 1; i < dimension - opt + 1; i++){
            int j = opt + i - 1;
            int i_prev = i-1;
            int j_next = j+1;
            int s_i = solut.getSolut().get(i);
            int s_j = solut.getSolut().get(j);
            int s_i_prev = solut.getSolut().get(i_prev);
            int s_j_next = solut.getSolut().get(j_next);

            // k -> reinsertion places
            for(int k = 0; k < i_prev; k++){
                int k_next = k+1;
                int s_k = solut.getSolut().get(k);
                int s_k_next = solut.getSolut().get(k_next);
                
                cost_concat_1 = solut.getSeq(0, k, info.T) + info.getCost(s_k, s_i);
                cost_concat_2 = cost_concat_1 + solut.getSeq(i, j, info.T) + info.getCost(s_j, s_k_next);
                cost_concat_3 = cost_concat_2 + solut.getSeq(k_next, i_prev, info.T) + info.getCost(s_i_prev, s_j_next);

                cost_new = solut.getSeq(0, k, info.C)                                                             /*        1st subseq */
                        + solut.getSeq(i, j, info.W)              * cost_concat_1 + solut.getSeq(i, j, info.C)                  /* concat 2nd subseq (reinserted seq) */
                        + solut.getSeq(k_next, i_prev, info.W)    * cost_concat_2 + solut.getSeq(k_next, i_prev, info.C)        /* concat 3rd subseq */
                        + solut.getSeq(j_next, dimension, info.W) * cost_concat_3 + solut.getSeq(j_next, dimension, info.C);    /* concat 4th subseq */

                if(cost_new < cost_best){
                    cost_best = cost_new - info.EPSILON;
                    I = i;
                    J = j;
                    POS = k;
                }
            }

            for(int k = i+opt; k < dimension; k++){
                int k_next = k+1;
                int s_k = solut.getSolut().get(k);
                int s_k_next = solut.getSolut().get(k_next);

                cost_concat_1 = solut.getSeq(0, i_prev, info.T) + info.getCost(s_i_prev, s_j_next);
                cost_concat_2 = cost_concat_1 + solut.getSeq(j_next, k, info.T) + info.getCost(s_k, s_i);
                cost_concat_3 = cost_concat_2 + solut.getSeq(i, j, info.T) + info.getCost(s_j, s_k_next);

                cost_new = solut.getSeq(0, i_prev, info.C)                                                        /*      1st subseq */
                        + solut.getSeq(j_next, k, info.W)         * cost_concat_1 + solut.getSeq(j_next, k, info.C)             /* concat 2nd subseq */
                        + solut.getSeq(i, j, info.W)              * cost_concat_2 + solut.getSeq(i, j, info.C)                  /* concat 3rd subseq (reinserted seq) */
                        + solut.getSeq(k_next, dimension, info.W) * cost_concat_3 + solut.getSeq(k_next, dimension, info.C);    /* concat 4th subseq */

                if(cost_new < cost_best){
                    cost_best = cost_new - info.EPSILON;
                    I = i;
                    J = j;
                    POS = k;
                }
            }
        }

        if(cost_best < solut.getCost()){
            reinsert(solut.getSolut(), I, J, POS+1);
            subseq_load(solut, info);
            return true;
        }

        return false;
    } 

    private void RVND(tSolution solut, tInfo info){
        ArrayList<Integer> neighbd_list = new ArrayList<Integer>() {{
            add(info.SWAP);
            add(info.TWO_OPT);
            add(info.REINSERTION);
            add(info.OR_OPT2);
            add(info.OR_OPT3);
        }};

        while(!neighbd_list.isEmpty()){
            
            int i_rand = info.rndCrnt();
            int neighbd = neighbd_list.get(i_rand);
            
            boolean improve = false;

            if (neighbd == info.REINSERTION) {
                    improve = search_reinsertion(solut, info, info.REINSERTION);
            } else if (neighbd == info.OR_OPT2) {
                    improve = search_reinsertion(solut, info, info.OR_OPT2);
            } else if (neighbd == info.OR_OPT3) {
                    improve = search_reinsertion(solut, info, info.OR_OPT3);
            } else if (neighbd == info.SWAP) {
                    improve = search_swap(solut, info);
            } else if (neighbd == info.TWO_OPT) {
                    improve = search_two_opt(solut, info);
            }
            
            if(improve){
                neighbd_list.clear();
                neighbd_list = new ArrayList<Integer>() {{
                    add(info.SWAP);
                    add(info.TWO_OPT);
                    add(info.REINSERTION);
                    add(info.OR_OPT2);
                    add(info.OR_OPT3);
                }};
            }else
                neighbd_list.remove(i_rand);
        }
    }

    private ArrayList<Integer> perturb(ArrayList<Integer> sl, tInfo info){
        ArrayList<Integer> sl_cpy = new ArrayList<>(sl); 

        int A_start = 1, A_end = 1;
        int B_start = 1, B_end = 1;

        while((A_start <= B_start && B_start <= A_end) || (B_start <= A_start && A_start <= B_end)){

            A_start = info.rndCrnt();
            A_end = A_start + info.rndCrnt();

            B_start = info.rndCrnt();
            B_end = B_start + info.rndCrnt();
        }

        if(A_start < B_start){
            reinsert(sl_cpy, B_start, B_end-1, A_end);
            reinsert(sl_cpy, A_start, A_end-1, B_end);
        }else{
            reinsert(sl_cpy, A_start, A_end-1, B_end);
            reinsert(sl_cpy, B_start, B_end-1, A_end);
        }

        return sl_cpy;
    }

    public void solve(){
        tSolution solut_best = new tSolution(info.getDimen(), Double.MAX_VALUE);
        tSolution solut_crnt = new tSolution(info.getDimen(), 0);
        tSolution solut_partial = new tSolution(info.getDimen(), 0);

        for(int i = 0; i < Imax; i++){
            int index = info.rndCrnt();
            double alpha = R[index];

            System.out.print("[+] Local Search ");
            System.out.println(i+1);
            System.out.println("\t[+] Constructing Inital Solution..");

            solut_crnt.storeSolut(construction(alpha, info));
            subseq_load(solut_crnt, info); 

            
            solut_partial.storeSolut(solut_crnt.getSolutCpy()); 
            solut_partial.setCost(solut_crnt.getCost());

            int iterILS = 0;
            System.out.println("\t[+] Looking for the best Neighbor..");
            while(iterILS < Iils){
                RVND(solut_crnt, info);
                //System.exit(1);
                if(solut_crnt.getCost() < solut_partial.getCost()){
                    solut_partial.setCost(solut_crnt.getCost());
                    solut_partial.storeSolut(solut_crnt.getSolutCpy());
                    iterILS = 0;
                }

                solut_crnt.storeSolut( perturb(solut_partial.getSolut(), info));
                subseq_load(solut_crnt, info);
                iterILS++;
            }
            
            if(solut_partial.getCost() < solut_best.getCost()){
                solut_best.storeSolut(solut_partial.getSolutCpy());
                solut_best.setCost(solut_partial.getCost());
            }

            System.out.print("\tCurrent best solution cost: ");
            System.out.println(solut_best.getCost());
        }

        System.out.print("COST: ");
        System.out.println(solut_best.getCost());
        System.out.print("SOLUTION: ");
        System.out.println(solut_best.getSolut());
    }
}
