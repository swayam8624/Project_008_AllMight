# 31 — Learning a Visual Classifier

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting|Next chapter]]

A visual classifier begins with data representation, labels and a decision rule. The chapter follows the path from image evidence to a learned model while separating measured behavior from assumptions.

**By the end:** Describe the steps that turn labeled examples into a visual classifier.

**Recall question:** Which parts of a classifier’s behavior come from its representation and training data?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 75311–76246; source-span SHA256 56d79a26d4080a400674898b49e3154b5c9662b90f619a5ed1b44139b34a8055. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Learning a visual classifier from examples

### What a computer vision system has to learn

A digital image is an array of numbers. A color image usually stores three values at every pixel: red, green, and blue. A classifier must turn that array into a useful answer, such as “cat.” This is a large jump from numbers to meaning. The lecture calls it the **semantic gap**.

The same object can produce very different arrays. The camera may move. The light may change. An object may be partly hidden, bent into a new pose, small in the image, or surrounded by clutter. Two members of one category can also look unlike each other. A hand-written list of rules would need to cover all these cases, plus cases the rule writer did not imagine.

A data-driven system learns from examples instead. We collect images and labels, fit a model on a training set, choose design settings on a validation set, and measure the final result on a held-out test set. The test set must stay out of model selection. Otherwise, decisions slowly adapt to the test examples, and the final score no longer measures performance on fresh data.

Classification is one task, not the whole field. **Object detection** predicts a box and label for each object. **Semantic segmentation** assigns a category to every pixel. **Instance segmentation** also keeps separate objects apart when they share a category. Video adds time. Vision and language models connect image features with words. These tasks share ideas, but their outputs are different.

The opening lecture places today's learning methods in a longer history: early attempts to describe vision with geometry and hand-built features, growth of image datasets, and the rise of deep learning when data, computation, and useful algorithms came together. The lecture also points to human impact. A model's errors and uses matter because vision systems operate in human settings. The analogy between a neural network and a brain is only a loose teaching aid; the later neural-network lecture warns against treating it as a biological explanation.

