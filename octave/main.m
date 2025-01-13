mainn()

function print_s(s)
    sz = size(s);
    sz = sz(2);

    for i = 1:sz
        fprintf('%d ', s(i))
    end
    fprintf('\n')
end

function ret = subseq_load (sol, info)
    tern1 = [1, 0];
    tern2 = [0, 1];
    for i = 1:info.dimen+1
        k = 1 - i - tern1((i ~= 1) + 1);

        sol.seq(i, i, info.T) = 0.0;
        sol.seq(i, i, info.C) = 0.0;
        sol.seq(i, i, info.W) = tern2((i ~= 1) + 1);
        for j = i+1:info.dimen+1
            j_prev = j-1;

            sol.seq(i, j, info.T) = info.cost(sol.s(j_prev), sol.s(j)) + sol.seq(i, j_prev, info.T);
            sol.seq(i, j, info.C) = sol.seq(i, j, info.T) + sol.seq(i, j_prev, info.C);
            sol.seq(i, j, info.W) = j+k;

        end
    end

    sol.cost = sol.seq(1, info.dimen+1, info.C);
    ret = sol;
end

function arr = sort(arr, r, info)
    sz = size(arr);
    sz = sz(2);
    arr = quicksort(arr, 1, sz, r, info);
end

function arr = quicksort(arr, left, right, r, info)
    if left < right
        [arr, pivotIndex] = partition(arr, left, right, r, info);
        arr = quicksort(arr, left, pivotIndex - 1, r, info);
        arr = quicksort(arr, pivotIndex + 1, right, r, info);
    end
end

function [arr, pivotIndex] = partition(arr, left, right, r, info)
    pivotValue = arr(right);
    i = left - 1;
    for j = left:right - 1
        if info.cost(r, arr(j)) < info.cost(r, pivotValue)
            i = i + 1;
            % Troca arr(i) e arr(j)
            [arr(i), arr(j)] = deal(arr(j), arr(i));
        end
    end
    % Troca o pivô
    [arr(i + 1), arr(right)] = deal(arr(right), arr(i + 1));
    pivotIndex = i + 1;
end

function [ret, index_new] = construction(alpha, info)
    s(1) = 1;
    for i = 1:info.dimen-1
        cL(i) = i+1;
    end

    r = 1;
    size_cL  = size(cL);
    size_cL = size_cL(2);
    while (size_cL > 0) 
        cL = sort(cL, r, info);

        
        index = info.rnd(info.rnd_index) + 1;
        info.rnd_index = info.rnd_index+1;

        cN = cL(index);

        cL(index) = [];
        size_cL = size_cL - 1;
        size_s = size(s);
        size_s = size_s(2);
        s(size_s+1) = cN;
        r = cN;
    end

    size_s = size(s);
    size_s = size_s(2);
    s(size_s+1) = 1;
    ret = s;
    index_new = info.rnd_index;
end

function ret = swap(s, i, j)
    tmp = s(i);
    s(i) = s(j);
    s(j) = tmp;

    ret = s;
    end

function ret = reverse(s, i, j)
    s(i:j) = flip(s(i:j));
    ret = s;
end

function ret = reinsert(s, i, j, pos)
    if (i < pos)
        tmp = s(i:j);
        inter = s(j+1:pos-1);
        s(i:i+length(inter)-1) = inter;
        s(i+length(inter):pos-1) = tmp;
    else
        tmp = s(i:j);
        inter = s(pos:i-1);
        s(pos:pos+length(tmp)-1) = tmp;
        s(pos+length(tmp):j) = inter;
    end

    ret = s;
end

