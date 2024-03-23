
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% calculate matrices for least-squares %%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [inv_GTWG, S] = calc_ls_matrices( ...
    N_sat, N_const, N_ss, ...
    W, G, ...
    ss_sat_mat, ss_const_mat)

% Description:
%   Compute the least-squares matrices for each subset.

% Arguments:
%   N_sat:        Integer number of satellites in view.
%   N_const:      Integer number of constellations.
%   N_ss:         Integer number of subsets for which to compute matrices,
%                 excluding the all-in-view set.
%   W:            N_sat*N_sat weighting matrix for all the satellites.
%   G:            N_sat*(3+N_const) geometry matrix for all the satellites.
%   ss_sat_mat:   N_ss*N_sat matrix with each row as an activation string
%                 for the satellite vehicles in the corresponding subset.
%   ss_const_mat: N_ss*N_const matrix with each row as an activation string
%                 for the constellations active in the corresponding
%                 subset.

% Return values:
%   inv_GTWG: List of N_ss (3+N_const)*(3+N_const) matrices holding the
%             inverted matrix: (G^T W^(k) G)^-1.
%   S:        List of N_ss+1 (3+N_const)*N_sat matrices holding the total
%             least-squares matrix: (G^T W^(k) G)^-1 G^T W^(k). The last
%             matrix corresponds to the all-in-view set.

% placeholders
inv_GTWG = zeros(3+N_const,3+N_const,N_ss);
S = zeros(3+N_const,N_sat,N_ss+1);

% compute the all-in-view matrix
S(:,:,N_ss+1) = inv(G' * W * G) * G' * W;

% compute for each subset
for k = 1:N_ss
    % determine indices of the satellite vehicles in the subset
    not_zero_sat = find(ss_sat_mat(k,:)>0);
    not_zero_const = [ 1 2 3 (find(ss_const_mat(k,:)>0) + 3) ];

    % exclude satellites and constellations
    this_G = G(not_zero_sat,not_zero_const);
    this_W = W(not_zero_sat,not_zero_sat);

    % perform multiplications
    this_inv_GTWG = inv(this_G' * this_W * this_G);

    % output
    inv_GTWG(not_zero_const,not_zero_const,k) = this_inv_GTWG;
    S(not_zero_const,not_zero_sat,k) = this_inv_GTWG * this_G' * this_W;
end

end
