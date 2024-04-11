
function [fd, faulty_sv] = compute_tests( ...
    N_sat, N_const, N_ss, ...
    var_pos, bias_pos, var_ss_pos, ...
    C_int, SS, y, ...
    K_fa, K_fa_r)

% Description:
%   Compute the global and local tests for fault detection and exclusion.

% Arguments:
%   N_sat:      Integer number of satellites in view.
%   N_const:    Integer number of constellations.
%   N_ss:       Integer number of subsets for which to compute matrices,
%               excluding the all-in-view set.
%   var_pos:    N_ss*3 matrix with the variances of the subset solution
%               coordinates. var_pos(k,q) is the variance of the q-th
%               coordinate for the k-th subset.
%   bias_pos:   N_ss*3 matrix with the biases on each subset solution
%               coordinate. bias_pos(k,q) is the bias on the q-th
%               coordinate for the k-th subset.
%   var_ss_pos: N_ss*3 matrix with the variances of the subset solution
%               separations. var_ss_pos(k,q) is the variance of the q-th
%               coordinate for the k-th subset solution separation.
%   SS:         N_ss (3+N_const)*N_sat matrices computed as the weighted
%               least-squares matrices.
%   y:          N_sat*1 matrix with the pseudorange residuals for each SV.
%   K_fa:       3*1 matrix with thresholds for each ENU coordinate.
%   K_fa_r:     Threshold for fault location for individual satellites.

% Return values:
%   fd:        N_ss*1 matrix with 0's for subsets without a detected fault
%              and 1's for subsets with a detected fault.
%   faulty_sv: N_sat*1 matrix with 0's for non-faulty satellites and 1's
%              for faulty satellites.

fd = zeros(N_ss, 1);
faulty_sv = zeros(N_sat, 1);

% global test
for k = 1:N_ss
    ssy = abs(SS(:,:,k) * y);
    for q = 1:3
        if ssy(q) > var_ss_pos(k,q) * K_fa(q,1)
            fd(k) = 1;
        end
    end
end

% local test
if fd
    for i = 1:N_sat
        if abs(y(i)) > sqrt(C_int(i,i)) * K_fa_r
            faulty_sv(i) = 1;
        end
    end
end

end