function [solut_new, ret] = search_swap(solut, info)
    cost_best = info.fmax;
    cost_new = 0.0;
    cost_concat_1 = 0.0;
    cost_concat_2 = 0.0;
    cost_concat_3 = 0.0;
    cost_concat_4 = 0.0;

    for i = 2:info.dimen-1
        i_prev = i-1;
        i_next = i+1;

        cost_concat_1 =                 solut.seq(1, i_prev, info.T) + info.cost(solut.s(i_prev), solut.s(i_next));
        cost_concat_2 = cost_concat_1 + solut.seq(i, i_next, info.T) + info.cost(solut.s(i), solut.s(i_next+1));

        cost_new = solut.seq(1, i_prev, info.C)                                                    + ...
                solut.seq(i, i_next, info.W)               * (cost_concat_1) + info.cost(solut.s(i_next), solut.s(i))  + ...
                solut.seq(i_next+1, info.dimen+1, info.W)   * (cost_concat_2) + solut.seq(i_next+1, info.dimen+1, info.C);

        if (cost_new < cost_best)
            cost_best = cost_new - eps;
            I_best = i;
            J_best = i_next;
        end

        for j = i_next+1:info.dimen

            j_prev = j-1;
            j_next = j+1;


            cost_concat_1 =                 solut.seq(1, i_prev, info.T)       + info.cost(solut.s(i_prev), solut.s(j));
            cost_concat_2 = cost_concat_1                           + info.cost(solut.s(j), solut.s(i_next));
            cost_concat_3 = cost_concat_2 + solut.seq(i_next, j_prev, info.T)  + info.cost(solut.s(j_prev), solut.s(i));
            cost_concat_4 = cost_concat_3                           + info.cost(solut.s(i), solut.s(j_next));


            cost_new = solut.seq(1, i_prev, info.C)                                                 + ...     % 1st subseq
                    cost_concat_1 + ...                                                           % concat 2nd subseq (single node)
                    solut.seq(i_next, j_prev, info.W)      * cost_concat_2 + solut.seq(i_next, j_prev, info.C) + ...    % concat 3rd subseq
                    cost_concat_3 + ...                                                           % concat 4th subseq (single node)
                    solut.seq(j_next, info.dimen+1, info.W) * cost_concat_4 + solut.seq(j_next, info.dimen+1, info.C);   % concat 5th subseq

            if (cost_new < cost_best)
                cost_best = cost_new - eps;
                I_best = i;
                J_best = j;
            end

        end
        
    end

    if (cost_best < solut.cost - eps)
        %cost_best
        solut.s = swap(solut.s, I_best, J_best);
        solut = subseq_load(solut, info);
        %solut.cost
        solut_new = solut;
        ret = true;
    else
        solut_new = solut;
        ret = false;
    end

end

function [solut_new, ret] = search_two_opt(solut, info)

    cost_best = info.fmax;
    cost_new = 0.0;
    cost_concat_1 = 0.0;
    cost_concat_2 = 0.0;

    for i = 2:info.dimen-1
        i_prev = i-1;
        rev_seq_cost = solut.seq(i, i+1, info.T);

        for j = i+2:info.dimen
            j_next = j+1;

            rev_seq_cost = rev_seq_cost + info.cost(solut.s(j-1), solut.s(j)) * (solut.seq(i, j, info.W)-1.0);

            cost_concat_1 =                 solut.seq(1, i_prev, info.T)   + info.cost(solut.s(j), solut.s(i_prev));
            cost_concat_2 = cost_concat_1 + solut.seq(i, j, info.T)        + info.cost(solut.s(j_next), solut.s(i));

            cost_new = solut.seq(1, i_prev, info.C)                                                        + ... %   1st subseq
                    solut.seq(i, j, info.W)                * cost_concat_1 + rev_seq_cost                  + ... % concat 2nd subseq (reversed seq)
                    solut.seq(j_next, info.dimen+1, info.W) * cost_concat_2 + solut.seq(j_next, info.dimen+1, info.C);       % concat 3rd subseq

            if (cost_new < cost_best)
                cost_best = cost_new - eps;
                I_best = i;
                J_best = j;
            end
        end
    end

    if (cost_best < solut.cost - eps)
        solut.s = reverse(solut.s, I_best, J_best);
        solut = subseq_load(solut, info);
        solut_new = solut;
        ret = true;
    else
        solut_new = solut;
        ret = false;
    end

end

