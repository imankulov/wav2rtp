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
        int seed = iniparser_getint(wr_options.output_options, "network_emulator:random_seed", 0);
        if (seed == 0){
            long ltime = time(NULL);
            unsigned stime = (unsigned) ltime/2;
            srand(stime);
        } else {
            srand ((unsigned)seed);
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
            netem->loss.chained_int.data_frames = calloc(netem->loss.chained_int.chain_size, sizeof(list_t));
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
        if ( netem->loss.chained.__position == 0 ){ 
            double rand = (double) random() / RAND_MAX;
            netem->loss.chained.__is_lost = (rand < netem->loss.chained.loss_rate) ? 1 : 0;
        }
        state->lost = netem->loss.chained.__is_lost;
        netem->loss.chained.__position = (++netem->loss.chained.__position) % netem->loss.chained.chain_size;
        /* printf("%c", state->lost  ? '-' : '+'); */
    } else if (netem->loss_model == CHAINED_INT_LOSS ){
        
        state->lost = 0;
        
        if ( netem->loss.chained_int.__position == 0 ){ 
            double rand = (double) random() / RAND_MAX;
            netem->loss.chained_int.__is_lost = (rand < netem->loss.chained_int.loss_rate) ? 1 : 0;
        }
        /* if data packet isn't lost, then store its data */
        if ( !netem->loss.chained_int.__is_lost ){
            /* printf("+"); */
            list_t * data_frames = & (netem->loss.chained_int.data_frames[netem->loss.chained_int.__position]);

            /* list isn't empty, remove all prev. saved data */
            if (list_empty(data_frames) != 0){ 
                list_iterator_start(data_frames);
                while(list_iterator_hasnext(data_frames)){
                    wr_data_frame_t * tmp  = list_iterator_next(data_frames);
                    free(tmp->data);
                }
                list_iterator_stop(data_frames);
                list_destroy(data_frames);               
            }

            /* copy data ... */
            list_init(data_frames); 
            list_iterator_start(state->data_frames);
            while (list_iterator_hasnext(state->data_frames)){
                wr_data_frame_t * tmp  = list_iterator_next(state->data_frames);
                wr_data_frame_t * frame = calloc(1, sizeof(wr_data_frame_t));
                frame->data = calloc(tmp->size, sizeof(uint8_t));
                memcpy(frame->data, tmp->data, tmp->size);
                frame->length_in_ms = tmp->length_in_ms;
                frame->size = tmp->size;
                list_append(data_frames, frame);
            }
            list_iterator_stop(state->data_frames);

        /* if data packet is lost ... */
        } else {
            list_t * data_frames = & (netem->loss.chained_int.data_frames[netem->loss.chained_int.__position]);
            /* nothing to restore ? */
            if (list_empty(data_frames) == 0){
                /* printf("."); */
                state->lost = 1;
            /* restore data from previously saved data */
            } else {
                /* printf("-"); */
                list_iterator_start(state->data_frames);
                while(list_iterator_hasnext(state->data_frames)){
                    wr_data_frame_t * tmp  = list_iterator_next(state->data_frames);
                    free(tmp->data);
                }
                list_iterator_stop(state->data_frames);
                list_destroy(state->data_frames);
                list_init(state->data_frames); 
                list_iterator_start(data_frames);
                while (list_iterator_hasnext(data_frames)){
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
        }
        netem->loss.chained_int.__position = (++netem->loss.chained_int.__position) % netem->loss.chained_int.chain_size;
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
