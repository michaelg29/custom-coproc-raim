function x = test(n)
    N_sat = 5;
    N_const = 1;

    %%%%%%%%%%%%%%%%%%%%%%%
    % calculate test data %
    %%%%%%%%%%%%%%%%%%%%%%%
    r = [1, 2, 0.5]; % linearized receiver position
    % satellite vehicle (SV) positions
    S = [
         5, 10, 5 ; % SV1 position
         7, 3, 8 ; % SV2 position
         2, 7, 8 ; % SV3 position
         -8, 9, 6 ; % SV4 position
        -1, 2, 7 ; % SV5 position
    ];
    % estimated pseudoranges
    d = [
        9.9   ;
        9.7   ;
        9.1   ;
        12.09 ;
        7.0   ;
    ];

    x = update(n, N_sat, N_const, r, S, d);
    r(1:3) = r(1:3) + transpose(x(1:3));
    x = update(n, N_sat, N_const, r, S, d);
    r(1:3) = r(1:3) + transpose(x(1:3));
    x = update(n, N_sat, N_const, r, S, d);
end

function x = update(n, N_sat, N_const, r, S, d)
    % calculate pseudorange errors
    e = zeros(N_sat,1);
    for i = 1:N_sat
        e(i) = d(i) - norm(S(i,1:3) - r);
    end

    % normalize distance vectors
    H_tot = ones(N_sat, 3+N_const);
    for i = 1:N_sat
        H_tot(i,1:3) = (S(i,1:3) - r) / norm(S(i,1:3) - r);
        H_tot(i,4) = 1;
    end

    % total set solution

    % subset 1 solution
    H = ones(n, 3+N_const);
    for i = 1:n
        H(i,1:3+N_const) = H_tot(i,1:3+N_const);
    end

    % pseudo-inverse (Ht * H)^-1 * Ht
    Ht = transpose(H); % Ht
    mat = Ht * H;      % Ht * H
    %mat = inv(mat); mat = mat * Ht;
    mat = mat\Ht;      % (Ht & H)^-1 * Ht
    
    x = mat * e(1:n);
end