function [solut_new, ret] = search_reinsertion(solut, info, opt)
    cost_best = info.fmax;
    cost_new = 0.0;
    cost_concat_1 = 0.0;
    cost_concat_2 = 0.0;
    cost_concat_3 = 0.0;

    for i = 2:info.dimen-opt+1
        j = opt+i-1;
        i_prev = i-1;
        j_next = j+1;

        for k = 1:i_prev-1
            k_next = k+1;

            cost_new =  solut.seq(1, info.dimen+1, info.C);
            cost_concat_1 =                 solut.seq(1, k, info.T)            + info.cost(solut.s(k), solut.s(i));
            cost_concat_2 = cost_concat_1 + solut.seq(i, j, info.T)            + info.cost(solut.s(j), solut.s(k_next));
            cost_concat_3 = cost_concat_2 + solut.seq(k_next, i_prev, info.T)  + info.cost(solut.s(i_prev), solut.s(j_next));

            cost_new = solut.seq(1, k, info.C)                                                             + ... %       1st subseq
                    solut.seq(i, j, info.W)                * cost_concat_1 + solut.seq(i, j, info.C)                  + ... % concat 2nd subseq (reinserted seq)
                    solut.seq(k_next, i_prev, info.W)      * cost_concat_2 + solut.seq(k_next, i_prev, info.C)        + ... % concat 3rd subseq
                    solut.seq(j_next, info.dimen+1, info.W) * cost_concat_3 + solut.seq(j_next, info.dimen+1, info.C);      % concat 4th subseq

            if (cost_new < cost_best)
                cost_best = cost_new - eps;
                I_best = i;
                J_best = j;
                POS_best = k;
            end

        end

        for k = i+opt:info.dimen
            k_next = k+1;

            cost_concat_1 =                 solut.seq(1, i_prev, info.T)   + info.cost(solut.s(i_prev), solut.s(j_next));
            cost_concat_2 = cost_concat_1 + solut.seq(j_next, k, info.T)   + info.cost(solut.s(k), solut.s(i));
            cost_concat_3 = cost_concat_2 + solut.seq(i, j, info.T)        + info.cost(solut.s(j), solut.s(k_next));

            cost_new = solut.seq(1, i_prev, info.C)                                                        + ... %       1st subseq
                    solut.seq(j_next, k, info.W)           * cost_concat_1 + solut.seq(j_next, k, info.C)             + ... % concat 2nd subseq
                    solut.seq(i, j, info.W)                * cost_concat_2 + solut.seq(i, j, info.C)                  + ... % concat 3rd subseq (reinserted seq)
                    solut.seq(k_next, info.dimen+1, info.W) * cost_concat_3 + solut.seq(k_next, info.dimen+1, info.C);      % concat 4th subseq

            if (cost_new < cost_best)
                cost_best = cost_new - eps;
                I_best = i;
                J_best = j;
                POS_best = k;
            end
        end
    end

    if (cost_best < solut.cost)
        solut.s = reinsert(solut.s, I_best, J_best, POS_best+1);
        solut = subseq_load(solut, info);
        solut_new = solut;
        ret = true;
    else
        solut_new = solut;
        ret = false;
    end

end

function [s, cost, index_new] = RVND(solut, info)

    neighbd_list = [info.SWAP, info.TWO_OPT, info.REINSERTION, info.OR_OPT_2, info.OR_OPT_3];
    while (isempty(neighbd_list) == false)
        index = info.rnd(info.rnd_index) + 1;
        info.rnd_index = info.rnd_index + 1;

        neighbd = neighbd_list(index);

        improve_flag = false;
        switch (neighbd)
            case info.REINSERTION
                [solut, improve_flag] = search_reinsertion(solut, info, info.REINSERTION);
            case info.OR_OPT_2
                [solut, improve_flag] = search_reinsertion(solut, info, info.OR_OPT_2);
            case info.OR_OPT_3
                [solut, improve_flag] = search_reinsertion(solut, info, info.OR_OPT_3);
            case info.SWAP
                [solut, improve_flag] = search_swap(solut, info);
            case info.TWO_OPT
                [solut, improve_flag] = search_two_opt(solut, info);
        end

        if (improve_flag)
            neighbd_list = [info.SWAP, info.TWO_OPT, info.REINSERTION, info.OR_OPT_2, info.OR_OPT_3];
            index;
        else
            neighbd_list(index) = [];
        end
    end

    s = solut.s;
    cost = solut.cost;
    index_new = info.rnd_index;
