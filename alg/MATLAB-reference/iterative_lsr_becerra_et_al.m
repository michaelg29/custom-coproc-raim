
% "A Reduced-Complexity Direct Learning Architecture for Digital Predistortion Through Iterative Pseudoinverse Calculation"
% Becerra et al., 2021
function [] = iterative_lsr_becerra_et_al()

    U = [ 0.5637  0.4582 -0.6872 1 0;
          0.5186  0.7509 -0.4088 1 0;
          0.9813  0.0881 -0.1711 1 0;
         -0.9267 -0.3346 -0.1714 1 0;
         -0.4833  0.8455 -0.2272 1 0;
          0.3378 -0.3202 -0.8851 1 0;
          0.8109 -0.1330 -0.5699 1 0;
         -0.6837  0.5340 -0.4975 1 0;
         -0.4757 -0.3973 -0.7848 1 0;
         -0.1052 -0.0968 -0.9897 1 0;
         -0.6412  0.0486 -0.7659 0 1;
          0.0824 -0.5225 -0.8487 0 1;
          0.6849 -0.6958 -0.2163 0 1;
         -0.4487 -0.6097 -0.6534 0 1;
         -0.1024  0.2300 -0.9678 0 1;
          0.2744  0.8529 -0.4441 0 1;
          0.8220 -0.5516 -0.1417 0 1];
    [n, alpha, alpha_act, max_dev, max_dev_perc] = iterative_weighted_pseudoinverse(30, U);
    fprintf("Converged in %d iterations, with alpha of %f (actual is %f), and a max deviation of %f (%f percentage)\n", n, alpha, alpha_act, max_dev, max_dev_perc);
    [n, alpha, alpha_act, max_dev, max_dev_perc] = iterative_weighted_pseudoinverse(30, U, 0.05);
    fprintf("Converged in %d iterations, with alpha of %f (actual is %f), and a max deviation of %f (%f percentage)\n", n, alpha, alpha_act, max_dev, max_dev_perc);
    

    U = [ 0.0225  0.9951 -0.0966 1.0 0.0 ;
          0.6750 -0.6900 -0.2612 1.0 0.0 ;
          0.0723 -0.6601 -0.7477 1.0 0.0 ;
         -0.9398  0.2553 -0.2269 1.0 0.0 ;
         -0.5907 -0.7539 -0.2877 1.0 0.0 ;
         -0.3236 -0.0354 -0.9455 0.0 1.0 ;
         -0.6748  0.4356 -0.5957 0.0 1.0 ;
          0.0938 -0.7004 -0.7075 0.0 1.0 ;
          0.5571  0.3088 -0.7709 0.0 1.0 ;
          0.6622  0.6958 -0.2780 0.0 1.0 ];
    [n, alpha, alpha_act, max_dev, max_dev_perc] = iterative_weighted_pseudoinverse(30, U);
    fprintf("Converged in %d iterations, with alpha of %f (actual is %f), and a max deviation of %f (%f percentage)\n", n, alpha, alpha_act, max_dev, max_dev_perc);
    [n, alpha, alpha_act, max_dev, max_dev_perc] = iterative_weighted_pseudoinverse(30, U, 0.05);
    fprintf("Converged in %d iterations, with alpha of %f (actual is %f), and a max deviation of %f (%f percentage)\n", n, alpha, alpha_act, max_dev, max_dev_perc);

end

function [n, alpha, alpha_act, max_dev, max_dev_perc] = iterative_pseudoinverse(n, U, alpha, delta)
    arguments
        n; U; alpha = 0.13; delta = 1e-6
    end

    C = length(U(1,:));

    % traditional Moore-Penrose Pseudoinverse
    exp = (U' * U) \ U';

    % iterative approach
    two_I = 2 * diag(ones(C,1)); % 2*I
    alpha_act = 2 / max(eig(U' * U));
    guess = alpha * U';
    for i = 1:n
        new_guess = (two_I - guess * U) * guess;
        if max(max(new_guess - guess)) < delta
             guess = new_guess;
             break;
        end
        guess = new_guess;
    end

    max_dev = max(max(abs(exp - guess)));
    max_dev_perc = max_dev / min(min(abs(exp))) * 100.0;
    n = i;
end

function [n, alpha, alpha_act, max_dev, max_dev_perc] = iterative_weighted_pseudoinverse(n, U, alpha, delta)
    arguments
        n; U; alpha = 0.13; delta = 1e-6
    end

    R = length(U(:,1));
    C = length(U(1,:));
    W = diag(rand(R,1))
    sqrt(W)

    % traditional Moore-Penrose Pseudoinverse
    exp = (U' * W * U) \ U' * W;

    U = sqrt(W) * U;

    % iterative approach
    two_I = 2 * diag(ones(C,1)); % 2*I
    alpha_act = 2 / max(eig(U' * U));
    guess = alpha * U';
    for i = 1:n
        new_guess = (two_I - guess * U) * guess;
        if max(max(new_guess - guess)) < delta
             guess = new_guess;
             break;
        end
        guess = new_guess;
    end

    guess = guess * sqrt(W);

    max_dev = max(max(abs(exp - guess)));
    max_dev_perc = max_dev / min(min(abs(exp))) * 100.0;
    n = i;
end
