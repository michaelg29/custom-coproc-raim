function [xhat, rad, delta] = raim_wang_et_al(n)
    % initial data - satellite positions and calculated pseudoranges
    N_sat = 6;
    N_const = 1;
    SVs = [
        -23517276.47 -8441337.22 9266091.87 ;
        827057.89 24219485.57 10199910.09 ;
        -12796270.59 17283306.26 15279722.70 ;
        13190124.89 21040942.63 9630736.62 ;
        -19028937.05 -630447.92 18670658.66 ;
        -20972725.27 13817010.49 -8692192.55 ;
    ];
    PRs = [
        25142929.85 ;
        21054624.95 ;
        20024645.90 ;
        23776922.74 ;
        22900110.44 ;
        23545705.70 ;
    ];

    sigma_URA_i = 0.75; % meters
    sigma_URE_i = 0.50; % meters

    % initial guess
    xhat = [
        0.0 ; % x
        0.0 ; % y
        0.0 ; % z
        0.0 ; % c*t
    ];

    xhat2 = 1.0e+07 * [
        0.8921;
        -1.4569;
        -1.0251;
        -0.6427
    ];

    % initial observation matrix
    G = ones(N_sat, 3+N_const);
    C_int = zeros(1,N_sat);
    for j = 1:N_sat
        diff = SVs(j,1:3) - transpose(xhat(1:3));
        G(j,1:3) = diff / norm(diff);
        G(j,4) = 1.0;

        % determine angle of elevation = atan2(z, sqrt(x^2 + y^2))
        ang = rad2deg(abs(atan2(G(j,3), sqrt(power(G(j,1),2)+power(G(j,2),2)))));

        % determine tropospheric delay
        tropo_delay_val = tropo_delay(ang);

        % determine user delay
        % TODO use different functions
        user_delay_val = user_delay_gps(ang);

        C_int(1,j) = power(sigma_URA_i,2) + power(tropo_delay_val,2) + power(user_delay_val,2);
    end

    % weight matrix
    W = diag(1./C_int);

    % least-squares matrix
    G_t = transpose(G);
    LS = inv(G_t * W * G) * G_t * W;

    delta = zeros(n, 4);
    rad = zeros(n, 1);
    for i = 1:n
        del_prs = zeros(N_sat,1);
        for j = 1:N_sat
            % pseudorange estimate - expected distance from SV to linearized point
            del_prs(j) = PRs(j) - norm(SVs(j,1:3) - transpose(xhat(1:3)));
        end

        xhat_new = LS * del_prs;

        delta(i,1:4) = transpose(xhat_new - xhat);
        xhat = xhat_new;
        rad(i) = norm(xhat(1:3));
    end

    %rad = ; % expecting 6378km = 6.3780e+06
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

% expecting delta
%1  3.3950+006  5.4498+006  3.9056+006 1.1798+006
%2  0.5180+006 -0.7848+006 -0.5973+006 1.1747+006
%3  1.4652+004 -1.9652+004 -1.6470+004 3.3162+004
%4  10.5367    -13.1790    -11.8865    22.8554
%5 -0.0316     -0.0195     -0.0000     0.0000
%6 -0.0316     -0.0195     -0.0000     -0.0000