end

function [ret, index_new] = perturb(solut, info)
    A_start = 1;
    A_end = 1;
    B_start = 1;
    B_end = 1;

    while ((A_start <= B_start && B_start <= A_end) || (B_start <= A_start && A_start <= B_end))

        A_start = info.rnd(info.rnd_index) + 1;
        info.rnd_index = info.rnd_index + 1;
        A_end = A_start + info.rnd(info.rnd_index);
        info.rnd_index = info.rnd_index + 1;

        B_start = info.rnd(info.rnd_index) + 1;
        info.rnd_index = info.rnd_index + 1;
        B_end = B_start + info.rnd(info.rnd_index);
        info.rnd_index = info.rnd_index + 1;
    end

    if (A_start < B_start)
        solut.s = reinsert(solut.s, B_start, B_end - 1, A_end);
        solut.s = reinsert(solut.s, A_start, A_end - 1, B_end);
    else
        solut.s = reinsert(solut.s, A_start, A_end - 1, B_end);
        solut.s = reinsert(solut.s, B_start, B_end - 1, A_end);
    end

    ret = solut.s;
    index_new = info.rnd_index;
end

function ret = solut_init(info)
    s.s = zeros(info.dimen+1);
    s.seq = zeros(info.dimen+1, info.dimen+1, 3);
    s.cost = Inf(1);

    ret = s;
end

function ret = GILS_RVND(Imax, Iils, R, info)
    solut_crnt = solut_init(info);
    solut_partial = solut_init(info);
    solut_best = solut_init(info);

    for i = 1:Imax
        index = info.rnd(info.rnd_index) + 1;
        info.rnd_index = info.rnd_index + 1;
        alpha = R(index);

        fprintf("[+] Search %d\n", i)
        fprintf("\t[+] Constructing..\n");

        [solut_crnt.s, info.rnd_index] = construction(alpha, info);
        solut_crnt = subseq_load(solut_crnt, info);

        solut_partial = solut_crnt;
        fprintf("\t[+] Looking for the best Neighbor..\n")
        fprintf("\t    Construction Cost: %.2f\n", solut_partial.cost)

        iterILS = 0;
        while (iterILS < Iils)
            [solut_crnt.s, solut_crnt.cost, info.rnd_index] = RVND(solut_crnt, info);

            if (solut_crnt.cost < solut_partial.cost - eps)
                solut_partial.s = solut_crnt.s;
                solut_partial.cost = solut_crnt.cost;
                iterILS = 0;
            end

            [solut_crnt.s, info.rnd_index] = perturb(solut_partial, info);
            solut_crnt = subseq_load(solut_crnt, info);
            iterILS = iterILS + 1;
        end

        if (solut_partial.cost < solut_best.cost)
            solut_best = solut_partial;
        end

        fprintf("\tCurrent best cost: %.2f\n", solut_best.cost)
        fprintf('SOLUCAO: ')
        print_s(solut_best.s)
    end

    fprintf('COST: %.2f\n', solut_best.cost)

    ret = solut_best;
end

function mainn
    [info.dimen, info.cost, info.rnd] = Data();
    info;
    info.fmax = Inf(1);
    info.T = 1;
    info.C = 2;
    info.W = 3;
    info.REINSERTION = 1;
    info.OR_OPT_2 = 2;
    info.OR_OPT_3 = 3;
    info.SWAP = 4;
    info.TWO_OPT = 5;
    info.rnd_index = 1;

    Imax = 10;
    Iils = 100;
    if info.dimen < 100
        Iils = info.dimen;
    end

    R = [0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.20, 0.21, 0.22, 0.23, 0.24, 0.25, 0.26];
    t0 = clock ();
    s = GILS_RVND(Imax, Iils, R, info);
    elapsed_time = etime (clock (), t0);
    fprintf('TIME: %.4f\n', elapsed_time)
    s.s;
    exit
end
