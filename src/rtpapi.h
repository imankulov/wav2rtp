/*
 * $Id$
 * 
 * Copyright (c) 2007, R.Imankulov
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the R.Imankulov nor the names of its contributors may
 *  be used to endorse or promote products derived from this software without
 *  specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef __RTPAPI_H
#define __RTPAPI_H
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
#include "error_types.h"
#include "options.h"
#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#define MAX_OBSERVERS 256


/** @defgroup rtp_packet RTP packets API
 *  This group describes RTP packets datastructure and API to use it
 *  @{
 */

#pragma pack(1)
/**
 * RTP header given from ortp 
 */
typedef struct __wr_rtp_header
{
#ifdef WORDS_BIGENDIAN
	uint16_t version:2;
	uint16_t padbit:1;
	uint16_t extbit:1;
	uint16_t cc:4;
	uint16_t markbit:1;
	uint16_t paytype:7;
#else
	uint16_t cc:4;
	uint16_t extbit:1;
	uint16_t padbit:1;
	uint16_t version:2;
	uint16_t paytype:7;
	uint16_t markbit:1;
#endif
	uint16_t seq_number;
	uint32_t timestamp;
	uint32_t ssrc;
	uint32_t csrc[16];
} wr_rtp_header_t;
#pragma pack()



/**
 * RTP data packet which contains link to the set of rtp data frames 
 */
typedef struct __wr_rtp_packet {
    int payload_type;                   /**< payload type of the packet */
    int sequence_number;                /**< sequence number */
    int markbit;                        /**< markbit is set to on/off */
    struct timeval lowlevel_timestamp;  /**< low-level (physical) timestamp, i.e. timestamp when packet actually received */
    uint32_t rtp_timestamp;             /**< RTP packet timestamp */
    list_t data_frames;                 /**< list of data frames (wr_data_frame_t objects ) */
} wr_rtp_packet_t; 

/** 
 * Data frame object
 * Note that you may use more than one wr_data_frame_t in RTP packet
 */
typedef struct __wr_data_frame {
    int length_in_ms;              /**< size of data in ms */
    size_t size;                   /**< size of data */
    uint8_t * data;                /**< pointer to the data */
} wr_data_frame_t;


/** 
 * initialize rtp packet
 * @return 0 if all OK, 1 oherwise
 */
int wr_rtp_packet_init(wr_rtp_packet_t * rtp_packet, int payload_type, int sequence_number, int markbit, uint32_t rtp_timestamp, struct timeval lowlevel_timestamp);

/** destroy rtp packet */
void wr_rtp_packet_destroy(wr_rtp_packet_t * rtp_packet);

/**
 * Copy existing rtp packet metadata ("data" member of the new packet picks to the same place as "data" of the old
 * packet).
 * This packet should not be destroyed via #wr_rtp_packet_destroy
 */
int wr_rtp_packet_copy(wr_rtp_packet_t * new_packet, wr_rtp_packet_t * old_packet);

/**
 * Copy existing rtp packet metadata and data ("data" member of the new packet is copied from the "data" of the old packet).
 * This packet should be destroyed via #wr_rtp_packet_destroy
 */
int wr_rtp_packet_copy_with_data(wr_rtp_packet_t * new_packet, wr_rtp_packet_t * old_packet);

/** 
 * add data frame to the packet
 * This create new data frame and add this frame to rtp packet
 */
wr_errorcode_t wr_rtp_packet_add_frame(wr_rtp_packet_t * packet, uint8_t * data, size_t size, int length_in_ms);


/** 
 * remove data from rtp packet at selected position
 * if position == -1 then last element will be removed from rtp
 */
int wr_rtp_packet_delete_frame(wr_rtp_packet_t * packet, int position);

/** @} */

/** @defgroup rtp_filter RTP filters API
 *  This group describes RTP filter datastructure and API for it
 *
 *  Each filter have to implement at least one method (wr_rtp_filter_t#notify). 
 *  There is three event types (see #wr_event_type_t):
 *    - transmission started
 *    - new packet transmitted
 *    - transmision ended
 *  
 *  If event type is NEW_PACKET then third parameter is set to pointer to the #wr_rtp_packet_t object
 *
 *  Intermediate filter (i.e. loss emulator) should copy its input RTP packet, then alter (if to needs to do it) *copyed* object and 
 *  invoke "notify_observers" with this object.
 *
 *  Filter cannot assume that link to packet which it receive via notify rest unchanged or not be removed at all.
 *
 *  @{
 */

typedef enum __wr_event_type {
    TRANSMISSION_START, /**< transmission of packets is started, observer should prepare itself */
    NEW_PACKET,         /**< new rtp packet is sent */
    TRANSMISSION_END,   /**< transmission of packets is stopped, observer should execute all actions to finalize */
} wr_event_type_t;



/**
 * RTP filter
 * Filter implements two interfaces:
 * - observer. This means that each rtp filter may be connected to other filter and receive from this filter events
 *    "rtp packet sended"
 * - subject. Subject is an object which can generate rtp packets itself and can have observers which reveives this
 *    data
 * When filter need to store its state between notify it should use internal wr_rtp_filter_t#state variable.
 * This variable have to be initialized when TRANSMISSION_START is invoked and uninitialized when TRANSMISSION_END is
 * invoked
 *
 */
typedef struct __wr_rtp_filter {
    /** id */
    char name[256];

    /** subject interface */
    struct __wr_rtp_filter * observers[MAX_OBSERVERS];

    /** observer interface */
    wr_errorcode_t (*notify)(struct __wr_rtp_filter * filter, wr_event_type_t event, wr_rtp_packet_t * packet);

    /** internal state of the filter */
    void * state;

} wr_rtp_filter_t;



/** 
 * Create RTP filter 
 * @param name some descriptive name
 * @param notify part of observer interface, invoked when object is notified
 */
void wr_rtp_filter_create(wr_rtp_filter_t * filter, char *name, 
        wr_errorcode_t (*notify)(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet) 
    );

/**
 * Append observer to the filter
 */
void wr_rtp_filter_append_observer(wr_rtp_filter_t * filter, wr_rtp_filter_t * observer);

/**
 * Notify all observers that new RTP data frame was transmitted
 */
void wr_rtp_filter_notify_observers(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet);

/**
 * Generic notify callback function (do nothing)
 * Used for example for filters which have not input subject filters (such as input wavfile filter)
 */
wr_errorcode_t wr_do_nothing_on_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet);

/** @} */

#endif
