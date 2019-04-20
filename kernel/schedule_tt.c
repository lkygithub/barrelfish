/**
 * \file
 * \brief Kernel round-robin tt mixed scheduling policy
 */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2013, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <kernel.h>
#include <dispatch.h>
#include <kcb.h>

#include <timer.h> // update_sched_timer
#include <systime.h>

#define TT_THRESHOLD (50 * 1000) //ns. decides whether the task should be scheduled.
#define ABS_SUB(a, b) ((a > b) ? (a - b) : (b - a))
/**
 * \brief Scheduler policy.
 *
 * \return Next DCB to schedule or NULL if wait for interrupts.
 */

unsigned int prev_sched_index(void) {

    if (kcb_current->current_task == 0) return kcb_current->n_sched - 2;
    else return kcb_current->current_task - 1;
}

struct dcb *schedule(void)
{
    systime_t now = systime_now();
    if (!kcb_current->tt_started)
    {
        //tt not up, fall back to rr.    
        if(kcb_current->ring_current == NULL) return NULL;
        struct dcb *dcb = kcb_current->ring_current;
#ifdef CONFIG_ONESHOT_TIMER
        update_sched_timer(kernel_now + kernel_timeslice);
#endif
        kcb_current->ring_current = kcb_current->ring_current->next;
        dcb->interval = ns_to_systime(CONFIG_TIMESLICE * 1000000);
        return dcb;
    }
    assert(kcb_current->n_sched > 1);
    if (!kcb_current->t_base) kcb_current->t_base = now;
    if (kcb_current->rr_counter == 0)
    {
        //tt mode
        if (kcb_current->current_task == kcb_current->n_sched - 1) {
            //The last task in the sched queue is not a valid task.
            //It serves calc of interval of the last - 1 task.
            //Thus current_task should be reset to 0.
            kcb_current->current_task = 0;
        }
        if (ABS_SUB(kcb_current->sched_tbl[kcb_current->current_task].tstart + kcb_current->t_base, now) 
                > ns_to_systime(TT_THRESHOLD)) {
            //Schedule called from dispatcher exit.
            unsigned int i = prev_sched_index();
            assert(kcb_current->sched_tbl[kcb_current->current_task].tstart + kcb_current->t_base > now);
            assert(kcb_current->sched_tbl[i].tstart + kcb_current->t_base < now);
            kcb_current->sched_tbl[i].dcb->etime += now - kcb_current->t_base - 
                    kcb_current->sched_tbl[i].tstart;
            return NULL;
        }
        systime_t t_delta = kcb_current->sched_tbl[kcb_current->current_task + 1].tstart -
                kcb_current->sched_tbl[kcb_current->current_task].tstart;
        struct dcb *dcb = kcb_current->sched_tbl[kcb_current->current_task].dcb;

        if (dcb == NULL)
        {
            // rr task interval
            // the dimension of t_delta is supposed to be us
            // the dimension of CONFIG_TIMESLICE is supposed to be ms, and is currently set to 1ms.
            kcb_current->rr_counter = t_delta / ns_to_systime(CONFIG_TIMESLICE * 1000000) + 1;
            kcb_current->last_timeslice = t_delta % ns_to_systime(CONFIG_TIMESLICE * 1000000);
            kcb_current->current_task++;
            return schedule();
        }
        else
        {
            // tt task
            unsigned int i = prev_sched_index();
            dcb->interval = t_delta;
            kcb_current->sched_tbl[i].dcb->etime += kcb_current->sched_tbl[i].dcb->interval;
#ifdef CONFIG_ONESHOT_TIMER
            update_sched_timer(kernel_now + t_delta);
#endif
            kcb_current->current_task++;
            return dcb;
        }
    }
    else
    {
        // rr mode
        kcb_current->rr_counter--;
        struct dcb *dcb = kcb_current->ring_current;
        if (dcb == NULL)
            return NULL;

        if (kcb_current->rr_counter == 0)
        {
            if (kcb_current->last_timeslice != 0)
            {
                dcb->interval = kcb_current->last_timeslice;
#ifdef CONFIG_ONESHOT_TIMER
                update_sched_timer(kernel_now + kcb_current->t_last_timeslice);
#endif
            }
            else
            {
                // the last timeslice is 0, should reschedule at once.
                return schedule();
            }
        }
        else
        {
            dcb->interval = ns_to_systime(CONFIG_TIMESLICE * 1000000);
#ifdef CONFIG_ONESHOT_TIMER
            update_sched_timer(kernel_now + kernel_timeslice);
#endif
        }
        kcb_current->ring_current = kcb_current->ring_current->next;
        return dcb;
    }
}

void schedule_now(struct dcb *dcb)
{
    // No-Op in tt scheduler
}

