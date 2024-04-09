
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% calculate initial matrices %%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [C_int, C_acc, W, W_acc, G] = init_matrices( ...
    N_sat, N_const, ...
    sig_URA2, sig_URE2, sig_tropo2, sig_user2, ...
    init_G, consts)

% Description:
%   Compute the initial covariance and weighting matrices for ARAIM.

% Arguments:
%   N_sat:      Integer number of satellites in view.
%   N_const:    Integer number of constellations.
%   sig_URA2:   N_sat*1 vector of variances of the clock and ephemeris
%               error for each satellite vehicle used for integrity.
%   sig_URE2:   N_sat*1 vector of variances of the clock and ephemeris
%               error for each satellite vehicle used for accuracy and
%               continuity.
%   sig_tropo2: N_sat*1 vector of variances of the tropospheric delay for
%               each satellite vehicle.
%   sig_user2:  N_sat*1 vector of variances of the user delay based on
%               line-of-sight for each satellite vehicle.
%   init_G:     N_sat*3 matrix containing normalized line-of-sight vectors
%               to each satellite vehicle.
%   consts:     N_sat*1 vector containing the index of the constellation
%               each satellite vehicle belongs to.

% Return values:
%   C_int: N_sat*N_sat covariance matrix for integrity.
%   C_acc: N_sat*N_sat covariance matrix for accuracy and continuity.
%   W:     N_sat*N_sat weighting matrix of the satellites used for
%          integrity.
%   W_acc: N_sat*N_sat weighting matrix of the satellites used for
%          accuracy and continuity.
%   G:     N_sat*(3+N_const) matrix extending the geometry matrix to
%          include the activation string for the satellite constellations.

% calculate diagonals
C_int_diag = sig_URA2 + sig_tropo2 + sig_user2;
C_acc_diag = sig_URE2 + sig_tropo2 + sig_user2;

% convert diagonals to matrices
% C_int(i,i) = sig_URA_i^2 + sig_tropo_i^2 + sig_user_i^2
C_int = diag(C_int_diag);
% C_acc(i,i) = sig_URE_i^2 + sig_tropo_i^2 + sig_user_i^2
C_acc = diag(C_acc_diag);
% W = C_int^-1 => W(i,i) = 1 / C_int(i,i)
W     = diag(1.0 ./ C_int_diag);
% W_acc = C_acc^-1 => W_acc(i,i) = 1 / C_acc(i,i)
W_acc = diag(1.0 ./ C_acc_diag);

% construct extended geometry matrix
G = zeros(N_sat,3+N_const);
G(:,1:3) = init_G; % copy LOS vectors
for i = 1:N_sat
    const_idx = 3+1+consts(i);
    G(i,const_idx) = 1.0; % activate corresponding constellation
end

end
