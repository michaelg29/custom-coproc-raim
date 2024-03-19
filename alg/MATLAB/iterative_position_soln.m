
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% iteratively calculate a position solution %%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [x_conv, resid] = iterative_position_soln( ...
    N_sat, max_n_it, ...
    LS_mat, prs, xhat_0, x_SVs)

% Description:
%   Iteratively calculate an updated LS solution until convergence.

% Arguments:
%   max_n_it: Integer maximum number of iterations.
%   LS_mat:
%   prs:
%   xhat_0:
%   x_SVs:

% Return values:
%   x_conv:
%   resid:

% initial guess
xhat = xhat_0;

delta = zeros(max_n_it:4);
for i = 1:max_n_it
    del_prs = zeros(N_sat,1);
    for j = 1:N_sat
        % pseudorange estimate - expected distance from SV to linearized point
        del_prs(j) = prs(j) - norm(x_SVs(j,1:3) - xhat(1:3)');
    end

    % get update
    del_x = LS_mat * del_prs;
    if norm(del_x) < 20
        break;
    end

    delta(i,1:4) = transpose(del_x);
    xhat = del_x + xhat;
end
delta

x_conv = xhat;
norm(x_conv(1:3))
resid = zeros(N_sat,1);
for j = 1:N_sat
    % pseudorange estimate - expected distance from SV to linearized point
    resid(j) = prs(j) - norm(x_SVs(j,1:3) - transpose(xhat(1:3)));
end

end
