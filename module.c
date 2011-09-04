
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
#include <linux/reboot.h>

#define MODULE_NAME "stonith"

static const struct in6_addr local_in6addr_any = IN6ADDR_ANY_INIT;

static struct {
    struct task_struct *thread;
    int running;
    struct socket *sock;
    struct sockaddr_in6 binding;
} listener;

static int
stonith_receive( unsigned char *buffer, int size ) {
    struct msghdr message;
    struct iovec iov;
    int bytes;
    mm_segment_t fs;

    iov.iov_base = buffer;
    iov.iov_len = size;

    message.msg_flags = MSG_DONTWAIT;
    message.msg_name = &listener.binding;
    message.msg_namelen = sizeof listener.binding;
    message.msg_control = NULL;
    message.msg_controllen = 0;
    message.msg_iov = &iov;
    message.msg_iovlen = 1;

    fs = get_fs();
    set_fs( KERNEL_DS );
    bytes = sock_recvmsg( listener.sock, &message, size, message.msg_flags );
    set_fs( fs );

    return bytes;
}

static void stonith_run(void) {
    unsigned char buffer[1500];
    int bytes;

    lock_kernel();
    listener.running = 1;
    current->flags |= PF_NOFREEZE;
    daemonize( MODULE_NAME );
    allow_signal( SIGKILL );
    unlock_kernel();

    for (;;) {
	bytes = stonith_receive( buffer, sizeof buffer );
	if ( bytes > 0 ) {
	    buffer[bytes] = '\0';
            printk( KERN_INFO MODULE_NAME": received '%s'\n", buffer );
	    if ( strncmp(buffer, "poweroff", 8) == 0 ) {
                printk( KERN_INFO MODULE_NAME": Powering Off\n" );
	        emergency_restart();
	    }
	}
	if ( signal_pending(current) )  break;
        /* what */
	msleep( 500 );
    }

    sock_release( listener.sock );
    printk( KERN_INFO MODULE_NAME": thread terminating\n" );

    listener.running = 0;
    mb();
}

static int __init stonith_init(void) {
    int err;

    memset( &listener, 0, sizeof listener );

    err = sock_create( AF_INET6, SOCK_DGRAM, IPPROTO_UDP, &listener.sock );
    if ( err < 0 ) {
        printk( KERN_INFO MODULE_NAME": failed to create socket\n" );
        return -ENOMEM;
    }
    listener.binding.sin6_family = AF_INET6;
    listener.binding.sin6_flowinfo = 0;
    listener.binding.sin6_addr = local_in6addr_any;
    listener.binding.sin6_port = htons(1337);

    err = listener.sock->ops->bind( listener.sock, (struct sockaddr *)&listener.binding, sizeof(struct sockaddr_in6) );
    if ( err < 0 ) {
        printk( KERN_INFO MODULE_NAME": bind failed\n" );
        return -EOPNOTSUPP;
    }

    listener.thread = kthread_run( (void *)stonith_run, NULL, MODULE_NAME );
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
