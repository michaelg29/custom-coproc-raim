% fit a p-degree polynomial to the data
function [x, y, e] = lsr(in_vec, out_vec, p)
    n = p;
    m = length(in_vec);

    % construct A
    A = zeros(m, n);
    for i = 1:m
        for k = 1:n
            A(i,k) = power(in_vec(i), k-1); % x ** k
        end;
    end;

    % compute A^T, (A^T A)^-1
    A_T = transpose(A);
    A_T_A_inv = inv(A_T * A);

    % construct coefficients
    out_vec_t = transpose(out_vec);
    x = A_T_A_inv*A_T*out_vec_t;

    % predict output
    y = zeros(length(in_vec), 1);
    y = A * x;

    % return error
    e = y - out_vec_t;

    scatter(transpose(in_vec), out_vec_t)

end