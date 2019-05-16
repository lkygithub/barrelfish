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
#include <exec.h>

#include <timers.h>
#include <timer.h> // update_sched_timer
#include <systime.h>

#define TT_THRESHOLD 1000 //us. decides whether the task should be scheduled.
#define TT_DBG
/**
 * \brief Scheduler policy.
 *
 * \return Next DCB to schedule or NULL if wait for interrupts.
 */

#ifdef TT_DBG
static systime_t atime[1024];
static systime_t stime[1024];
static unsigned int tindex = 0;
static systime_t lognow[1024];
static systime_t lognext[1024];
static unsigned int lindex = 0;
#endif

unsigned int prev_sched_index(void)
{
    if (kcb_current->current_task == 0)
        return kcb_current->n_sched - 2;
    else
        return kcb_current->current_task - 1;
}

void switch_tt_flag(void)
{
    kcb_current->time_triggered = !kcb_current->time_triggered;
}

struct dcb *schedule(void)
{
    if (kcb_current->tt_status == 0)
    {
        // rr mode
        if (kcb_current->ring_current == NULL)
        {
            return NULL;
        }
        struct dcb *dcb = kcb_current->ring_current;
        kcb_current->ring_current = kcb_current->ring_current->next;
        if (!kcb_current->t_base)
        {
            // tt scheduler not started.
            timer_reset(CONFIG_TIMESLICE);
        }
        else
        {
            // tt scheduler running in rr mode.
            systime_t next_tstart = kcb_current->sched_tbl[kcb_current->current_task].tstart_shift + kcb_current->t_base;
            if (systime_now() + MS_TO_SYS_SCALE(CONFIG_TIMESLICE) >= next_tstart)
            {
                //assert(systime_now() < next_tstart);
                if (systime_now() >= next_tstart)
                {
#ifdef TT_DBG
                    if (lindex < 1024)
                    {
                        lognow[lindex] = systime_now();
                        lognext[lindex] = next_tstart;
                        lindex++;
                    } else {
                        printk(LOG_ERR, "over 1024 logs, flush.\n");
                        for (lindex = 0; lindex < 1024; lindex++) {
                            printk(LOG_ERR, "my checkp assertion failed, now/next_tstart:%ld/%ld.\n", lognow[lindex], lognext[lindex]);
                        }
                    }
#endif
                    //printk(LOG_ERR, "my checkp assertion failed, now/next_tstart:%ld/%ld.\n", systime_now(), next_tstart);
                }
                //last timeslice in rr mode
                kcb_current->tt_status = 1;
                timer_set(next_tstart);
            }
            else
            {
                timer_reset(CONFIG_TIMESLICE);
            }
        }
        return dcb;
    }
    else
    {
        // tt mode
        assert(kcb_current->n_sched >= 2);

        if (!kcb_current->time_triggered && kcb_current->t_base)
        {
            // make sure entered in timer interrupt. No-op otherwise.
            /*if (kcb_current->t_base + kcb_current->sched_tbl[kcb_current->current_task].tstart_shift <= systime_now()) {
                printk(LOG_ERR, "my checkp current_task, tbase, shift, now:%d, %ld, %ld, %ld.\n", kcb_current->current_task, kcb_current->t_base, kcb_current->sched_tbl[kcb_current->current_task].tstart_shift, systime_now());
            }*/
            systime_t next_tstart = kcb_current->t_base + kcb_current->sched_tbl[kcb_current->current_task].tstart_shift;
            assert(next_tstart > systime_now());
            timer_set(next_tstart);
            return NULL;
            //wait_for_interrupt();
        }

        if (kcb_current->current_task == kcb_current->n_sched - 1)
        {
            //The last task in the sched queue is not a valid task.
            //It serves calc of tstart of the first task in the next round.
            //Thus current_task should be reset to 0.
            kcb_current->current_task = 0;
        }

        // accumulate the last schduled task's execution time before reset tbase.
        unsigned int i = prev_sched_index();
        if (kcb_current->sched_tbl[i].dcb)
        {
            kcb_current->sched_tbl[i].dcb->etime += systime_now() - kcb_current->t_base - kcb_current->sched_tbl[i].tstart_shift;
        }

        if (kcb_current->current_task == 0)
        {
            //Should reset shift base when a round is finished.
            kcb_current->t_base = systime_now();
        }

        /*systime_t tstart = kcb_current->sched_tbl[kcb_current->current_task].tstart_shift + kcb_current->t_base;
        if (tstart + US_TO_SYS_SCALE(TT_THRESHOLD) < systime_now() ) {
            // Schedule failure, should reset all statistical values.
            for (i = 0; i < N_BUCKETS; i++) {
                for (struct dcb *tmp = kcb_current->hash_tbl[i]; tmp != NULL; tmp = tmp->next) {
                    tmp->etime = 0;
                    // Maybe some other statistical values should be reseted too.
                }
            }
            printk(LOG_ERR, "too late, tstart/now:%ld/%ld.\n", tstart, systime_now());
        } else if (systime_now() + US_TO_SYS_SCALE(TT_THRESHOLD) < tstart) {
            // Should wait for interrupt.
            printk(LOG_ERR, "too soon.\n");
            timer_set(tstart);
            return NULL;
            //wait_for_interrupt();
        }*/

        struct dcb *dcb = kcb_current->sched_tbl[kcb_current->current_task].dcb;
        if (dcb == NULL)
        {
            // rr task interval

#ifdef TT_DBG
            if (tindex < 1024)
            {
                //atime[tindex] = systime_now();
                //stime[tindex] = kcb_current->t_base + kcb_current->sched_tbl[kcb_current->current_task].tstart_shift;
                atime[tindex] = systime_now() - kcb_current->t_base;
                stime[tindex] = kcb_current->sched_tbl[kcb_current->current_task].tstart_shift;
                tindex++;
                if (tindex >= 1024)
                {
                    for (int tmpindex = 0; tmpindex < lindex; tmpindex++) {
                        printk(LOG_ERR, "my checkp assertion failed, now/next_tstart:%ld/%ld.\n", lognow[lindex], lognext[lindex]);
                    }
                    for (tindex = 0; tindex < 1024; tindex++)
                    {
                        printk(LOG_ERR, "my checkp atime/stime:%ld/%ld.\n", atime[tindex], stime[tindex]);
                    }
                    /*for (tindex = 0; tindex < 511; tindex++)
                    {
                        printk(LOG_ERR, "my checkp i/ainterval/sinterval:%ld/%ld/%ld.\n", tindex, atime[tindex + 1] - atime[tindex], stime[tindex + 1] - stime[tindex]);
                    }*/
                    tindex++;
                    //return to rr scheduler.
                    kcb_current->t_base = 0;
                    kcb_current->tt_status = 0;
                }
            }
#endif

            kcb_current->tt_status = 0;
            kcb_current->current_task++;
            return schedule();
        }
        else
        {
            // tt task

#ifdef TT_DBG
            if (tindex < 1024)
            {
                //atime[tindex] = systime_now();
                //stime[tindex] = kcb_current->t_base + kcb_current->sched_tbl[kcb_current->current_task].tstart_shift;
                atime[tindex] = systime_now() - kcb_current->t_base;
                stime[tindex] = kcb_current->sched_tbl[kcb_current->current_task].tstart_shift;
                tindex++;
                if (tindex >= 1024)
                {
                    for (int tmpindex = 0; tmpindex < lindex; tmpindex++) {
                        printk(LOG_ERR, "my checkp assertion failed, now/next_tstart:%ld/%ld.\n", lognow[lindex], lognext[lindex]);
                    }
                    for (tindex = 0; tindex < 1024; tindex++)
                    {
                        printk(LOG_ERR, "my checkp atime/stime:%ld/%ld.\n", atime[tindex], stime[tindex]);
                    }
                    /*for (tindex = 0; tindex < 511; tindex++)
                    {
                        printk(LOG_ERR, "my checkp i/ainterval/sinterval:%ld/%ld/%ld.\n", tindex, atime[tindex + 1] - atime[tindex], stime[tindex + 1] - stime[tindex]);
                    }*/
                    tindex++;
                    //return to rr scheduler.
                    kcb_current->t_base = 0;
                    kcb_current->tt_status = 0;
                }
            }
#endif
            timer_set(kcb_current->t_base + kcb_current->sched_tbl[kcb_current->current_task + 1].tstart_shift);
            kcb_current->current_task++;
            return dcb;
        }
    }
}

