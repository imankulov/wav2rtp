#ifndef NETWORK_EMULATOR_H
#define NETWORK_EMULATOR_H

#include "contrib/simclist.h"

/** None losses */
#define NONE_LOSS        0
/** losses are represented as independent random variates */
#define INDEPENDENT_LOSS 1
/** losses are represented as random Markov process */
#define MARKOV_LOSS      2
/** chained losses */
#define CHAINED_LOSS     3
/** chained losses with interpolation */
#define CHAINED_INT_LOSS 4


/** None delay */
#define NONE_DELAY       0
/** Uniform delay */
#define UNIFORM_DELAY    1
/** Gamma delay */
#define GAMMA_DELAY      2 
/** Delay distribution based on external statistic data */
#define STATISTIC_DELAY  3


/**
 * Internal structure which stores information about network emulator parameters and its current state 
 */
typedef struct __wr_network_emulator {

    /** One of these loss models may be used: none losses, independent losses or lossed represented by markov chain  */
    int loss_model;

    /** One of these delay models may be used: none, uniform, gamma or statistic data based delay */
    int delay_model;

    /**
     * loss rate
     */
    union {
        /* independent losses */
        struct {
            double loss_rate;
        } independent;

        /* markov losses */
        struct {
            double loss_0_1;
            double loss_1_1;
            int __prev_packet_lost; /* internal variable */
        } markov;

        /* chained losses */
        struct {
            double loss_rate; 
            int chain_size;
            int __position; /* internal variable - postion of the current packet in the chain */
            int __is_lost;  /* internal variable - non-zero if current chain have to be lost whole */
        } chained;

        /* chained_int */
        struct {
            double loss_rate;
            int chain_size;
            list_t * data_frames;  /* this stores an array of list_t objects of data frames */
            int __position; /* internal variable - position of the current packet in the chain */
            int __is_lost;  /* internal variable - non-zero if current chain have to be lost whole */
        } chained_int;
    } loss;

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
    list_t * data_frames; /**< pointer to the data frames which may be changed by network emulator */
} wr_packet_state_t;

wr_network_emulator_t * wr_network_emulator_init(wr_network_emulator_t * netem);
int wr_network_emulator_next(wr_network_emulator_t * netem, wr_packet_state_t * state);
#endif
