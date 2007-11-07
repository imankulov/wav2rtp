#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "options.h"
#include "contrib/iniparser.h"
#include "network_emulator.h"
#include "error_types.h"
#include "misc.h"

/**
 * Network emulator initialization:
 * @return: netem or NULL if smth fails
 */
wr_network_emulator_t * wr_network_emulator_init(wr_network_emulator_t * netem)
{
    bzero(netem, sizeof(*netem));
    {
        char * seed = iniparser_getstring(wr_options.output_options, "network_emulator:random_seed", "0");
        bzero(netem->seed, sizeof(netem->seed));
        if (seed[0] == '0' && seed[1] == '\0'){             
            time_t time_loc;
            time(&time_loc);
            memcpy(netem->seed, &time, sizeof(time_t));
            if(!initstate(1, netem->seed, sizeof(netem->seed))){
                wr_set_error("Unable to initialize random number generator");
                return NULL;
            }
        } else {
            strncpy(netem->seed, seed, sizeof(netem->seed)-2);
            if(!initstate(1, netem->seed, sizeof(netem->seed))){
                wr_set_error("Unable to initialize random number generator");
                return NULL;
            }
        }
    }    
    {
        char * loss_model;
        loss_model = iniparser_getstring(wr_options.output_options, "network_emulator:loss_model", "none");
        if (strncmp(loss_model, "none", 5) == 0){
            netem->loss_model = NONE_LOSS;
        } else if (strncmp(loss_model, "independent", 12) == 0) {
            netem->loss_model = INDEPENDENT_LOSS;
            netem->loss_rate[0] = iniparser_getdouble(wr_options.output_options, "network_emulator:loss_rate", 0);
            if (netem->loss_rate[0] < 0 )  netem->loss_rate[0] = 0;
            if (netem->loss_rate[0] > 1 )  netem->loss_rate[0] = 1; 
        } else {
            netem->loss_model = MARKOV_LOSS;
            netem->loss_rate[0] = iniparser_getdouble(wr_options.output_options, "network_emulator:loss_0_1", 0);
            if (netem->loss_rate[0] < 0 )  netem->loss_rate[0] = 0;
            if (netem->loss_rate[0] > 1 )  netem->loss_rate[0] = 1;
            netem->loss_rate[1] = iniparser_getdouble(wr_options.output_options, "network_emulator:loss_1_1", 0);
            if (netem->loss_rate[1] < 0 )  netem->loss_rate[1] = 0;
            if (netem->loss_rate[1] > 1 )  netem->loss_rate[1] = 1;        
        }
    }  
    {
        char * delay_model;
        delay_model = iniparser_getstring(wr_options.output_options, "network_emulator:delay_model", "none");
        if (strncmp(delay_model, "none", 5) == 0){
            netem->delay_model = NONE_DELAY;
        }else if (strncmp(delay_model, "uniform", 8) == 0){
            netem->delay_model = UNIFORM_DELAY;
            netem->delay.uniform.min = iniparser_getpositiveint(wr_options.output_options, "network_emulator:delay_uniform_min", 0);
            netem->delay.uniform.max = iniparser_getpositiveint(wr_options.output_options, "network_emulator:delay_uniform_max", 0);
            if (netem->delay.uniform.min > netem->delay.uniform.max){
                int tmp = netem->delay.uniform.min; netem->delay.uniform.min = netem->delay.uniform.max; netem->delay.uniform.max = tmp;
            }
        }
        /* TODO: other delays not yet implemented */
    }
    return netem;
}

/**
 * Update internal state of the network emulator and return packet state in fileds of "state"
 * @return: 0 if all OK 
 * */
int wr_network_emulator_next(wr_network_emulator_t * netem, wr_packet_state_t * state)
{
    bzero(state, sizeof(*state));
    if (netem->loss_model == NONE_LOSS){
        netem->__prev_packet_lost = 0;
    } else if (netem->loss_model == INDEPENDENT_LOSS){
        double rand = (double)random() / RAND_MAX;
        if (rand < netem->loss_rate[0]){
            netem->__prev_packet_lost = 1;
        } else {
            netem->__prev_packet_lost = 0;
        }
    } else {
        double rand = (double)random() / RAND_MAX;
        double threshold;
        if (netem->__prev_packet_lost){
            threshold = netem->loss_rate[1];
        }else{
            threshold = netem->loss_rate[0];
        }
        if (rand < threshold){
            netem->__prev_packet_lost = 1;
        } else {
            netem->__prev_packet_lost = 0;
        }
        /* printf("%f %f %d\n", rand, threshold, netem->__prev_packet_lost); */
    }
    state->lost = netem->__prev_packet_lost;

    if (netem->delay_model == NONE_DELAY){
        state->delay = 0;
    } else if (netem->delay_model == UNIFORM_DELAY) {
        int a = netem->delay.uniform.min;
        int b = netem->delay.uniform.max;
        long randlong = rand();        
        double rand = (double)randlong/RAND_MAX; /*  assume that rand always < 1  */
        state->delay = a + ( b + 1 - a ) * rand;
    } else { /* TODO: Not yet implemented */
        state->delay = 0;
    }
    return 0;
}
