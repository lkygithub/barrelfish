/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */
#ifndef DESCQ_H_
#define DESCQ_H_ 1


#include <devif/queue_interface_backend.h>
#include <barrelfish/notificator.h>
#include <barrelfish/barrelfish.h>

#define DESCQ_DEFAULT_SIZE 2048
#define DESCQ_ALIGNMENT 64

struct descq;

typedef errval_t (*descq_create_t) (struct descq *q, bool notifications, uint8_t role, uint64_t *queue_id);
typedef errval_t (*descq_destroy_t) (struct descq *q);
typedef errval_t (*descq_notify_t) (struct descq *q);
typedef errval_t (*descq_register_t)(struct descq *q, struct capref cap,
                                    regionid_t region_id);
typedef errval_t (*descq_deregister_t)(struct descq *q, regionid_t region_id);
typedef errval_t (*descq_control_t)(struct descq *q,
                                   uint64_t request,
                                   uint64_t value,
                                   uint64_t *result);
typedef errval_t (*descq_enqueued_t)(struct descq* q);

struct descq_func_pointer {
    descq_create_t create;
    descq_destroy_t destroy;
    descq_notify_t notify;
    descq_register_t reg;
    descq_deregister_t dereg;
    descq_control_t control;
};

struct __attribute__((aligned(DESCQ_ALIGNMENT))) desc {
    genoffset_t offset; // 8
    genoffset_t length; // 16
    genoffset_t valid_data; // 24
    genoffset_t valid_length; // 32
    uint64_t flags; // 40
    uint64_t seq; // 48
    regionid_t rid; // 52
    uint8_t pad[12];
};

union __attribute__((aligned(DESCQ_ALIGNMENT))) pointer {
    volatile size_t value;
    uint8_t pad[64];
};

struct descq {
    struct devq q;
    struct descq_func_pointer f;

    // General info
    size_t slots;
    char* name;
    bool bound_done;
 
    // Descriptor Ring
    struct desc* rx_descs;
    struct desc* tx_descs;
    
    // Flow control
    uint64_t rx_seq;
    uint64_t tx_seq;
    union pointer* rx_seq_ack;
    union pointer* tx_seq_ack;
  
    // Flounder
    struct descq_binding* binding;
    bool local_bind;
    uint64_t resend_args;
  
    // linked list
    struct descq* next;
    uint64_t qid;
    
    struct notificator notificator;
    bool notifications;
};

struct descq_endpoint_state {
    bool exp_done;
    char* name;
    struct descq_func_pointer f;
    struct descq* head;
    struct descq* tail;
    uint64_t qid;
};

/**
 * @brief initialized a descriptor queue
 *
 * @param q                     Return pointer to the descriptor queue
 * @param slots                 Number of slots in the queue
 * @param name                  Name of the exported/connected to channel
 * @param exp                   Export desq_ctrl/descq_data flounder interface
 *                              (At least one of the sides of the channel hast to do so)
 * @param f                     Function pointers to be called on message recv
 *
 * @returns error on failure or SYS_ERR_OK on success
 */
errval_t descq_create(struct descq** q,
                      size_t slots,
                      char* name,
                      bool exp,
                      bool notifications,
                      uint8_t role,
                      uint64_t *queue_id,
                      struct descq_func_pointer* f);


#endif /* DESCQ_H_ */
