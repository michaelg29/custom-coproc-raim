# RAIM

## Definitions
$$
\begin{align*}
n &\equiv N_{sat}
\newline
m &\equiv 3+N_{const}
\newline
\text{Receiver located at: } &[x_r \space y_r \space z_r]
\newline
\text{Receiver time offset: } &[t_{r_1}...t_{r_{N_{const}}}]
\end{align*}
$$

## Initial equations
$$
\begin{align*}
z &\equiv V_*^{-1/2} * \vec{z}_* = H \vec{x} + \vec{v} + \vec{f} \in \R^{n*1}
\newline
H&\equiv\text{normalized observation matrix} \in \R^{n*m}
\newline
\vec{x} = [x_r \space y_r \space z_r \space t_{r_1}...t_{r_{N_{const}}}]^T&\equiv\text{state vector} \in \R^{m*1}
\newline
\vec{v}&\equiv\text{normalized fault vector} \in \R^{n*1}
\newline
\vec{f} \sim N(0_{n*1},I_n) &\equiv\text{normalized noise vector} \in \R^{n*1}
\end{align*}
$$

## Formulation with inputs
$$
\begin{align*}
S^T &\equiv (H^T H)^{-1} H^T
\newline
\hat{\vec{x}} &\equiv S^T \vec{z} = [\hat{x}_r\space\hat{y}_r\space\hat{z}_r\space\hat{t}_{r_1}...\hat{t}_{r_{N_{const}}}]^T&
\newline
H &\in \R^{n*m}
\newline
\vec{z} &\in \R^{n*1}
\newline
H^T &\in \R^{m*n}
\newline
H^T H, (H^T H)^{-1} &\in \R^{m*m}
\newline
(H^T H)^{-1} H &\in \R^{m*n}
\newline
\Rightarrow \hat{\vec{x}} &\in \R^{m*1}
\newline
\text{Produces an error:}&
\newline
\vec{\epsilon} &\sim N(S^T \vec{f}), \sigma^2 = (H^T H)^{-1})
\end{align*}
$$

## Solution separation RAIM
* Multiple solutions (select one element from the vector $\hat{\vec{x}}_i$ to get $\hat{x}_i$)
    * Full set solution with 5 satellites: $\hat{x}_0$
        * composed of  ${SV}_1$, ${SV}_1$, ${SV}_2$, ${SV}_3$, ${SV}_4$
    * Subset solution with 4 satellites: $\hat{x}_i$
        * e.g. $\hat{x}_1$ composed of ${SV}_1$, ${SV}_1$, ${SV}_2$, ${SV}_3$
* Subtract different solutions
    * $\Delta_i \equiv \hat{x}_i - \hat{x}_0 = \epsilon_i - \epsilon_0$
    * $\Delta_i \sim N((S_i^T - S_0^T)\vec{f}, \sigma_{\Delta_i}^2=\sigma_i^2-\sigma_0^2)$
* Maximum solution separation: $\max_i(|\hat{x}_i - \hat{x}_0|)$
    * Produces $N_{subset}$ solution separations
