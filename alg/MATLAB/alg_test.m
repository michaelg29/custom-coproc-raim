
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% Script to test the algorithm. %%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [] = alg_test(test)

% configuration data
N_sat = 10;
N_const = 2;
N_ss = 2;
N_sat_max = 2;
N_const_max = 1;

% assumed variances and biases
sig_URA_i2 = 0.75 * 0.75; % meters^2
sig_URE_i2 = 0.50 * 0.50; % meters^2
bias_nom_i = 0.50; % meters

% placeholder variables
init_G = ones(N_sat, 3);
consts = zeros(N_sat, 1);
sig_URA2 = zeros(N_sat, 1);
sig_URE2 = zeros(N_sat, 1);
sig_tropo2 = zeros(N_sat, 1);
sig_user2 = zeros(N_sat, 1);

% test data from Blanch et al. Appendix H
init_G(1:N_sat,1:3) = [ 0.0225  0.9951 -0.0966;
                        0.6750 -0.6900 -0.2612;
                        0.0723 -0.6601 -0.7477;
                       -0.9398  0.2553 -0.2269;
                       -0.5907 -0.7539 -0.2877;
                       -0.3236 -0.0354 -0.9455;
                       -0.6748  0.4356 -0.5957;
                        0.0938 -0.7004 -0.7075;
                        0.5571  0.3088 -0.7709;
                        0.6622  0.6958 -0.2780 ];
consts(1:5,1) = zeros(5,1);
consts(6:10,1) = ones(5,1);
ss_sat_mat = [ 1 1 1 1 1 0 0 0 0 0;
               0 0 0 0 0 1 1 1 1 1 ];
ss_const_mat = [ 1 0;
                 0 1 ];

% expected results from Blanch et al. Appendix H
C_int_exp = diag([ 3.8865 1.4377 0.8604 1.6383 1.3229 0.8434 0.8963 0.8669 0.8573 1.3616 ]);
C_acc_exp = diag([ 3.5740 1.1252 0.5479 1.3258 1.0104 0.5309 0.5838 0.5544 0.5448 1.0491 ]);
sig_fault_const1_vert_exp = 2.5760;
sig_fault_const1_sep_vert_exp = 1.5307;
bias_fault_const1_vert_exp = 3.8935;
sig_fault_const2_vert_exp = 2.5577;
sig_fault_const2_sep_vert_exp = 1.5292;
bias_fault_const2_vert_exp = 2.0875;

% calculate per-satellite variances and initial observation matrix
for i = 1:N_sat
    % copy sigma_URA2 and sigma_URE2
    sig_URA2(i) = sig_URA_i2;
    sig_URE2(i) = sig_URE_i2;

    % determine angle of elevation = atan2(z, sqrt(x^2 + y^2))
    ang = rad2deg(abs(atan2(init_G(i,3), sqrt(power(init_G(i,1),2)+power(init_G(i,2),2)))));

    % determine tropospheric delay
    sig_tropo2(i) = tropo_delay2(ang);

    % determine user delay
    if consts(i) == 0
        sig_user2(i) = user_delay2_gps(ang);
    elseif consts(i) == 1
        sig_user2(i) = user_delay2_galileo(ang);
    else
        sig_user2(i) = user_delay2_gps(ang); % default GPS
    end
    sig_user2(i) = user_delay2_gps(ang);
end

if test == "init_matrices"
    % unit test - matrix initialization
    fprintf("Running unit test on init_matrices\n");
    [C_int, C_acc, W, ~] = init_matrices( ...
        N_sat, N_const, ...
        sig_URA2, sig_URE2, sig_tropo2, sig_user2, ...
        init_G, consts);

    assert(C_int == C_int_exp);
    assert(C_acc == C_acc_exp);
    assert(W == inv(C_int_exp));
elseif test == "calc_ls_matrices"
    [C_int, C_acc, W, G] = init_matrices( ...
        N_sat, N_const, ...
        sig_URA2, sig_URE2, sig_tropo2, sig_user2, ...
        init_G, consts);

    [inv_GTWG, S] = calc_ls_matrices( ...
        N_sat, N_const, N_ss, ...
        W, G, ...
        ss_sat_mat, ss_const_mat)

    % first subset assumes fault in constellation 2
    assert(abs(sqrt(inv_GTWG(3,3,1)) - sig_fault_const2_vert_exp) < 1e-4);
    % second subset assumes fault in constellation 1
    assert(abs(sqrt(inv_GTWG(3,3,2)) - sig_fault_const1_vert_exp) < 1e-4);
else
    % integration test - call the whole algorithm
    fprintf("Running integration test on alg\n");
    [pos_vars, nominal_bias_impact, sol_sep_vars] = alg( ...
         N_sat, N_const, N_ss, ...
         sig_URA2, sig_URE2, sig_tropo2, sig_user2, ...
         init_G, consts, x_SVs, prs, xhat)
end

fprintf("Done with test\n");

end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% Helper functions %%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [delay2] = user_delay2_gps(th_i)
% Description:
%   Calculate the user delay for a GPS satellite.

% Arguments:
%   th_i: Angle of elevation in degrees to the GPS satellite.

% Return values:
%   delay2: sig_user_i^2.

delay_mp = 0.13 + 0.53 * exp(-th_i / 10);
delay_noise = 0.15 + 0.43 * exp(-th_i / 6.9);

f_l1 = 1575.42e6;
f_l5 = 1176.45e6;

delay2 = (power(f_l1,4) + power(f_l5,4))/power(power(f_l1,2)-power(f_l5,2),2) * (power(delay_mp,2) + power(delay_noise,2));
end

function [delay2] = user_delay2_galileo(th_i)
% Description:
%   Calculate the user delay for a Galileo satellite.

% Arguments:
%   th_i: Angle of elevation in degrees to the Galileo satellite.

% Return values:
%   delay2: sig_user_i^2.

% table of delays starting from th=5deg to th=90deg
delays = [ ...
    0.4529, 0.3553, 0.3063, 0.2638, 0.2593, ...
    0.2555, 0.2504, 0.2438, 0.2396, 0.2359, ...
    0.2339, 0.2302, 0.2295, 0.2278, 0.2297, ...
    0.2310, 0.2274, 0.2277];
th1 = floor(th_i / 5);
th2 = th1 + 1;

% return linear interpolation between tabular values
delay = delays(th1) + (delays(th2) - delays(th1)) * ((th_i / 5) - th1) / (th2 - th1);
delay2 = delay * delay;
end

function [delay] = tropo_delay2(th)
% Description:
%   Calculate the tropospheric delay for a satellite.

% Arguments:
%   th_i: Angle of elevation in degrees to the satellite.

% Return values:
%   delay2: sig_tropo_i^2.

delay = power(0.12 * 1.001, 2) / (0.002001 + power(sin(pi()*th/180),2));
end
