/* 
 * MIT License
 *
 * Copyright (c) 2019 极简美 @ konishi5202@163.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h>

#include "mt_timer.h"

int timer_init(MT_TIMER_OBJECT *object, void *(*thread_handler)(void *arg), int max_num)
{
    struct epoll_event event;
    assert(NULL != object);
    if(true == object->timer_active_flag)
        return 0;

    object->timer_event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if(-1 == object->timer_event_fd)
    {
        MT_TIMER_PRINT_INL("MT-Timer error: eventfd() failed, %s!\n", strerror(errno));
        return -1;
    }
    object->timer_max = max_num;
    object->timer_active_flag = true;
    object->timer_epoll_fd = epoll_create(max_num);
    if(object->timer_epoll_fd < 0)
    {
        MT_TIMER_PRINT_INL("MT-Timer error: epoll_create failed %s.\n", strerror(errno));
        close(object->timer_event_fd);
        object->timer_event_fd = -1;
        return -1;
    }

    event.data.ptr = NULL;
    event.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(object->timer_epoll_fd, EPOLL_CTL_ADD, object->timer_event_fd, &event) < 0)
    {
        fprintf(stderr, "epoll: epoll_ctl ADD failed, %s\n", strerror(errno));
        close(object->timer_epoll_fd);
        close(object->timer_event_fd);
        object->timer_epoll_fd = -1;
        object->timer_event_fd = -1;
        return -1;
    }

    if(pthread_create(&object->timer_thread_id, NULL, thread_handler, NULL) != 0)
    {
        MT_TIMER_PRINT_INL("MT-Timer error: pthread_create failed %s.\n", strerror(errno));
        close(object->timer_epoll_fd);
        close(object->timer_event_fd);
        object->timer_epoll_fd = -1;
        object->timer_event_fd = -1;
        return -1;
    }
    
    return 0;
}

void timer_deinit(MT_TIMER_OBJECT *object)
{
    uint64_t quit = 0x51;
    assert(NULL != object);
    if(false == object->timer_active_flag)
        return ;
    object->timer_active_flag = false;
    usleep(100);
    write(object->timer_event_fd, &quit, sizeof(uint64_t)); /* wakeup thread */
    pthread_join(object->timer_thread_id, NULL);
    timer_clear(object);
    close(object->timer_epoll_fd);
    close(object->timer_event_fd);
    object->timer_epoll_fd = -1;
    object->timer_event_fd = -1;
}

int timer_add(MT_TIMER_OBJECT *object, struct itimerspec *itimespec,
                int repeat, timer_callback_t cb, void *data, timer_release_t rb)
{
    uint64_t quit = 0x51;
    struct epoll_event event;
    MT_TIMER_NODE *handler = NULL;
    assert(NULL != object);
    handler = (MT_TIMER_NODE *)malloc(sizeof(MT_TIMER_NODE));
    if(NULL == handler)
    {
        MT_TIMER_PRINT_INL("MT-Timer error: malloc failed %s.\n", strerror(errno));
        return -1;
    }
    
    handler->timer_cb = cb;
    handler->release_cb = rb;
    handler->timer_cnt = repeat;
    handler->timer_data = data;
    handler->timer_fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC|TFD_NONBLOCK);
    if(handler->timer_fd < 0)
    {
        MT_TIMER_PRINT_INL("MT-Timer error: timerfd_create failed %s.\n", strerror(errno));
        free(handler);
        return -2;
    }
    if(timerfd_settime(handler->timer_fd, 0, itimespec, NULL) == -1)
    {
        MT_TIMER_PRINT_INL("MT-Timer error: timerfd_settime failed %s.\n", strerror(errno));
        close(handler->timer_fd);
        free(handler);
        return -3;
    }

    event.events = EPOLLIN | EPOLLET;
    event.data.ptr = handler;
    if(epoll_ctl(object->timer_epoll_fd, EPOLL_CTL_ADD, handler->timer_fd, &event) < 0)
    {
        MT_TIMER_PRINT_INL("MT-Timer error: epoll_ctl ADD failed %s.\n", strerror(errno));
        close(handler->timer_fd);
        free(handler);
        return -4;
    }
    write(object->timer_event_fd, &quit, sizeof(uint64_t)); /* wakeup thread */
    pthread_rwlock_wrlock(&object->timer_rwlock);
    HASH_ADD_INT(object->timer_head, timer_fd, handler);
    pthread_rwlock_unlock(&object->timer_rwlock);
    
    return handler->timer_fd;
}

int timer_del(MT_TIMER_OBJECT *object, int timerfd)
{
    struct epoll_event event;
    MT_TIMER_NODE *handler = NULL;
    assert(NULL != object);
    pthread_rwlock_rdlock(&object->timer_rwlock);
    HASH_FIND_INT(object->timer_head, &timerfd, handler);
    pthread_rwlock_unlock(&object->timer_rwlock);
    if(NULL == handler)
        return 0;
    
    event.data.ptr = (void *)handler;
    event.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(object->timer_epoll_fd, EPOLL_CTL_DEL, handler->timer_fd, &event) < 0)
    {
        MT_TIMER_PRINT_INL("MT-Timer error: epoll_ctl DEL failed %s.\n", strerror(errno));
        return -1;
    }
    
    pthread_rwlock_wrlock(&object->timer_rwlock);
    HASH_DEL(object->timer_head, handler);
    pthread_rwlock_unlock(&object->timer_rwlock);
    close(handler->timer_fd);
    if(handler->release_cb)
        handler->release_cb(handler->timer_data);
    free(handler);
    
    return 0;
}

int timer_count(MT_TIMER_OBJECT *object)
{
    assert(NULL != object);
    return HASH_COUNT(object->timer_head);
}

int timer_clear(MT_TIMER_OBJECT *object)
{
    struct epoll_event event;
    MT_TIMER_NODE *handler = NULL;
    assert(NULL != object);
    event.events = EPOLLIN | EPOLLET;
    for(handler = object->timer_head; handler != NULL; handler = handler->hh.next)
    {
        event.data.ptr = (void *)handler;
        if(epoll_ctl(object->timer_epoll_fd, EPOLL_CTL_DEL, handler->timer_fd, &event) < 0)
        {
            MT_TIMER_PRINT_INL("MT-Timer error: epoll_ctl CLEAR failed %s.\n", strerror(errno));
            return -1;
        }
        pthread_rwlock_wrlock(&object->timer_rwlock);
        HASH_DEL(object->timer_head, handler);
        pthread_rwlock_unlock(&object->timer_rwlock);
        close(handler->timer_fd);
        if(handler->release_cb)
            handler->release_cb(handler->timer_data);
        free(handler);
    }
    return 0;
}

TIMER_CREATE(uart_timer);
int uart_timer_id = 0; 
int uart_simple_timer_start(int ms,timer_callback_t userCallback,void *arg)
{
    struct itimerspec itimespec = {0};
    TIMER_INIT(uart_timer, 1);
    itimespec.it_value.tv_nsec = ms*1000000;
    uart_timer_id = TIMER_ADD(uart_timer, &itimespec, 1, userCallback, arg, NULL);
    return uart_timer_id;
}

void uart_simple_timer_end(void)
{
    TIMER_DEL(uart_timer, uart_timer_id);
    TIMER_CLEAR(uart_timer);
    TIMER_DEINIT(uart_timer);
}

