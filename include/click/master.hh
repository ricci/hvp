// -*- c-basic-offset: 4; related-file-name: "../../lib/master.cc" -*-
#ifndef CLICK_MASTER_HH
#define CLICK_MASTER_HH
#include <click/vector.hh>
#include <click/timer.hh>
#include <click/task.hh>
#include <click/sync.hh>
#include <click/atomic.hh>
#if CLICK_USERLEVEL
# include <unistd.h>
# if HAVE_POLL_H
#  include <poll.h>
# endif
#endif
#if CLICK_NS
# include <click/simclick.h>
#endif
CLICK_DECLS
class Element;

#define CLICK_DEBUG_MASTER 0

class Master { public:

    Master(int nthreads);
    ~Master();

    void use();
    void unuse();

    int nthreads() const			{ return _threads.size() - 1; }
    RouterThread *thread(int id) const		{ return _threads[id + 1]; }

    const volatile int *runcount_ptr() const	{ return &_runcount; }
    
    int timer_delay(struct timeval *);
    void run_timers();
    
#if CLICK_USERLEVEL
    int add_select(int fd, Element *, int mask);
    int remove_select(int fd, Element *, int mask);
    void run_selects(bool more_tasks);
#endif

    void remove_router(Router *);
    
#if CLICK_NS
    void initialize_ns(simclick_sim, simclick_click);
    simclick_sim siminst() const		{ return _siminst; }
    simclick_click clickinst() const		{ return _clickinst; }
#endif

#if CLICK_DEBUG_MASTER
    String info() const;
#endif

  private:

    Spinlock _master_lock;
    volatile int _master_paused;
    Router *_routers;
    int _refcount;

    Vector<RouterThread *> _threads;

    Spinlock _runcount_lock;
    volatile int _runcount;
    
    Task _task_list;
    SpinlockIRQ _task_lock;
    
    Timer _timer_list;
    Spinlock _timer_lock;
    
#if CLICK_USERLEVEL
# if !HAVE_POLL_H
    struct pollfd {
	int fd;
	int events;
    };
    fd_set _read_select_fd_set;
    fd_set _write_select_fd_set;
    int _max_select_fd;
# endif
    Vector<struct pollfd> _pollfds;
    Vector<Element *> _read_poll_elements;
    Vector<Element *> _write_poll_elements;
    Spinlock _select_lock;
#endif

#if CLICK_NS
    simclick_sim _siminst;
    simclick_click _clickinst;
#endif
    
    Master(const Master &);
    Master &operator=(const Master &);
    
    void register_router(Router *);
    void run_router(Router *);

    void remove_pollfd(int);

    bool check_driver();
    void process_pending(RouterThread *);
    
    friend class Task;
    friend class Timer;
    friend class RouterThread;
    friend class Router;
    
};

CLICK_ENDDECLS
#endif
