
#include "atcftlcore.h"
#include "linux/hdreg.h"
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>
#include <linux/blkdev.h>

#define MTK_MAX_DEVICES 8

extern int mtklib_attach_mtd_dev(struct mtk_dev *mtk);

extern int mtklib_dettach_mtd_dev(struct mtk_dev *mtk);

extern int mtklib_suspend(struct mtk_dev *mtk);

extern int mtklib_resume(struct mtk_dev *mtk);

extern int mtklib_read(struct mtk_dev *mtk, unsigned long sector_start, unsigned int sector_num, int *retlen, char *buf);

extern int mtklib_write(struct mtk_dev *mtk, unsigned long sector_start, unsigned int sector_num, int *retlen, char *buf);

extern int mtklib_delete(struct mtk_dev *mtk, unsigned long sector_start, unsigned int sector_num);


static struct mutex mtks_lock;
static unsigned int mtd_dev_count = 0;
static struct mtk_dev *dev_list[MTK_MAX_DEVICES]={NULL};
/* Numbers of elements set in the @mtd_dev_param array */
static int __initdata mtd_devs;
/* MTD devices specification parameters */
static struct mtk_dev_param __initdata mtd_dev_param[MTK_MAX_DEVICES];
#define MTKLIB_PM
#ifdef MTKLIB_PM
static struct mutex pm_lock;
static int suspend_resume = 0; // 0 means resume, 1 means suspend
#endif

#define DEBUG 

/* erase_write: just write the data in buf to pos (nand)
 * mtk: mtk device the data should be written to
 * pos: the offset in bytes
 * len: the len of the buf
 * buf: data buffer
 *
 * just write the data to nand
 */
