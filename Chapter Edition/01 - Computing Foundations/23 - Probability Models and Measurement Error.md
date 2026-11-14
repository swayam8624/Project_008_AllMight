# 23 — Probability Models and Measurement Error

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition|Next chapter]]

Probability begins by naming outcomes and assigning a model, not by treating randomness as an unexplained property. This chapter builds sample spaces, distributions, expectation, variance, and standardized error as tools for reasoning about uncertain graphics measurements.

**By the end:** Define a probability model, distinguish discrete outcomes from continuous densities, and interpret uncertainty and standardized residuals.

**Recall question:** Why must a continuous density be integrated over a range before it becomes a probability?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 52397–55711; source-span SHA256 73fd34c30597176a133d430d47d2b4228ef8276f8db89d1d3a118e1946d3b79d. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## 1. A probability model begins with possible outcomes

We have completed Phase 9's treatment of deterministic mathematics: functions, derivatives, integrals, differential equations, numerical integration, and optimization. Given an input and a deterministic function, we know how to compute the result, analyze how it changes, and optimize its parameters.

Phase 10 introduces something fundamentally different: uncertainty.

A renderer may need to evaluate an integral containing millions of possible light paths. A camera measurement may contain noise. A reconstructed 3D position may be uncertain. A physics simulation may contain stochastic forcing. An optimization algorithm may encounter measurements with different degrees of reliability.

It is tempting to describe these problems by saying that something is random. But randomness alone is not a sufficient mathematical description. We need to know what outcomes are possible, which events have probabilities, how different quantities depend on each other, and how those uncertainties propagate through computations.

The central object of this phase is a random variable:

$$
 X:\Omega\rightarrow\mathbb R, 
$$

where $\Omega$ is a set of possible outcomes.

Later, we will work with random vectors:

$$
 \mathbf X:\Omega\rightarrow\mathbb R^n, 
$$

and summarize their behavior through a mean vector:

$$
 \boldsymbol\mu=\mathbb E[\mathbf X] 
$$

and covariance matrix:

$$
 \Sigma= \mathbb E[ (\mathbf X-\boldsymbol\mu) (\mathbf X-\boldsymbol\mu)^T ]. 
$$

These equations connect probability to the gradients, matrices, camera projections, optimization solvers, and Gaussian representations we developed earlier.

### Experiments, outcomes, and sample spaces

Imagine a graphics engine running a stochastic visibility test.

A ray is sampled according to some distribution. Depending on its direction and the scene geometry, it may hit a light source, hit another object, or escape the scene.

Before writing any probability formula, we must identify what the random experiment actually produces.

An experiment is a procedure with an outcome that is not known in advance under the probabilistic model being used.

An outcome is one possible complete result of that experiment.

