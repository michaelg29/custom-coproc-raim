
%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% main algorithm %%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%
function [pos_vars, nominal_bias_impact, sol_sep_vars] = alg( ...
    N_sat, N_const, N_ss, ...
    sig_URA2, sig_URE2, sig_tropo2, sig_user2, ...
    init_G, consts, ...
    ss_sat_mat, ss_const_mat)

% Description:
%   Compute the ARAIM steps targeted to the co-processor.

% Arguments:
% --configuration
%   N_sat:        Integer number of satellites in view.
%   N_const:      Integer number of constellations.
%   N_ss:         Integer number of subsets for which to compute solutions.
% --variance
%   sig_URA2:     N_sat*1 vector of variances of the clock and ephemeris
%                 error for each satellite vehicle used for integrity.
%   sig_URE2:     N_sat*1 vector of variances of the clock and ephemeris
%                 error for each satellite vehicle used for accuracy and
%                 continuity.
%   sig_tropo2:   N_sat*1 vector of variances of the tropospheric delay for
%                 each satellite vehicle.
%   sig_user2:    N_sat*1 vector of variances of the user delay based on
%                 line-of-sight for each satellite vehicle.
% --geometry
%   init_G:       N_sat*3 matrix containing normalized line-of-sight
%                 vectors to each satellite vehicle.
%   consts:       N_sat*1 vector containing the index of the constellation
%                 each satellite vehicle belongs to.
% --subsets
%   ss_sat_mat    N_ss*N_sat matrix with each row as an activation string
%                 for the satellite vehicles in the corresponding subset.
%   ss_const_mat: N_ss*N_const matrix with each row as an activation string
%                 for the constellations active in the corresponding
%                 subset.


% Return values:
%   pos_vars:            N_ss*3 matrix of the variances of the computed
%                        solutions for each subset for the East, North,
%                        and Up components.
%   nominal_bias_impact: N_ss*3 matrix worst case impact of the nominal
%                        biases on the subset position solutions.
%   sol_sep_vars:        N_ss*3 matrix of the variances of the solution
%                        separations for each subset for the East, North,
%                        and Up components.

% calculate initial matrices
[C_int, C_acc, W, G] = init_matrices( ...
    N_sat, N_const, ...
    sig_URA2, sig_URE2, sig_tropo2, sig_user2, ...
    init_G, consts);

% compute least-squares matrix for each subset
[S] = calc_ls_matrices( ...
    N_sat, N_const, N_ss, ...
    W, G, ...
    ss_sat_mat, ss_const_mat);

pos_vars = zeros(N_ss,3);
nominal_bias_impact = zeros(N_ss,3);
sol_sep_vars = zeros(N_ss,3);

end