**Source:** SC-01, full captions; course schedule and official slides: [CS231n 2025 schedule](https://cs231n.stanford.edu/2025/schedule.html), [Lecture 1, part 1](https://cs231n.stanford.edu/slides/2025/lecture_1_part_1.pdf), [Lecture 1, part 2](https://cs231n.stanford.edu/slides/2025/lecture_1_part_2.pdf).

### Classification begins with a representation

For a fixed-size RGB image of height $H$, width $W$, and three channels, flattening the pixels gives a vector with $D=3HW$ entries. For a CIFAR-sized image, $H=W=32$, so

$$

D=3\cdot32\cdot32=3072.

$$

A classifier takes this vector $x$ and returns one score for each class. A high score means the model favors that class. The score is not yet a probability.

The raw pixel vector is a poor description of object identity. If an image shifts by one pixel, many entries change. A person still sees the same object, but a pixel-by-pixel distance can become large. Conversely, unrelated objects can have similar color patterns. This is why a useful visual representation must tolerate changes that preserve meaning and respond to changes that alter meaning.

The data set is written as pairs $(x_i,y_i)$: image $x_i$ and its correct class $y_i$. A model with parameters $W$ computes scores from each image. A loss function measures how badly those scores fit the correct labels. Training changes the parameters to lower the loss.

### Nearest neighbors: remember first, compare at prediction time

The $k$-nearest-neighbor (k-NN) classifier stores the training images and labels. It does almost no fitting. To classify a new image, it compares the new vector with every stored vector, selects the $k$ closest, and votes among their labels.

For vectors $x,z\in\mathbb{R}^D$, two common distances are

$$

d_1(x,z)=\sum_{j=1}^{D}|x_j-z_j|,
\qquad
 d_2(x,z)=\sqrt{\sum_{j=1}^{D}(x_j-z_j)^2}.

$$

For the two-dimensional difference $(0.5,0.5)$, the L1 distance is $0.5+0.5=1$. The L2 distance is $\sqrt{0.5^2+0.5^2}\approx0.707$. L1 measures travel along coordinate directions; L2 measures straight-line distance. Their equal-distance contours have different shapes, so they can rank points differently.

A small $k$ makes the vote sensitive to individual examples. With $k=1$, one mislabeled or unusual point can decide the answer. A larger $k$ smooths the boundary, though it can blur a small class region or create ties. The choice of $k$ and the distance rule are **hyperparameters**: settings chosen outside the fitting process. Choose them on validation data, not on the test set.

For $N$ training images of $D$ values each, a direct prediction compares about $ND$ values. This makes prediction expensive when the data set or images are large. Training is cheap because the method mainly stores the examples. Its deeper weakness is that raw pixel distance does not match human similarity. In the lecture's CIFAR-10 example, the best validation choice reached only about 28–29% accuracy, far above the 10% random baseline but still weak. Treat this as the lecture's reported experiment, not a universal k-NN score.

A simple conceptual implementation is:

```python
from collections import Counter

def majority_vote(labels):
    if len(labels) == 0:
        raise ValueError("At least one neighbor is required")
    counts = Counter(labels)
    # Labels are integers; a tied vote chooses the smallest label.
    return min(counts, key=lambda label: (-counts[label], label))

# Inputs are NumPy arrays: x_train (N, D), y_train (N,), x (D,).
if not 1 <= k <= len(x_train):
    raise ValueError("k must be between 1 and the training-set size")
distances = ((x_train - x) ** 2).sum(axis=1) ** 0.5
nearest = distances.argsort()[:k]
prediction = majority_vote(y_train[nearest])
```

This code exposes the direct computation. It does not solve the representation problem, and a real implementation should also define tie handling and validation splits.

### Linear scores and their geometric limit

A linear classifier computes

$$

s=Wx+b,

$$

where $x\in\mathbb{R}^{D}$, $W\in\mathbb{R}^{C\times D}$, $b\in\mathbb{R}^{C}$, and $s\in\mathbb{R}^{C}$ contains one score for each of $C$ classes. For CIFAR-10, $W$ has shape $10\times3072$ and $b$ has 10 entries.

Each row of $W$ is a class-specific weight vector. Its dot product with the image adds evidence from pixels: positive weights favor that class when the matching pixel values are large; negative weights count against it. A useful picture is a class template, although the learned row need not be a clean image of the object.

In two dimensions, the boundary between two classes is where their scores are equal. A line can separate data only when the classes are linearly separable. XOR is a standard counterexample: opposite corners belong to one class and the other corners to another. No single line can separate them. More weights alone do not change a linear map into a nonlinear one. The model needs a nonlinear transformation of its input.

### Softmax turns scores into probabilities

Given class scores, or **logits**, softmax defines

$$

p_k=\frac{e^{s_k}}{\sum_j e^{s_j}}.

$$

Every probability is nonnegative and all probabilities sum to one. If the correct class is $y$, cross-entropy loss is

$$

L=-\log p_y.

$$

This loss is small when the model assigns high probability to the correct label and grows without bound as that probability approaches zero. For $C$ classes with equal logits, each class gets probability $1/C$, so the loss is $-\log(1/C)=\log C$. With 10 classes, this is about 2.303 using natural logarithms.

The lecture's example logits are $[3.2,5.1,-1.7]$. If the correct class is the first one, then

$$

p_y=\frac{e^{3.2}}{e^{3.2}+e^{5.1}+e^{-1.7}}\approx0.129,
\qquad L\approx-\log(0.129)\approx2.05.

$$

The largest score belongs to the second class, so the model is confidently leaning the wrong way. For numerical stability, softmax can subtract the largest logit $m$ from every logit: $e^{s_k-m}/\sum_j e^{s_j-m}$. This leaves the probabilities unchanged while keeping exponentials smaller.

Softmax is a useful classification loss, but it is not the only choice. Lecture 4 previews hinge loss: it asks the correct score to exceed each incorrect score by a margin. Unlike softmax, hinge loss does not turn scores into probabilities.

### Generalization is the target of regularization

The **data loss** measures fit to examples. A regularizer adds a preference about the model's parameters:

$$

L_{\text{total}}=L_{\text{data}}+\lambda R(W).

$$

The strength $\lambda$ controls the trade-off. At $\lambda=0$, the penalty has no effect. If $\lambda$ is too large, it can constrain the weights so much that the classifier underfits. The point is not to make training accuracy worse for its own sake. The point is to reduce overfitting so performance improves on unseen data.

For L2 regularization, $R(W)=\sum_j W_j^2$. For L1, $R(W)=\sum_j|W_j|$. Consider two weight vectors with the same score for a particular input: $w_a=(1,0,0,0)$ and $w_b=(0.5,0.5,0.5,0.5)$. Their L1 penalties are both 1. Their squared L2 penalties are 1 and 1/4. L2 therefore prefers the spread-out vector in this example. L1 can favor sparse solutions when removing a coefficient barely changes data loss. These are tendencies, not guarantees for every training problem.

### Optimization follows derivatives through the loss

Imagine a loss landscape whose horizontal axes are model parameters and whose height is loss. A derivative tells how fast loss changes along one direction. For many parameters, the **gradient** collects one derivative per parameter. It points toward steepest local increase; its negative points toward steepest local decrease.

A numerical derivative estimates a slope by changing one parameter by a small $h$ and measuring the change in loss. This is easy to implement, but it is slow and subject to floating-point error. An analytic derivative uses calculus and the chain rule. It is fast and exact when implemented correctly. A numerical gradient check compares both methods on a small problem to catch mistakes.

Gradient descent updates the parameters by

$$

W_{t+1}=W_t-\eta\nabla_W L(W_t),

$$

where $\eta$ is the learning rate. A rate that is too large can jump across a narrow valley or increase loss. A rate that is too small makes progress painfully slow. Stochastic gradient descent estimates the gradient from a mini-batch instead of the entire data set. This lowers the cost of each step but adds noise. One epoch is one pass over all training examples, usually in shuffled order.

Momentum keeps part of the previous update direction. It can reduce zig-zagging in a narrow valley and carry a useful direction through flat regions. RMSProp scales steps using a running average of squared gradients. Adam combines momentum-like averaging with this adaptive scaling. Its bias correction matters early in training because the running averages start at zero. AdamW applies weight decay separately from Adam's moment calculations. These methods have useful intuitions, but the lecture treats optimizer choice and learning-rate schedules as empirical choices to check on validation data.

A schedule changes the learning rate during training. Common examples include a step drop, cosine decay, and linear warm-up before a decay. A high rate may be useful early; a lower rate can help settle near a minimum. The lecture also introduces second-order methods, which use curvature information from the Hessian. They can be useful for smaller problems, but storing and using a Hessian becomes too costly for models with millions or billions of parameters.

### A hidden layer adds nonlinear features

A one-layer linear model cannot make a curved decision boundary. A two-layer network can first transform the input and then classify the transformed values:

$$

h=\phi(W_1x+b_1),\qquad s=W_2h+b_2.

$$

Here, $h$ is the hidden representation, and $\phi$ is a nonlinear activation applied element by element. If $\phi$ were also linear, the two weight matrices could be multiplied into one matrix. The whole network would still be linear. Nonlinearity is what gives depth added expressive power.

ReLU is $\phi(a)=\max(0,a)$. It passes positive inputs through and maps negative inputs to zero. This makes it simple and often effective. A unit that stays on the negative side receives zero gradient through ReLU and may stop learning; this is the “dead ReLU” problem. Leaky ReLU keeps a small negative slope to reduce that risk. Sigmoid and tanh can have vanishing gradients when their outputs saturate. Activation choice depends on the architecture and task; ReLU is a common starting point, not a law.

A hidden layer can learn useful parts or patterns that several classes share. But adding capacity can also let a model memorize the training set. The lecture recommends managing the balance with validation and regularization, rather than assuming that a smaller network is always better.

### Backpropagation: apply the chain rule one operation at a time

A neural network is a composition of simple operations. A **computational graph** records those operations from input to loss. During the forward pass, each node stores the value it computes. During the backward pass, it receives an upstream gradient and sends gradients to its inputs using its local derivative.

For a scalar output $f=(x+y)z$, set $x=-2$, $y=5$, and $z=-4$. The forward pass gives $q=x+y=3$, then $f=qz=-12$. At the multiply node, $\partial f/\partial q=z=-4$ and $\partial f/\partial z=q=3$. At the add node, $\partial q/\partial x=1$ and $\partial q/\partial y=1$. The chain rule gives

$$

\frac{\partial f}{\partial x}=\frac{\partial f}{\partial q}\frac{\partial q}{\partial x}=-4,
\quad
\frac{\partial f}{\partial y}=-4,
\quad
\frac{\partial f}{\partial z}=3.

$$

The upstream gradient is the derivative arriving from later operations. A local gradient is the derivative of one operation's output with respect to one input. Multiply them to get the downstream gradient. If two branches feed into the same value, add their contributions. This local rule lets a system differentiate a deep network without expanding the whole network into one enormous formula.

Several operation rules are easy to remember:

| Operation | Local backward rule |
| --- | --- |
| Addition $q=x+y$ | Send the upstream gradient unchanged to both inputs. |
| Multiplication $q=xy$ | Send $g y$ to $x$ and $g x$ to $y$, where $g=\partial L/\partial q$. |
| ReLU $q=\max(0,x)$ | Send $g$ where $x>0$, otherwise send zero (at zero, implementations choose a convention). |
| Copy/fan-out | Add gradient contributions from all paths that used the copied value. |

For vectors and matrices, derivatives have structure. A scalar loss differentiated with respect to a vector has one gradient entry per input element. A vector output with respect to a vector input can be represented by a Jacobian matrix. In principle, a matrix multiplication could have an enormous Jacobian. In practice, software uses compact matrix rules and the saved forward-pass inputs. It does not build that giant matrix. This is why automatic differentiation can scale to large networks.

**Source:** SC-02–SC-04, full original-English captions. For official primary material and lecture sequence, see [CS231n 2025 schedule](https://cs231n.stanford.edu/2025/schedule.html), [Lecture 2 slides](https://cs231n.stanford.edu/slides/2025/lecture_2.pdf), [Lecture 3 slides](https://cs231n.stanford.edu/slides/2025/lecture_3.pdf), and [Lecture 4 slides](https://cs231n.stanford.edu/slides/2025/lecture_4.pdf).

## Convolutional models preserve image neighborhoods

### Convolutions keep image neighborhoods intact

A fully connected layer flattens an image and compares each learned template with the entire input. That treats distant pixels as if their positions were just entries in one long list. A **convolution layer** uses small learned filters instead. Each filter looks at a local patch, spans all input channels, and slides across the image. At every position, it takes a dot product between the filter and that patch. The resulting number records how strongly the patch matches the filter. One filter makes one 2D activation map; a bank of filters makes a stack of maps, one output channel per filter.

For an input with shape $N\times C_{in}\times H\times W$, a bank of filters has shape $C_{out}\times C_{in}\times K_H\times K_W$. Here, $N$ is batch size, $C_{in}$ is the number of input channels, and $C_{out}$ is the number of filters. The kernel must span all input channels. The output has $C_{out}$ channels. Filter values and biases are learned by backpropagation; filter count and kernel size are architecture choices set before training.

For a single image with 3 channels and spatial size $32\times32$, six filters of size $3\times5\times5$, stride 1, and no padding produce six feature maps of size $28\times28$. With a bias for each filter, the learned parameter count is

$$

6(3\cdot5\cdot5+1)=6(76)=456.

$$

For 10 filters on the same input with a $5\times5$ kernel, stride 1, and padding 2, the parameter count becomes

$$

10(3\cdot5\cdot5+1)=760.

$$

The spatial size stays $32\times32$. Each of the $10\cdot32\cdot32=10{,}240$ output values uses a dot product over 75 values, or roughly $10{,}240\cdot75=768{,}000$ multiply-accumulate operations. The lecture's spoken estimate says about 10,000 outputs and 768,000 operations; exact FLOP conventions can count a multiply and addition separately.

For one spatial dimension, input size $W$, kernel size $K$, padding $P$, and stride $S$ give

$$

W_{out}=\left\lfloor\frac{W+2P-K}{S}\right\rfloor+1.

$$

Without padding, a $7\times7$ input and $3\times3$ kernel at stride 1 produce $5\times5$. Padding adds values around the edge, often zeros. With an odd kernel $K=2q+1$, choosing $P=q$ and stride 1 preserves height and width. Padding makes shape management easier, though border outputs see artificial values.

A **stride** says how far the filter moves each time. Stride 2 skips positions and downsamples the feature map. This can grow the **receptive field** faster: the receptive field of a later activation is the region of the original image that could affect it. Stacking local filters grows this region; downsampling can make it grow more quickly. This helps deeper layers combine local patterns into larger structures.

A stack of only convolutions is still linear if nothing nonlinear sits between them. Two linear maps composed together can be represented as one linear map. Add an activation such as ReLU between the convolution layers to increase what the network can represent.

### Pooling downsamples feature maps

A pooling layer reduces spatial size without learning a new filter bank. It acts on each channel separately. In **max pooling**, split a feature map into windows and keep the largest value in each window. A $2\times2$ window with stride 2 cuts each spatial dimension in half. For example, the windows $[1,6;2,3]$, $[4,7;8,5]$, $[0,2;1,3]$, and $[9,4;6,2]$ become $[6,8;3,9]$.

Max pooling is nonlinear and routes its gradient to the winning entry in each window. Average pooling takes the mean instead; it is linear, so a separate nonlinear activation may still be needed. Pooling is cheap compared with convolution. Strided convolution can also downsample and can learn the operation, while pooling uses a fixed reduction rule. Both choices affect the network's spatial resolution and receptive field.

If training images have different sizes, a batch tensor cannot hold them directly as one rectangular block. Common approaches resize images, pad them to a common shape, process them separately, or group images with similar aspect ratios into buckets. Resizing changes shape and may distort the image; padding adds artificial borders. Each choice has consequences.

### Translation equivariance is a useful design property

If an image shifts, a convolution's feature map shifts in the corresponding way, subject to boundary handling and stride. This is **translation equivariance**. It means the system can detect the same local pattern at different positions. Equivariance is not the same as invariance: an equivariant output moves with the input; an invariant output would stay unchanged. Pooling and other later operations may reduce sensitivity to small shifts, but convolution itself preserves location information in its output map.

This property reflects a useful image prior: a vertical edge is still a vertical edge when it appears on the left or right. A convolution shares the same filter weights across locations, so the network does not need to learn a separate edge detector for every pixel. That weight sharing reduces parameters and builds spatial structure into the model. It does not make every vision problem translation-invariant; object position can matter, and padding or striding can break exact equivariance at borders or for some shifts.

**Source:** SC-05, full original-English captions, including the lecture's stated limits and examples. Official lecture sequence: [CS231n 2025 schedule](https://cs231n.stanford.edu/2025/schedule.html), [Lecture 5 slides](https://cs231n.stanford.edu/slides/2025/lecture_5.pdf).

### CNN layers need stable signals and enough variation

Normalization layers control the scale and center of activations. A common pattern is: first normalize values to mean 0 and standard deviation 1, then apply learned scale and shift parameters. The exact values used to compute the mean and standard deviation depend on the layer:

| Layer | Values used for the statistics |
| --- | --- |
| Batch normalization | For each channel, values across the mini-batch and spatial positions during training. |
| Layer normalization | For each example, values across its feature dimensions; for image tensors, this can cover channels, height, and width. |
| Instance normalization | For each example and channel, values across spatial positions. |
| Group normalization | For each example, values across groups of channels and spatial positions. |

The useful question is always: *which values share the same statistics?* Batch normalization depends on batch composition. Layer normalization treats each example independently. Group and instance normalization make different trade-offs. Learned scale and shift let the model choose a useful distribution after the normalization step.

**Dropout** is a training-time regularizer. It randomly sets some activations to zero on each forward pass. This makes it harder for a model to depend on one narrow combination of features. It may learn several cues for “cat,” such as ears, fur, and claws, because any one cue can be missing in a training pass. At test time, dropout is disabled and activation magnitudes must be scaled consistently. Implementations commonly use *inverted dropout*: scale the surviving activations during training so no test-time scale adjustment is needed. The key invariant is that expected activation scale should match between training and testing.

### Activations make the network nonlinear

A sigmoid activation saturates near 0 and 1. In those flat regions, its derivative is near zero. During backpropagation, multiplying many small derivatives can make gradients in early layers almost vanish. ReLU, $\max(0,x)$, has derivative 1 on positive inputs and 0 on negative inputs. This helps positive activations carry gradients, but a unit that stays negative can become inactive. GELU and SiLU/Swish use smooth curves with a nonzero slope around part of the negative region. They are common choices in modern architectures. No activation is best in every setting; architecture conventions and validation guide the choice.

### Architectural patterns: VGG and residual connections

VGG stacks small $3\times3$ convolutions, nonlinearities, and pooling. Three stride-1 $3\times3$ layers see a $7\times7$ region of the original image: the receptive field grows from 3 to 5 to 7. If the channel count stays $C$, the convolution weights for these three layers are about $3\cdot3\cdot3C^2=27C^2$, compared with $7\cdot7C^2=49C^2$ for one $7\times7$ layer. The stacked version also has more activation steps, so it can represent more nonlinear functions.

Plain networks became harder to train as they grew deeper. In the reported experiment, a deeper plain model had worse training error as well as worse test error. That points to an optimization problem, rather than ordinary overfitting. In theory, a deeper model can copy a shallower one by making extra layers act like identity functions. In practice, learning that identity mapping can be difficult.

A **residual block** gives the network a direct route around a group of layers:

$$

y=x+F(x).

$$

The block learns a residual, or change, $F(x)$. If the extra transformation is not useful, it can approach zero and the block can pass $x$ forward. This makes an identity-like solution easier to represent and improves optimization of deep networks. The addition requires the two branches to have compatible shapes; when dimensions change, architectures need a projection or another shape adjustment. ResNets use repeating residual blocks and periodically reduce spatial size while increasing channel count.

### Initialization keeps signals from shrinking or exploding

A neural network starts with random weights. If their scale is too small, activations can shrink toward zero as they pass through many layers. If the scale is too large, activations can grow rapidly. Either case makes learning harder because the signal and gradient scales become poorly behaved.

For ReLU networks, Kaiming (He) initialization draws weights with a scale based on the number of inputs to each unit. In variance form, a common rule is

$$

\operatorname{Var}(W_{ij})=\frac{2}{\text{fan\_in}},
\quad\text{so standard deviation is }\sqrt{\frac{2}{\text{fan\_in}}}.

$$

For a convolution with six input channels and a $3\times3$ kernel, $\text{fan\_in}=6\cdot3\cdot3=54$. The standard deviation is $\sqrt{2/54}\approx0.192$. This is a starting rule that helps keep activation scales stable. Different activations or layer types may call for different initialization rules.

For image input normalization, compute a mean and standard deviation for each color channel from the training data, then normalize each channel. Pretrained models often expect the statistics used during their own training. Use the matching preprocessing when applying such a model.

### Augmentation and pretrained models expand what small data can do

**Data augmentation** applies label-preserving changes to training examples. Random crops, resize-and-crop, horizontal flips, color changes, and simulated occlusion can make the model less able to memorize the exact training images. An augmentation is useful only when the transformed image should keep the same label. A horizontal flip may help with ordinary objects but hurt a text-reading model. A vertical flip may make sense for overhead imagery and be wrong for everyday scenes.

At test time, **test-time augmentation** can average predictions from several crops or flips of one image. This can improve accuracy, but it costs more inference time and often gives diminishing returns. More data variation is not automatically better: transformations must match the real task.

When a labeled data set is small, a pretrained CNN can serve as a feature extractor. Freeze the earlier layers, replace the final classifier, and train only the new classifier. If there is more data, fine-tune some or all of the pretrained model. Similarity between the pretraining data and the target data matters: a model trained on everyday images may transfer poorly to medical or astronomical images. The lecture presents this as a practical strategy, not a guarantee.

### A practical training and debugging loop

If training fails, first try to overfit one or a few examples. The model should drive training loss very low on a tiny sample. If it cannot, check the data, labels, loss, gradients, and model code before scaling up.

Then test a coarse range of learning rates and inspect both loss and accuracy. Training loss that improves while validation performance worsens suggests overfitting; stronger regularization or more varied data may help. If both training and validation performance still improve, continue training. Random search across sensible hyperparameter ranges often explores important settings more effectively than a rigid grid when some settings matter much more than others.

**Source:** SC-06, full original-English captions. The lecture distinguishes empirical advice from derivations; the notes retain that distinction. Official materials: [CS231n 2025 schedule](https://cs231n.stanford.edu/2025/schedule.html), [Lecture 6 slides](https://cs231n.stanford.edu/slides/2025/lecture_6.pdf).

## Sequence models and attention

### Recurrent networks carry a state through a sequence

A standard feed-forward network maps one fixed-size input to one output. A **recurrent neural network (RNN)** processes inputs one step at a time and carries a hidden state forward. A simple form is

$$

h_t=\phi(W_{xh}x_t+W_{hh}h_{t-1}+b_h),\qquad y_t=W_{hy}h_t+b_y.

$$

The input at time $t$ is $x_t$. The hidden state $h_t$ is a fixed-size summary of what the model has seen so far. The same matrices $W_{xh}$, $W_{hh}$, and $W_{hy}$ are reused at every time step. This lets the model accept sequences of different lengths without needing a separate layer for each position. A vanilla RNN often uses $\tanh$, which keeps activations between $-1$ and $1$, though that can also shrink gradients.

Sequence tasks can have different input and output shapes:

| Pattern | Example |
| --- | --- |
| One input to many outputs | Image captioning: one image produces a word sequence. |
| Many inputs to one output | Video classification: many frames produce one label. |
| Many inputs to many outputs | Per-frame labeling or sequence-to-sequence mapping. |

For a character-level language model, each input character predicts the next one. A one-hot vector has a single 1 at the character's index and zeros elsewhere. An embedding layer replaces that sparse vector with a learned dense vector. The model applies softmax to score the next character, uses cross-entropy against the actual next character, and feeds a sampled prediction back in during generation. Choosing the highest-probability token every time is greedy decoding. Sampling from the distribution or using beam search can produce different sequences.

A small state can track a simple rule. For the task “output 1 when the current and previous inputs are both 1,” the state only needs enough information to remember the previous input and the current one. A hand-built toy RNN can do this with three state entries, then compute $\max(0,\text{previous}+\text{current}-1)$. For inputs 1, 1, the result is 1; for 1, 0, it is 0. Real models learn the state update and output weights from data.

### Backpropagation through time shares gradients across steps

An unrolled RNN is a computation graph with one copy of the state update at each time step. The copies use the same parameters. If there is a loss at every step, the total loss can be a sum:

$$

L=\sum_{t=1}^{T} L_t.

$$

Each use of a shared weight contributes to its gradient, so the gradient contributions across time are added. This is **backpropagation through time (BPTT)**. If the sequence is long, storing all activations for the entire unrolled graph can use too much memory. **Truncated BPTT** processes a fixed window at a time and carries the hidden state forward between windows while stopping the gradient at the boundary. This saves memory and computation, but it also prevents the current window's loss from sending a full gradient through arbitrarily early steps.

The repeated state update creates another problem. Gradients across many steps multiply Jacobians. If their typical scale is below 1, the product can shrink toward zero; this is the **vanishing-gradient problem**. If the scale is above 1, gradients can grow too large; this is the **exploding-gradient problem**. Gradient clipping can limit large updates. It does not restore information lost to tiny gradients.

An **LSTM** adds gates and a separate cell-state path. The input gate controls what new information to write. The forget gate controls what old information to keep. The output gate controls what to expose as the hidden state. A more direct path through the cell can preserve information across more time steps. This helps, but it does not guarantee that every long dependency will be learned.

RNNs process a sequence in order, so the next step cannot be computed before the previous hidden state exists. This limits parallelism during training. Their fixed-size state can also lose details from very long sequences. Their computation grows roughly linearly with sequence length and the model's parameter count does not grow with sequence length. Transformers instead compare positions through attention, which can capture long-range relations more directly but has higher sequence-length costs in standard forms. The lecture introduces recurrent-style models such as RWKV and Mamba as attempts to combine useful sequence scaling with strong modeling.

### Sequence models inherit the biases in their data

An RNN can combine a visual encoder with text generation. For image captioning, a CNN can encode an image, and an RNN can use that representation plus previous words to predict the next word. The lecture's examples show a failure that remains easy to understand: the model may name an object or action that often co-occurs with a scene in its training data, even when it is absent from this particular image. A captioning loss rewards matching captions; it does not automatically teach the model to distinguish a scene's usual associations from visible evidence.

This is a useful reminder for all learned vision systems. Better sequence modeling does not remove data bias. A model can produce fluent output while making a visual mistake. Evaluation must check the actual task and the evidence in each input.

**Source:** SC-07, full original-English captions. The first minutes also correct the prior dropout explanation: if training drops a fraction $r$, test-time scaling should preserve the expected activation level (or an equivalent inverted-dropout convention can scale during training).

### Attention lets each output retrieve relevant input

An encoder RNN compresses an input sentence into one context vector. That can become a bottleneck: the vector has fixed size even when the input grows to a paragraph or book. Attention removes this single-summary requirement. At every decoder step, the model can look back at all encoder states and build a new context vector for the output it is producing now.

For a query $q$ and input states $h_1,\ldots,h_n$, the model scores each state, applies softmax, and uses the probabilities to combine the states:

$$
e_i=f(q,h_i),\qquad
\alpha_i=\frac{\exp(e_i)}{\sum_j\exp(e_j)},\qquad
c=\sum_i\alpha_i h_i.
$$

The query represents what the decoder needs at this step. Each $h_i$ is a possible source of information. The weights $\alpha_i$ are nonnegative and sum to 1, so $c$ is a weighted average. A new decoder state creates a new query, so the weights and context can change at every output step.

For example, suppose one output word needs to combine two input states with values $h_1=(2,0)$ and $h_2=(0,4)$. If the attention weights are $(0.75,0.25)$, then the context is

$$
c=0.75(2,0)+0.25(0,4)=(1.5,1).
$$

The network learns the scoring rule from the translation objective. It does not need a human to label which input word aligns with each output word. A heat map of attention weights can help inspect a model's behavior, but a high weight is not by itself proof that a human-readable explanation is faithful.

### Scaled dot-product attention and its dimensions

A simple similarity score is a dot product. To keep scores from growing too large as vector width grows, scale the dot product by the square root of the query/key dimension. Let $Q$ contain $n_q$ queries, $K$ contain $n_x$ keys, and $V$ contain $n_x$ values:

$$
Q\in\mathbb{R}^{n_q\times d_k},\quad
K\in\mathbb{R}^{n_x\times d_k},\quad
V\in\mathbb{R}^{n_x\times d_v}.
$$

Then the score matrix, attention weights, and outputs are

$$
S=\frac{QK^\top}{\sqrt{d_k}},\qquad
A=\operatorname{softmax}(S)\text{ row by row},\qquad
Y=AV.
$$

The score matrix has shape $n_q\times n_x$: one score for every query/key pair. Each row of $A$ is a probability distribution over the keys. The output has shape $n_q\times d_v$: each query gets one weighted combination of the value vectors.

A small numerical example makes the scaling and weighting concrete. Let one query be $q=(1,0)$, keys be $k_1=(1,0)$, $k_2=(0,1)$, and values be $v_1=(2,0)$, $v_2=(0,4)$. With $d_k=2$, the scaled scores are $(1/\sqrt2,0)\approx(0.707,0)$. Softmax gives about $(0.67,0.33)$. The output is approximately

$$
0.67(2,0)+0.33(0,4)=(1.34,1.32).
$$

The dimensions need not match for values and queries: $d_v$ can differ from $d_k$. The query/key dimensions must match for their dot product.

### Queries, keys, values, and two attention patterns

Attention separates three roles. The **query** says what the model is looking for. A **key** says what kind of information an input location offers for matching. A **value** is the information that gets copied into the output when that location receives attention. Learned linear projections produce these vectors, so the model can learn different matching and retrieval spaces.

In **cross-attention**, queries and key/value data come from different sources. A decoder query can retrieve from encoder states, or a text query can retrieve from image features. In **self-attention**, one input set produces its own queries, keys, and values. Each output can then combine information from any allowed input position. The input and output contain the same number of positions, though their vector dimensions may change.

Multi-head attention runs several learned attention projections in parallel. Each head can use a different projection and can learn a different relation. The head outputs are joined and passed through another learned projection. The lecture emphasizes the implementation view: these operations can be batched into matrix multiplications rather than one loop per query or head.

### Position, masking, and attention's limits

Self-attention without position information is **permutation equivariant**: if the input vectors are reordered, the outputs are reordered the same way. The layer can compare content, but it cannot tell which token came first. A positional embedding adds information about each vector's location so the model can learn order-sensitive patterns.

A language model must not use future tokens when predicting the next token. A **causal mask** sets forbidden attention scores to negative infinity before softmax. Since $\exp(-\infty)=0$, the softmax assigns zero probability to those positions. At position $t$, the model can attend only to positions at or before $t$. This prevents the model from seeing the answer during training.

Attention lets all positions interact in one layer, which can help with long-range relationships. Standard full attention compares every query with every key. For sequence length $n$, its score matrix has $n^2$ entries. The work and memory therefore grow quadratically with length, which becomes costly for very long inputs. Efficient hardware and parallel computation help, but they do not remove the quadratic growth of the full score matrix.

### A transformer is attention plus per-position processing

A transformer block combines multi-head self-attention with a small feed-forward network (an MLP) applied independently at each position. The attention layer lets positions exchange information. The MLP transforms each position's vector after that exchange. Residual connections give information a direct route around each sublayer, and layer normalization helps stabilize training. Repeating blocks builds a transformer.

For images, a vision transformer divides the image into patches, projects each patch into a vector, adds position information, and processes the patch vectors with transformer blocks. A classifier can pool the final vectors and map the pooled representation to class scores. The same basic architecture can process language tokens or image patches because both are represented as sets or sequences of vectors.

RNNs, convolutions, and attention encode different assumptions. RNNs process an ordered sequence one step at a time and reuse a state. Convolutions mix local neighborhoods on a grid and share filters across positions. Self-attention lets every position compare with every other position in parallel, but full attention costs $O(n^2)$ in sequence length. The right architecture depends on the data structure, length, compute budget, and task.

**Source:** SC-08, full original-English captions. The lecture's claim that transformers were then the dominant large-scale architecture is time-specific course context; the notes preserve the underlying mechanisms and trade-offs instead of treating that forecast as permanent.

## Predict pixels and localize objects

### Dense prediction: give an answer at every pixel

Image classification returns one label for a whole image. **Semantic segmentation** returns a label for each pixel. It can mark every pixel as road, person, tree, or background, but it does not distinguish two people of the same class. **Instance segmentation** must also say which pixels belong to person 1 and which belong to person 2.

One simple segmentation idea is to classify each pixel from a local patch around it. That supplies context: a pixel alone usually does not reveal whether it belongs to a wheel or a road. But running a full network once per pixel wastes work. Nearby patches overlap heavily. A **fully convolutional network (FCN)** instead processes the image as a grid and predicts a whole label map in one pass. A pixel-wise training objective sums or averages a classification loss over image locations:

$$
L_{\text{pixel}}=\frac{1}{HW}\sum_{u=1}^{H}\sum_{v=1}^{W}\operatorname{CE}(s_{uv},y_{uv}),
$$

where $s_{uv}$ is the vector of class scores at pixel $(u,v)$, and $y_{uv}$ is that pixel's ground-truth class. This requires pixel-labeled training examples. The network learns both contextual features and a label for every location.

### Encoder-decoder shape recovery and U-Net skips

A segmentation network needs spatial detail at its output. If it only downsamples, its feature map is too small to label fine boundaries. An encoder-decoder solves this by repeatedly reducing spatial resolution, then increasing it again. Downsampling gives later layers a wider field of view and reduces computation; upsampling restores a map at the original image size.

There are several ways to upsample. Nearest-neighbor upsampling copies a low-resolution value to nearby high-resolution positions. Max-unpooling can place a pooled value back where its maximum came from, if the encoder saved those locations. A learned upsampling layer can spread each input through a learned filter. **Transposed convolution** is a learned operation that expands the spatial grid. It is called “transposed” because its matrix form uses the transpose of the linear map for convolution; it is not a guaranteed inverse of convolution and does not recover information that was discarded.

U-Net adds **skip connections** from encoder feature maps to decoder layers at matching scales. These shortcuts restore spatial cues that downsampling blurred or removed. The decoder then combines fine location information from early layers with broader context from deeper layers. This often produces sharper object boundaries. The lecture notes U-Net's continued value in some medical segmentation settings, especially when a task-specific model is wanted without a larger foundation model.

### Detection: locate each object as well as name it

Object detection must return a class and a box for each object. A box can be represented by its center $(x,y)$, width $w$, and height $h$. For one object, a model can predict class scores and four box values. A training loss can combine class error with a box regression error:

$$
L=L_{\text{class}}+\lambda L_{\text{box}}.
$$

Here $\lambda$ balances the two objectives. With many objects, a fixed list of outputs becomes awkward: the model must decide how many objects to output and which output belongs to which target. Exhaustively testing every possible box is also too expensive.

Two-stage detectors address this with **region proposals**. First, a proposal mechanism suggests a manageable set of likely object regions. Then the detector classifies each region and refines its coordinates. Early R-CNN ran a CNN separately on every proposed crop, which was costly. Fast R-CNN computed a feature map for the full image once, then extracted region features from that map. Region Proposal Networks learned to suggest regions using the same image features. This shares computation, but the proposal and recognition stages still add work.

Single-stage detectors such as the YOLO family predict boxes and class scores in one network pass. In the lecture's grid picture, each cell predicts several candidate boxes and scores. Thresholding removes weak candidates; **non-maximum suppression** then keeps a high-scoring box and removes nearby boxes that likely describe the same object. The key design trade-off is speed and simplicity versus the accuracy and computation of more elaborate proposals and refinement stages. Specific systems change over time, so the lecture's description of which detector is “popular” should be read as course context rather than a permanent ranking.

A transformer detector such as DETR replaces hand-built region proposals with learned **object queries**. These queries are trainable vectors, not boxes or image patches. The image is encoded into tokens. A decoder uses each query to retrieve image information through cross-attention and predicts a class plus box coordinates. Some predictions can be “no object.” The lecture emphasizes that the queries are learned parameters and that class and box supervision shapes their outputs. It does not explain the detector's full matching loss in this portion, so this note does not infer those details from the name alone.

### Instance masks extend detection

A detector's box says where an object is, but it includes background pixels around the object. Mask R-CNN adds a mask-prediction branch for each detected region. The network therefore predicts a class, a refined box, and a pixel mask within that box. Repeating this per detected instance preserves the distinction between two objects of the same class. The mask branch makes the output more detailed and the model more computationally involved than box-only detection.

### Visualization can probe a model, but is not a complete explanation

A model's predictions alone do not show which image regions influenced them. The lecture describes several ways to inspect this. Early convolution filters can sometimes be visualized directly as simple edge or color patterns. For deeper feature maps, raw channel values do not map neatly to a human-readable image.

A **saliency map** measures the sensitivity of a class score to each input pixel. If the score for class $c$ is $s_c(x)$, the gradient

$$
\nabla_x s_c(x)
$$

shows how a small change in each pixel could change that score near the current image. Large gradient magnitude marks locally sensitive pixels. This is a local sensitivity measurement; it does not, by itself, establish that the model uses a meaningful causal reason or that the map is stable under small changes.

**Class activation mapping (CAM)** uses the final convolutional feature maps and the classifier's class weights to build a coarse spatial map for a class. Because convolutional locations correspond to image regions, the map can be projected back toward the image. The original CAM form requires a particular architecture, with a final convolutional map feeding a pooled linear classifier.

**Grad-CAM** extends the idea to a chosen convolutional layer. It uses gradients of a class score with respect to that layer's activations to weight the channels, combines the weighted maps, and keeps positive evidence with a ReLU. The result is usually a coarse heat map that can be resized over the input. It indicates which regions are associated with the score under this procedure; it is not a pixel-accurate segmentation mask.

For a vision transformer, attention weights already describe interactions among tokens, which can be plotted over image patches. Those maps are convenient to inspect, but attention weights also do not automatically prove a faithful explanation of a prediction. Visualization is useful for debugging, hypothesis generation, and communicating model behavior. It should be checked against the task and other evidence, especially in high-stakes uses such as medical imaging.

**Source:** SC-09, full original-English captions. The lecture covers task definitions, encoder-decoder segmentation, U-Net, families of detectors, object queries at a high level, instance masks, saliency, CAM, and Grad-CAM. It does not supply a complete account of detector matching, IoU/AP evaluation, or the full DETR training objective in the reviewed lecture segment.

## Video adds motion and temporal cost

### Video adds time, cost, and motion

A video is a sequence of frames. If an RGB frame has shape $3\times H\times W$, a clip of $T$ frames has shape $3\times T\times H\times W$. The extra time axis matters because many video tasks ask what action is happening, not only what objects or scene appear. It also makes the input much larger. A long, high-resolution video may not fit into memory as one training example.

A practical first baseline is to sample a few frames, classify each with an image model, and average the frame predictions. This works when each frame already gives strong evidence, such as a person visibly running. It can miss actions whose meaning depends on motion or order. Sampling itself matters: uniformly spaced frames are simple, while learned or content-aware sampling can spend compute on more informative moments.

For a long clip, a model can classify short windows. At test time, run it on several windows and combine their predictions. This reduces input size but creates a design choice: too few windows may miss the action; too many cost more compute. It also raises a distinction between **clip classification** (one label for a short segment), **temporal action localization** (when an action starts and ends), and **spatiotemporal detection** (where an actor or object is, and when the action occurs).

### Three ways to fuse video frames

A framewise image CNN can process each frame independently. **Late fusion** combines the frame features or predictions after those CNN passes. Concatenating all frame features preserves more detail, but its feature vector grows with the number of frames and can require a large classifier. Pooling features over time keeps the representation size fixed, but can discard which event came first or how one event changed into another.

**Early fusion** combines frames at the input. One simple construction treats the time and color channels together, so the first 2D convolution sees information from all selected frames. It can detect temporal differences early, but collapses the entire clip's time axis at once. The model has no local temporal filter that can slide over a motion and recognize it at different times.

**Slow fusion** uses 3D convolutions over time, height, and width. A 3D kernel covers a local space-time neighborhood and slides through the clip. Early layers see short motion patterns; later layers combine them into broader patterns. This shares a filter across time, so a short motion can be recognized whether it occurs near the start or end. The cost is higher than a 2D image CNN because each operation now spans time as well as image space.

These methods make different trade-offs:

| Method | When frames interact | Strength | Main limitation |
|---|---|---|---|
| Framewise / late fusion | After each frame has been processed | Reuses image models; strong simple baseline | Motion cues may already be lost |
| Early fusion | At the first layer | Sees multiple frames immediately | Collapses all time into one operation |
| 3D convolution / slow fusion | Gradually across layers | Learns local motion patterns shared over time | More compute and memory |

The lecture illustrates the difference with a color transition. A 2D convolution applied to all frames as channels can detect that colors differ, but it treats the temporal positions as fixed channel slots. A 3D filter of short temporal width can slide over the sequence and detect the same transition at different times. That is temporal shift equivariance from weight sharing; it does not mean every video transformation is harmless or that the model understands arbitrary timing.

### Motion streams, recurrent models, and attention

Appearance and motion provide different evidence. **Optical flow** estimates how image points move between adjacent frames. For each pixel $(x,y)$, a flow vector $(u,v)$ estimates its displacement to the next frame. One can visualize horizontal and vertical flow separately. A two-stream network processes RGB appearance in one branch and stacked flow fields in another, then combines their predictions. In the lecture's example, motion features can be more useful than appearance for action categories because the movement itself is the cue. That is a result on a particular benchmark, not a rule that motion always beats appearance.

A recurrent model can process a sequence of frame or clip features and keep a state over time. It can make one prediction for the whole video or one per time step. But recurrence imposes sequential computation: later states depend on earlier states. It can therefore be slow for long clips. A recurrent convolutional network adapts the idea to feature maps: convolutions update the spatial state at each time step, while the recurrent connection carries information forward.

Self-attention offers another way to connect video positions. The model projects space-time features into queries, keys, and values, computes which positions should exchange information, and combines the values. Attention can connect distant frames or image regions directly and can process many positions in parallel. The full attention matrix can be large, so 3D feature grids make efficiency important. The lecture describes non-local blocks as a way to insert such global space-time interactions into a convolutional video network.

A different approach, I3D, inflates a successful 2D image architecture into 3D by adding temporal extent to convolution and pooling kernels. Image-trained weights can initialize the inflated model. Repeating a 2D kernel across $k_t$ time steps and dividing by $k_t$ makes the response match the original kernel when all input frames are identical. Fine-tuning on video can then adapt those weights to motion. This is a useful transfer initialization, not a claim that image weights already encode every temporal pattern.

### Video understanding includes more than action labels

A video can also carry audio. Combining image, motion, and sound supports tasks such as separating the voices of visible speakers, separating instruments, and using audio or vision to select important clips. The lecture also points toward egocentric video, recorded from a person's point of view, where multiple microphones and visual context can help infer who is speaking or listening to whom.

Long videos make efficiency a central constraint. A system may sample only a subset of clips, use a sampler to predict which moments matter, or choose whether a task needs video, audio, or both. Video-language models add another interface: they map video information into a representation that a language model can use to answer questions or describe what is happening. These are active research directions described by the guest lecturer; the lecture does not establish that one particular model or sampling strategy is best for every deployment.

**Source:** SC-10, full original-English captions. The guest lecture covers frame sampling, fusion strategies, 3D convolution, optical flow, recurrent and attention-based models, I3D transfer, temporal localization, audiovisual tasks, and efficient/egocentric video understanding.

## Scaling model training across hardware

### Large-model training is a communication problem too

A modern GPU cluster is a hierarchy. Within one accelerator, arithmetic units share a hierarchy of small, fast caches and larger, slower memory. Across accelerators, data must move over links whose speed depends on whether the devices share a server, rack, or a wider network. A GPU can be fast at matrix multiplication and still sit idle while waiting for data or another device. Distributed training therefore tries to keep computation busy while overlapping communication with useful work.

A single accelerator's compute and memory layout changes over time, so numbers such as memory capacity, bandwidth, or peak operations in this lecture describe the hardware used for its examples, not permanent properties. The durable lesson is that data movement is slower and more constrained as it crosses farther levels of the system. A training setup must account for both arithmetic and communication.

### Data parallelism averages gradients across replicas

In **data parallelism**, every GPU stores a full copy of the model. Each GPU processes a different part of the mini-batch and computes a local gradient. The GPUs then combine those gradients, usually with an all-reduce, so every replica can make the same weight update.

If the global batch contains $m$ shards and each shard has $n$ examples, the average loss is

$$
L(W)=\frac{1}{mn}\sum_{r=1}^{m}\sum_{i=1}^{n}\ell(x_{ri};W).
$$

Because differentiation is linear over sums,

$$
\nabla_W L=\frac{1}{m}\sum_{r=1}^{m}\nabla_W\left(\frac{1}{n}\sum_{i=1}^{n}\ell(x_{ri};W)\right).
$$

So each GPU can compute its shard gradient independently, then the replicas average them. This is the same gradient as processing the full batch on one device, apart from numerical ordering and implementation details. The update must be synchronized: if every GPU starts with the same weights and receives the same averaged gradient, it stays in sync with the others. The GPUs must read different examples; accidentally repeating the same batch on every replica wastes the parallel work.

The forward and backward passes on each shard need no communication until gradients are combined. An all-reduce can combine partial sums and distribute the result. Training systems can overlap communication for later layers with backward computation for earlier layers. That overlap hides some network delay. It does not eliminate it, and the actual bottleneck depends on model size, batch, interconnect, and the hardware.

### Sharding model state trades memory for communication

Plain data parallelism duplicates model weights, gradients, and optimizer state on every GPU. With an optimizer such as Adam, the state per parameter can include the weight, gradient, and two moving averages. If each is stored in 16-bit form, four values at two bytes each already require eight bytes per parameter; some implementations also retain higher-precision copies. A billion parameters therefore needs several gigabytes per replica before counting activations, temporary buffers, or other state.

**Fully sharded data parallelism (FSDP)** assigns each layer's weights, gradients, and optimizer state to an owner device. Before a layer runs, its owner gathers or broadcasts that layer's weights so every participating device can compute the layer on its local data. Non-owners can discard the temporary weight copy afterward. In the backward pass, the weights are made available again; each device computes its local gradient, and the gradients are sent to the owner to combine and update. The updated weights are shared again when needed in the next forward pass.

Sharding saves persistent memory, but it adds communication. Training can prefetch the next layer's weights while computing the current layer. During backward, it can overlap gradient transfer and optimizer work on one layer with computation on another. Whether this is faster depends on how much communication the cluster can sustain. **Hybrid sharded data parallelism** uses two groups: shard state within a group of closely connected GPUs, then keep a full model replica per group and average group gradients across groups. This can use fast links for frequent sharding communication and slower links for less frequent replica communication.

### Checkpoint activations when memory is the limit

The forward pass stores intermediate **activations** that the backward pass needs. For a deep model, those stored tensors can consume substantial memory. **Activation checkpointing** saves selected activations and recomputes the others during backward. It reduces memory use by spending extra arithmetic.

If a network has $n$ layers, storing every activation uses memory that grows roughly with $n$. If we keep only a checkpoint at intervals of $c$ layers and recompute within each interval, the lecture's simple accounting gives memory proportional to $c$ and extra recomputation on the order of $n^2/c$. Choosing $c\approx\sqrt n$ gives roughly $\sqrt n$ checkpoint memory and $n^{3/2}$ recomputation in that simplified scheme. Practical systems use more careful checkpoint schedules, but the central trade is the same: more recomputation allows fewer stored activations.

### Match parallelism to the tensor axes and hardware

A transformer computes over several axes, and each suggests a different split:

| Parallelism | What is split | Main benefit | Communication challenge |
|---|---|---|---|
| Data | Examples in the batch | Independent work per device | Average gradients |
| Fully sharded data | Weights and optimizer state | Lower persistent memory per device | Gather weights and combine gradients |
| Context | Positions in a long sequence | Fit long sequences across devices | Attention needs interactions between positions |
| Pipeline | Consecutive layer groups | Fit deep models across devices | Activations and gradients must cross stages; idle “bubble” time |
| Tensor | A weight matrix's dimensions | Split large matrix operations | Partial results must be combined |

**Context parallelism** splits a long sequence. Many transformer operations act independently on positions, but attention compares positions with one another. Ring attention arranges blockwise exchanges so devices can build the needed attention interactions; Ulysses-style approaches distribute attention heads and sequence work differently. These methods matter when a sequence is too long for one device.

**Pipeline parallelism** assigns successive layers to different devices. A single mini-batch creates idle time: each stage waits for activations from the previous stage on the forward pass and gradients from the next stage on the backward pass. This idle interval is the pipeline **bubble**. Splitting a batch into multiple micro-batches lets the stages work on different pieces at once and shrinks the bubble. Too many micro-batches, however, mean more activations to manage.

**Tensor parallelism** splits a matrix multiplication across devices. For $Y=XW$, each device can compute a slice of the output from a slice of $W$, then combine partial results. With a pair of adjacent linear layers, choosing complementary row and column splits can avoid communicating between every layer; the devices combine information after the pair. This matches the two linear layers in a transformer's feed-forward block. The method is useful for very wide layers, but it adds communication at the split points.

Large training runs commonly combine these dimensions. The best arrangement depends on memory, sequence length, layer width, batch size, and which devices can communicate quickly. A strategy that works on one cluster may waste time on another because its network topology differs.

### Use model FLOP utilization to diagnose throughput

Peak hardware throughput is a theoretical ceiling. Hardware FLOP utilization (HFU) compares a compute kernel with that ceiling. But training also spends time on data loading, communication, memory movement, optimizer work, and recomputation. **Model FLOP utilization (MFU)** asks how much of the device's theoretical compute rate is realized by the model's forward and backward work in the actual training loop.

A simplified estimate is

$$
\operatorname{MFU}=\frac{\text{estimated model FLOPs per training step}}
{\text{peak FLOPs per second}\times\text{measured seconds per step}}.
$$

For example, suppose a step's model computation is estimated at $2\times10^{15}$ floating-point operations. The device peak is $10^{15}$ operations per second, and the measured step takes 5 seconds. The theoretical time at peak is 2 seconds, so the estimated MFU is $2/5=0.4$, or 40%. This estimate depends on correctly counting model work and the precision-specific peak; it is a tuning signal, not a measure of model quality.

When throughput is low, inspect what keeps the accelerator waiting: communication, small batches, data loading, activation recomputation, or inefficient kernels. The lecture recommends tuning the parallelism and batch/checkpoint choices toward higher MFU. Its numerical guidance reflects the course's training hardware and workload examples; it should not be treated as a universal threshold for every model.

**Source:** SC-11, full original-English captions. The lecture walks from accelerator memory and matrix hardware through data/FSDP/HSDP, checkpointing, context/pipeline/tensor parallelism, and MFU. Hardware figures and scaling thresholds are time- and system-specific; the reusable concepts are the memory/communication hierarchy and each parallelization trade-off.

## Self-supervision and evaluation

### Self-supervision creates training targets from the data

A supervised model learns from human-provided labels. A **self-supervised** model creates a target from the input itself. The task that creates this target is called a **pretext task**. The model learns an encoder representation while solving the pretext task; later, we reuse that encoder for a downstream task, such as classification, with fewer labeled examples.

The hope is that the pretext task forces the encoder to learn useful visual structure. But success on the pretext task does not guarantee useful features for every later task. A model can learn shortcuts that solve the constructed puzzle without learning what matters downstream. Evaluate the representation on the actual target task. A **linear probe** freezes the encoder and learns only a linear classifier, testing whether the information is already easy to read from its features. **Fine-tuning** also updates some or all of the encoder, testing how well it can adapt.

### Transformation and reconstruction pretext tasks

Several early pretext tasks make labels by transforming an image:

- **Rotation prediction:** rotate an image by one of four angles and classify which rotation was used. To solve this, the model may learn object orientation and shape, but it may also exploit dataset-specific cues.
- **Patch location or jigsaw:** shuffle image patches and predict their relative positions or one of a limited set of allowed permutations. There are $9!$ possible permutations for a 3-by-3 grid, so a practical task chooses a smaller set. The model must use context across patches, though patch borders and texture can become shortcuts.
- **Inpainting:** hide part of an image and predict the missing pixels. The target is available from the original image, so no human label is required. Pixel reconstruction can average over many plausible answers and produce blurry results; an adversarial objective can encourage realism, but it introduces a different training problem.
- **Colorization:** convert an image to a color space with a lightness channel and two color channels, then predict color from lightness. The target color is already in the source data. A split-brain version predicts one set of channels from another, such as color from lightness and lightness from color.

These tasks teach the model to make use of image context, but each task has its own bias. Rotation prediction is a classification problem with only four outputs. Colorization can exploit common color-object associations. Reconstructing hidden pixels may reward smooth averages even when the true missing region is ambiguous. Good pretext-task accuracy is therefore only one kind of evidence; downstream performance and robustness also matter.

### Masked autoencoders predict hidden patches

A **masked autoencoder (MAE)** divides an image into patches, hides many patches, and asks a model to reconstruct them. The encoder receives only the visible patches. A separate, smaller decoder receives the encoded visible patches plus learned mask tokens at missing positions and predicts the full image. In the lecture's example, masking about 75% of patches makes each image support many different training views and keeps the expensive encoder focused on visible patches.

Let $x$ be the image represented as patches, $M$ be the set of masked patch indices, and $\hat{x}_i$ be the decoder's reconstruction. A simple objective is mean squared error only over the missing patches:

$$
L_{\text{MAE}}=\frac{1}{|M|}\sum_{i\in M}\|x_i-\hat{x}_i\|_2^2.
$$

The mask tells the model which targets it must infer. Visible patches pass through a vision-transformer encoder; a shared learned mask token fills the missing positions for the decoder. The decoder predicts patch content, and the loss compares those predictions with the original hidden patches. The encoder can then be used alone for a downstream task.

Masking a large fraction reduces encoder computation and creates a hard context-prediction problem. But a reconstruction loss may emphasize low-level pixel similarity instead of every semantic property a later task needs. Linear probing and fine-tuning answer different questions, so it is useful to report both. The lecture compares MAE with other representation-learning systems, but its rankings are tied to particular datasets, model sizes, and training budgets.

### Contrastive learning pulls related views together

Instead of reconstructing pixels, **contrastive learning** shapes the representation space. Start with an image $x$ and apply two random augmentations to make two views of the same image. Their embeddings are a **positive pair**. Other images in the batch supply **negative examples**. The encoder should give the positive pair a higher similarity score than the negatives.

For an anchor embedding $z_i$, its positive embedding $z_{i^+}$, and a set of comparison embeddings $z_j$, the InfoNCE loss can be written

$$
L_i=-\log\frac{\exp(\operatorname{sim}(z_i,z_{i^+})/\tau)}
{\sum_j\exp(\operatorname{sim}(z_i,z_j)/\tau)},
$$

where $\operatorname{sim}$ is often cosine similarity and $\tau$ is a temperature that controls how sharply the softmax responds to score differences. The numerator is the positive pair. The denominator includes that positive and the negatives. Lowering the loss raises the positive's share of the softmax probability. In a batch of $N$ images with two views each, each view can serve as an anchor; its paired view is positive and the other views are treated as negatives.

This resembles multiclass cross-entropy: the positive is the correct class among the candidates. More negatives can make the comparison more informative, but large batches use more memory and computation. The lecture mentions a mutual-information interpretation of the objective; that is a theoretical bound under assumptions, not a claim that the loss directly measures every kind of semantic information.

SimCLR uses two augmentations per image, an encoder, and a projection head. It computes contrastive loss on the projection-head outputs. The encoder representation before that head is retained for downstream use. The lecture's intuition is that the projection space can discard details that help the contrastive objective, while the broader encoder representation can retain information useful for other tasks. Augmentation design matters: if an augmentation removes the evidence required by the downstream task, the model is trained to ignore something it may later need.

MoCo addresses the need for many negatives with a queue of previously computed embeddings. Since queued representations are no longer part of the current gradient computation, a second, momentum-updated encoder produces their keys. The query encoder receives gradients; the key encoder slowly tracks it. This gives a larger comparison set without requiring every negative to be in the current batch. MoCo v2 combines design choices from SimCLR and MoCo. The lecture briefly names CPC and DINO but does not derive their objectives here; DINO uses student/teacher learning and is not simply another contrastive loss.

### Choose the evaluation to match the claim

A pretext task measures whether a model can solve the task it was given. A linear probe measures whether a frozen representation separates labels with a simple classifier. Fine-tuning measures how well the representation adapts when its parameters can change. Robustness and generalization tests measure behavior under shifts or perturbations. These results are not interchangeable: a model can excel at reconstruction yet need substantial fine-tuning, or perform well on one benchmark while failing under a different domain.

The practical workflow is therefore: pretrain on unlabeled data, evaluate representations with a clearly stated protocol, then fine-tune or train a downstream head using the labels actually available for the application. Keep the pretraining data, augmentations, and evaluation split separate enough to detect leakage. Most of all, inspect whether the pretext objective preserves the information the downstream task needs.

**Source:** SC-12, full original-English captions. The lecture covers transformation tasks, reconstruction, MAE, and the basics of contrastive learning, SimCLR, and MoCo. It mentions CPC, MoCo v3, and DINO as follow-up directions but does not provide full derivations for them in this session.

## Generative models learn distributions and samplers

### Generative models represent possible outputs

A discriminative classifier models a label distribution given an input, $p(y\mid x)$. For each input image, it assigns probabilities across its fixed set of labels. If the only labels are “cat” and “dog,” it must choose between them even when the image is a horse, unless the system includes a rejection mechanism.

A generative model instead represents data itself, such as $p(x)$, or a distribution over outputs given a condition, $p(x\mid y)$. A conditional model can represent many images that fit one text prompt. That matters when the task has several valid answers: one prompt can describe many possible pictures. Bayes' rule connects discriminative and generative probabilities:

$$
p(y\mid x)=\frac{p(x\mid y)p(y)}{p(x)}.
$$

The three views answer different questions. A discriminative model asks which label fits this input? An unconditional generator asks which samples look likely under the data distribution? A conditional generator asks which outputs fit this requested condition? A model that can estimate $p(x)$ may help identify unusual samples, but the lecture notes that unconditional generation gives little control over what is drawn. Conditional generation is useful when the user can supply a prompt or other condition.

A probability density must normalize: its total probability over the space is one. This means possible outcomes compete for a fixed amount of probability mass. Generative modeling therefore asks more than “which label should I predict?” It must describe which data are plausible and how likely different possibilities are. In practice, the term “generative model” is often used loosely; check whether a method models an unconditional distribution, a conditional distribution, or only provides samples.

### Likelihood and autoregressive factorization

**Maximum likelihood estimation (MLE)** chooses model parameters that assign high likelihood to observed training examples. For independent examples $x_1,\ldots,x_N$,

$$
\theta^*=\arg\max_\theta \prod_{i=1}^{N}p_\theta(x_i)
=\arg\max_\theta \sum_{i=1}^{N}\log p_\theta(x_i).
$$

The logarithm turns a product into a sum and is easier to compute stably. We hold the observed samples fixed and change the model distribution; this is why the objective is called likelihood. The data are treated as independent draws from an unknown source distribution, and MLE fits the model to the finite sample we have.

An **autoregressive model** factors a sample into an ordered sequence of pieces and applies the probability chain rule:

$$
p(x_1,\ldots,x_T)=\prod_{t=1}^{T}p(x_t\mid x_1,\ldots,x_{t-1}).
$$

This identity is exact. The model predicts the next piece from the prefix. An RNN carries the prefix in its hidden state; a transformer can use a causal mask so position $t$ sees only positions up to $t$. For discrete text tokens, each step predicts a softmax distribution and cross-entropy trains the observed next token. The probability of the whole sequence is the product of the observed-token probabilities at all steps, or the sum of their log probabilities.

Images can be rasterized into sequences too. An 8-bit RGB image has three discrete channel values per pixel, so a $1024\times1024$ image contains more than three million scalar values. Predicting each value in turn is exact in principle, but long sequences are costly. A learned tokenizer can compress image regions into fewer tokens, trading pixel-level detail for a shorter sequence.

### Autoencoders and latent variables

A traditional autoencoder has an encoder $z=f_\phi(x)$ and a decoder $\hat{x}=g_\theta(z)$. It learns to reconstruct the input. A bottleneck makes the task nontrivial: if the latent vector $z$ could simply copy every input value, the system could learn the identity function without useful compression. A compact representation encourages the model to preserve patterns needed to reconstruct the data.

A deterministic autoencoder does not by itself tell us how to sample new latent vectors. Its codes may occupy an irregular region of latent space. Sampling a random point and decoding it can therefore produce an invalid or poor sample. A **variational autoencoder (VAE)** makes the encoder output a distribution over latent codes, $q_\phi(z\mid x)$, and defines a simple prior, usually $p(z)=\mathcal{N}(0,I)$. The decoder defines $p_\theta(x\mid z)$. At generation time, draw $z$ from the prior and decode it.

The exact data likelihood would require marginalizing over all possible latent vectors:

$$
p_\theta(x)=\int p_\theta(x\mid z)p(z)\,dz.
$$

For a neural decoder and high-dimensional $z$, this integral is usually intractable. The encoder distribution $q_\phi(z\mid x)$ gives a tractable approximation. The VAE maximizes the evidence lower bound (ELBO):

$$
\log p_\theta(x)\geq
\mathbb{E}_{q_\phi(z\mid x)}[\log p_\theta(x\mid z)]
- D_{\mathrm{KL}}\big(q_\phi(z\mid x)\,\|\,p(z)\big).
$$

The first term rewards accurate reconstruction: sample a latent code from the encoder and ask the decoder to assign high probability to the original input. The second term, the KL divergence, penalizes an encoder distribution that strays too far from the prior. The KL divergence is nonnegative, which is why dropping an intractable nonnegative term in the derivation gives a lower bound rather than the exact likelihood.

For a diagonal Gaussian encoder, the network predicts a mean vector $\mu(x)$ and standard deviation vector $\sigma(x)$. The reparameterization trick writes a sample as

$$
z=\mu(x)+\sigma(x)\odot\epsilon,\qquad \epsilon\sim\mathcal{N}(0,I).
$$

Randomness enters through $\epsilon$, while $\mu$ and $\sigma$ remain differentiable outputs of the encoder. This lets gradients train the encoder through the sampled code. If the decoder uses a Gaussian with fixed variance, maximizing its reconstruction log likelihood is equivalent, up to a scale and additive constant, to minimizing squared error.

The two ELBO terms pull in different directions. Reconstruction favors distinct, precise codes for different inputs. The prior term pushes those codes toward a shared, smooth distribution. A one-dimensional numerical example shows the regularizer's effect: for a Gaussian encoder $q(z\mid x)=\mathcal{N}(\mu,\sigma^2)$ and unit-Gaussian prior, the KL term is

$$
D_{\mathrm{KL}}=\frac12\big(\mu^2+\sigma^2-1-\log\sigma^2\big).
$$

If $\mu=0$ and $\sigma=1$, the KL is zero: the encoder matches the prior. If $\mu=2$ and $\sigma=1$, it is 2 nats, so the model pays a penalty for centering this input's code far from the prior. Too much pressure toward the prior can weaken reconstruction; too little can leave latent regions that are difficult to sample. The useful setting balances both.

A VAE makes generation possible because prior samples are likely to lie in latent regions the decoder learned to use. This can also create smooth latent traversals: moving between codes can gradually change a generated image. That does not guarantee that every latent dimension has a clean human meaning or that samples will be sharp. The lecture's taxonomy separates exact explicit densities (such as autoregressive models), approximate explicit densities (VAEs), implicit one-pass samplers (GANs), and implicit iterative samplers (diffusion, discussed in the next lecture).

**Source:** SC-13, full original-English captions. The lecture derives MLE, autoregressive factorization, and the VAE lower bound; the next session is explicitly reserved for GANs and diffusion. Historical statements about which model family is most useful are time-specific context, not universal rankings.

### GANs learn by competing

A **generative adversarial network (GAN)** draws a latent vector $z$ from a simple prior and maps it to a sample $G(z)$. A discriminator $D(x)$ predicts whether an image is real or generated. The discriminator learns a real-versus-fake classification task. The generator learns to make generated samples that the discriminator classifies as real.

The original minimax objective is

$$
\min_G\max_D\;V(D,G)=
\mathbb{E}_{x\sim p_{\text{data}}}[\log D(x)]
+\mathbb{E}_{z\sim p(z)}[\log(1-D(G(z)))].
$$

The discriminator maximizes this expression: it wants $D(x)$ near 1 for real data and $D(G(z))$ near 0 for generated data. The generator minimizes it: it wants generated samples to receive high real scores. Gradients flow from the discriminator through the generated image and into the generator. After training, sampling is fast: draw $z$, run $G$, and discard the discriminator.

The objective describes a two-player game, not a single loss curve that must steadily fall. A strong discriminator can give the generator weak gradients early in training. The common non-saturating generator objective instead maximizes $\log D(G(z))$; it gives stronger gradients when the discriminator confidently rejects early, poor samples. The generator and discriminator also change each other's training data, so the optimization is non-stationary. Training can oscillate, become numerically unstable, or suffer **mode collapse**: many latent vectors map to only a few generated examples. A low discriminator loss or plausible sample grid alone does not prove the generator covers the full data distribution.

GANs can produce crisp samples, and latent interpolation can change an image smoothly. But a standard GAN maps from latent code to image without learning an encoder from image back to code. It has no explicit likelihood value, and it can be difficult to diagnose or tune. The lecture gives an idealized result in which a perfect-capacity discriminator and generator have an optimum when generated and real distributions match; finite neural networks, limited data, and gradient training do not guarantee reaching that optimum.

### Rectified flow turns sampling into an integration problem

Diffusion methods begin with a simple noise distribution and learn to move a noisy sample toward the data distribution through many small steps. The lecture focuses on **rectified flow**, a concrete version that uses a straight interpolation between a real training sample $x$ and a random noise sample $z$. Choose $t$ uniformly from $[0,1]$, with $t=0$ at data and $t=1$ at noise:

$$
x_t=(1-t)x+t z,\qquad v=z-x.
$$

The training network sees $(x_t,t)$ and predicts the velocity $v$. It is trained with squared error:

$$
L(\theta)=\mathbb{E}_{x,z,t}\left[\|f_\theta(x_t,t)-(z-x)\|_2^2\right].
$$

For a one-dimensional example, let a data value be $x=2$, a noise draw be $z=-1$, and $t=0.25$. Then $x_t=0.75(2)+0.25(-1)=1.25$, and the training target velocity is $v=-1-2=-3$. The model is asked to predict the direction and rate of change as $t$ moves from clean data toward noise.

At generation time, start with noise at $t=1$ and integrate backward toward $t=0$. With a step size $\Delta t>0$, the simple Euler update is

$$
x_{t-\Delta t}=x_t-\Delta t\,f_\theta(x_t,t).
$$

The minus sign matters: the training velocity points from data to noise as $t$ increases, while generation moves from noise back to data. With $N$ evenly spaced steps, $\Delta t=1/N$. More steps usually cost more compute but can better approximate the learned path; the lecture gives dozens of steps as a practical starting point, not a universal setting.

Rectified flow is easier to monitor than the adversarial game because it has a direct regression loss. The straight line between a particular data point and a noise draw is a training construction; the model learns a velocity field that combines many such examples. In high dimensions, the geometry is more complex than a two-dimensional diagram suggests. The model is not given the correct pairing between each noise sample and real sample ahead of time; learning a useful transport between the distributions is the central challenge.

### Conditioning and classifier-free guidance

Conditional generation supplies an additional input $y$, such as a text embedding or image. The velocity network becomes $f_\theta(x_t,t,y)$, and it learns to move toward samples that fit that condition. To control how strongly the output follows $y$, **classifier-free guidance** trains the model with the condition removed on some examples. It therefore learns both conditional and unconditional predictions.

At sampling time, combine them:

$$
v_{\text{guided}}=v_{\text{uncond}}+s\big(v_{\text{cond}}-v_{\text{uncond}}\big).
$$

At $s=1$, this equals the conditional prediction. When $s>1$, it moves farther in the direction that distinguishes the conditioned prediction from the unconditional one. Stronger guidance can improve prompt adherence, but may reduce variety or produce artifacts. It requires two network evaluations per sampling step unless the implementation combines them efficiently.

### Latent diffusion combines several model families

Operating directly on high-resolution pixels is expensive. **Latent diffusion** first encodes an image into a smaller latent tensor, then runs diffusion on that representation. A decoder converts the final clean latent back to pixels. In common pipelines the encoder and decoder form a VAE-like autoencoder. A discriminator can also be used when training the decoder to improve reconstruction sharpness, because a plain pixel reconstruction objective often produces blurry outputs. The diffusion model learns to generate in the compressed latent space.

A text-to-image pipeline typically has a pretrained text encoder, a diffusion transformer, and an image decoder. The text encoder turns the prompt into embeddings. The diffusion transformer receives noisy image latents, the time step, and the text features. It predicts an update, repeated over multiple steps. The decoder maps the final latent to the image. Each component has a different role; the complete system is not just one generative network.

A video pipeline adds a time axis to the latent. More frames and higher resolution mean more tokens. Since attention can scale quadratically with token count, video generation can be much more expensive than image generation. Distillation aims to reduce the number of sampling steps, usually trading some sample quality for speed. Transformer diffusion models inject time information through scale/shift modulation and text or other conditions through cross-attention or joint attention.

### Diffusion has several mathematical views

The lecture presents diffusion from three related but distinct perspectives. The first is denoising: learn to reverse a process that adds noise. The second is a latent-variable or likelihood view, where the training objective can be related to a lower bound on data likelihood. The third is the **score function**, $\nabla_x\log p(x)$, which points toward regions of higher probability density; models learn this direction for data corrupted at different noise levels. A fourth language describes sampling as solving a stochastic differential equation (SDE), then numerically integrating it from noise toward data.

These views motivate different objectives and sampling algorithms. They are not identical derivations, and the lecture intentionally does not give a full treatment of each. Rectified flow's simple Euler update is one numerical integration approach; more advanced solvers can trade evaluation count against accuracy. Keeping the distinction clear prevents treating one simplified training recipe as the entire theory of diffusion.

**Source:** SC-14, full original-English captions. The lecture develops the GAN game, its practical instability, rectified-flow training and sampling, classifier-free guidance, latent diffusion, and several theoretical views of diffusion. Its model rankings and product examples are tied to the lecture date; the mathematical mechanisms and speed/quality trade-offs are the lasting material.

## Vision-language models and grounded tools

### Foundation models and adaptation

A task-specific model is trained for one job, such as image classification, and then tested on examples reserved for evaluation. A **foundation model** is first trained on broad data and objectives, then adapted to many downstream tasks. Adaptation can mean adding a small trained classifier, supplying a prompt, or connecting the model to another component. “Foundation model” is a broad label, not a guarantee of robustness, safety, or general reasoning. The lecture notes that the definition is contested.

For language models, next-token prediction creates a flexible interface: provide a prefix and ask the model to continue it. Image encoders trained only to match image and text representations do not automatically have this interface. They need a separate way to adapt to a new task. This difference motivates the progression from CLIP-style encoders to vision-language models that generate text, masks, or actions through connected components.

### CLIP: align images with their descriptions

CLIP uses two encoders: an image encoder and a text encoder. Each maps its input to a vector in a shared space. Training data consists of matching image-caption pairs. A correct image and caption should have high similarity; a mismatched pair should have lower similarity. This is **contrastive learning**: pull matching examples together and push other examples apart.

For a mini-batch of $N$ pairs, let normalized image vector $v_i$ and text vector $t_j$ have similarity $s_{ij}=v_i^\top t_j/\tau$, where $\tau$ is a temperature that controls how sharply the scores are separated. The image-to-text loss is

$$
L_{I\to T}=-\frac{1}{N}\sum_{i=1}^{N}\log\frac{\exp(s_{ii})}{\sum_{j=1}^{N}\exp(s_{ij})}.
$$

This is cross-entropy over the batch: for each image, the matching caption is the correct class among the $N$ captions. A symmetric text-to-image loss swaps the roles of images and captions:

$$
L=\frac{1}{2}(L_{I\to T}+L_{T\to I}).
$$

The symmetry matters. Each image must retrieve its caption, and each caption must retrieve its image. Other batch examples act as negatives. If the batch contains only a dog and a truck, it teaches less about the difference among dog breeds than a batch that also contains several similar dogs. Larger batches increase the chance of including these **hard negatives**, which are plausible but incorrect alternatives. They also demand more memory and compute, and they do not guarantee that training examples cover every important distinction.

After training, image and text vectors can be compared directly. For zero-shot classification, write each class name as a text prompt, encode the prompts, encode a new image, and choose the class with the highest cosine similarity. For example, compare an image vector with text vectors for “a photo of a plane,” “a photo of a dog,” and “a photo of a bird.” This is a nearest-neighbor decision in the shared vector space. A prompt such as “a photo of a dog” can work better than the bare word “dog” because the model was trained on image-caption phrases. One can encode several templates per class and average their vectors to reduce dependence on one wording.

Zero-shot evaluation uses the fixed pretrained model and prompts. A **linear probe** freezes the image encoder and trains a small linear classifier on labeled examples. A linear probe can improve performance, but it requires task-specific labels and training. Zero-shot behavior is useful when a new class set has no labeled training examples.

CLIP's strength comes from broad image-text associations, not only category names. Captions can mention colors, shapes, actions, and context. This can help when test images differ from common benchmark images. The lecture contrasts internet-scale pairs with a much smaller labeled classification dataset; its counts and benchmark results describe the particular 2025 talk and should not be treated as current general rankings.

CLIP has important limits. An image-level caption may say that a person crosses a street without identifying the person's exact location, the car's location, or their relation. The model can recognize “mug” and “grass” but still confuse “a mug in grass” with “grass in a mug.” That is a **compositionality** problem: knowing the parts does not ensure that the model understands how those parts relate. Larger batches and carefully selected hard negatives can improve fine distinctions, but curated hard negatives may also hurt generalization in some settings. The lecture presents that result as an open research issue rather than a settled theory.

Open-vocabulary retrieval is another practical strength. Once a dataset is embedded, a text query can retrieve images without training a new fixed-category classifier. But a text prompt does not add missing evidence. If the training captions omit spatial relations or rare objects, retrieval cannot reliably recover them just because the vocabulary is open.

### Add captioning and spatial features to vision-language models

CoCa combines image-text contrastive training with a captioning objective. The image encoder and text encoder still learn to align paired examples. A decoder also attends to the image features and generates a caption. The two objectives ask for different levels of information: matching a caption can identify the broad topic, while generating the caption requires predicting a sequence of words. The lecture argues that this richer signal can improve the learned image features. The result is still dependent on the training data and evaluation setup.

A vision-language model (VLM) connects visual features to a language model that generates text one token at a time. The image must first be converted into tokens the language model can use. A common design uses a pretrained vision encoder, then a learned **projector**—often a linear layer—to map visual features into the language model's feature space. These projected tokens are placed with the text context. The language model can then answer questions or describe the image.

In a CLIP Vision Transformer, the class token is trained to summarize the image for image-text matching. Other patch tokens may retain spatial detail, but the lecture cautions that their usefulness depends on which layers and supervision are used. It describes using features from just before the final pooled representation because they retain more spatial information. A single global vector is often insufficient when a task asks where an object is.

Flamingo uses a different connection. A Perceiver resampler reduces visual features to a fixed, manageable set of tokens. Cross-attention layers are inserted into the language model so that its text representations can query image features as needed. A **cross-attention** operation uses one sequence as queries and another as keys and values; here, language tokens query the image tokens. Gates and residual connections control how much visual information is added to the language stream. The lecture describes most of the pretrained vision and language weights as frozen in the presented setup, while the resampler and inserted cross-attention components are trained.

This architecture can process interleaved sequences such as image, text, image, text. A mask makes each generated description attend to its corresponding image, while the wider sequence can provide conversational context. This supports multi-image questions and multi-turn dialogue. **In-context learning** means the prompt includes examples of the desired input-output behavior, such as several image-question-answer pairs, and the model attempts a new one without updating its weights. With no examples, the same setup is zero-shot. These are behaviors observed in particular models and prompts; the presence of a fluent answer does not verify that its visual claim is correct.

### Data quality and visual grounding

The lecture's guest presents a VLM example that outputs points as evidence for answers. For a counting question, it points to the items it counted and then gives a number. For OCR, it points to the region containing a bus route number before transcribing it. Coordinates or points provide **grounding**: they connect part of the answer to a location in the image. A point can also be passed to another model, such as a segmentation system, to turn a location into a mask.

The speaker argues that data density and content can matter as much as raw pair count. Internet captions are often incidental: they may express the uploader's reaction or omit obvious spatial facts. Targeted annotation can ask about position, size, shape, material, text, or other details that ordinary captions rarely mention. The lecture describes human elicitation studies and spoken annotation as ways to gather richer descriptions. It reports a particular model and user-study result, but those claims are speaker-reported and time-bound; a preference study or a strong benchmark score is not proof that a model is reliable in every use.

The central lesson is broader than that model: visual tasks need supervision for the information they require. A caption that says “a phone” will not teach the time displayed on the screen or the battery level. A question about relative location needs spatial annotations or another mechanism to inspect pixels. More images alone do not guarantee better coverage if their associated text omits the relevant evidence.

### Prompted segmentation as a foundation task

A segmentation model outputs a mask: a value for each pixel indicating whether it belongs to the target region. The target may be ambiguous. If an image has two cats, “segment the cat” does not say which one. A point, bounding box, or text prompt can specify the user's intent more precisely.

SAM separates this process into three parts: an image encoder computes reusable image features; a prompt encoder converts points, boxes, or other prompts into features; and a lightweight mask decoder combines them to produce one or more candidate masks. A point can still be ambiguous. It might refer to a whole object or to a meaningful part. Instead of forcing the model to guess one interpretation during training, the described system predicts multiple masks at different levels of detail and uses the candidate that best matches the annotated target for the loss. At use time, a person or downstream system can choose the suitable granularity.

The model depends on extensive mask data. The lecture describes an iterative data engine: begin with human annotations, train a model, use it to propose more masks, and have people correct or refine those proposals. This human-in-the-loop process makes a large mask collection possible, but annotation remains costly. The lecture's reported dataset growth is specific to the cited release and should not be generalized to later systems.

### Chaining models into programs

A single model may not reliably solve every subtask. **Chaining** connects models so that one model's output becomes another model's input. For example, a point-grounded VLM can point to a cricket bat; a video segmentation model can use that point to track the bat across frames. A point can also guide a robot's motion planner toward a water bottle. The language model proposes or identifies a goal; a separate system handles localization, segmentation, motion, or control.

A more general form is visual programming. Given a question such as “Are there six people across these two boat images?”, a language model can write a small program that calls an object detector on both images, counts the detections, adds the counts, and compares the result with six. The model does not need to perform every low-level visual computation internally. It selects tools and combines their outputs. The system needs a clear tool interface: what each tool accepts, what it returns, and what the output means.

Programs can be fixed in advance or generated for each question. In-context examples can show the model how to call available functions. Retrieval can select examples that resemble the current question, but this adds another component that can fail. Chaining can expand capability, yet each call adds latency, compute cost, and a new opportunity for error. An incorrect detector output can produce a wrong final count even if the program adds correctly. The final result should therefore preserve intermediate evidence and be checked when the task matters.

Chaining is also useful for image editing. A segmentation model can identify the desert region; an image generator can replace that region with grass; compositing can retain pixels outside the mask. This can be more controllable than asking one generator to redraw the whole image, but segmentation boundaries and generated content still need inspection.

### Grounding reduces, but does not remove, hallucinations

A model that points while answering exposes some evidence that can be checked. It may reduce unsupported answers, but a point can still land on the wrong object, and a correct point does not prove the text conclusion follows. Stronger systems can pass outputs through independent verifiers, check a count against detections, or ask a human to review uncertain cases. Verification is an additional process, not an automatic guarantee.

The lecture also raises the idea of automatically building new tools when a task requires a capability the system does not already have. It describes this as early-stage research. Until a generated tool has been tested, its output should not be treated as reliable just because a model proposed it.

**Source:** SC-16, full original-English captions. The notes follow the lecture's progression from image-text contrastive training, through VLM and segmentation architectures, to data grounding and tool chaining. Historical model names, release dates, benchmark scores, and user-study claims are speaker-reported examples tied to the lecture period; they are not presented as current comparative evaluations.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/30 - Mesh Operators, Boundaries and Diffusion|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/32 - Pose Estimation and Robust Fitting|Next chapter]]