A [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error#1. A probability model begins with possible outcomes|sample space]], conventionally written $\Omega$, is the set of all outcomes that the model permits.

For a coin toss:

$$
 \Omega=\{H,T\}. 
$$

For rolling one conventional six-sided die:

$$
 \Omega=\{1,2,3,4,5,6\}. 
$$

For sampling a continuous number between zero and one:

$$
 \Omega=[0,1]. 
$$

For sampling a two-dimensional direction in a local rendering parameterization:

$$
 \Omega=[0,1]^2 
$$

might represent the initial random parameters, even though the generated direction lives on a hemisphere or sphere.

This distinction is important. The sample space depends on which experiment and representation we choose to model.

#### 1 A sample space is not necessarily a collection of equally likely outcomes

Suppose a rendering experiment selects one of three lights:

$$
 \Omega=\{L_1,L_2,L_3\}. 
$$

We might assign:

$$
 P(L_1)=0.6,\qquad P(L_2)=0.3,\qquad P(L_3)=0.1. 
$$

The sample space only describes what can happen. It does not imply that every outcome has equal probability.

If we incorrectly assume uniform selection, we would use $1/3$ for every light and fundamentally change the probability model.

Whether that change produces a biased rendering result depends on how the estimator compensates for the sampling distribution. We will derive that relationship in the sampling and Monte Carlo chapters.

#### 2 Discrete versus continuous outcome spaces

For a finite sample space, individual outcomes can carry nonzero probabilities.

For an ideal continuous uniform variable on $[0,1]$:

$$
 P(X=0.3)=0. 
$$

This does not mean that $0.3$ is impossible. It means an individual exact real value carries zero probability under a continuous distribution.

Probability belongs to intervals or more general measurable sets, not necessarily to individual points.

For example:

$$
 P(0.2\le X\le0.5)=0.3 
$$

under the standard uniform model.

The difference between zero probability and logical impossibility is a foundational subtlety. An event can be possible within the sample space yet have probability zero.

#### 3 Randomness as a model, not a mystical property

A pseudorandom-number generator in C++ is a deterministic state machine.

Given its complete initial state and algorithm, its output sequence is determined.

Yet we model the resulting values probabilistically because the sequence is intended to approximate certain distributional properties, and because uncertainty about the state or sampling mechanism can justify a probability model.

Probability theory is a mathematical framework for uncertainty. It does not require us to prove that the underlying physical or computational process is metaphysically indeterministic.

This distinction matters for reproducible simulation and rendering experiments.

### Events and set operations

An event is a set of outcomes.

Suppose:

$$
 \Omega=\{1,2,3,4,5,6\}. 
$$

Define:

$$
 A=\{2,4,6\} 
$$

as the event that the die result is even.

Define:

$$
 B=\{4,5,6\} 
$$

as the event that the result is greater than three.

Then:

$$
 A\cap B=\{4,6\} 
$$

means both events occur.

The union:

$$
 A\cup B=\{2,4,5,6\} 
$$

means at least one event occurs.

The complement:

$$
 A^c=\{1,3,5\} 
$$

means $A$ does not occur.

The difference:

$$
 A\setminus B=\{2\} 
$$

means $A$ occurs but $B$ does not.

#### 1 Probability of a union

Suppose two events overlap.

If we add $P(A)+P(B)$, outcomes in the intersection are counted twice.

Therefore we subtract the intersection once:

$$
 \boxed{ P(A\cup B) = P(A)+P(B)-P(A\cap B) }. 
$$

For a fair die:

$$
 P(A)=\frac36=\frac12, 
$$

$$
 P(B)=\frac36=\frac12, 
$$

$$
 P(A\cap B)=\frac26=\frac13. 
$$

Thus:

$$
 P(A\cup B) = \frac12+\frac12-\frac13 =\boxed{\frac23}. 
$$

We can verify this by counting the union's four outcomes:

$$
 P(A\cup B)=\frac46=\frac23. 
$$

#### 2 Disjoint events

Two events are disjoint or mutually exclusive when:

$$
 A\cap B=\varnothing. 
$$

They cannot both occur in the same trial.

For example, when rolling one die, the events $X=2$ and $X=5$ are disjoint.

In that case:

$$
 P(A\cup B)=P(A)+P(B). 
$$

But disjointness should not be confused with independence. These concepts answer entirely different questions, as we will prove in the independence section.

#### 3 The formal measurable space

For finite examples, we can generally assign probabilities to every subset of $\Omega$.

For infinite outcome spaces, probability theory uses a collection of permitted events called a sigma-algebra, written $\mathcal F$.

A sigma-algebra satisfies three important closure properties: it includes the whole sample space, it is closed under complements, and it is closed under countable unions. These imply closure under countable intersections.

The pair:

$$
 (\Omega,\mathcal F) 
$$

is a measurable space.

Why not simply declare every conceivable subset of the real numbers measurable?

In fully general set theory, there exist subsets of $\mathbb R$ that cannot be assigned a translation-invariant length-like measure consistently with the usual axioms. A suitable measurable-event collection avoids that problem.

For most engineering distributions, the relevant sets are Borel sets or sets belonging to a completed probability sigma-algebra.

A complete treatment of measure theory is larger than the probability foundations chapter, but you should understand why probability is formally defined on events rather than indiscriminately on arbitrary collections of outcomes.

### The three probability axioms

A probability measure is a function:

$$
 P:\mathcal F\rightarrow[0,1] 
$$

satisfying the Kolmogorov axioms.

#### Axiom 1: Nonnegativity

For every event $A$:

$$
 \boxed{P(A)\ge0}. 
$$

Probabilities cannot be negative.

#### Axiom 2: Normalization

For the complete sample space:

$$
 \boxed{P(\Omega)=1}. 
$$

One of the model's permitted outcomes occurs.

#### Axiom 3: Countable additivity

For pairwise disjoint events $A_1,A_2,\ldots$:

$$
 \boxed{ P\left(\bigcup\_{i=1}^{\infty}A_i\right) = \sum\_{i=1}^{\infty}P(A_i) }. 
$$

This is stronger than merely requiring probabilities of two disjoint events to add.

Countable additivity is essential for deriving continuity properties of probability measures and constructing continuous probability distributions.

#### 1 Deriving $P(\varnothing)=0$

The empty event represents no possible outcomes.

Because:

$$
 \Omega=\Omega\cup\varnothing 
$$

and the two sets are disjoint:

$$
 P(\Omega)=P(\Omega)+P(\varnothing). 
$$

Subtract $P(\Omega)$:

$$
 \boxed{P(\varnothing)=0}. 
$$

#### 2 Deriving the complement rule

Since:

$$
 A\cup A^c=\Omega 
$$

and the events are disjoint:

$$
 P(A)+P(A^c)=1. 
$$

Therefore:

$$
 \boxed{P(A^c)=1-P(A)}. 
$$

#### 3 Deriving monotonicity

If:

$$
 A\subseteq B, 
$$

then:

$$
 B=A\cup(B\setminus A) 
$$

with disjoint components.

Therefore:

$$
 P(B)=P(A)+P(B\setminus A). 
$$

By nonnegativity:

$$
 \boxed{P(A)\le P(B)}. 
$$

A larger event cannot have a smaller probability than an event entirely contained within it.

#### 4 The union bound

For arbitrary events $A_1,\ldots,A_n$:

$$
 \boxed{ P\left(\bigcup\_{i=1}^{n}A_i\right) \le \sum\_{i=1}^{n}P(A_i) }. 
$$

The right side may overcount overlapping outcomes, but it provides an upper bound.

The result also extends to countable collections.

This inequality is useful in reliability analysis, randomized algorithms, and proofs about failure probabilities.

For instance, suppose a rendering pipeline contains four rare failure events, each with probability at most $10^{-5}$. Without assuming independence, the probability that at least one occurs is bounded above by $4\times10^{-5}$.

### Conditional probability

Suppose we are interested in an event $A$, but we have learned that another event $B$ occurred.

That knowledge may change how likely $A$ is.

The conditional probability of $A$ given $B$ is:

$$
 \boxed{ P(A\mid B) = \frac{P(A\cap B)}{P(B)} } 
$$

when $P(B)>0$.

The vertical bar means given that.

#### 1 Deriving the formula by restricting the sample space

Imagine rolling a fair die.

Let:

$$
 A=\{2,4,6\} 
$$

be the event that the outcome is even.

Let:

$$
 B=\{4,5,6\} 
$$

be the event that the outcome exceeds three.

Before receiving any information:

$$
 P(A)=\frac12. 
$$

Now suppose we know the outcome is in $B$.

The possible outcomes are restricted to:

$$
 \{4,5,6\}. 
$$

Among them, two are even:

$$
 \{4,6\}. 
$$

Thus:

$$
 P(A\mid B)=\frac23. 
$$

Using the formula:

$$
 P(A\cap B)=\frac26, \qquad P(B)=\frac36, 
$$

so:

$$
 P(A\mid B) = \frac{2/6}{3/6} =\boxed{\frac23}. 
$$

Conditional probability renormalizes the probability assigned to the intersection relative to the event we know occurred.

#### 2 The multiplication rule

Rearrange the definition:

$$
 P(A\cap B) = P(A\mid B)P(B). 
$$

Equivalently:

$$
 \boxed{ P(A\cap B) = P(B\mid A)P(A) }. 
$$

These two expressions describe the same joint event from different directions.

For several events:

$$
 \boxed{ P(A_1\cap\cdots\cap A_n) = P(A_1) \prod\_{k=2}^{n} P(A_k\mid A_1\cap\cdots\cap A\_{k-1}) } 
$$

provided the conditioning events have positive probability.

This is the chain rule of probability.

It is fundamental to probabilistic sequence models, graphical models, stochastic scene-generation processes, and the factorization of joint distributions.

#### 3 Conditioning can change the relevant probability distribution

Suppose an algorithm samples a ray direction $\omega$, then we learn that the ray hit a particular surface.

The distribution of directions conditioned on hitting that surface generally differs from the original sampling distribution.

Some directions become impossible under the condition; others receive a larger share of the renormalized probability.

This is exactly what conditional probability formalizes.

#### 4 Conditioning on a zero-probability event

The elementary ratio is undefined when $P(B)=0$.

For continuous random variables, the event $Y=y$ generally has probability zero for a fixed $y$, yet we routinely speak about conditional densities $p(x\mid y)$.

These are defined through density factorizations or more general regular conditional probability constructions, not by naively dividing two zero probabilities.

That distinction becomes essential when we derive conditional Gaussian distributions.

### Independence: when information does not change probability

Two events are independent if:

$$
 \boxed{ P(A\cap B)=P(A)P(B) }. 
$$

When $P(B)>0$, this is equivalent to:

$$
 P(A\mid B)=P(A). 
$$

Knowing that $B$ occurred does not change the probability of $A$.

#### 1 Independence versus mutually exclusive events

These are not the same thing.

Suppose $A$ and $B$ are disjoint, with both having positive probability.

Then:

$$
 P(A\cap B)=0. 
$$

But:

$$
 P(A)P(B)>0. 
$$

Therefore the independence condition fails.

In fact, learning that $B$ occurred tells us that $A$ cannot have occurred.

Disjoint positive-probability events are maximally incompatible in that sense, not independent.

#### 2 Independent coin tosses

Let $A$ mean the first toss is heads and $B$ mean the second toss is heads.

For independent fair tosses:

$$
 P(A)=\frac12, \qquad P(B)=\frac12. 
$$

The probability of both is:

$$
 P(A\cap B)=\frac14. 
$$

Since:

$$
 P(A)P(B)=\frac14, 
$$

the events are independent.

#### 3 Pairwise independence is not mutual independence

This is a subtle but essential distinction.

Consider two independent fair bits $U,V\in\{0,1\}$.

Define three random bits:

$$
 X=U, \qquad Y=V, \qquad Z=U\oplus V, 
$$

where $\oplus$ denotes exclusive OR.

Each bit is equally likely to be zero or one.

Any pair is independent.

But all three are not jointly independent because $Z$ is completely determined by $X$ and $Y$.

For instance:

$$
 P(X=0,Y=0,Z=0)=\frac14, 
$$

while the product of their marginal probabilities is:

$$
 \frac12\cdot\frac12\cdot\frac12=\frac18. 
$$

The probabilities differ.

Therefore pairwise independence does not imply mutual independence.

In randomized algorithms, it is essential to know which level of independence a statistical guarantee actually requires.

#### 4 Conditional independence

Two events can be independent after conditioning on another variable even when they are dependent marginally.

Suppose two sensors independently measure a physical quantity once the true state is known. Their readings may be conditionally independent given the true state, but correlated when that state is unknown.

This is a common structure in Bayesian sensor fusion, probabilistic graphical models, and uncertainty-aware reconstruction.

Conditional independence is written:

$$
 X\perp Y\mid Z. 
$$

It means the conditional joint distribution factors:

$$
 p(x,y\mid z)=p(x\mid z)p(y\mid z) 
$$

where these conditional densities are defined.

### Bayes' theorem

Bayes' theorem is not an additional axiom. It follows from the definition of conditional probability.

We have:

$$
 P(A\mid B)=\frac{P(A\cap B)}{P(B)} 
$$

and:

$$
 P(B\mid A)=\frac{P(A\cap B)}{P(A)}. 
$$

Rearrange the second:

$$
 P(A\cap B)=P(B\mid A)P(A). 
$$

Substitute into the first:

$$
 \boxed{ P(A\mid B) = \frac{P(B\mid A)P(A)}{P(B)} }. 
$$

This is Bayes' theorem.

The terminology is:

$$
 \underbrace{P(A\mid B)}\_{\text{posterior}} = \frac{ \underbrace{P(B\mid A)}\_{\text{likelihood}} \underbrace{P(A)}\_{\text{prior}} }{ \underbrace{P(B)}\_{\text{evidence}} }. 
$$

The prior describes the probability before seeing the new information.

The likelihood describes how probable the observed information would be under the hypothesis.

The evidence normalizes across all possible hypotheses.

The posterior describes the updated probability after observing the information.

#### 1 The law of total probability

Suppose mutually exclusive events $H_1,\ldots,H_n$ form a partition of the sample space:

$$
 \bigcup_iH_i=\Omega. 
$$

For any event $E$:

$$
 E=\bigcup_i(E\cap H_i) 
$$

with disjoint intersections.

Thus:

$$
 P(E)=\sum_iP(E\cap H_i). 
$$

Apply the multiplication rule:

$$
 \boxed{ P(E)=\sum_iP(E\mid H_i)P(H_i) }. 
$$

Bayes' theorem for hypothesis $H_j$ becomes:

$$
 \boxed{ P(H_j\mid E) = \frac{ P(E\mid H_j)P(H_j) }{ \sum_iP(E\mid H_i)P(H_i) } }. 
$$

The denominator accounts for all alternative explanations of the observation.

#### 2 Worked graphics example: a noisy visibility detector

Suppose only 10% of queried rays truly intersect a small target object.

Let:

$$
 H=\text{ray truly hits the object}. 
$$

Then:

$$
 P(H)=0.10. 
$$

Suppose an approximate visibility detector reports a hit correctly 95% of the time:

$$
 P(+\mid H)=0.95. 
$$

But it also reports a false hit for 8% of rays that actually miss:

$$
 P(+\mid H^c)=0.08. 
$$

The probability of a positive report is:

$$
 \begin{aligned} P(+) &=P(+\mid H)P(H) +P(+\mid H^c)P(H^c)\\ &=0.95(0.10)+0.08(0.90)\\ &=0.095+0.072\\ &=\boxed{0.167}. \end{aligned} 
$$

Now suppose the detector reports a hit. What is the probability that the ray really hit the object?

By Bayes:

$$
 P(H\mid+) = \frac{0.95(0.10)}{0.167}. 
$$

Therefore:

$$
 \boxed{ P(H\mid+)\approx0.568862 }. 
$$

Even though the detector identifies true hits with 95% sensitivity, a positive report is correct only about 56.9% of the time under this model.

Why? Because true hits are relatively rare, while false positives are drawn from the much larger population of misses.

#### 3 Posterior odds form

Bayes can also be written using odds:

$$
 \frac{P(H\mid E)}{P(H^c\mid E)} = \frac{P(H)}{P(H^c)} \frac{P(E\mid H)}{P(E\mid H^c)}. 
$$

The last ratio is the likelihood ratio.

For our example:

$$
 \text{prior odds}=\frac{0.1}{0.9}=\frac19. 
$$

The positive-result likelihood ratio is:

$$
 \frac{0.95}{0.08}=11.875. 
$$

The posterior odds are:

$$
 \frac19(11.875)\approx1.319444. 
$$

Convert odds $o$ into probability:

$$
 P=\frac{o}{1+o}, 
$$

which again gives approximately $0.568862$.

Odds form is useful when combining several conditionally independent observations, because likelihood ratios multiply and log-likelihood ratios add.

Bayesian updates are not merely formulas for classifiers. They provide a principled method for updating uncertainty as measurements arrive.

![[Visual Recall/Math Foundations/F10-1_dice-sum.png]]

**F10-1. Ordered outcomes and the exact two-dice sum PMF.** The supplied lesson already derives this example; the figure is a visual check of the 36 ordered pairs. PMF numerators sum to 36 and P(S=7)=1/6.


<!-- end V10.02 -->

<!-- begin V10.03 -->
### Measure uncertainty along an actual direction

Return to the panel's measured corner. A random vector $X$ represents repeated possible measurements, rather than one deterministic arrow. Its mean $\mu=\mathbb E[X]$ is computed componentwise. Its covariance is

$$
\Sigma=\mathbb E[(X-\mu)(X-\mu)^{\mathsf T}].
$$

The diagonal records coordinate variances; an off-diagonal entry records how two centered components change together. The expectation assumes the required second moments exist. A distribution with sufficiently heavy tails need not have a finite covariance. Writing a matrix symbol does not establish that the underlying quantities are defined.

A covector $a$ asks one scalar question about the uncertain measurement: $Y=a^{\mathsf T}X$. By distributing the products,

$$
\mathbb E[Y]=a^{\mathsf T}\mu,\qquad
\operatorname{Var}(Y)=a^{\mathsf T}\Sigma a.
$$

Thus covariance defines a directional uncertainty ruler. It is positive semidefinite because every scalar variance is nonnegative. It can be singular when some linear combination has no variation. Positive semidefinite does not mean that every listed coordinate has positive uncertainty or that an inverse exists.

For

$$
\Sigma=\begin{pmatrix}4&3\\3&4\end{pmatrix},
\quad a_+=\frac{(1,1)}{\sqrt2},\quad
a_-=\frac{(1,-1)}{\sqrt2},
$$

the directional variances are $7$ and $1$. Both coordinate variances were $4$, yet the diagonal directions have very different uncertainty. A diagonal-only approximation would report $4$ for both and discard the correlation that matters to the panel fit.

**Uncorrelated is not independent.** Let $X$ be uniform on $[-1,1]$ and let $Y=X^2$. Symmetry gives $\mathbb E[X]=\mathbb E[X^3]=0$, so $\operatorname{Cov}(X,Y)=0$. But $Y$ is determined by $X$ and the variables are not independent. Covariance captures linear dependence, not every possible relationship. In a jointly Gaussian model, zero cross-covariance does imply independence; that stronger conclusion belongs to the model assumption.

Under a deterministic linear transformation $Z=AX+b$, covariance transforms as $A\Sigma A^{\mathsf T}$. The translation disappears after subtracting the mean. Under a nonlinear transformation $Z=f(X)$, a local approximation uses $J\Sigma J^{\mathsf T}$, where $J$ is the derivative matrix at the chosen expansion point. It can fail for large uncertainty or strong curvature. For a simple warning, $Y=X^2$ expanded around mean-zero $X$ has derivative zero, so first-order propagation predicts zero variance even though $X^2$ varies. Higher-order terms or direct integration are needed.

### Whiten residuals without pretending every ellipse has the same probability

A **residual** is a prediction error, such as measured minus predicted image coordinates. If its covariance $\Sigma$ is positive definite, factor it as $\Sigma=LL^{\mathsf T}$ with invertible lower-triangular $L$. Solve $Lz=r$ to obtain a **whitened residual** $z$. Whitening makes a random residual with that covariance have identity covariance. It does not make a non-Gaussian distribution Gaussian.

The squared Mahalanobis distance is

$$
d_M^2=r^{\mathsf T}\Sigma^{-1}r=z^{\mathsf T}z.
$$

This is a dimensionless error score when the covariance carries the correct units. For the covariance above, a residual $r=(2,2)$ gives $d_M^2=8/7$, whereas $r=(2,-2)$ gives $8$. They have equal Euclidean size, but one points along a much less certain direction and is less surprising under this model.

An ellipse $r^{\mathsf T}\Sigma^{-1}r\le1$ is a geometric level set. Its probability content depends on dimension and distribution. For a centered two-dimensional Gaussian with known covariance, the squared distance follows a chi-square distribution with two degrees of freedom, meaning the sum of squares of two independent standard normal variables; the unit ellipse contains $1-e^{-1/2}\approx39.35\%$, not the familiar one-dimensional 68% interval probability. Estimating covariance from finite data changes the inference problem again.

If covariance is singular, a pseudoinverse score must be accompanied by a support check. A residual in a direction that the model declares impossible cannot be made acceptable merely because the pseudoinverse assigns that null direction zero weight. Add an explicit observation-noise model or restrict calculations to the supported subspace as appropriate.

These definitions and linear transformation identities build on *All of Statistics*, physical PDF pages 63–71. The panel-direction examples, nonlinear counterexample, whitening trace and probability-content warning are original developments connecting probability to the vector geometry already taught here.


![[Visual Recall/Integrated Mathematics/metrics-and-measurements.png]]

**Figure — one vector, several measurement questions.** The unit balls compare three norms; the reciprocal basis separates reconstruction from coordinate measurements; the plane example compares Euclidean and weighted nearest points; the covariance ellipse has directional variances 7 and 1. It is a unit Mahalanobis level set, not a universally calibrated confidence region. Every plotted coordinate and metric is specified in the worked examples above.

## 2. Random quantities and their probability laws

### Random variables: turning outcomes into mathematical quantities

A probability space describes possible outcomes and assigns probabilities to events. But numerical algorithms usually do not operate directly on abstract outcomes. They operate on numbers and vectors derived from outcomes.

A random variable is a function that assigns a value to each outcome:

$$
 \boxed{X:\Omega\rightarrow\mathbb R}. 
$$

More precisely, it is a measurable function, meaning events of the form $X\le x$ belong to the probability space's event collection.

Consider rolling two dice. The complete outcome is an ordered pair:

$$
 \omega=(d_1,d_2). 
$$

The sample space has 36 outcomes:

$$
 \Omega=\{1,\ldots,6\}^2. 
$$

We might define a random variable:

$$
 X(d_1,d_2)=d_1+d_2. 
$$

Then $X$ represents the sum of the dice, with possible values from 2 to 12.

Notice that many outcomes produce the same random-variable value. For example:

$$
 X(1,3)=X(2,2)=X(3,1)=4. 
$$

The probability that $X=4$ is the total probability of all outcomes mapped to four.

For independent fair dice:

$$
 P(X=4)=\frac3{36}=\frac1{12}. 
$$

A random variable is therefore not the same as a single random outcome. It is a numerical measurement or transformation of the outcome.

#### 1 Random variables are functions, not mysterious changing numbers

The notation $X$ represents the entire function from outcomes to values. A realized value $X(\omega)$ is one number.

When we write:

$$
 P(X\le4), 
$$

we mean:

$$
 P(\{\omega\in\Omega:X(\omega)\le4\}). 
$$

This explains how events involving numerical values arise from the original probability space.

#### 2 Random vectors

A random vector is:

$$
 \mathbf X:\Omega\rightarrow\mathbb R^n. 
$$

For a noisy camera observation:

$$
 \mathbf X= \begin{bmatrix} U\\V \end{bmatrix} 
$$

could represent measured pixel coordinates.

Both coordinates arise from the same measurement process, so their uncertainties may be correlated.

A random vector allows us to represent the joint behavior of those coordinates instead of modeling each independently.

This becomes essential for multivariate Gaussians, Kalman filtering, sensor fusion, and uncertainty-aware geometry.

#### 3 A random variable need not have a density

Every real-valued random variable has a cumulative distribution function. It does not necessarily have a probability density function or a probability mass function covering all of its behavior.

Some variables are discrete, some continuous with a density, and some have mixed or singular distributions.

For example, suppose a detector returns exactly zero with probability $0.2$, but otherwise produces a continuous value.

Then the distribution has a point mass at zero and a continuous component elsewhere.

Such models are useful for zero-inflated measurements, contact activation, clipped observations, and sparse events.

We should not assume that every random variable is either purely discrete or purely continuous with a PDF.

### Discrete random variables

A discrete random variable takes values in a finite or countable set.

Examples include the number of rays that hit a light, the number of detected features, the number of failures before success, or the number of photons arriving in a time interval under an appropriate model.

Let:

$$
 X\in\{0,1,2,\ldots\}. 
$$

Its behavior is determined by the probabilities of its possible values:

$$
 P(X=k). 
$$

#### 1 Countably infinite distributions

Discrete does not necessarily mean finitely many outcomes.

A Poisson variable takes nonnegative integer values with no finite upper bound:

$$
 0,1,2,\ldots 
$$

A geometric variable can represent how many Bernoulli trials are needed before a first success.

These variables have countably infinitely many possible values.

Their probabilities must sum to one.

#### 2 An example with two dice

Let $X$ denote the sum of two fair dice.

The probability of each sum is not uniform.

| Sum $x$ | Number of outcomes | $P(X=x)$ |
| ----------- | ------------------ | ------------ |
| 2           | 1                  | $1/36$   |
| 3           | 2                  | $2/36$   |
| 4           | 3                  | $3/36$   |
| 5           | 4                  | $4/36$   |
| 6           | 5                  | $5/36$   |
| 7           | 6                  | $6/36$   |
| 8           | 5                  | $5/36$   |
| 9           | 4                  | $4/36$   |
| 10          | 3                  | $3/36$   |
| 11          | 2                  | $2/36$   |
| 12          | 1                  | $1/36$   |

The most probable sum is seven because more underlying outcomes map to seven than to any other sum.

This demonstrates that even uniformly sampled underlying outcomes can produce nonuniform distributions after applying a function.

### Continuous random variables

A continuous random variable with a probability density assigns probability to intervals through integration.

Suppose:

$$
 X\sim U(0,1), 
$$

meaning $X$ is uniformly distributed between zero and one.

Then:

$$
 P(0.2\le X\le0.5)=0.3. 
$$

But:

$$
 P(X=0.3)=0. 
$$

#### 1 Why individual exact values have zero probability

Imagine a tiny interval around $x$ of width $\Delta x$.

For a sufficiently smooth density:

$$
 P(x\le X\le x+\Delta x) \approx p_X(x)\Delta x. 
$$

As:

$$
 \Delta x\to0, 
$$

the probability approaches zero, provided the density remains finite near that point.

The density itself need not approach zero.

This is why a PDF value of $0.4$ is not a 40% probability.

A PDF is probability per unit of the variable, not probability assigned to one exact value.

#### 2 Units of a density

Suppose $X$ is a measured length in metres.

Its density $p_X(x)$ has units:

$$
 \mathrm m^{-1}. 
$$

Then:

$$
 p_X(x)\\,dx 
$$

is dimensionless because:

$$
 \mathrm m^{-1}\times\mathrm m=1. 
$$

If a random vector contains three spatial coordinates, its density with respect to 3D volume has units:

$$
 \mathrm m^{-3}. 
$$

This unit dependence is important in rendering.

A directional PDF defined with respect to solid angle has units of inverse steradians. A PDF defined with respect to surface area has units of inverse square metres.

They cannot be substituted into an estimator without accounting for the change of measure.

We will derive those conversions in the sampling and Monte Carlo chapters.

### Probability mass functions

For a discrete random variable, define the probability mass function, or PMF:

$$
 \boxed{ p_X(x)=P(X=x) }. 
$$

The PMF must satisfy:

$$
 p_X(x)\ge0 
$$

for every possible value, and:

$$
 \boxed{ \sum_xp_X(x)=1 }. 
$$

For a Bernoulli variable with success probability $p$:

$$
 p_X(0)=1-p, \qquad p_X(1)=p. 
$$

All other values have probability zero.

#### 1 PMFs can be represented explicitly or procedurally

For a small finite distribution, we can store every probability in an array.

For example, a light-selection distribution might use:

$$
 [p_0,p_1,p_2]=[0.5,0.3,0.2]. 
$$

For a Poisson or binomial distribution, calculating mass from a formula may be more convenient than storing a table.

For a large distribution with repeated sampling, an alias table can make sampling efficient, while cumulative sums or binary-search structures provide alternative trade-offs.

Those algorithms belong to the sampling and Monte Carlo chapters, but the critical invariant already comes from the PMF:

$$
 \sum_ip_i=1. 
$$

If we store unnormalized nonnegative weights $w_i$, we must convert them into probabilities:

$$
 \boxed{ p_i=\frac{w_i}{\sum_jw_j} } 
$$

provided the total is finite and positive.

#### 2 A PMF is not a histogram

A PMF is a property of a probability distribution.

A histogram is a numerical summary of observed samples.

Suppose we take 100 random samples and observe 32 successes.

The observed frequency:

$$
 \frac{32}{100}=0.32 
$$

is an estimate of the success probability, not proof that the true Bernoulli parameter equals $0.32$.

A histogram may approximate a distribution, but sampling variability and bin choices affect the result.

We will examine estimation and statistical error in the statistical validation chapters.

### Probability density functions

For a continuous random variable whose distribution is absolutely continuous with respect to Lebesgue measure, a probability density function, or PDF, is a nonnegative function $p_X$ satisfying:

$$
 \boxed{ P(a\le X\le b) = \int_a^bp_X(x)\\,dx }. 
$$

Normalization requires:

$$
 \boxed{ \int\_{-\infty}^{\infty}p_X(x)\\,dx=1 }. 
$$

#### 1 A density can exceed one

Consider:

$$
 X\sim U(0,0.1). 
$$

Its density is:

$$
 p_X(x)=10 
$$

inside the interval and zero elsewhere.

That PDF value exceeds one.

There is no contradiction because the density is not a probability.

Its integrated probability is:

$$
 \int_0^{0.1}10\\,dx=1. 
$$

#### 2 The probability associated with a small region

For a sufficiently smooth density:

$$
 P(x\le X\le x+h) \approx p_X(x)h. 
$$

The approximation error depends on how much the density varies over the interval.

In several dimensions:

$$
 P(\mathbf X\in R) = \int_Rp\_{\mathbf X}(\mathbf x)\\,d\mathbf x. 
$$

If $R$ is sufficiently small, we may approximate:

$$
 P(\mathbf X\in R) \approx p\_{\mathbf X}(\mathbf x_0)\operatorname{Volume}(R) 
$$

when the density is nearly constant over that region.

This is directly relevant to Gaussian footprints, spatial uncertainty, and local probability approximation.

#### 3 Densities depend on coordinates

Suppose:

$$
 Y=2X, 
$$

where $X\sim U(0,1)$.

Then:

$$
 Y\sim U(0,2). 
$$

The new density is:

$$
 p_Y(y)=\frac12 
$$

on $[0,2]$.

Why did the density decrease?

The transformation doubled interval lengths. To preserve total probability, density must be halved.

The general one-dimensional transformation rule for a smooth monotone invertible function $Y=g(X)$ is:

$$
 \boxed{ p_Y(y) = p_X(g^{-1}(y)) \left| \frac{d}{dy}g^{-1}(y) \right| }. 
$$

For an invertible multivariable mapping $\mathbf y=g(\mathbf x)$:

$$
 \boxed{ p\_{\mathbf Y}(\mathbf y) = p\_{\mathbf X}(g^{-1}(\mathbf y)) \left| \det J\_{g^{-1}}(\mathbf y) \right| }. 
$$

These formulas follow from conservation of probability under a change of variables. More general transformations may require summing contributions from multiple inverse branches.

This is the bridge between probability densities and Phase 9's Jacobians. It becomes central to importance sampling and probability-density transformations.

### Cumulative distribution functions

The cumulative distribution function, or CDF, is defined for every real-valued random variable:

$$
 \boxed{ F_X(x)=P(X\le x) }. 
$$

It measures all probability accumulated up to the threshold $x$.

Unlike a PDF or PMF, the CDF is defined for discrete, continuous, mixed, and singular distributions.

#### 1 Fundamental properties

A CDF is nondecreasing:

$$
 x_1\<x_2 \implies F_X(x_1)\le F_X(x_2). 
$$

It is right-continuous, and:

$$
 \lim\_{x\to-\infty}F_X(x)=0, 
$$

$$
 \lim\_{x\to+\infty}F_X(x)=1. 
$$

For a continuous distribution with density:

$$
 \boxed{ F_X(x)=\int\_{-\infty}^{x}p_X(t)\\,dt }. 
$$

Where differentiable:

$$
 \boxed{ F_X'(x)=p_X(x) }. 
$$

This is the fundamental theorem of calculus applied to probability accumulation.

#### 2 CDF of a uniform variable

For:

$$
 X\sim U(a,b), 
$$

the CDF is:

$$
 \boxed{ F_X(x)= \begin{cases} 0,&x\<a,\\ \dfrac{x-a}{b-a},&a\le x\<b,\\ 1,&x\ge b. \end{cases} } 
$$

The density is constant in the interval, so cumulative probability increases linearly.

#### 3 CDF of a discrete variable

For a discrete random variable:

$$
 F_X(x)= \sum\_{t\le x}p_X(t). 
$$

Its graph contains jumps at values with nonzero probability mass.

In fact:

$$
 \boxed{ P(X=x) = F_X(x)-F_X(x^-) } 
$$

where $F_X(x^-)$ denotes the left-hand limit.

This formula shows how discrete point masses are encoded by jumps in the CDF.

#### 4 Quantiles and inverse CDFs

A quantile function can be defined as:

$$
 \boxed{ Q(u)=\inf\{x:F_X(x)\ge u\} }, \qquad u\in(0,1). 
$$

For a continuous strictly increasing CDF, this reduces to:

$$
 Q(u)=F_X^{-1}(u). 
$$

If $U\sim U(0,1)$, then:

$$
 X=Q(U) 
$$

has the desired distribution under the standard inverse-transform construction.

This is a powerful connection: a uniform random number can be transformed into a random variable from another distribution using the inverse CDF.

We will implement this technique in the sampling and Monte Carlo chapters.

**Concept link:** [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error#2. Random quantities and their probability laws|random law]] — A random variable is a measurable mapping from outcomes….



<!-- end V10.03 -->

<!-- begin V10.04 -->
## 3. Expected values describe centers; covariance describes dependence

### Expected value: what a random quantity averages to

The expected value describes a probability-weighted average.

For a discrete variable:

$$
 \boxed{ \mathbb E[X]=\sum_xx\\,p_X(x) } 
$$

when the sum is well defined.

For a continuous variable with PDF:

$$
 \boxed{ \mathbb E[X]= \int\_{-\infty}^{\infty}x\\,p_X(x)\\,dx } 
$$

when the expectation exists.

More generally:

$$
 \mathbb E[X]=\int\_\Omega X(\omega)\\,dP(\omega). 
$$

That last expression is the measure-theoretic definition and handles random variables without requiring a PDF.

#### 1 Expectation is not necessarily a possible outcome

For a fair six-sided die:

$$
 \mathbb E[X] = \frac{1+2+3+4+5+6}{6} =\boxed{3.5}. 
$$

The die can never actually show $3.5$.

Expected value is a summary of a distribution, not necessarily a realizable result.

#### 2 Deriving expectation of a function

Suppose we want:

$$
 \mathbb E[g(X)]. 
$$

For discrete $X$:

$$
 \boxed{ \mathbb E[g(X)] = \sum_xg(x)p_X(x) }. 
$$

For continuous $X$:

$$
 \boxed{ \mathbb E[g(X)] = \int g(x)p_X(x)\\,dx }. 
$$

This is often called the law of the unconscious statistician, or LOTUS.

We do not need to derive the full distribution of $Y=g(X)$ before calculating its expectation.

For example, if:

$$
 X\sim U(0,1), 
$$

then:

$$
 \mathbb E[X^2] = \int_0^1x^2\\,dx =\boxed{\frac13}. 
$$

#### 3 Linearity of expectation

For random variables $X,Y$ and constants $a,b,c$:

$$
 \boxed{ \mathbb E[aX+bY+c] = a\mathbb E[X]+b\mathbb E[Y]+c }. 
$$

This follows from linearity of summation or integration.

Independence is not required.

This is one of the most useful probability rules in algorithm analysis.

Suppose:

$$
 X=\sum\_{i=1}^NI_i 
$$

where each indicator $I_i\in\{0,1\}$ represents whether a ray hits a surface.

Then:

$$
 \mathbb E[X] = \sum_i\mathbb E[I_i] = \sum_iP(I_i=1). 
$$

The expected number of hits is the sum of the individual hit probabilities even when the hit events are dependent.

#### 4 Expectation of a product

For independent integrable random variables under suitable assumptions:

$$
 \boxed{ \mathbb E[XY]=\mathbb E[X]\mathbb E[Y] }. 
$$

But this factorization does not generally hold for dependent variables.

Its failure motivates covariance.

#### 5 Expected value as an integral: the rendering connection

Consider an integral:

$$
 I=\int_Df(x)\\,dx. 
$$

If we sample $X$ uniformly over a domain $D$ of finite volume $V$, its PDF is:

$$
 p_X(x)=\frac1V. 
$$

Then:

$$
 \mathbb E[f(X)] = \int_Df(x)\frac1V\\,dx. 
$$

Therefore:

$$
 \boxed{ I=V\\,\mathbb E[f(X)] }. 
$$

More generally, if a PDF $p(x)>0$ wherever the integrand contributes:

$$
 \boxed{ I= \mathbb E\_{X\sim p} \left[ \frac{f(X)}{p(X)} \right] }. 
$$

This identity is the mathematical foundation of Monte Carlo integration and importance sampling. It does not yet tell us how many samples we need, how accurate an estimator is, or how to minimize its variance; those belong to the sampling and Monte Carlo chapters.

Eric Veach's Robust Monte Carlo Methods for Light Transport Simulation develops this framework for light-transport integrals and multiple importance sampling.

### Variance: measuring spread around the mean

Expected value gives the distribution's centre in an average sense.

But two random variables can have the same mean and very different uncertainty.

Consider:

$$
 X=5 
$$

with probability one, and:

$$
 Y= \begin{cases} 0,&\text{probability }1/2,\\ 10,&\text{probability }1/2. \end{cases} 
$$

Both have expected value five.

Yet $X$ is perfectly predictable, while $Y$ varies substantially.

Variance captures that difference.

Define the mean:

$$
 \mu=\mathbb E[X]. 
$$

The variance is:

$$
 \boxed{ \operatorname{Var}(X)= \mathbb E[(X-\mu)^2] }. 
$$

The square makes deviations nonnegative and gives greater weight to larger deviations.

#### 1 Deriving the computational formula

Expand:

$$
 (X-\mu)^2=X^2-2\mu X+\mu^2. 
$$

Take expectation:

$$
 \operatorname{Var}(X) = \mathbb E[X^2] -2\mu\mathbb E[X] +\mu^2. 
$$

Because:

$$
 \mathbb E[X]=\mu, 
$$

we get:

$$
 \operatorname{Var}(X) = \mathbb E[X^2]-2\mu^2+\mu^2. 
$$

Thus:

$$
 \boxed{ \operatorname{Var}(X)= \mathbb E[X^2]-(\mathbb E[X])^2 }. 
$$

This identity is mathematically useful, but it can be numerically unstable when both terms are large and nearly equal.

For sampled data, a centered two-pass or online Welford-type method may be preferable to subtracting two nearly equal large numbers. We will examine that in the statistical validation chapters.

#### 2 Numerical example: variance of a two-point distribution

For $Y\in\{0,10\}$ with equal probabilities:

$$
 \mu=5. 
$$

Then:

$$
 \begin{aligned} \operatorname{Var}(Y) &=\frac12(0-5)^2+\frac12(10-5)^2\\ &=\frac12(25)+\frac12(25)\\ &=\boxed{25}. \end{aligned} 
$$

For the constant variable $X=5$:

$$
 \operatorname{Var}(X)=0. 
$$

A random variable has zero variance if and only if it equals its mean almost surely, provided the relevant expectation exists.

#### 3 Variance under scaling and translation

Let:

$$
 Y=aX+b. 
$$

Its mean is:

$$
 \mathbb E[Y]=a\mu+b. 
$$

The centred quantity is:

$$
 Y-\mathbb E[Y]=a(X-\mu). 
$$

Therefore:

$$
 \boxed{ \operatorname{Var}(aX+b)=a^2\operatorname{Var}(X) }. 
$$

Adding a constant does not affect variance.

Multiplying by two multiplies variance by four.

This matters when converting units. If position uncertainty is represented in metres and then converted to millimetres, its variance is multiplied by $1000^2=10^6$.

#### 4 Variance of a sum

For two random variables:

$$
 \operatorname{Var}(X+Y) = \operatorname{Var}(X)+ \operatorname{Var}(Y)+ 2\operatorname{Cov}(X,Y). 
$$

Only when covariance is zero does the variance equal the sum of individual variances.

Independence implies zero covariance when the required moments exist, but zero covariance does not generally imply independence.

We will derive covariance next.

### Standard deviation

Variance uses squared deviations, so its units are the square of the underlying variable's units.

If $X$ is measured in metres, then:

$$
 \operatorname{Var}(X) 
$$

has units of square metres.

The standard deviation is:

$$
 \boxed{ \sigma_X=\sqrt{\operatorname{Var}(X)} }. 
$$

It has the same physical units as $X$.

For the earlier random variable with variance 25:

$$
 \sigma_Y=\sqrt{25}=5. 
$$

The standard deviation measures the scale of dispersion, but it does not universally mean that most observations lie within one standard deviation of the mean.

That familiar interpretation requires information about the distribution's shape.

For a Gaussian, roughly 68% of probability lies within one standard deviation, 95% within two, and 99.7% within three.

For a heavy-tailed or highly asymmetric distribution, these percentages can be very different.

#### 1 Population versus sample standard deviation

The theoretical standard deviation of a distribution is a population parameter.

Given a finite sample $x_1,\ldots,x_n$, we estimate the underlying spread.

Two common sample variance formulas are:

$$
 \frac1n\sum_i(x_i-\bar x)^2 
$$

and:

$$
 \frac1{n-1}\sum_i(x_i-\bar x)^2. 
$$

The second is the usual unbiased sample variance estimator under independent identically distributed sampling with finite variance.

But taking its square root does not generally produce an unbiased estimator of the population standard deviation.

We will derive the Bessel correction and streaming algorithms in the statistical validation chapters. For now, keep the distinction between a distribution's exact moment and an estimated moment from finite observations.

### Covariance: measuring coupled uncertainty

Variance describes the spread of one variable.

Covariance describes how two variables vary together.

Let:

$$
 \mu_X=\mathbb E[X], \qquad \mu_Y=\mathbb E[Y]. 
$$

The covariance is:

$$
 \boxed{ \operatorname{Cov}(X,Y)= \mathbb E[(X-\mu_X)(Y-\mu_Y)] }. 
$$

#### 1 Deriving the alternative formula

Expand:

$$
 (X-\mu_X)(Y-\mu_Y) = XY-\mu_XY-\mu_YX+\mu_X\mu_Y. 
$$

Take expectation:

$$
 \begin{aligned} \operatorname{Cov}(X,Y) &=\mathbb E[XY] -\mu_X\mathbb E[Y] -\mu_Y\mathbb E[X] +\mu_X\mu_Y\\ &=\mathbb E[XY] -\mu_X\mu_Y-\mu_X\mu_Y+\mu_X\mu_Y. \end{aligned} 
$$

Therefore:

$$
 \boxed{ \operatorname{Cov}(X,Y) = \mathbb E[XY]-\mathbb E[X]\mathbb E[Y] }. 
$$

#### 2 Positive covariance

Suppose $X$ measures horizontal camera-position error and $Y$ measures horizontal image-position error.

If increasing $X$ tends to increase $Y$, the covariance may be positive.

Positive covariance means their centred values tend to have the same sign in a probability-weighted sense.

It does not mean every individual observation follows that relationship.

#### 3 Negative covariance

Suppose increasing an input position tends to decrease a projected output coordinate because of a coordinate-sign convention.

Then the random errors may exhibit negative covariance.

Negative covariance means the centred deviations tend to have opposite signs.

#### 4 Zero covariance

If:

$$
 \operatorname{Cov}(X,Y)=0, 
$$

the variables are uncorrelated.

This means there is no covariance-measured linear relationship between their centred values.

It does not rule out nonlinear dependence.

#### 5 Covariance under scaling

For constants $a,b,c,d$:

$$
 \boxed{ \operatorname{Cov}(aX+b,cY+d) = ac\operatorname{Cov}(X,Y) }. 
$$

Translation has no effect.

Scaling both variables changes covariance according to the product of their scale factors.

If one variable reverses sign, covariance reverses sign.

#### 6 Deriving variance of a sum

Let:

$$
 Z=X+Y. 
$$

Its centred value is:

$$
 Z-\mathbb E[Z] = (X-\mu_X)+(Y-\mu_Y). 
$$

Square:

$$
 \begin{aligned} (Z-\mathbb E[Z])^2 &=(X-\mu_X)^2+(Y-\mu_Y)^2\\ &\quad+2(X-\mu_X)(Y-\mu_Y). \end{aligned} 
$$

Take expectation:

$$
 \boxed{ \operatorname{Var}(X+Y) = \operatorname{Var}(X)+ \operatorname{Var}(Y)+ 2\operatorname{Cov}(X,Y) }. 
$$

For a difference:

$$
 \boxed{ \operatorname{Var}(X-Y) = \operatorname{Var}(X)+ \operatorname{Var}(Y)- 2\operatorname{Cov}(X,Y) }. 
$$

These identities explain why positive correlation can increase uncertainty in a sum but decrease uncertainty in a difference.

#### 7 The covariance matrix

For:

$$
 \mathbf X= \begin{bmatrix}X_1\\X_2\\vdots\\X_n\end{bmatrix}, 
$$

define:

$$
 \boldsymbol\mu=\mathbb E[\mathbf X]. 
$$

Its [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error#3. Expected values describe centers; covariance describes dependence|covariance matrix]] is:

$$
 \boxed{ \Sigma= \mathbb E[ (\mathbf X-\boldsymbol\mu) (\mathbf X-\boldsymbol\mu)^T ] }. 
$$

The entries are:

$$
 \Sigma\_{ij}=\operatorname{Cov}(X_i,X_j). 
$$

Diagonal entries are variances:

$$
 \Sigma\_{ii}=\operatorname{Var}(X_i). 
$$

Off-diagonal entries are pairwise covariances.

We will study the matrix's symmetry, eigenvalues, and positive-semidefinite requirement fully in the distribution and covariance chapters.

### Correlation: normalized covariance

Covariance depends on the units of its variables.

For example, covariance between position and velocity has units:

$$
 \mathrm{m^2/s}. 
$$

It is difficult to compare such raw covariances across unrelated measurement scales.

The Pearson correlation coefficient normalizes covariance by the standard deviations:

$$
 \boxed{ \rho\_{XY}= \frac{ \operatorname{Cov}(X,Y) }{ \sigma_X\sigma_Y } }. 
$$

It is defined when both standard deviations are finite and strictly positive.

#### 1 Why correlation lies between -1 and 1

The Cauchy–Schwarz inequality for square-integrable random variables gives:

$$
 |\mathbb E[UV]| \le \sqrt{\mathbb E[U^2]\mathbb E[V^2]}. 
$$

Let:

$$
 U=X-\mu_X, \qquad V=Y-\mu_Y. 
$$

Then:

$$
 |\operatorname{Cov}(X,Y)| \le \sigma_X\sigma_Y. 
$$

Divide by the positive product:

$$
 \boxed{ -1\le\rho\_{XY}\le1 }. 
$$

Correlation $+1$ or $-1$ corresponds to an exact affine relationship almost surely when both variances are positive.

#### 2 Zero correlation is not independence

Let:

$$
 X\sim U(-1,1) 
$$

and define:

$$
 Y=X^2. 
$$

Clearly, $Y$ is completely determined by $X$, so the variables are dependent.

But:

$$
 \mathbb E[X]=0 
$$

by symmetry.

Also:

$$
 \mathbb E[XY] = \mathbb E[X^3]=0 
$$

because the integrand is odd over the symmetric interval.

Therefore:

$$
 \operatorname{Cov}(X,Y) = \mathbb E[XY]-\mathbb E[X]\mathbb E[Y] =0. 
$$

Thus:

$$
 \boxed{\rho\_{XY}=0} 
$$

even though $Y=X^2$.

The correlation coefficient captures linear association, not all possible dependence.

A special exception will appear for jointly Gaussian variables: within a joint multivariate Gaussian model, zero cross-covariance implies independence of the corresponding components or blocks.

### Joint distributions

So far, we have often treated random variables one at a time.

But uncertainty in a real system is usually coupled.

A joint distribution describes multiple variables together.

For discrete $X,Y$, define:

$$
 \boxed{ p\_{X,Y}(x,y)=P(X=x,Y=y) }. 
$$

For continuous variables with a joint density:

$$
 \boxed{ P((X,Y)\in R) = \iint_Rp\_{X,Y}(x,y)\\,dx\\,dy }. 
$$

#### 1 A complete joint distribution example

Consider two binary variables $X,Y$ with joint PMF:

|              | $Y=0$ | $Y=1$ | Row total |
| ------------ | --------- | --------- | --------- |
| $X=0$    | 0.40      | 0.10      | 0.50      |
| $X=1$    | 0.20      | 0.30      | 0.50      |
| Column total | 0.60      | 0.40      | 1.00      |

Every entry is nonnegative, and all joint probabilities sum to one.

The marginal probability of $X=1$ is:

$$
 P(X=1)=0.20+0.30=0.50. 
$$

The marginal probability of $Y=1$ is:

$$
 P(Y=1)=0.10+0.30=0.40. 
$$

The probability of both being one is:

$$
 P(X=1,Y=1)=0.30. 
$$

If $X$ and $Y$ were independent, the joint probability of both being one would be:

$$
 0.5(0.4)=0.2. 
$$

But the actual value is $0.3$, so they are dependent.

#### 2 Compute the covariance

Since $X,Y\in\{0,1\}$:

$$
 \mathbb E[X]=P(X=1)=0.5, 
$$

$$
 \mathbb E[Y]=P(Y=1)=0.4. 
$$

Their product $XY$ is one only when both variables equal one. Therefore:

$$
 \mathbb E[XY]=0.3. 
$$

Thus:

$$
 \operatorname{Cov}(X,Y) = 0.3-(0.5)(0.4) =\boxed{0.1}. 
$$

The variances are:

$$
 \operatorname{Var}(X)=0.5(0.5)=0.25, 
$$

$$
 \operatorname{Var}(Y)=0.4(0.6)=0.24. 
$$

The correlation is:

$$
 \rho= \frac{0.1}{\sqrt{0.25(0.24)}} =\boxed{0.408248\ldots}. 
$$

So the two binary variables have positive linear association.

### Marginal distributions

A marginal distribution describes one variable without retaining the detailed value of the other variables.

For discrete random variables:

$$
 \boxed{ p_X(x)=\sum_yp\_{X,Y}(x,y) }. 
$$

For continuous variables:

$$
 \boxed{ p_X(x)= \int\_{-\infty}^{\infty} p\_{X,Y}(x,y)\\,dy }. 
$$

We are summing or integrating over the variables we no longer want to represent explicitly.

This operation is called marginalization.

#### 1 Why marginalization works

The event $X=x$ can occur together with any permitted value of $Y$.

In the discrete case:

$$
 \{X=x\} = \bigcup_y\{X=x,Y=y\}. 
$$

The component events for distinct $y$ are disjoint.

Therefore:

$$
 P(X=x)=\sum_yP(X=x,Y=y). 
$$

The continuous case is the analogous integration over the unwanted variable.

#### 2 Marginalization loses dependence information

Return to our binary joint distribution.

The marginals are:

$$
 P(X=1)=0.5, \qquad P(Y=1)=0.4. 
$$

Knowing only these two values does not tell us whether $X$ and $Y$ are independent, positively correlated, or negatively correlated.

Different joint distributions can share the same marginals.

For instance, the independent joint distribution with the same marginals would have:

$$
 P(X=1,Y=1)=0.2. 
$$

Our actual joint distribution has:

$$
 P(X=1,Y=1)=0.3. 
$$

Thus marginal distributions describe the individual variables but do not preserve the full dependence structure.

This matters when combining uncertain sensor measurements or modeling covariance between reconstructed coordinates.

### Conditional distributions

A conditional distribution describes one variable after accounting for information about another.

For discrete variables:

$$
 \boxed{ p\_{Y\mid X}(y\mid x) = \frac{p\_{X,Y}(x,y)}{p_X(x)} } 
$$

when $p_X(x)>0$.

For continuous variables with a joint density:

$$
 \boxed{ p\_{Y\mid X}(y\mid x) = \frac{p\_{X,Y}(x,y)}{p_X(x)} } 
$$

where the marginal density is positive and the relevant conditional density exists.

The formulas look identical, but their interpretations differ: one divides probability masses, the other divides densities.

#### 1 Conditional distribution from our joint table

We know:

$$
 P(X=1)=0.5. 
$$

Therefore:

$$
 P(Y=1\mid X=1) = \frac{0.3}{0.5} =\boxed{0.6}. 
$$

Similarly:

$$
 P(Y=1\mid X=0) = \frac{0.1}{0.5} =\boxed{0.2}. 
$$

Knowing $X$ changes the success probability of $Y$ considerably.

The conditional distribution of $Y$ is Bernoulli with parameter $0.6$ when $X=1$, and Bernoulli with parameter $0.2$ when $X=0$.

#### 2 Law of total expectation

The expected value of $Y$ conditional on $X$ is itself a function of $X$:

$$
 \mathbb E[Y\mid X]. 
$$

The law of total expectation states:

$$
 \boxed{ \mathbb E[Y]= \mathbb E[\mathbb E[Y\mid X]] }. 
$$

For our example:

$$
 \mathbb E[Y\mid X=0]=0.2, 
$$

$$
 \mathbb E[Y\mid X=1]=0.6. 
$$

Since:

$$
 P(X=0)=P(X=1)=0.5, 
$$

the outer expectation is:

$$
 \begin{aligned} \mathbb E[\mathbb E[Y\mid X]] &=0.5(0.2)+0.5(0.6)\\ &=0.1+0.3\\ &=\boxed{0.4}. \end{aligned} 
$$

This agrees with the marginal expected value of $Y$.

#### 3 Law of total variance

An even deeper result is:

$$
 \boxed{ \operatorname{Var}(Y) = \mathbb E[\operatorname{Var}(Y\mid X)] + \operatorname{Var}(\mathbb E[Y\mid X]) }. 
$$

The first term measures uncertainty remaining within each conditional situation.

The second measures uncertainty caused by not knowing which conditional situation applies.

Derive it by writing:

$$
 Y-\mathbb E[Y] = (Y-\mathbb E[Y\mid X]) + (\mathbb E[Y\mid X]-\mathbb E[Y]). 
$$

Square both sides and take expectation. The cross term vanishes because the conditional residual has conditional expectation zero.

For our example, conditional Bernoulli variances are:

$$
 \operatorname{Var}(Y\mid X=0)=0.2(0.8)=0.16, 
$$

$$
 \operatorname{Var}(Y\mid X=1)=0.6(0.4)=0.24. 
$$

Their average is:

$$
 \mathbb E[\operatorname{Var}(Y\mid X)] = 0.5(0.16)+0.5(0.24) =\boxed{0.20}. 
$$

The conditional means are $0.2$ and $0.6$, each with probability $0.5$. Their overall mean is $0.4$, so:

$$
 \operatorname{Var}(\mathbb E[Y\mid X]) = 0.5(0.2-0.4)^2+ 0.5(0.6-0.4)^2 =\boxed{0.04}. 
$$

Therefore:

$$
 \operatorname{Var}(Y)=0.20+0.04=\boxed{0.24}. 
$$

This again matches the marginal Bernoulli variance:

$$
 0.4(0.6)=0.24. 
$$

#### 4 Why this matters for graphics and uncertainty

Suppose a reconstructed point's position is uncertain because of two sources: sensor noise and uncertainty in camera pose.

Conditioning on a known camera pose may leave only sensor-induced position uncertainty.

When camera pose is also uncertain, the total position variance contains additional spread arising from different possible camera poses.

The law of total variance provides the mathematical structure for separating these contributions.

This is especially important in probabilistic reconstruction, hierarchical uncertainty models, sensor fusion, and uncertainty-aware rendering.

the probability foundations chapter established the general language of probability. We can now describe events, condition on information, construct random variables, evaluate distributions, and compute moments.

the distribution and covariance chapters studies specific distribution families.

A distribution family is useful because it packages a probability law into a small set of parameters. Instead of storing a complete arbitrary PMF or PDF, we describe the distribution through quantities such as a success probability, event rate, mean, variance, or covariance matrix.

Each distribution has a particular mathematical structure and set of modeling assumptions. Choosing the wrong family can produce misleading uncertainty estimates even if every subsequent calculation is algebraically correct.

![[Visual Recall/Math Foundations/F10-2_covariance-dependence.png]]

**F10-2. Zero covariance need not mean independence.** For standard normal X, Cov(X,X squared)=0 by symmetry although X and X squared are dependent. Sample covariance is a finite-run witness.


<!-- end V10.04 -->

<!-- begin V10.05 -->
## 4. Discrete and continuous distribution families

### Bernoulli distribution

A Bernoulli random variable represents one binary trial.

Its possible values are:

$$
 X\in\{0,1\}. 
$$

Let $p\in[0,1]$ be the probability of success:

$$
 P(X=1)=p, 
$$

$$
 P(X=0)=1-p. 
$$

We write:

$$
 \boxed{X\sim\operatorname{Bernoulli}(p)}. 
$$

The PMF can be expressed compactly as:

$$
 \boxed{ P(X=x)=p^x(1-p)^{1-x}, \qquad x\in\{0,1\} }. 
$$

When $x=1$, the expression gives $p$. When $x=0$, it gives $1-p$. At the endpoints $p=0$ and $p=1$, the two-case definition is preferable to relying on expressions involving $0^0$.

#### 1 Deriving the mean

By definition:

$$
 \mathbb E[X] = 0(1-p)+1(p). 
$$

Therefore:

$$
 \boxed{\mathbb E[X]=p}. 
$$

The mean of a Bernoulli variable is exactly its success probability.

#### 2 Deriving the variance

Because $X$ is either zero or one:

$$
 X^2=X. 
$$

Thus:

$$
 \mathbb E[X^2]=\mathbb E[X]=p. 
$$

Using:

$$
 \operatorname{Var}(X)= \mathbb E[X^2]-(\mathbb E[X])^2, 
$$

we obtain:

$$
 \boxed{ \operatorname{Var}(X)=p(1-p) }. 
$$

The variance is zero at $p=0$ and $p=1$, where the outcome is deterministic.

It is maximized at $p=0.5$, where:

$$
 \operatorname{Var}(X)=0.25. 
$$

To prove the maximum, differentiate:

$$
 v(p)=p-p^2, 
$$

$$
 v'(p)=1-2p. 
$$

Set:

$$
 v'(p)=0 
$$

to obtain $p=1/2$. Since $v''(p)=-2$, this is a maximum.

#### 3 Bernoulli variables as indicators

For any event $A$, define the indicator:

$$
 I_A= \begin{cases} 1,&A\text{ occurs},\\ 0,&A\text{ does not occur}. \end{cases} 
$$

Then:

$$
 I_A\sim\operatorname{Bernoulli}(P(A)). 
$$

Thus:

$$
 \boxed{\mathbb E[I_A]=P(A)}. 
$$

This identity is extremely useful.

A visibility test, hit/miss result, successful sample, accepted rejection-sampling candidate, or completed stochastic trial can often be represented by an indicator.

#### 4 Bernoulli likelihood and cross-entropy

Suppose an observed binary outcome is $x\in\{0,1\}$, and our model predicts probability $p$.

The likelihood of the observation is:

$$
 L(p)=p^x(1-p)^{1-x}. 
$$

Take logarithms:

$$
 \log L(p)= x\log p+(1-x)\log(1-p). 
$$

The negative log-likelihood is:

$$
 \boxed{ -\log L(p)= -x\log p-(1-x)\log(1-p) }. 
$$

This is the binary cross-entropy expression used in statistical classification.

It is not an arbitrary machine-learning formula. It comes directly from the Bernoulli probability law.

### Binomial distribution

A binomial random variable counts the number of successes in $n$ independent, identically distributed Bernoulli trials.

Let:

$$
 X_i\sim\operatorname{Bernoulli}(p) 
$$

for $i=1,\ldots,n$, with the $X_i$ independent.

Define:

$$
 K=\sum\_{i=1}^nX_i. 
$$

Then:

$$
 \boxed{K\sim\operatorname{Binomial}(n,p)}. 
$$

Its possible values are:

$$
 K=0,1,\ldots,n. 
$$

#### 1 Deriving the binomial PMF

Suppose we want exactly $k$ successes and $n-k$ failures.

One particular sequence, such as $k$ successes followed by $n-k$ failures, has probability:

$$
 p^k(1-p)^{n-k} 
$$

by independence.

But the successes can occur in different positions.

The number of ways to choose the $k$ success positions is:

$$
 \binom nk= \frac{n!}{k!(n-k)!}. 
$$

All those distinct sequences have the same probability and are mutually exclusive.

Therefore:

$$
 \boxed{ P(K=k) = \binom nkp^k(1-p)^{n-k} }. 
$$

#### 2 Worked example

Suppose five independent ray tests each have a $0.3$ probability of success.

What is the probability that exactly two succeed?

We have:

$$
 n=5,\qquad k=2,\qquad p=0.3. 
$$

The number of possible sequences containing two successes is:

$$
 \binom52=\frac{5!}{2!3!}=10. 
$$

One such sequence has probability:

$$
 (0.3)^2(0.7)^3. 
$$

Therefore:

$$
 \begin{aligned} P(K=2) &=10(0.3)^2(0.7)^3\\ &=10(0.09)(0.343)\\ &=\boxed{0.3087}. \end{aligned} 
$$

#### 3 Deriving mean and variance

Since:

$$
 K=\sum_iX_i, 
$$

linearity of expectation gives:

$$
 \mathbb E[K]=\sum_i\mathbb E[X_i]. 
$$

Each Bernoulli mean is $p$, so:

$$
 \boxed{\mathbb E[K]=np}. 
$$

For the variance:

$$
 \operatorname{Var}(K) = \operatorname{Var}\left(\sum_iX_i\right). 
$$

Because the trials are independent, their pairwise covariances vanish.

Therefore:

$$
 \operatorname{Var}(K)= \sum_i\operatorname{Var}(X_i). 
$$

Each variance is $p(1-p)$, so:

$$
 \boxed{ \operatorname{Var}(K)=np(1-p) }. 
$$

For $n=5,p=0.3$:

$$
 \mathbb E[K]=1.5, 
$$

$$
 \operatorname{Var}(K)=5(0.3)(0.7)=1.05. 
$$

The expected number of successes need not be an integer.

#### 4 What if the trials are dependent?

If the trials are dependent:

$$
 \operatorname{Var}(K)= \sum_i\operatorname{Var}(X_i) + 2\sum\_{i\<j}\operatorname{Cov}(X_i,X_j). 
$$

The usual binomial variance no longer applies unless those covariance terms vanish under the relevant assumptions.

Likewise, if success probabilities differ between trials, the count follows a Poisson-binomial distribution under independence, not an ordinary binomial distribution.

This matters when rays, sensor observations, or physical events are correlated.

#### 5 Numerically stable binomial probabilities

The factorial formula is mathematically correct but numerically problematic for large $n$.

A naive implementation might compute:

```text
factorial(n) / (factorial(k) * factorial(n-k))
```

but factorial values overflow ordinary integer and floating-point types quickly.

Instead, compute the log probability:

$$
 \begin{aligned} \log P(K=k) &= \log\Gamma(n+1) -\log\Gamma(k+1) -\log\Gamma(n-k+1)\\ &\quad+k\log p+(n-k)\log(1-p). \end{aligned} 
$$

Here $\Gamma$ is the gamma function satisfying:

$$
 \Gamma(n+1)=n! 
$$

for nonnegative integers $n$.

Then exponentiate when an ordinary probability value is required.

For extreme tails, retaining log probabilities can be more reliable than exponentiating values that underflow to zero.

### Uniform distribution

The continuous uniform distribution assigns equal probability density across an interval.

Write:

$$
 X\sim U(a,b), \qquad a\<b. 
$$

Its density is:

$$
 \boxed{ p_X(x)= \begin{cases} \dfrac1{b-a},&a\le x\le b,\\ 0,&\text{otherwise}. \end{cases} } 
$$

The density value at the exact endpoints can be chosen differently without affecting probabilities, because individual endpoints have zero probability under this continuous distribution.

#### 1 Deriving the normalization

Suppose the density equals some constant $c$ over $[a,b]$.

Normalization requires:

$$
 \int_a^bc\\,dx=1. 
$$

Thus:

$$
 c(b-a)=1, 
$$

so:

$$
 \boxed{c=\frac1{b-a}}. 
$$

#### 2 Deriving the mean: uniform distribution

The mean is:

$$
 \mathbb E[X] = \int_a^b x\frac1{b-a}\\,dx. 
$$

Integrate:

$$
 \mathbb E[X] = \frac1{b-a} \left[\frac{x^2}{2}\right]\_a^b. 
$$

Therefore:

$$
 \mathbb E[X] = \frac{b^2-a^2}{2(b-a)}. 
$$

Factor:

$$
 b^2-a^2=(b-a)(b+a). 
$$

Cancel:

$$
 \boxed{ \mathbb E[X]=\frac{a+b}{2} }. 
$$

The expected value lies at the interval midpoint.

#### 3 Deriving the variance: uniform distribution

We can compute:

$$
 \mathbb E[X^2] = \int_a^b\frac{x^2}{b-a}\\,dx. 
$$

Thus:

$$
 \mathbb E[X^2] = \frac{b^3-a^3}{3(b-a)}. 
$$

Factor:

$$
 b^3-a^3=(b-a)(a^2+ab+b^2). 
$$

Therefore:

$$
 \mathbb E[X^2] = \frac{a^2+ab+b^2}{3}. 
$$

Subtract the squared mean:

$$
 \operatorname{Var}(X) = \frac{a^2+ab+b^2}{3} -\frac{(a+b)^2}{4}. 
$$

Using a common denominator of 12:

$$
 \operatorname{Var}(X) = \frac{ 4(a^2+ab+b^2)-3(a^2+2ab+b^2) }{12}. 
$$

Simplify:

$$
 \operatorname{Var}(X) = \frac{a^2-2ab+b^2}{12}. 
$$

Hence:

$$
 \boxed{ \operatorname{Var}(X)=\frac{(b-a)^2}{12} }. 
$$

#### 4 Worked example

For:

$$
 X\sim U(2,6), 
$$

the mean is:

$$
 \mathbb E[X]=4. 
$$

The variance is:

$$
 \operatorname{Var}(X) = \frac{(6-2)^2}{12} = \boxed{\frac43}. 
$$

The standard deviation is:

$$
 \sigma=\sqrt{\frac43}\approx1.154701. 
$$

The probability of the interval $[3,5]$ is:

$$
 P(3\le X\le5) = \int_3^5\frac14\\,dx =\boxed{0.5}. 
$$

#### 5 Uniform in coordinates is not always uniform geometrically

This is particularly important in graphics.

Sampling an azimuth angle uniformly and a polar angle uniformly does not produce a uniform distribution of directions over a sphere.

Why?

The spherical surface-area element is:

$$
 dA=r^2\sin\theta\\,d\theta\\,d\phi. 
$$

Equal changes in $\theta$ correspond to different surface areas depending on latitude.

Uniform polar-angle sampling overrepresents directions near the poles relative to surface area.

To sample uniformly over a sphere, one can choose:

$$
 z\sim U(-1,1), \qquad \phi\sim U(0,2\pi), 
$$

then construct:

$$
 \mathbf d= \begin{bmatrix} \sqrt{1-z^2}\cos\phi\\ \sqrt{1-z^2}\sin\phi\\ z \end{bmatrix}. 
$$

The resulting direction is uniform with respect to spherical surface area.

This is a direct consequence of probability-density transformations, not a trick specific to rendering.

### Gaussian or normal distribution

The Gaussian distribution is one of the most important continuous probability families.

It appears in measurement uncertainty, stochastic perturbation models, regression residuals, image filtering, reconstruction, sensor fusion, and probability-based inference.

A scalar Gaussian is parameterized by a mean $\mu$ and positive standard deviation $\sigma$:

$$
 \boxed{ X\sim\mathcal N(\mu,\sigma^2) }. 
$$

Its density is:

$$
 \boxed{ p_X(x)= \frac1{\sigma\sqrt{2\pi}} \exp\left( -\frac{(x-\mu)^2}{2\sigma^2} \right) }. 
$$

The notation uses $\sigma^2$, the variance, as the second distribution parameter.

The density is symmetric around $\mu$, reaches its maximum there, and decreases as the squared distance from the mean increases.

#### 1 Why does the exponent contain a square?

The expression:

$$
 (x-\mu)^2 
$$

measures squared deviation from the centre.

The factor:

$$
 2\sigma^2 
$$

controls how quickly density decreases.

Larger $\sigma$ produces a wider distribution.

Smaller $\sigma$ produces a narrower and taller distribution.

The exponential ensures a smooth, strictly positive density at every real number for $\sigma>0$.

But the formula must also integrate to one.

The normalizing coefficient:

$$
 \frac1{\sigma\sqrt{2\pi}} 
$$

is essential for turning the bell-shaped exponential into a probability density.

#### 2 Deriving the Gaussian normalization constant

Consider:

$$
 I=\int\_{-\infty}^{\infty}e^{-x^2/2}\\,dx. 
$$

This integral is not evaluated using an elementary antiderivative.

Instead, square it:

$$
 I^2= \left(\int\_{-\infty}^{\infty}e^{-x^2/2}\\,dx\right) \left(\int\_{-\infty}^{\infty}e^{-y^2/2}\\,dy\right). 
$$

Combine the independent integration variables:

$$
 I^2= \iint\_{\mathbb R^2} e^{-(x^2+y^2)/2}\\,dx\\,dy. 
$$

Switch to polar coordinates:

$$
 x=r\cos\theta, \qquad y=r\sin\theta. 
$$

The area element transforms as:

$$
 dx\\,dy=r\\,dr\\,d\theta. 
$$

Therefore:

$$
 I^2= \int_0^{2\pi}\int_0^\infty e^{-r^2/2}r\\,dr\\,d\theta. 
$$

Evaluate the radial integral using:

$$
 u=\frac{r^2}{2}, \qquad du=r\\,dr. 
$$

Then:

$$
 \int_0^\infty e^{-r^2/2}r\\,dr = \int_0^\infty e^{-u}\\,du =1. 
$$

Thus:

$$
 I^2=2\pi. 
$$

Since $I>0$:

$$
 \boxed{I=\sqrt{2\pi}}. 
$$

Therefore the standard Gaussian must include:

$$
 \frac1{\sqrt{2\pi}} 
$$

to integrate to one.

For a Gaussian with mean $\mu$ and standard deviation $\sigma$, substituting:

$$
 z=\frac{x-\mu}{\sigma} 
$$

introduces a factor of $\sigma$ in the integration measure, requiring the additional $1/\sigma$ normalization.

#### 3 Deriving the mean and variance

Because the standard Gaussian is symmetric around zero:

$$
 \mathbb E[Z]=0. 
$$

Its variance is:

$$
 \operatorname{Var}(Z)=\mathbb E[Z^2]. 
$$

We can evaluate:

$$
 \mathbb E[Z^2] = \frac1{\sqrt{2\pi}} \int\_{-\infty}^{\infty}z^2e^{-z^2/2}\\,dz. 
$$

Use integration by parts with:

$$
 u=z,\qquad dv=ze^{-z^2/2}\\,dz. 
$$

Then:

$$
 du=dz, \qquad v=-e^{-z^2/2}. 
$$

The boundary term vanishes because $ze^{-z^2/2}\to0$ as $|z|\to\infty$.

We obtain:

$$
 \int\_{-\infty}^{\infty}z^2e^{-z^2/2}\\,dz = \int\_{-\infty}^{\infty}e^{-z^2/2}\\,dz =\sqrt{2\pi}. 
$$

Thus:

$$
 \boxed{\operatorname{Var}(Z)=1}. 
$$

Now let:

$$
 X=\mu+\sigma Z. 
$$

Linearity of expectation gives:

$$
 \mathbb E[X]=\mu. 
$$

Variance scaling gives:

$$
 \operatorname{Var}(X)=\sigma^2. 
$$

These are exactly the distribution parameters.

#### 4 Gaussian shapes

Gaussian probability densities

Same mean, different standard deviation. The narrower distribution has a taller peak; both densities integrate to one.

The chart illustrates density height, not probability at individual exact values. To obtain the probability of an interval, integrate the curve over that interval.

#### 5 Gaussian log-likelihood and least squares

Suppose a measured value $y$ follows:

$$
 Y\sim\mathcal N(f(\theta),\sigma^2) 
$$

where $f(\theta)$ is a model prediction.

The density of observing $y$ is:

$$
 p(y\mid\theta) = \frac1{\sigma\sqrt{2\pi}} \exp\left( -\frac{(y-f(\theta))^2}{2\sigma^2} \right). 
$$

Take the negative logarithm:

$$
 -\log p(y\mid\theta) = \frac{(y-f(\theta))^2}{2\sigma^2} +\log\sigma+\frac12\log(2\pi). 
$$

If $\sigma$ is known and constant, the final two terms do not depend on $\theta$.

Therefore minimizing the negative log-likelihood is equivalent to minimizing:

$$
 \boxed{ \frac{(y-f(\theta))^2}{2\sigma^2} }. 
$$

This is weighted least squares.

So the objective functions from the optimization lessons have a statistical interpretation: they correspond to maximum-likelihood estimation under an appropriate Gaussian noise model.

That does not make Gaussian noise universally correct. It tells us exactly which assumption produces the squared-error objective.

#### 6 Gaussian assumptions and the central limit theorem

The central limit theorem helps explain why approximately Gaussian distributions often appear in sums or averages of many small random contributions under suitable assumptions.

But it does not imply that every noise source is Gaussian.

Errors can be heavy-tailed, multimodal, asymmetric, bounded, discrete, correlated, or affected by outliers.

A model should use Gaussian uncertainty because it is justified by the application or is a useful approximation with evaluated limitations, not merely because Gaussian formulas are convenient.

### The standard normal distribution

The standard [[Chapter Edition/01 - Computing Foundations/23 - Probability Models and Measurement Error#4. Discrete and continuous distribution families|normal distribution]] is the special case:

$$
 \boxed{Z\sim\mathcal N(0,1)}. 
$$

Its PDF is:

$$
 \boxed{ \phi(z)= \frac1{\sqrt{2\pi}}e^{-z^2/2} }. 
$$

Its CDF is conventionally denoted:

$$
 \boxed{ \Phi(z)= \int\_{-\infty}^{z}\phi(t)\\,dt }. 
$$

#### 1 Standardization

Suppose:

$$
 X\sim\mathcal N(\mu,\sigma^2). 
$$

Define:

$$
 \boxed{ Z=\frac{X-\mu}{\sigma} }. 
$$

Then:

$$
 Z\sim\mathcal N(0,1). 
$$

This follows because shifting removes the mean and dividing by $\sigma$ makes the variance one.

The standardized variable $Z$ is dimensionless.

#### 2 Numerical example: standardized pixel error

Suppose pixel localization error is modeled as:

$$
 X\sim\mathcal N(0,2^2) 
$$

with error measured in pixels.

What is the probability that the error lies between $-2$ and $2$ pixels?

Standardize:

$$
 P(-2\le X\le2) = P\left(-1\le Z\le1\right). 
$$

Using the standard normal CDF:

$$
 P(-1\le Z\le1) = \Phi(1)-\Phi(-1). 
$$

By symmetry:

$$
 \Phi(-1)=1-\Phi(1). 
$$

Therefore:

$$
 P(-1\le Z\le1) = 2\Phi(1)-1. 
$$

Numerically:

$$
 \boxed{P(-2\le X\le2)\approx0.682689}. 
$$

The familiar 68% rule comes from this exact Gaussian calculation.

#### 3 The error function

The normal CDF can be expressed using the error function:

$$
 \operatorname{erf}(x)= \frac2{\sqrt\pi} \int_0^xe^{-t^2}\\,dt. 
$$

For a general normal variable:

$$
 \boxed{ F_X(x) = \frac12\left[ 1+\operatorname{erf} \left(\frac{x-\mu}{\sigma\sqrt2}\right) \right] }. 
$$

An equivalent expression uses the complementary error function:

$$
 \operatorname{erfc}(x)=1-\operatorname{erf}(x). 
$$

Thus:

$$
 \boxed{ F_X(x) = \frac12\operatorname{erfc} \left( -\frac{x-\mu}{\sigma\sqrt2} \right) }. 
$$

This second expression is often preferable for numerical evaluation in extreme lower tails because it avoids explicitly adding one to an `erf` result that may be almost negative one.

For upper-tail probability:

$$
 P(X>x) = \frac12\operatorname{erfc} \left( \frac{x-\mu}{\sigma\sqrt2} \right). 
$$

This is often more reliable than subtracting a CDF close to one from one.

#### 4 Why standardized errors are useful

Suppose two sensors have different noise scales.

Sensor A has standard deviation $1$ pixel.

Sensor B has standard deviation $5$ pixels.

A raw error of $3$ pixels is unusually large for A but less significant for B.

Their standardized residuals are:

$$
 z_A=3, 
$$

$$
 z_B=\frac35=0.6. 
$$

Normalizing by uncertainty allows us to compare errors on a common probabilistic scale.

This is why covariance-weighted residuals are widely used in calibration, filtering, and statistical estimation.


<!-- end V10.05 -->

<!-- begin V10.06 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/22 - Accumulation, Quadrature, and the Bridge to Probability|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/24 - Waiting Times, Covariance, and Matrix Decomposition|Next chapter]]
