
% "Advanced RAIM User Algorithm Description: Integrity Support Message Processing, Fault Detection, Exclusion, and Protection Level Calculation"
% Blanch et al., 2012
function [] = araim_blanch_et_al()
    appendix_j();
end

function [delay] = user_delay_gps(th)
    % th in degrees

    delay_mp = 0.13 + 0.53 * exp(-th / 10);
    delay_noise = 0.15 + 0.43 * exp(-th / 6.9);

    f_l1 = 1575.42e6;
    f_l5 = 1176.45e6;

    delay = sqrt((power(f_l1,4) + power(f_l5,4))/power(power(f_l1,2)-power(f_l5,2),2)) * sqrt(power(delay_mp,2) + power(delay_noise,2));
end

function [delay] = user_delay_galileo(th)
    % table of delays starting from th=5deg to th=90deg
    delays = [0.4529, 0.3553, 0.3063, 0.2638, 0.2593, 0.2555, 0.2504, 0.2438, 0.2396, 0.2359, 0.2339, 0.2302, 0.2295, 0.2278, 0.2297, 0.2310, 0.2274, 0.2277];
    th1 = floor(th / 5);
    th2 = th1 + 1;

    % return linear interpolation between tabular values
    delay = delays(th1) + (delays(th2) - delays(th1)) * ((th / 5) - th1) / (th2 - th1);
end

function [delay] = tropo_delay(th)
    % th in degrees

    delay = 0.12 * 1.001 / sqrt(0.002001 + power(sin(pi()*th/180),2));
end

function [] = appendix_j()
    % geometry matrix, N_sat rows, 3+N_const columns
    G = [0.0225 0.9951 -0.0966 1 0;
        0.6750 -0.6900 -0.2612 1 0;
        0.0723 -0.6601 -0.7477 1 0;
        -0.9398 0.2553 -0.2269 1 0;
        -0.5907 -0.7539 -0.2877 1 0;
        -0.3236 -0.0354 -0.9455 0 1;
        -0.6748 0.4356 -0.5957 0 1;
        0.0938 -0.7004 -0.7075 0 1;
        0.5571 0.3088 -0.7709 0 1;
        0.6622 0.6958 -0.2780 0 1];
    N_sat = size(G,1);
    N_const = size(G,2) - 3;

    % assumptions
    sigma_URA_i = 0.75; % meters
    sigma_URE_i = 0.50; % meters
    P_sat_i = 10e-4;
    b_nom_i = 0.50; % meters
    P_const_j = 10e-4;

    % constants
    PHMI = 10e-7; % integrity budget
    PHMI_v = 9.8e-8; % vertical integrity budget
    PHMI_h = 2e-9; % horizontal integrity budget
    P_const = 4e-8; % threshold for risk from constellation fault
    P_sat = 4e-8; % threshold for risk from satellite fault
    P_fa = 4e-6; % continuity budget for disruptions from false alert
    P_fa_vert = 3.9e-6; % vertical continuity budget
    P_fa_hor = 9e-8; % horizontal continuity budget
    P_fa_chi2 = 10e-8; % continuity budget for chi-square test
    TOL_PL = 5e-2; % meters, tolerance for prot. level computation
    K_acc = 1.96; % # std deviations for accuracy
    K_ff = 5.33; % # std deviations for 10e-7 fault free vert pos error
    P_emt = 10e-5; % effective monitor threshold
    T_check = 300; % seconds, time between consistency checks for excluded
    T_recov = 600; % seconds, time period an excluded satellite is out of view

    % inputs
    % PR_i : Pseudorange after corrections for satellite i
    % sigma_URA_i : std dev of clock/ephemeris error of sat. i for integrity
    % sigma_URE_i : std dev of clock/ephemeris error of sat. i for accuracy/continuity
    % b_nom_i : max nominal bias for sat i for integrity
    % P_sat_i : prior fault prob for sat i
    % P_const_j : prior fault prob for constellation j
    % I_const_j : indices of satellites in constellation j
    % N_sat : number of satellites
    % N_const : number of constellations

    % C_int(i,i) = sig_URA_i^2 + sig_tropo_i^2 + sig_user_i^2
    % C_int = diag([3.8865 1.4377 0.8604 1.6383 1.3229 0.8434 0.8963 0.8669 0.8573 1.3616]);
    % C_int(i,i) = sig_URA_i^2 + sig_tropo_i^2 + sig_user_i^2
    % C_acc = diag([3.5740 1.1252 0.5479 1.3258 1.0104 0.5309 0.5838 0.5544 0.5448 1.0491]);
    C_int = zeros(1,N_sat);
    C_acc = zeros(1,N_sat);

    for i = 1:N_sat
        % determine angle of elevation = atan2(z, sqrt(x^2 + y^2))
        ang = rad2deg(abs(atan2(G(i,3), sqrt(power(G(i,1),2)+power(G(i,2),2)))));
        %fprintf("%f %f %f => %f, %f => %f\n", G(i,1), G(i,2), G(i,3), sqrt(power(G(i,1),2)+power(G(i,2),2)), G(i, 3), ang);

        % determine tropospheric delay
        tropo_delay_val = tropo_delay(ang);

        % determine user delay
        % TODO use different functions
        user_delay_val = user_delay_gps(ang);

        C_int(1,i) = power(sigma_URA_i,2) + power(tropo_delay_val,2) + power(user_delay_val,2);
        C_acc(1,i) = power(sigma_URE_i,2) + power(tropo_delay_val,2) + power(user_delay_val,2);
    end;

    C_int = diag(C_int);
    C_acc = diag(C_acc);
    W = inv(C_int);



    % N_sat,max=2
    % N_const,max=1
    % => subset fault modes include n-1 and n-2 subsets, and two
    % constellation fault modes
    % sig_3 = 2.5760m, 2.5577m
    % sig_ss,3 = 1.5307m, 1.5292m
    % b_k = 2.8935m, 2.0875m

    % K_fa,3 = Q^-1(P_FA,VERT/2N_faultmodes) = Q^-1(3.9e-6/2/57) = 5.3953

    % (24) => VPL = 19.7m
    % (26) => HPL = 14.9m
    % (32) => EMT = 11.8m
    % sig_v,acc = 1.47m = std. dev of all-in-view from (27)

end