# ARAIM

## Definitions

Name | Description | Source
-- | -- | --
$k$ | Subset index. The initial value, $k=0$ denotes the all-in-view subset assuming no faults. | -
$N_{sat}$ | Integer number of satellites. | RPU
$N_{const}$ | Integer number of constellations. | RPU
$N_{ss}$ | Integer subsets for which to calculate solutions. | GPP
$\sigma_{URA,i}$ | FP standard deviation of the clock and ephemeris error of $SV_i$ used for integrity. | GPP
$\sigma_{URE,i}$ | FP standard deviation of the clock and ephemeris error of $SV_i$ used for accuracy and continuity. | GPP
$\sigma_{tropo,i}^2$ | FP variance for the tropospheric delay for $SV_i$. | GPP
$\sigma_{user,i}^2$ | FP variance for the user delay based on line-of-sight geometry for $SV_i$. | GPP
$C_{int} \in \R^{N_{sat} \times N_{sat}}$ | FP $N_{sat} \times N_{sat}$ covariance matrix for integrity. | RPU
$C_{acc} \in \R^{N_{sat} \times N_{sat}}$ | FP $N_{sat} \times N_{sat}$ covariance matrix for accuracy and continuity. | RPU
$W^{(k)} \in \R^{N_{sat} \times N_{sat}}$ | FP $N_{sat} \times N_{sat}$ weighting matrix for subset $k$. If $SV_i$ is in subset $k$, $W^{(k)}(i,i)=W(i,i)$. Otherwise, the subset solution excludes that SV, so $W^{(k)}(i,i)=0$. | RPU
$G \in \R^{N_{sat} \times (3+N_{const})}$ | FP $N_{sat} \times (3+N_{const})$ geometry matrix. The first three columns are the unit vector pointing from the linearized receiver position to the satellite position in ENU coordinates. The final $N_{const}$ columns are an activation string for the SV's constellation. | GPP
$S^{(k)} \in \R^{N_{sat} \times (3+N_{const})}$ | FP $N_{sat} \times (3+N_{const})$ matrix for least squares derived from the geometry and weight matrices. | RPU
$\Delta PR \in \R^{N_{sat}}$ | FP $N_{sat}^{(k)}$-dimensional vector of the difference between the pseudorange measurements and the expected ranges from the linearized point to each satellite. | RPU

## Equations

Initial matrices the RPU must calculate:
$$
\begin{align}
C_{int}(i,i) &\equiv \sigma_{URA,i}^2 + \sigma_{tropo,i}^2 + \sigma_{user,i}^2
\newline
C_{acc}(i,i) &\equiv \sigma_{URE,i}^2 + \sigma_{tropo,i}^2 + \sigma_{user,i}^2
\newline
W &\equiv C_{int}^{-1}
\Rightarrow W(i,i)=\frac{1}{\sigma_{URA,i}^2 + \sigma_{tropo,i}^2 + \sigma_{user,i}^2}
\end{align}
$$

All-in-view iterative solution:
$$
\begin{align}
\Delta \hat{x}_j&=(G^T W G)^{-1} G^T W \Delta PR_j
\newline
\Delta PR_j(i)&=PR_i-|\vec{x}_{SV_{i}}-\hat{x}_j|
\newline
\hat{x}_{j+1} &= \hat{x}_j + \Delta \hat{x}_j
\newline
\vec{y} &\equiv \Delta PR_J
\end{align}
$$

Least-squares matrix for each subset:
$$
\begin{align}
S^{(k)} &= (G^T W^{(k)} G)^{-1} G^T W^{(k)} \in \R^{(3+N_{const}) \times N_{sat}}
\newline
W^{(k)} &\in \R^{N_{sat} \times N_{sat}}
\newline
G &\in \R^{N_{sat} \times (3+N_{const})}
\Rightarrow G^T \in \R^{(3+N_{const}) \times N_{sat}}
\newline
\Rightarrow G^T W^{(k)} &\in \R^{(3+N_{const}) \times N_{sat}}
\newline
\Rightarrow G^T W^{(k)} G, (G^T W^{(k)} G)^{-1} &\in \R^{(3+N_{const}) \times (3+N_{const})}
\newline
\Rightarrow S^{(k)} =(G^T W^{(k)} G)^{-1} G^T W^{(k)} &\in \R^{(3+N_{const}) \times N_{sat}}
\newline

\end{align}
$$

The following equations are applicable for all subsets ($\forall \ k \in [1,N_{ss}]$) and the three ENU components ($\forall \ q \in \{1,2,3\}$). Subset solution separations, along with variances and biases:
$$
\begin{align}
\Delta \hat{x}^{(k)} &= \hat{x}^{(k)} - \hat{x}^{(0)} = (S^{(k)} - S^{(0)})\vec{y} \in \R^{(3+N_{const})}
\newline
\sigma_q^{(k)2} &= [(G^T W^{(k)} G)^{-1}](q,q)
\newline
b_q^{(k)} &= \sum_{i=1}^{N_{sat}}|S_{q,i}^{(k)}|*b_{nom,i}
\newline
\sigma_{ss,q}^{(k)2} &= [(S^{(k)}-S^{(0)})C_{acc}(S^{(k)}-S^{(0)})^T](q,q)
\end{align}
$$