void schedule_now(struct dcb *dcb)
{
    // No-Op in tt scheduler
}

struct dcb *insert_into_hash_tbl(struct dcb *dcb)
{
    // No-op for rr interval;
    if (dcb->task_id < 0)
        return NULL;
    if (dcb->task_id > 0)
    {
        int index = dcb->task_id % N_BUCKETS;
        if (kcb_current->hash_tbl[index] == NULL)
        {
            kcb_current->hash_tbl[index] = dcb;
        }
        else
        {
            struct dcb *tmp = kcb_current->hash_tbl[index];
            do
            {
                if (tmp->task_id == dcb->task_id)
                    return tmp;
            } while (tmp->next != NULL);
            tmp->next = dcb;
            dcb->prev = tmp;
        }
    }
    return dcb;
}

void insert_into_sched_tbl(struct dcb *dcb, int64_t tstart_shift)
{
    kcb_current->sched_tbl[kcb_current->n_sched].dcb = dcb;
    kcb_current->sched_tbl[kcb_current->n_sched].tstart_shift = US_TO_SYS_SCALE(tstart_shift);
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
    /*if (dcb->prev == NULL || dcb->next == NULL)
    {
        struct dispatcher_shared_generic *dsg =
            get_dispatcher_shared_generic(dcb->disp);
        panic("Yield of %.*s not in scheduler queue", DISP_NAME_LEN,
              dsg->name);
    }*/

    // No-op for the tt scheduler
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
