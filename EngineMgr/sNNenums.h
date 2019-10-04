#pragma once

//-- Training Protocols
#define TP_STOCHASTIC	0
#define TP_BATCH		1
#define TP_ONLINE		2

//-- Activation Functions
#define NN_ACTIVATION_TANH     1	// y=tanh(x)				=> range is [-1 ÷ 1]
#define NN_ACTIVATION_EXP4     2	// y = 1 / (1+exp(-4*x))	=> range is [ 0 ÷ 1]
#define NN_ACTIVATION_RELU     3	// y=max(0,x)
#define NN_ACTIVATION_SOFTPLUS 4	// y=ln(1+e^x)
#define NN_ACTIVATION_TANH2    5	// y=a*tanh(b*x)			=> range is [-2 ÷ 2]

//-- Backpropagation algorithms
#define BP_STD			0
#define BP_QING			1
#define BP_RPROP		2
#define BP_QUICKPROP	3
#define BP_SCGD			4 // Scaled Conjugate Gradient Descent
#define BP_LM			5 // Levenberg-Marquardt
