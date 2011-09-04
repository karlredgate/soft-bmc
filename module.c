
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>

#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/in.h>
#include <linux/in6.h>

#include <linux/delay.h>

#define MODULE_NAME "stonith"

struct {
    struct task_struct *thread;
    int running;
    struct socket *sock;
    struct sockaddr_in address;
} listener;

static void run(void) {
    lock_kernel();
    listener.running = 1;
    current->flags |= PF_NOFREEZE;
    daemonize( MODULE_NAME );
    allow_signal( SIGKILL );
    unlock_kernel();

    for (;;) {
	if ( signal_pending(current) )  break;
        /* what */
	msleep( 500 );
    }
    printk( KERN_INFO MODULE_NAME": thread terminating\n" );

    listener.running = 0;
    mb();
}

static int __init stonith_init(void) {
    int err;

    err = sock_create( AF_INET6, SOCK_DGRAM, IPPROTO_UDP, &listener.sock );
    if ( err < 0 ) {
        printk( KERN_INFO MODULE_NAME": failed to create socket\n" );
        return -ENOMEM;
    }
    listener.address.sin_family = AF_INET6;
    listener.address.sin_addr.s_addr = htonl(INADDR_ANY);
    listener.address.sin_port = htons(1337);

    memset( &listener, 0, sizeof listener );
    listener.thread = kthread_run( (void *)run, NULL, MODULE_NAME );
    if ( IS_ERR(listener.thread) ) {
        printk( KERN_INFO MODULE_NAME": unable to start kernel thread\n" );
	return -ENOMEM;
    }
    printk( KERN_INFO MODULE_NAME": loaded\n" );
    return 0;
}

static void __exit stonith_exit(void) {
    if ( listener.thread == NULL ) {
        printk( KERN_INFO MODULE_NAME": no kernel thread to kill\n" );
    } else {
        lock_kernel();
	kill_proc( listener.thread->pid, SIGKILL, 1 );
	unlock_kernel();
        printk( KERN_INFO MODULE_NAME": waiting for thread to die\n" );
	while ( listener.running == 1 ) {
            msleep(10);
	    mb();
        }
    }

    printk( KERN_INFO MODULE_NAME": unloaded\n" );
}

module_init(stonith_init);
module_exit(stonith_exit);

MODULE_DESCRIPTION("kernel implementation of STONITH device");
MODULE_AUTHOR("Karl N. Redgate <Karl.Redgate@gmail.com>");
MODULE_LICENSE("GPL");

/*
 * vim:autoindent
 */
