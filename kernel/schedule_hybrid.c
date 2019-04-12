/**
 * \file
 * \brief Kernel realtime round-robin hybrid scheduling policy
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

/**
 * \brief Scheduler policy.
 *
 * \return Next DCB to schedule or NULL if wait for interrupts.
 */

struct dcb *schedule(void)
{

    systime_t now = systime_now();

    if (kcb_current->rr_counter == 0)
    {
        //rt mode
        struct dcb *dcb = kcb_current->ringfifo_current_rt;
        if (dcb == NULL)
            return NULL;
        // assume that there are at least 2 elements in the rt queue.
        assert(dcb->next != dcb);
        systime_t t_delta = dcb->next->stime - now;
        if (dcb->task_id < 0)
        {
            // rr task interval
            // whats the dimension of t_delta? ms?
            // the dimension of CONFIG_TIMESLICE is supposed to be ms, and is currently set to 1ms.
            kcb_current->rr_counter = t_delta / CONFIG_TIMESLICE + 1;
            kcb_current->t_last_timeslice = t_delta % CONFIG_TIMESLICE;
            kcb_current->ringfifo_current_rt = kcb_current->ringfifo_current_rt->next;
            return schedule();
        }
        else
        {
            // rt task
            dcb->interval = t_delta; 
#ifdef CONFIG_ONESHOT_TIMER
            update_sched_timer(kernel_now + 
                    ns_to_systime(t_delta * 1000000));
#endif
            kcb_current->ringfifo_current_rt = kcb_current->ringfifo_current_rt->next;
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

        if (kcb_current->rr_counter == 0) {
            if (kcb_current->t_last_timeslice != 0) {
                dcb->interval = kcb_current->t_last_timeslice;
#ifdef CONFIG_ONESHOT_TIMER
                update_sched_timer(kernel_now +
                        ns_to_systime(kcb_current->t_last_timeslice * 1000000));
#endif
            } else {
                // the last timeslice is 0, should reschedule at once.
                return schedule();
            }
        } else {
            dcb->interval = CONFIG_TIMESLICE;
#ifdef CONFIG_ONESHOT_TIMER
            update_sched_timer(kernel_now + kernel_timeslice);
#endif
        }
        kcb_current->ring_current = kcb_current->ring_current->next;
        return dcb;
    }

    return kcb_current->ring_current;
}

void schedule_now(struct dcb *dcb)
{
    // No-Op in Hybrid scheduler
}

void make_runnable(struct dcb *dcb)
{
    if (dcb->task_id != 0)
    {
        // rr interval or rt task
        if (kcb_current->ringfifo_head_rt == NULL)
        {
            dcb->prev = dcb;
            dcb->next = dcb;
            kcb_current->ringfifo_head_rt = dcb;
            kcb_current->ringfifo_current_rt = dcb;
        }
        else
        {
            dcb->prev = kcb_current->ringfifo_head_rt->prev;
            dcb->next = kcb_current->ringfifo_head_rt;
            kcb_current->ringfifo_head_rt->prev->next = dcb;
            kcb_current->ringfifo_head_rt->prev = dcb;
        }
    }
    else
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
    if (dcb->task_id != 0)
    {
        // rr interval or rt task
        assert(kcb_current->ringfifo_head_rt != NULL);
        struct dcb *tmp = kcb_current->ringfifo_head_rt;
        do
        {
            if (tmp == dcb)
            {
                //remove it
                if (tmp->next == tmp)
                {
                    // last in queue
                    tmp->next = NULL;
                    tmp->prev = NULL;
                    kcb_current->ringfifo_head_rt = NULL;
                    kcb_current->ringfifo_current_rt = NULL;
                }
                else
                {
                    if (tmp == kcb_current->ringfifo_head_rt)
                        kcb_current->ringfifo_head_rt = tmp->next;
                    if (tmp == kcb_current->ringfifo_current_rt)
                        kcb_current->ringfifo_current_rt = tmp->next;
                    tmp->next->prev = tmp->prev;
                    tmp->prev->next = tmp->next;
                    tmp->next = NULL;
                    tmp->prev = NULL;
                }
            }
            tmp = tmp->next;
        } while (tmp != kcb_current->ringfifo_head_rt);
        // not in queue, no-op.
    }
    else
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