struct dcb* insert_into_hash_tbl(struct dcb *dcb) {
    // No-op for rr interval;
    if (dcb->task_id < 0) return NULL;
    if (dcb->task_id > 0) {
        int index = dcb->task_id % N_BUCKETS;
        if (kcb_current->hash_tbl[index] == NULL) {
            kcb_current->hash_tbl[index] = dcb;
        } else {
            struct dcb *tmp = kcb_current->hash_tbl[index];
            do {
                if (tmp->task_id == dcb->task_id) return tmp;
            } while(tmp->next != NULL);
            tmp->next = dcb;
            dcb->prev = tmp;
        }
    }
    return dcb;
}

void insert_into_sched_tbl(struct dcb *dcb, systime_t tstart) {
    if (tstart < 0) {
        kcb_current->tt_started = true;
        tstart = -tstart;
    }
    kcb_current->sched_tbl[kcb_current->n_sched].dcb = dcb;
    kcb_current->sched_tbl[kcb_current->n_sched].tstart = tstart;
    kcb_current->n_sched++;
}

void make_runnable(struct dcb *dcb)
{
    if (dcb->task_id == 0)
    {
        // rr task
        // Insert into schedule ring if not in there already
        // This is needed because rr tasks are never removed from the schedule ring.
        if (dcb->prev == NULL && dcb->next == NULL)
        {
            // Ring empty
            if (kcb_current->ring_current == NULL)
            {
                kcb_current->ring_current = dcb;
                dcb->next = dcb;
            }

            // Insert after current ring position
            dcb->prev = kcb_current->ring_current;
            dcb->next = kcb_current->ring_current->next;
            kcb_current->ring_current->next->prev = dcb;
            kcb_current->ring_current->next = dcb;
        }
    }
    // No-op for tt task and rr interval.
}

/**
 * \brief Remove 'dcb' from scheduler ring.
 *
 * Removes dispatcher 'dcb' from the scheduler ring. If it was not in
 * the ring, this function is a no-op. The postcondition for this
 * function is that dcb is not in the ring.
 *
 * \param dcb   Pointer to DCB to remove.
 */
void scheduler_remove(struct dcb *dcb)
{
    if (dcb->task_id == 0)
    {
        // rr task
        // No-op if not in scheduler ring
        if (dcb->prev == NULL && dcb->next == NULL)
            return;
        struct dcb *next = kcb_current->ring_current->next;

        // Remove dcb from scheduler ring
        dcb->prev->next = dcb->next;
        dcb->next->prev = dcb->prev;
        dcb->prev = dcb->next = NULL;

        // Removing ring_current
        if (dcb == kcb_current->ring_current)
        {
            if (dcb == next)
            {
                // Only guy in the ring
                kcb_current->ring_current = NULL;
            }
            else
            {
                // Advance ring_current
                kcb_current->ring_current = next;
            }
        }
    }
}

/**
 * \brief Yield 'dcb' for the rest of the current timeslice.
 *
 * Re-sorts 'dcb' into the scheduler queue with its release time increased by
 * the timeslice period. It is an error to yield a dispatcher not in the
 * scheduler queue.
 *
 * \param dcb   Pointer to DCB to remove.
 */
void scheduler_yield(struct dcb *dcb)
{
    if (dcb->prev == NULL || dcb->next == NULL)
    {
        struct dispatcher_shared_generic *dsg =
            get_dispatcher_shared_generic(dcb->disp);
        panic("Yield of %.*s not in scheduler queue", DISP_NAME_LEN,
              dsg->name);
    }

    // No-op for the round-robin scheduler
}

void scheduler_reset_time(void)
{
    // No-Op in RR scheduler
}

void scheduler_convert(void)
{
    enum sched_state from = kcb_current->sched;
    switch (from)
    {
    case SCHED_RBED:
    {
        // initialize RR ring
        struct dcb *last = NULL;
        for (struct dcb *i = kcb_current->queue_head; i; i = i->next)
        {
            i->prev = last;
            last = i;
        }
        // at this point: we have a dll, but need to close the ring
        kcb_current->queue_head->prev = kcb_current->queue_tail;
        kcb_current->queue_tail->next = kcb_current->queue_head;
        break;
    }
    case SCHED_RR:
        // do nothing
        break;
    default:
        printf("don't know how to convert %d to RBED state\n", from);
        break;
    }
    kcb_current->ring_current = kcb_current->queue_head;
    for (struct dcb *i = kcb_current->ring_current; i != kcb_current->ring_current; i = i->next)
    {
        printf("dcb %p\n  prev=%p\n  next=%p\n", i, i->prev, i->next);
    }
}

void scheduler_restore_state(void)
{
    // No-Op in RR scheduler
}
