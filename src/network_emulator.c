#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "options.h"
#include "contrib/iniparser.h"
#include "contrib/ranlib/ranlib.h"
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
        setall(random(), random());
    } 
    {
        char * loss_model;
        loss_model = iniparser_getstring(wr_options.output_options, "network_emulator:loss_model", "none");
        if (strncmp(loss_model, "none", 5) == 0){
            netem->loss_model = NONE_LOSS;

        } else if (strncmp(loss_model, "independent", 12) == 0) {
            netem->loss_model = INDEPENDENT_LOSS;
            netem->loss.independent.loss_rate = iniparser_getdouble(wr_options.output_options, "network_emulator:loss_rate", 0);
            if (netem->loss.independent.loss_rate < 0 )  netem->loss.independent.loss_rate = 0;
            if (netem->loss.independent.loss_rate > 1 )  netem->loss.independent.loss_rate = 1; 

        } else if (strncmp(loss_model, "markov", 7) == 0) {
            netem->loss_model = MARKOV_LOSS;
            netem->loss.markov.loss_0_1 = iniparser_getdouble(wr_options.output_options, "network_emulator:loss_0_1", 0);
            if (netem->loss.markov.loss_0_1 < 0 )  netem->loss.markov.loss_0_1 = 0;
            if (netem->loss.markov.loss_0_1 > 1 )  netem->loss.markov.loss_0_1 = 1;
            netem->loss.markov.loss_1_1 = iniparser_getdouble(wr_options.output_options, "network_emulator:loss_1_1", 0);
            if (netem->loss.markov.loss_1_1 < 0 )  netem->loss.markov.loss_1_1 = 0;
            if (netem->loss.markov.loss_1_1 > 1 )  netem->loss.markov.loss_1_1 = 1;

        } else if (strncmp(loss_model, "chained", 8) == 0) {
            netem->loss_model = CHAINED_LOSS;
            netem->loss.chained.loss_rate = iniparser_getdouble(wr_options.output_options, "network_emulator:loss_rate", 0);
            if (netem->loss.chained.loss_rate < 0 )  netem->loss.chained.loss_rate = 0;
            if (netem->loss.chained.loss_rate > 1 )  netem->loss.chained.loss_rate = 1;
            netem->loss.chained.chain_size = iniparser_getpositiveint(wr_options.output_options, "network_emulator:chain_size", 1);

        } else if (strncmp(loss_model, "chained_int", 12) == 0) {
            netem->loss_model = CHAINED_INT_LOSS;
            netem->loss.chained_int.loss_rate = iniparser_getdouble(wr_options.output_options, "network_emulator:loss_rate", 0);
            if (netem->loss.chained_int.loss_rate < 0 )  netem->loss.chained_int.loss_rate = 0;
            if (netem->loss.chained_int.loss_rate > 1 )  netem->loss.chained_int.loss_rate = 1; 
            netem->loss.chained_int.chain_size = iniparser_getpositiveint(wr_options.output_options, "network_emulator:chain_size", 1);

        } else {
            wr_set_error("Unable to determine loss model");
            return NULL;
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
        }else if (strncmp(delay_model, "gamma", 6) == 0){
            netem->delay_model = GAMMA_DELAY;
            netem->delay.gamma.shape = iniparser_getpositiveint(wr_options.output_options, "network_emulator:delay_gamma_shape", 0);
            netem->delay.gamma.scale = iniparser_getpositiveint(wr_options.output_options, "network_emulator:delay_gamma_scale", 0);
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
    if (netem->loss_model == NONE_LOSS){
        state->lost = 0;
    } else if (netem->loss_model == INDEPENDENT_LOSS){
        double rand = (double)random() / RAND_MAX;
        state->lost = (rand < netem->loss.independent.loss_rate) ? 1 : 0; 
    } else if (netem->loss_model == MARKOV_LOSS){
        double rand = (double)random() / RAND_MAX;
        double threshold =  (netem->loss.markov.__prev_packet_lost) ? netem->loss.markov.loss_1_1 : netem->loss.markov.loss_0_1;
        netem->loss.markov.__prev_packet_lost = (rand < threshold) ? 1 : 0;
        state->lost = netem->loss.markov.__prev_packet_lost;
    } else if (netem->loss_model == CHAINED_LOSS ){        
        if ( netem->loss.chained.__prev_lost ){ /* several previous packets already lost */
            if (netem->loss.chained.__prev_lost == netem->loss.chained.chain_size){ /* no more losses */
                netem->loss.chained.__prev_lost = 0;
                state->lost = 0;
            } else { /* yet another packet have to be lost */
                netem->loss.chained.__prev_lost ++;
                state->lost = 1;
            }
        }
        if ( !netem->loss.chained.__prev_lost ) { /* nothing is yet lost  */
            double rand = (double)random() / RAND_MAX;
            double threshold = netem->loss.chained.loss_rate / netem->loss.chained.chain_size;
            netem->loss.chained.__prev_lost = (rand < threshold) ? 1 : 0;
            state->lost = netem->loss.chained.__prev_lost;               
        }
    } else if (netem->loss_model == CHAINED_INT_LOSS ){        
        state->lost = 0;
        if ( netem->loss.chained_int.__prev_lost ){/* several previous packets already lost */
            if (netem->loss.chained_int.__prev_lost == netem->loss.chained_int.chain_size){ /* no more losses */
                netem->loss.chained_int.__prev_lost = 0;
            } else { /* yet another packet have to be lost */
                printf("-\n");
                list_t * data_frames = &(netem->loss.chained_int.data_frames);
                /* remove all current data ... */
                list_iterator_start(state->data_frames);
                while(list_iterator_hasnext(state->data_frames)){
                    wr_data_frame_t * tmp  = list_iterator_next(state->data_frames);
                    free(tmp->data);
                }
                list_iterator_stop(state->data_frames);
                list_destroy(state->data_frames);
                /* ... and restore saved list instead */
                list_init(state->data_frames); 
                list_iterator_start(data_frames);
                while (list_iterator_hasnext(data_frames)){
                    /* store all data frames */
                    wr_data_frame_t * tmp  = list_iterator_next(data_frames);
                    wr_data_frame_t * frame = calloc(1, sizeof(wr_data_frame_t));
                    frame->data = calloc(tmp->size, sizeof(uint8_t));
                    memcpy(frame->data, tmp->data, tmp->size);
                    frame->length_in_ms = tmp->length_in_ms;
                    frame->size = tmp->size;
                    list_append(state->data_frames, frame);
                }
                list_iterator_stop(data_frames);
                netem->loss.chained_int.__prev_lost ++;
            }
        }


        if ( !netem->loss.chained_int.__prev_lost ) { /* nothing is yet lost  */
            double rand = (double)random() / RAND_MAX;
            double threshold = netem->loss.chained_int.loss_rate / netem->loss.chained_int.chain_size;
            //double threshold = netem->loss.chained_int.loss_rate;
            netem->loss.chained_int.__prev_lost = (rand < threshold) ? 1 : 0;
            if (netem->loss.chained_int.__prev_lost) { /* current packet have to be lost */
                printf("-\n");
                list_t * data_frames = &(netem->loss.chained_int.data_frames);
                if (list_empty(data_frames) == 0){ /* list is empty, nothing to duplicate */
                    state->lost = 1;
                } else {
                    /* remove all current data ... */
                    list_iterator_start(state->data_frames);
                    while(list_iterator_hasnext(state->data_frames)){
                        wr_data_frame_t * tmp  = list_iterator_next(state->data_frames);
                        free(tmp->data);
                    }
                    list_iterator_stop(state->data_frames);
                    list_destroy(state->data_frames);
                    /* ... and restore saved list instead */
                    list_init(state->data_frames); 
                    list_iterator_start(data_frames);
                    while (list_iterator_hasnext(data_frames)){
                        /* store all data frames */
                        wr_data_frame_t * tmp  = list_iterator_next(data_frames);
                        wr_data_frame_t * frame = calloc(1, sizeof(wr_data_frame_t));
                        frame->data = calloc(tmp->size, sizeof(uint8_t));
                        memcpy(frame->data, tmp->data, tmp->size);
                        frame->length_in_ms = tmp->length_in_ms;
                        frame->size = tmp->size;
                        list_append(state->data_frames, frame);
                    }
                    list_iterator_stop(data_frames);
                }
            } else { /* current packet have not to be lost, so save its data in the internal buffer (if following packet were lost ) */
                printf("+\n");
                list_t * data_frames = &(netem->loss.chained_int.data_frames);
                if (list_empty(data_frames) == 1){ /* if buffer already stores some data clean up it */
                    list_iterator_start(data_frames);
                    while(list_iterator_hasnext(data_frames)){
                        wr_data_frame_t * tmp  = list_iterator_next(data_frames);
                        free(tmp->data);
                    }
                    list_iterator_stop(data_frames);
                    list_destroy(data_frames);
                }
                list_init(data_frames); 
                list_iterator_start(state->data_frames);
                while (list_iterator_hasnext(state->data_frames)){
                    /* store all data frames */
                    wr_data_frame_t * tmp  = list_iterator_next(state->data_frames);
                    wr_data_frame_t * frame = calloc(1, sizeof(wr_data_frame_t));
                    frame->data = calloc(tmp->size, sizeof(uint8_t));
                    memcpy(frame->data, tmp->data, tmp->size);
                    frame->length_in_ms = tmp->length_in_ms;
                    frame->size = tmp->size;
                    list_append(data_frames, frame);
                }
                list_iterator_stop(state->data_frames);
            }
        }
    } else {}

    if (netem->delay_model == NONE_DELAY){
        state->delay = 0;
    } else if (netem->delay_model == UNIFORM_DELAY) {
        int a = netem->delay.uniform.min;
        int b = netem->delay.uniform.max;
        long randlong = rand();        
        double rand = (double)randlong/RAND_MAX; /*  assume that rand always < 1  */
        state->delay = a + ( b + 1 - a ) * rand;
    } else if (netem->delay_model == GAMMA_DELAY) {
        state->delay = (int)gengam(1/(float)netem->delay.gamma.scale, netem->delay.gamma.shape);
    } else { /* TODO: Not yet implemented */
        state->delay = 0;
    }
    return 0;
}
