
#ifndef EPOLL_INTERNAL_H_INCLUDED_
#define EPOLL_INTERNAL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EVENT__HAVE_WEPOLL
/** Notification function, used to tell an event base to wake up from another
 * thread.  Only works when event_epoll_notify_base_() has previously been
 * called successfully on that base. */
int event_epoll_notify_base(struct event_base *base);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif