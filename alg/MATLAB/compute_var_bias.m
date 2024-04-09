
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% calculate variances and biases for subsets %%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [var_pos, bias_pos, var_ss_pos] = compute_var_bias( ...
    N_sat, N_const, N_ss, ...
    W, W_acc, S, ...
    b_nom)

% Arguments:
%   N_sat:    Integer number of satellites in view.
%   N_const:  Integer number of constellations.
%   N_ss:     Integer number of subsets for which to compute matrices,
%             excluding the all-in-view set.
%   W:        N_sat*N_sat weighting matrix of the satellites used for
%             integrity.
%   W_acc:    N_sat*N_sat weighting matrix of the satellites used for
%             accuracy and continuity.
%   S:        N_ss+1 (3+N_const)*N_sat matrices computed as the weighted
%             least-squares matrices. S(0,:,:) is the all-in-view LS matrix.
%   b_nom:    N_sat*1 matrix with the nominal biases for each SV.

% Return values:
%   var_pos:    N_ss*3 matrix with the variances of the subset solution
%               coordinates. var_pos(k,q) is the variance of the q-th
%               coordinate for the k-th subset.
%   bias_pos:   N_ss*3 matrix with the biases on each subset solution
%               coordinate. bias_pos(k,q) is the bias on the q-th
%               coordinate for the k-th subset.
%   var_ss_pos: N_ss*3 matrix with the variances of the subset solution
%               separations. var_ss_pos(k,q) is the variance of the q-th
%               coordinate for the k-th subset solution separation.

    var_pos = zeros(N_ss,3);
    bias_pos = zeros(N_ss,3);
    var_ss_pos = zeros(N_ss,3);

    for k = 1:N_ss
        for q = 1:3
            % position bias
            bias_pos(k,q) = 0;
            for i = 1:N_sat
                bias_pos(k,q) = bias_pos(k,q) + abs(S(q,i,k+1)) * b_nom(i);
            end
        end

        var_pos(k,:) = compute_pos_variance(S(:,:,k+1), diag(W), N_sat);
        var_ss_pos(k,:) = compute_pos_variance((S(:,:,k+1) - S(:,:,1)), diag(W_acc), N_sat);
    end
end

function [var] = compute_pos_variance(S, W_diag, N_sat)

    var = zeros(1, 3);

    W_inv = diag(1 ./ sqrt(W_diag));
    SPR = S * W_inv;

    for q = 1:3
        for i = 1:N_sat
            var(1,q) = var(1,q) + SPR(q,i) * SPR(q,i);
        end
    end

end
