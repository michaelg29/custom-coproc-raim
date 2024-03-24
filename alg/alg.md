# ARAIM

## Definitions

Name | Description
-- | --
$k$ | Subset index. The initial value, $k=0$ denotes the all-in-view subset assuming no faults.
$N_{sat}$ | Integer number of satellites.
$N_{const}$ | Integer number of constellations.
$N_{ss}$ | Integer subsets for which to calculate solutions.

## Equations

Notation | Relation | Dimensions | Description
-- | -- | -- | --
$x_{SV_i}$ | $SV_i$ | $3 \times 1$ | Unit vector pointing from the linearized receiver position to the satellite position in ENU coordinates
$c_{SV_i}$ | $SV_i$ | $1 \times 1$ | Constellation number.
$\sigma_{tropo,i}^2$ | $SV_i$ | $1 \times 1$ | Variance for the tropospheric delay.
$\sigma_{user,i}^2$ | $SV_i$ | $1 \times 1$ | Variance for the user delay based on line-of-sight geometry.
$\sigma_{URA,i}^2$ | $SV_i$ | $1 \times 1$ | Variance of the clock and ephemeris error used for integrity.
$\sigma_{URE,i}^2$ | $SV_i$ | $1 \times 1$ | Variance of the clock and ephemeris error used for accuracy and continuity.
$b_{nom,i}$ | $SV_i$ | $1 \times 1$ | Maximum nominal bias for integrity.
$idx_{ss,k}$ | $SS_k$ | $1 \times 1$ | $N_{sat}$-bit activation string to indicate satellites included in a subset.
$\vec{y}$ | $SS_k$ | $N_{sat} \times 1$ | Vector to multiply with the weighted least-squares matrix.

Initial matrices the RPU must calculate:

$C_{int}(i,i) \equiv \sigma_{URA,i}^2 + \sigma_{tropo,i}^2 + \sigma_{user,i}^2$

$C_{acc}(i,i) \equiv \sigma_{URE,i}^2 + \sigma_{tropo,i}^2 + \sigma_{user,i}^2$

$W \equiv C_{int}^{-1} \Rightarrow W(i,i)=\frac{1}{\sigma_{URA,i}^2 + \sigma_{tropo,i}^2 + \sigma_{user,i}^2}$

All-in-view iterative solution:

$\Delta \hat{x}_j=(G^T W G)^{-1} G^T W \Delta PR_j$

$\Delta PR_j(i)=PR_i-|\vec{x}_{SV_{i}}-\hat{x}_j|$

$\hat{x}_{j+1} = \hat{x}_j + \Delta \hat{x}_j$

$\vec{y} \equiv \Delta PR_J$

Least-squares matrix for each subset:

$S^{(k)} = (G^T W^{(k)} G)^{-1} G^T W^{(k)} \in \R^{(3+N_{const}) \times N_{sat}}$

$W^{(k)} \in \R^{N_{sat} \times N_{sat}}$

$G \in \R^{N_{sat} \times (3+N_{const})} \Rightarrow G^T \in \R^{(3+N_{const}) \times N_{sat}}$

$\Rightarrow G^T W^{(k)} \in \R^{(3+N_{const}) \times N_{sat}}$

$\Rightarrow G^T W^{(k)} G, (G^T W^{(k)} G)^{-1} \in \R^{(3+N_{const}) \times (3+N_{const})}$

$\Rightarrow S^{(k)} =(G^T W^{(k)} G)^{-1} G^T W^{(k)} \in \R^{(3+N_{const}) \times N_{sat}}$

The following equations are applicable for all subsets ($\forall \ k \in [1,N_{ss}]$) and the three ENU components ($\forall \ q \in \{1,2,3\}$). Subset solution separations, along with variances and biases:

$\Delta \hat{x}^{(k)} = \hat{x}^{(k)} - \hat{x}^{(0)} = (S^{(k)} - S^{(0)})\vec{y} \in \R^{(3+N_{const})}$

$\sigma_q^{(k)2} = [(G^T W^{(k)} G)^{-1}](q,q)$

$b_q^{(k)} = \sum_{i=1}^{N_{sat}}|S^{(k)}(q,i)|*b_{nom,i}$

$\sigma_{ss,q}^{(k)2} = [(S^{(k)}-S^{(0)})C_{acc}(S^{(k)}-S^{(0)})^T](q,q)$

### FDE tests

Notation | Relation | Dimensions | Description
-- | -- | -- | --
$K_{fa}$ | conf | $3 \times 1$ | Threshold factors derived from the q-function and false alarm probabilities. See (19) and (20) in [X] for more details.

$|\Delta \hat{x}^{(k)}| = |\hat{x}^{(k)} - \hat{x}^{(0)}| \le K_{fa,q}*\sigma_{ss,q}^{(k)2}$

### Least-squares adaptation

$U \in \R^{M \times N} \Rightarrow U \in \R^{N \times M}$

$U^{\dagger} = (U^T U)^{-1} U^T \in \R^{N \times M}$

$U^{\dagger}_{k+1}=(2I_N - U^{\dagger}_k U) U^{\dagger}_k$

$U^{\dagger}_0 = \alpha U^T, \alpha \in [0,2/\max(\text{eigenvals}(U^T U))]$

$S^{(k)} = (G^T W^{(k)} G)^{-1} G^T W^{(k)} \in \R^{(3+N_{const}) \times N_{sat}}$

$U = W^{1/2}G \in \R^{(3+N_{const}) \times N_{sat}}$

$U^{\dagger} = ((W^{1/2}G)^T W^{1/2}G)^{-1} (W^{1/2}G)^T$

$(AB)^T = B^T A^T$

$U^{\dagger} = (G^T (W^{1/2})^T W^{1/2}G)^{-1} G^T (W^{1/2})^T$

$W^{1/2} = (W^{1/2})^T$

$U^{\dagger} = (G^T W G)^{-1} G^T W^{1/2}$

$S^{(k)} = U^{\dagger} W^{1/2} = (G^T W G)^{-1} G^T W$

## Notes

### Weight matrix multiplication
```
ones(6,4)*diag([0.1, 0.2 0.3 0.4]) =
    0.1000    0.2000    0.3000    0.4000
    0.1000    0.2000    0.3000    0.4000
    0.1000    0.2000    0.3000    0.4000
    0.1000    0.2000    0.3000    0.4000
    0.1000    0.2000    0.3000    0.4000
    0.1000    0.2000    0.3000    0.4000
```
