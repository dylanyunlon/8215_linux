#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <net/sock.h>
#include <net/net_namespace.h>

#define ATC_TAG_LOG "[ATC_STORAGE_HELPER]"

MODULE_LICENSE("GPL");

struct sock *nl_sk = NULL;
/*
*This Func try to send "mount" or "Unmount" information to Usrspace by using NETLINK  Method
*/
int atc_storage_sendMountInfo2Usrspace(const char *send_message)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;
    unsigned char *old_tail;
    int ret = 0;

    if (!nl_sk)
	return -1;

    nl_skb = alloc_skb(NLMSG_SPACE(strlen(send_message)),GFP_KERNEL);
    if(!nl_skb)
	printk("%s netlink:alloc_skb fail.\n", ATC_TAG_LOG);

    nlh = nlmsg_put(nl_skb,0,0,0,NLMSG_SPACE(strlen(send_message))-sizeof(struct nlmsghdr),0);
    old_tail = nl_skb->tail;
    memcpy(NLMSG_DATA(nlh),send_message,strlen(send_message));
    nlh->nlmsg_len = nl_skb->tail - old_tail;
    NETLINK_CB(nl_skb).portid = 0;
    NETLINK_CB(nl_skb).dst_group = 1;

    ret = netlink_broadcast(nl_sk,nl_skb,0,1,GFP_KERNEL);
    return ret;
}
EXPORT_SYMBOL(atc_storage_sendMountInfo2Usrspace);

static int __init atc_storage_sendMountInfo2Usrspace_init(void)
{
    struct netlink_kernel_cfg cfg = {
	.groups =1,
 	.flags = NL_CFG_F_NONROOT_RECV,
	};
    
    nl_sk = netlink_kernel_create(&init_net,NETLINK_ATCSTORAGE_HOTPLUG,&cfg);
    if(!nl_sk)
	printk("%s create kernel sockt failed!.\n", ATC_TAG_LOG);

    return 0;
}

static void __exit atc_storage_sendMountInfo2Usrspace_exit(void)
{
    netlink_kernel_release(nl_sk);
    return;
}

module_init(atc_storage_sendMountInfo2Usrspace_init);
module_exit(atc_storage_sendMountInfo2Usrspace_exit);
