#ifndef NETWORK_EMULATOR_H
#define NETWORK_EMULATOR_H 1

#include "contrib/simclist.h"

/** None losses */
#define NONE_LOSS        0
/** losses are represented as independent random variates */
#define INDEPENDENT_LOSS 1
/** losses are represented as random Markov process */
#define MARKOV_LOSS      2


/** None delay */
#define NONE_DELAY       0
/** Uniform delay */
#define UNIFORM_DELAY    1
/** Gamma delay */
#define GAMMA_DELAY      2 
/** Delay distribution based on external statistic data */
#define STATISTIC_DELAY  3


/**
 * Internal structure which store information about network emulator parameters and current state 
 */
typedef struct __wr_network_emulator {

    /** One of these loss models may be used: none losses, independent losses or lossed represented by markov chain  */
    int loss_model;

    /** One of these delay models may be used: none, uniform, gamma or statistic delay */
    int delay_model;

    /** Initial seed */
    char seed[256];

    /**
     * loss rate
     * If loss is represented as "independent" then algorithm use only first value from array 
     * Elsewere loss_rate[0] is the loss probability if previous packed was NOT be lost and 
     * loss_rate[1] is the loss probability if previous packed was lost.
     * 0<= loss_rate[i] <= 1 
     */
    double loss_rate[2];

    /**
     * Union which  contains information about delay parameters
     */
    union {
        /* for uniform distribution */
        struct {
            int min; 
            int max;
        } uniform;
        /* for gamma distribution */
        struct {
            int shape;
            int scale;
        } gamma;
        /* for external statistic data distribution */
        list_t * statistic_data;
    } delay;

    /** Internal boolean variable which store if prev packet was lost */
    int __prev_packet_lost;

} wr_network_emulator_t;

/**
 * Data structure which is returned by 
 * network_emulator_next and store parameters
 * of the current packet
 */
typedef struct  __wr_packet_state {
    int lost; /**< data packet should be lost */
    int delay; /**< delay (int) of the packet (in microseconds) */    
} wr_packet_state_t;

wr_network_emulator_t * wr_network_emulator_init(wr_network_emulator_t * netem);
int wr_network_emulator_next(wr_network_emulator_t * netem, wr_packet_state_t * state);
#endif