static int erase_write (struct mtk_dev *mtk, unsigned long pos,
			unsigned int len, char *buf)
{
	int ret;
	size_t retlen;
	ret = mtklib_write(mtk, pos, len, &retlen, buf);
	if (ret){
		printk("%s: mtklib_write return %d\r\n", __FUNCTION__, ret);
		return ret;
	}
	if (retlen != len){
		printk("%s: mtklib_write length != retlen\r\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

/*write_cached_data: write the data in correlation buffer to nand
 * mtk: mtk device the data write to
 *
 * write the data in correlation buffer to mtk device which is pointed by mtk
 */
static int write_cached_data (struct mtk_dev *mtk)
{
	struct mtd_info *mtd = mtk->mbd.mtd;
	int ret;

	if (mtk->cache_state != STATE_DIRTY)
		return 0;

	DEBUG("mtk: writing cached data for \"%s\" "
			"at 0x%lx, size 0x%x\r\n", mtd->name,
			mtk->cache_offset, mtk->cache_size);
	
	ret = erase_write (mtk, mtk->cache_offset,
			   mtk->cache_size, mtk->cache_data);
	if (ret){
		printk("%s: error!\r\n", __FUNCTION__);
		mtk->cache_state = STATE_EMPTY;
		return ret;
	}

	/*
	 * Here we could argubly set the cache state to STATE_CLEAN.
	 * However this could lead to inconsistency since we will not
	 * be notified if this content is altered on the flash by other
	 * means.  Let's declare it empty and leave buffering tasks to
	 * the buffer cache instead.
	 */
	mtk->cache_state = STATE_EMPTY;
	return 0;
}

/*do_cached_write: write the data in buf to mtk device
 * mtk: mtk device the data should be written to
 * pos: the offset in bytes
 * len: length of buffer
 * buf: data buffer
 *
 * do cached write, operate the correlation buffer of mtk device to cache
 * data instead write to nand directly, this mechanism can improve nand write
 * performance
 */
 int do_cached_write (struct mtk_dev *mtk, unsigned long pos,
			   unsigned int len, char *buf)
{
	struct mtd_info *mtd = mtk->mbd.mtd;
	unsigned int sect_size = mtk->cache_size;
	size_t retlen;
	int ret;

	DEBUG("mtk: write on \"%s\" at 0x%lx, size 0x%x\r\n",
		mtd->name, pos, len);
	
	// this will never happend :)
	if (!sect_size)
		return mtklib_write(mtk, pos, len, &retlen, buf);//mtd->write(mtd, pos, len, &retlen, buf);

	while (len > 0) {
		unsigned long sect_start = (pos>>mtk->cachesize_shift)*sect_size;
		unsigned int offset = pos - sect_start;
		unsigned int size = sect_size - offset;
		if( size > len )
			size = len;

		if (size == sect_size) {
			/*
			 * We are covering a whole sector.  Thus there is no
			 * need to bother with the cache while it may still be
			 * useful for other partial writes.
			 */
			 if(pos == mtk->cache_offset)
			 	mtk->cache_state = STATE_EMPTY;
			ret = erase_write (mtk, pos, size, buf);
			if (ret){
				printk("%s erase_write function return %d\r\n", __FUNCTION__, ret);
				return ret;
			}
		} else {
			/* Partial sector: need to use the cache */
			if (mtk->cache_state == STATE_DIRTY &&
			    mtk->cache_offset != sect_start) {
				ret = write_cached_data(mtk);
				if (ret){
					printk("%s write_cached_data function return %d\r\n", __FUNCTION__, ret);
					return ret;
				}
			}
			if (mtk->cache_state == STATE_EMPTY ||
			    mtk->cache_offset != sect_start) {
				/* fill the cache with the current sector */
				mtk->cache_state = STATE_EMPTY;
				ret = mtklib_read(mtk, sect_start, sect_size, &retlen, mtk->cache_data);//mtd->read(mtd, sect_start, sect_size, &retlen, mtk->cache_data);
				mtk->cache_offset = sect_start;
				mtk->cache_state = STATE_CLEAN;
			}
			/* write data to our local cache */
			memcpy (mtk->cache_data + offset, buf, size);
			mtk->cache_state = STATE_DIRTY;
		}

		buf += size;
		pos += size;
		len -= size;
	}

	return 0;
}

/*do_cached_read: read the data from mtk device to buffer
 * mtk: mtk device the data should be read from
 * pos: the offset in bytes
 * len: length of buffer
 * buf: data buffer
 *
 * do cached read, read the data from correlation buffer firstly instead of read 
 * from nand directly, this mechanism can improve nand read performance
 */
int do_cached_read (struct mtk_dev *mtk, unsigned long pos,
			   unsigned int len, char *buf)
{
	struct mtd_info *mtd = mtk->mbd.mtd;
	unsigned int sect_size = mtk->cache_size;
	size_t retlen;
	int ret;

	DEBUG("mtk: read on \"%s\" at 0x%lx, size 0x%x\r\n",
			mtd->name, pos, len);

	if (!sect_size)
		return mtklib_read(mtk, pos, len, &retlen, buf);//mtd->read(mtd, pos, len, &retlen, buf);

	while (len > 0) {
		unsigned long sect_start = (pos>>mtk->cachesize_shift)*sect_size;
		unsigned int offset = pos - sect_start;
		unsigned int size = sect_size - offset;
		if (size > len)
			size = len;
		/*
		 * Check if the requested data is already cached
		 * Read the requested amount of data from our internal cache if it
		 * contains what we want, otherwise we read the data directly
		 * from flash.
		 */
		if (mtk->cache_state != STATE_EMPTY &&
		    mtk->cache_offset == sect_start) {
			memcpy (buf, mtk->cache_data + offset, size);
		} else {
			//all of the situation that mtklib can not read all of the data we required, mtklib should return -1;
			if(mtk->cache_state == STATE_DIRTY){
				ret = write_cached_data(mtk);
				if(ret){
					printk("%s write dirty data to flash!\r\n", __FUNCTION__);
				}
			}
			ret = mtklib_read(mtk, sect_start, sect_size, &retlen, mtk->cache_data);//mtd->read(mtd, pos, size, &retlen, buf);
			if (ret){
				printk("%s read function read %d\r\n", __FUNCTION__, ret);
				return ret;
			}
			mtk->cache_state = STATE_CLEAN;
			mtk->cache_offset = sect_start;
			continue;
		}
		buf += size;
		pos += size;
		len -= size;
	}

	return 0;
}


static int mtk_readsect(struct mtd_blktrans_dev *dev,
			      unsigned long block, char *buf)
{
	int ret = -1;
	struct mtk_dev *mtk = container_of(dev, struct mtk_dev, mbd);
	mutex_lock(&pm_lock);
	mutex_lock(&mtk->cache_mutex);
	//printk("[MTKLib][%d]%s:write kernel block %d, mtklib block %d\r\n", mtk->index, __FUNCTION__, block, (block<<MTK_BLOCK_SHIFT)/8192);
	//msleep(10);
	if (unlikely(!mtk->cache_data && mtk->cache_size)) {
		mtk->cache_data = vmalloc(mtk->cache_size);
		if (!mtk->cache_data){
			mutex_unlock(&mtk->cache_mutex);
			mutex_unlock(&pm_lock);
			return -EINTR;
		}
		mtk->cache_state = STATE_EMPTY;
	}
	
	ret = do_cached_read(mtk, block<<MTK_BLOCK_SHIFT, MTK_BLOCK_SIZE, buf);
	mutex_unlock(&mtk->cache_mutex);
	mutex_unlock(&pm_lock);
	return ret;
}

/*mtk_flush: flush correlation buffer of dev
 * dev: the dev is operating
 *
 * flush the data in correlation buffer to mtk device
 */
static int mtk_flush(struct mtd_blktrans_dev *dev)
{
	struct mtk_dev *mtk = container_of(dev, struct mtk_dev, mbd);
	mutex_lock(&pm_lock);
	mutex_lock(&mtk->cache_mutex);
	write_cached_data(mtk);
	mutex_unlock(&mtk->cache_mutex);
	mutex_unlock(&pm_lock);
	return 0;
}

/*mtk_writesect: 
 */
static int mtk_writesect(struct mtd_blktrans_dev *dev,
			      unsigned long block, char *buf)
{
	struct mtk_dev *mtk = container_of(dev, struct mtk_dev, mbd);
	int ret = -1;
	mutex_lock(&pm_lock);
	mutex_lock(&mtk->cache_mutex);
	//printk("[MTKLib][%d]%s:read kernel block %d, mtklib block %d\r\n", mtk->index, __FUNCTION__, block, (block<<MTK_BLOCK_SHIFT)/8192);
	//msleep(10);
	if(unlikely(!mtk->cache_data && mtk->cache_size)) {
		mtk->cache_data = vmalloc(mtk->cache_size);
		mtk->cache_offset = -1;
		if (!mtk->cache_data){
			mutex_unlock(&mtk->cache_mutex);
			mutex_unlock(&pm_lock);
			return -EINTR;
		}
		mtk->cache_state = STATE_EMPTY;
	}
	
	ret = do_cached_write(mtk, block<<MTK_BLOCK_SHIFT, MTK_BLOCK_SIZE, buf);
	mutex_unlock(&mtk->cache_mutex);
	mutex_unlock(&pm_lock);
	return ret;
}

#if ADD_TIMER
static void mtk_timer_handle(unsigned long arg){
	struct mtk_dev *mtk = container_of((struct mtd_blktrans_dev *)arg, struct mtk_dev, mbd);
	mod_timer(&mtk->mtklib_timer, jiffies + 1000);
	mtk->timer_cnt++;
	complete(&mtk->flush_sync);
}

static int mtk_flush_thread(void *arg){
	struct mtk_dev *mtk = container_of((struct mtd_blktrans_dev *)arg, struct mtk_dev, mbd);
	while(1){
		wait_for_completion(&mtk->flush_sync);
		if(mtk->flag == 1){
			printk("Exit flush thread!\r\n");
			break;
		}
		mtk_flush(&mtk->mbd);
	}
	mtk->flush_thread = NULL;
	return 0;
}
#endif

static int mtk_open(struct mtd_blktrans_dev *mbd)
{
	struct mtk_dev *mtk = container_of(mbd, struct mtk_dev, mbd);

	DEBUG("mtk_open\r\n");
	printk("Opening mtk device mtkd%d!\r\n", mtk->index);

	mutex_lock(&mtks_lock);
	if (mtk->count) {
		mtk->count++;
		mutex_unlock(&mtks_lock);
		return 0;
	}

	/* OK, it's not open. Create cache info for it */
	mtk->count = 1;
	mutex_init(&mtk->cache_mutex);
	mtk->cache_state = STATE_EMPTY;
	if (!(mbd->mtd->flags & MTD_NO_ERASE) && mbd->mtd->erasesize) {
		mtk->cache_size = mbd->mtd->writesize;
		mtk->cachesize_shift = mbd->mtd->writesize_shift;
		mtk->cache_data = NULL;
	}
#if ADD_TIMER
	init_timer(&mtk->mtklib_timer);
	mtk->mtklib_timer.function = mtk_timer_handle;
	mtk->mtklib_timer.expires = jiffies + 1000;
	mtk->mtklib_timer.data = (unsigned long)mtk;
	mtk->timer_cnt = 0;
	if(!mtk->flush_thread){
		mtk->flush_thread = kthread_create(mtk_flush_thread, (void *)mtk, "mtk_flush%d", mtk->index);
		if (IS_ERR(mtk->flush_thread)) {
			printk("Can not start flush thread!\r\n");
			mutex_unlock(&mtks_lock);
			DEBUG("ok\r\n");
			return 0;
		}
		set_user_nice(mtk->flush_thread, 10);
		init_completion(&mtk->flush_sync);
		wake_up_process(mtk->flush_thread);
		mtk->flag = 0;
	}
	add_timer(&mtk->mtklib_timer);
	printk("Start Timer!\r\n");
#endif
	mutex_unlock(&mtks_lock);

	DEBUG("ok\r\n");

	return 0;
}

static int mtk_release(struct mtd_blktrans_dev *mbd)
{
	struct mtk_dev *mtk = container_of(mbd, struct mtk_dev, mbd);

   	DEBUG("mtk_release\r\n");
	printk("Putting mtk device!\r\n");

	mutex_lock(&mtks_lock);

	mutex_lock(&mtk->cache_mutex);
	write_cached_data(mtk);
	mutex_unlock(&mtk->cache_mutex);

	if (!--mtk->count) {
		vfree(mtk->cache_data);
		mtk->cache_state = STATE_EMPTY;
		mtk->cache_data = NULL;
#if ADD_TIMER
		mtk->flag = 1;
		complete(&mtk->flush_sync);
		del_timer(&mtk->mtklib_timer);
#endif
	}

	mutex_unlock(&mtks_lock);

	DEBUG("ok\r\n");

	return 0;
}

static void mtk_add_mtd(struct mtd_blktrans_ops *tr, struct mtd_info *mtd)
{
	int i, flag = 0;
	struct mtk_dev *dev;
	unsigned long add_start = 0;
	unsigned long add_end = 0;
	static int ver_flag = 0;
	if(ver_flag == 0){
		printk(VERSION);
		ver_flag = 1;
	}
#if 0
	if(mtd_devs > MTK_MAX_DEVICES){
		printk("too many mtd devices, maximum is %d\r\n", MTK_MAX_DEVICES);
		return;
	}

	
	for(i = 0; i<MTK_MAX_DEVICES; i++){
		if((memcmp(mtd_dev_param[i].name, mtd->name, strlen(mtd->name))==0)){
			flag = 1;
			break;
		}
	}
	if(flag == 0){
		printk("Can not find device %s in mtd device array! Do not attach it!\r\n", mtd->name);
		return;
	}

#else
/*
	if(!strstr(mtd->name, "_ext4"))
		return ;
*/
	if((memcmp(mtd->name, "usrdata", 7)!=0) && (memcmp(mtd->name, "system", 6)!=0)){
		return;
	}

	if (strstr(boot_command_line, "slot_suffix=_a")) {
		if (memcmp(mtd->name, "system_b", 8)==0)
			return;
	} else if (strstr(boot_command_line, "slot_suffix=_b")) {
		if (memcmp(mtd->name, "system_a", 8)==0)
			return;
	}
#endif
	add_start = jiffies;
	dev = kzalloc(sizeof(struct mtk_dev), GFP_KERNEL);
	if (!dev)
		return;

	printk("MTKLib: add device %s\r\n", mtd->name);
	dev->mbd.mtd = mtd;
	dev->mbd.devnum = mtd->index;
	dev->index = mtd_dev_count++;
	dev->size = mtd->size;
	dev->mbd.tr = tr;
	if(dev_list[mtd_dev_count]==NULL){
		dev_list[mtd_dev_count]=dev;
	}else{
		printk("[MTKLib]: add device %d has exist\r\n", mtd_dev_count);
	}

	if (!(mtd->flags & MTD_WRITEABLE))
		dev->mbd.readonly = 1;

	if(mtklib_attach_mtd_dev(dev)){
		printk("can not attach mtklib for dev %s\r\n", mtd->name);
		return;
	}

	dev->mbd.size = dev->dev_size>>MTK_BLOCK_SHIFT;

	if (add_mtd_blktrans_dev(&dev->mbd)){
		mtklib_dettach_mtd_dev(dev);
		kfree(dev);
	}
	add_end = jiffies;
	printk("ADD MTKLib Time: %d\r\n", (add_end-add_start)*1000/HZ);
}

static void mtk_remove_dev(struct mtd_blktrans_dev *mbd)
{
	struct mtk_dev *mtk = NULL;
	if (!mbd) {
		printk("[MTKLib] empty mtd device, just return..\n");
		return ;
	}
	mtk = container_of(mbd, struct mtk_dev, mbd);

	printk("[MTKLib][%d] clean up queue start!\n", mtk->index);
	blk_cleanup_queue(mbd->rq);
	printk("[MTKLib][%d] clean up queue finished!\n", mtk->index);

	mtklib_dettach_mtd_dev(mtk);
	del_mtd_blktrans_dev(mbd);
	dev_list[mtk->index] = NULL;
	// mtk_dev cannot be freed here!!!!!!!!
	// kfree(mtk);
}

static int mtklib_notify(struct notifier_block *nb,
			       unsigned long mode, void *_unused)
{
	int count = 0;
	switch (mode) {
	case PM_SUSPEND_PREPARE:	
	case PM_HIBERNATION_PREPARE:
	case PM_RESTORE_PREPARE:
		printk("%s: PM_SUSPEND_PREPARE start\n", __func__);
#if 0
		for(count = 0; count < MTK_MAX_DEVICES; count++){
			if(dev_list[count]){
				mtk_flush(&(dev_list[count]->mbd));
				mtklib_suspend(dev_list[count]);
			}
		}
#endif
		break;
	case PM_POST_SUSPEND:
	case PM_POST_HIBERNATION:
	case PM_POST_RESTORE:
		printk("%s: PM_POST_SUSPEND start\n", __func__);
		mutex_lock(&pm_lock);
		for(count = 0; count < MTK_MAX_DEVICES; count++){
			if(dev_list[count]){
				mtklib_resume(dev_list[count]);
			}
		}
		mutex_unlock(&pm_lock);
		break;
	}
	return 0;
}

#define ALIGN_UP(A, B) 		(((A) + (B)) & (~B))
#define ALIGN_DOWN(A, B) 		((A) & (~B))

static int mtklib_discard(struct mtd_blktrans_dev *dev,
	unsigned long compat_block, unsigned nr_compat_block)
{
	struct mtk_dev *mtk = container_of(dev, struct mtk_dev, mbd);
	unsigned long discard_start_addr = 0;
	unsigned long discard_last_addr = 0;
	unsigned long nr_discard_blocks = 0;
	unsigned long align_mask = 0;

	if (!dev) {
		return -EINVAL;
	}
	align_mask = ((dev->mtd->writesize / MTK_BLOCK_SIZE) - 1);

	discard_start_addr = ALIGN_UP(compat_block, align_mask);
	discard_last_addr = ALIGN_DOWN(compat_block + nr_compat_block - 1, align_mask);
	if (discard_last_addr < discard_start_addr) {
		return 0;
	}
	nr_discard_blocks = (discard_last_addr - discard_start_addr) + 1;
	if (mtklib_delete(mtk, discard_start_addr * MTK_BLOCK_SIZE, nr_discard_blocks * MTK_BLOCK_SIZE) < 0) {
		return -EIO;
	}

	return 0;
}

struct notifier_block mtklib_pm_notifier = {
	.notifier_call = mtklib_notify,
};

static struct mtd_blktrans_ops mtk_tr = {
	.name		= "mtkd",
	.major		= 96,
	.part_bits	= 0,
	.blksize 	= MTK_BLOCK_SIZE,
	.open		= mtk_open,
	.release	= mtk_release,
	.flush		= mtk_flush,
	.readsect	= mtk_readsect,
	.writesect	= mtk_writesect,
	.discard = mtklib_discard,
	.add_mtd	= mtk_add_mtd,
	.remove_dev	= mtk_remove_dev,
	.owner		= THIS_MODULE,
};

static int __init mt33xx_mtklib_init(void)
{
	printk("\r\nmt33xx_mtklib_init\r\n");
	suspend_resume = 0;
	mutex_init(&mtks_lock);
	mutex_init(&pm_lock);
	register_mtd_blktrans(&mtk_tr);
	register_pm_notifier(&mtklib_pm_notifier);
	printk("\r\nmt33xx_mtklib_init ok\r\n");
	return 0;
}

static void __exit mt33xx_mtklib_exit(void)
{
	printk("\r\nmt33xx_mtklib_exit\r\n");
	unregister_pm_notifier(&mtklib_pm_notifier);
	deregister_mtd_blktrans(&mtk_tr);
	printk("\r\nmt33xx_mtklib_exit\r\n");
}

module_init(mt33xx_mtklib_init);
module_exit(mt33xx_mtklib_exit);

#if 0
/**
 * bytes_str_to_int - convert a number of bytes string into an integer.
 * @str: the string to convert
 *
 * This function returns positive resulting integer in case of success and a
 * negative error code in case of failure.
 */
static int __init bytes_str_to_int(const char *str)
{
	char *endp;
	unsigned long result;

	result = simple_strtoul(str, &endp, 0);
	if (str == endp || result >= INT_MAX) {
		printk(KERN_ERR "UBI error: incorrect bytes count: \"%s\"\r\n",
		       str);
		return -EINVAL;
	}

	switch (*endp) {
	case 'G':
		result *= 1024;
	case 'M':
		result *= 1024;
	case 'K':
		result *= 1024;
		if (endp[1] == 'i' && endp[2] == 'B')
			endp += 2;
	case '\0':
		break;
	default:
		printk(KERN_ERR "UBI error: incorrect bytes count: \"%s\"\r\n",
		       str);
		return -EINVAL;
	}

	return result;
}


static int __init mtklib_mtd_param_parse(const char *val, struct kernel_param *kp)
{
	int i, len;
	struct mtk_dev_param *p;
	char buf[MAX_PARAM_NAME_LEN];
	char *pbuf = &buf[0];
	char *tokens[2] = {NULL, NULL};

	printk("mtklib start to parse the value %s\r\n", val);
	if (!val)
		return -EINVAL;

	if (mtd_devs == MTK_MAX_DEVICES) {
		printk(KERN_ERR "UBI error: too many parameters, max. is %d\r\n",
		       MTK_MAX_DEVICES);
		return -EINVAL;
	}

	len = strnlen(val, MAX_PARAM_NAME_LEN);
	if (len == MAX_PARAM_NAME_LEN) {
		printk(KERN_ERR "UBI error: parameter \"%s\" is too long, "
		       "max. is %d\r\n", val, MAX_PARAM_NAME_LEN);
		return -EINVAL;
	}

	if (len == 0) {
		printk(KERN_WARNING "UBI warning: empty 'mtd=' parameter - "
		       "ignored\r\n");
		return 0;
	}

	strcpy(buf, val);

	/* Get rid of the final newline */
	if (buf[len - 1] == '\n')
		buf[len - 1] = '\0';

	for (i = 0; i < 2; i++)
		tokens[i] = strsep(&pbuf, ",");

	if (pbuf) {
		printk(KERN_ERR "MTKLIB error: too many arguments at \"%s\"\r\n",
		       val);
		return -EINVAL;
	}

	printk("mtklib parse buff is %x\r\n", (unsigned int)pbuf);

	p = &mtd_dev_param[mtd_devs];
	strcpy(&p->name[0], tokens[0]);

	if (tokens[1])
		p->vid_hdr_offs = bytes_str_to_int(tokens[1]);

	if (p->vid_hdr_offs < 0)
		return p->vid_hdr_offs;

	mtd_devs += 1;
	printk("mtklib: add device %s\r\n", mtd_dev_param[mtd_devs-1].name);
	return 0;
}

module_param_call(mtd, mtklib_mtd_param_parse, NULL, NULL, 000);
MODULE_PARM_DESC(mtd, "MTD devices to attach. Parameter format: "
		      "mtd=<name|num|path>[,<vid_hdr_offs>].\n"
		      "Multiple \"mtd\" parameters may be specified.\n"
		      "MTD devices may be specified by their number, name, or "
		      "path to the MTD character device node.\n"
		      "Optional \"vid_hdr_offs\" parameter specifies UBI VID "
		      "header position to be used by UBI.\n"
		      "Example 1: mtd=/dev/mtd0 - attach MTD device "
		      "/dev/mtd0.\n"
		      "Example 2: mtd=content,1984 mtd=4 - attach MTD device "
		      "with name \"content\" using VID header offset 1984, and "
		      "MTD device number 4 with default VID header offset.");

#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas Pitre <nico@fluxnic.net> et al.");
MODULE_DESCRIPTION("Caching read/erase/writeback block device emulation access to MTD devices");
