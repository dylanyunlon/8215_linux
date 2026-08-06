#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/videodev2.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/debugfs.h>
#include <media/videobuf-dma-contig.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/atc/vdp_mdd.h>
#include <media/atc/display.h>
#include <media/atc/pmx_hal.h>
#include <media/atc/cp.h>
#include "atc_voutlib.h"
#include "atc_voutdef.h"
#include "screen_hvdetect.h"
#include "x_ver.h"
struct mutex interface_lock;
MODULE_AUTHOR("Autiochips Inc");
MODULE_DESCRIPTION("ATC Video for Linux Video Output Driver");
MODULE_LICENSE("GPL");

/* Driver Configuration macros */
#define VOUT_NAME		"atc_vout"

#define MMISC_MODE_NAME                   "VOUT"
#define MMISC_VER_MAJOR                   01
#define MMISC_VER_MINOR                   00
#define MMISC_VER_REV                     00


u32 vout_log_lvl = VOUT_LOG_LVL_TRACE;
u8 *vout_lvl_str[] = {
	"OFF",
	"ERR",
	"WARN",
	"INFO",
	"TRACE",
	"DBG",
	"IRQ",
};

u32 osd_buf_pa = 0;
u32 osd_buf_va = 0;
static void * vbuf_va;
static struct videobuf_queue_ops video_vbq_ops;

/* dump variables
*  dump_buffer_flag 0:disable 1:enable
*  dump_options 0:full picture 1:top 2:middle top 3:middle bottom 4:bottom
*  dump_maxcount 0:keep on dumping for default maxcount
*  usage: echo dumpbuffer dump_buffer_flag dump_options dump_maxcount >
*  /sys/kernel/debug/vout-debug
*/
#define DEFAULT_DUMP_MAX_COUNT 100
static struct dentry *vout_debugfs;
static unsigned int dump_buffer_flag = 0;
static unsigned int dump_options = 0;
static unsigned int dump_maxcount = 0;
static unsigned int dump_count = 0;

static unsigned int dump_time_flag = (1 << 6);

/* list of image formats supported by ATC video pipelines */
static const struct v4l2_fmtdesc atc_formats[] = {
	{
		/* Note:  V4L2 defines RGB565 as:
		 *
		 *	Byte 0			  Byte 1
		 *	g2 g1 g0 r4 r3 r2 r1 r0   b4 b3 b2 b1 b0 g5 g4 g3
		 *
		 * We interpret RGB565 as:
		 *
		 *	Byte 0			  Byte 1
		 *	g2 g1 g0 b4 b3 b2 b1 b0   r4 r3 r2 r1 r0 g5 g4 g3
		 */
		.description = "RGB565, le",
		.pixelformat = V4L2_PIX_FMT_RGB565,
	},
	{
		/* Note:  V4L2 defines RGB32 as: RGB-8-8-8-8  we use
		 *  this for RGB24 unpack mode, the last 8 bits are ignored
		 * */
		.description = "RGB32, le",
		.pixelformat = V4L2_PIX_FMT_RGB32,
	},
	{
		/* Note:  V4L2 defines RGB24 as: RGB-8-8-8  we use
		 *	  this for RGB24 packed mode
		 *
		 */
		.description = "RGB24, le",
		.pixelformat = V4L2_PIX_FMT_RGB24,
	},
	{
		.description = "YUYV (YUV 4:2:2), packed",
		.pixelformat = V4L2_PIX_FMT_YUYV,
	},
	{
		.description = "UYVY, packed",
		.pixelformat = V4L2_PIX_FMT_UYVY,
	},
};

#define NUM_OUTPUT_FORMATS (ARRAY_SIZE(atc_formats))

static void atc_vout_reset_video_data(struct atc_vout_device *vout)
{
	struct v4l2_pix_format *pix;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return;
	}

	/* set the default pix */
	pix = &vout->pix;

	/* Set the default pix size */
	pix->width = DEFAULT_WIDTH;
	pix->height = DEFAULT_HEIGHT;

	/* Default pixel format is YUV 420 MB */
	pix->pixelformat = v4l2_fourcc('A', 'M', '1', '2');
	pix->field = V4L2_FIELD_ANY;
	pix->bytesperline = pix->width * YC420_BPP;
	pix->sizeimage = pix->bytesperline * pix->height;
	pix->colorspace = V4L2_COLORSPACE_JPEG;

	/* Default output size is panel resolution */
	vout->bpp = YC420_BPP;
	vout->fbuf.fmt.width  =  DEFAULT_OUT_WIDTH;
	vout->fbuf.fmt.height =  DEFAULT_OUT_HEIGHT;
	vout->fbuf.flags = 0;
	vout->fbuf.capability = V4L2_FBUF_CAP_LOCAL_ALPHA | V4L2_FBUF_CAP_SRC_CHROMAKEY | V4L2_FBUF_CAP_CHROMAKEY;

	/* Set the data structures for the overlay parameters*/
	vout->win.global_alpha = 255;
	vout->win.chromakey = 0;
	vout->win.field = 1;
	vout->extwin.global_alpha = 255;
	vout->extwin.chromakey = 0;

	atc_vout_new_format(pix, &vout->fbuf, &vout->crop, &vout->win, &vout->extwin);
	atc_vout_reset_cp(vout, VOUT_HW_VDP1);
	atc_vout_reset_cp(vout, VOUT_HW_VDP2);

	vout->prio = V4L2_PRIORITY_DEFAULT;
	vout->format = VOUT_FMT_UNKNOWN;
	vout->output = VOUT_OUTPUT_FRONT;
	vout->orgout = VOUT_OUTPUT_NONE;
	vout->context = VOUT_CONTEXT_FRONT;
	vout->qbufcnt = 0;
	vout->dqbufcnt = 0;
	vout->stateflags = 0;
	vout->enable = false;
	vout->datavalid = false;
	vout->streaming = false;
	vout->encoding = COLOR_YCBCR_BT601;
	vout->range = COLOR_YCBCR_FULL_RANGE;

	memset(&vout->param, 0, sizeof(vout->param));
	memset(&vout->osd_param, 0, sizeof(vout->osd_param));
	memset(&vout->screen_size, 0, sizeof(vout->screen_size));
}

/*
 * atc_vout_uservirt_to_phys: This inline function is used to convert user
 * space virtual address to physical address.
 */
static u32 atc_vout_uservirt_to_phys(u32 virtp)
{
	unsigned long physp = 0;
	struct vm_area_struct *vma;
	struct mm_struct *mm = current->mm;

	/* For kernel direct-mapped memory, take the easy way */
	if (virtp >= PAGE_OFFSET) {
		return virt_to_phys((void *) virtp);
	}

	down_read(&current->mm->mmap_sem);
	vma = find_vma(mm, virtp);

	if (vma && (vma->vm_flags & VM_IO) && vma->vm_pgoff) {
		/* this will catch, kernel-allocated, mmaped-to-usermode addresses */
		physp = (vma->vm_pgoff << PAGE_SHIFT) + (virtp - vma->vm_start);
		up_read(&current->mm->mmap_sem);
	} else {
		/* otherwise, use get_user_pages() for general userland pages */
		int res, nr_pages = 1;
		struct page *pages;

		res = get_user_pages(current, current->mm, virtp, nr_pages, 1, 0, &pages, NULL);
		up_read(&current->mm->mmap_sem);

		if (res == nr_pages) {
			physp =  __pa(page_address(&pages[0]) + (virtp & ~PAGE_MASK));
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s get_user_pages failed res %d nr_pages %d\r\n"
				, __func__, res, nr_pages);
			return 0;
		}
	}

	return physp;
}

/*
 * Free the V4L2 buffers
 */
void atc_vout_free_buffers(struct atc_vout_device *vout)
{
	int i, numbuffers;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return;
	}

	if (vout->format == VOUT_FMT_VIDEO) {
		/* Allocate memory for the buffers */
		numbuffers = DEFAULT_BUF_NUM;
		vout->buffer_size = DEFAULT_BUF_SIZE;

		for (i = 0; i < numbuffers; i++) {
			if (vout->buf_virt_addr[i]) {
				atc_vout_free_vdo_buffer(vout->buf_virt_addr[i], vout->buffer_size);
			}

			vout->buf_phy_addr[i] = 0;
			vout->buf_virt_addr[i] = 0;
		}
	} else {
		/* Allocate memory for the buffers */
		numbuffers = DEFAULT_BUF_NUM;
		vout->buffer_size = DEFAULT_BUF_SIZE;

		for (i = 0; i < numbuffers; i++) {
			if (vout->buf_virt_addr[i]) {
				atc_vout_free_vdo_buffer(vout->buf_virt_addr[i], vout->buffer_size);
			}

			vout->buf_phy_addr[i] = 0;
			vout->buf_virt_addr[i] = 0;
		}
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d to do release osd buffer\r\n"
			, __func__, vout->vid);
	}
}

void atc_vout_isr(void *arg, u32 phy_isr)
{
	unsigned long vbq_lock_flags = 0;
	struct atc_vout_device *vout = (struct atc_vout_device *)arg;

	if(vout == NULL) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return;
	}

	/* output none / front / front rear update in front isr */
	if ((vout->output == VOUT_OUTPUT_REAR) && (phy_isr == VOUT_HW_VDP1)) {
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d not front buffer output %d\r\n", __func__
			, vout->vid, vout->output);
		return;
	} else if ((vout->output != VOUT_OUTPUT_REAR) && (phy_isr == VOUT_HW_VDP2)) {
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d not rear buffer output %d\r\n", __func__
			, vout->vid, vout->output);
		return;
	}

	if (!vout->streaming) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d streaming %d\r\n", __func__, vout->vid, vout->streaming);
		return;
	}

	if ((vout->format == VOUT_FMT_VIDEO) && (vout->param.u4Idx > VOUT_HW_VDP2)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d not support %d!\r\n", __func__, vout->vid, vout->param.u4Idx);
		return;
	}

	spin_lock_irqsave(&vout->vbq_lock, vbq_lock_flags);

	if (vout->cur_frm != vout->next_frm) {
		/*u32 i = vout->cur_frm->i;
		struct VOUT_PARAM *vout_param = (struct VOUT_PARAM *)vout->buf_virt_addr[i];
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d buffer done idx %d  0x%x 0x%x\r\n", __func__
			, vout->vid, i, vout_param->y_phy_addr, vout_param->c_phy_addr);*/
		vout->cur_frm->state = VIDEOBUF_DONE;
		wake_up_interruptible(&vout->cur_frm->done);
		vout->cur_frm = vout->next_frm;
	}

	vout->first_int = 0;

	if (list_empty(&vout->dma_queue)) {
		goto vout_isr_err;
	}

	vout->next_frm = list_entry(vout->dma_queue.next, struct videobuf_buffer, queue);
	if (vout->next_frm == NULL) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d next_frm is NULL\r\n", __func__, vout->vid);
	} else {
		list_del(&vout->next_frm->queue);
		vout->next_frm->state = VIDEOBUF_ACTIVE;
	}

vout_isr_err:
	spin_unlock_irqrestore(&vout->vbq_lock, vbq_lock_flags);
}
EXPORT_SYMBOL(atc_vout_isr);

/*
 * Buffer setup function is called by videobuf layer when REQBUF ioctl is
 * called. This is used to setup buffers and return size and count of
 * buffers allocated. After the call to this buffer, videobuf layer will
 * setup buffer queue depending on the size and count of buffers
 */
static int atc_vout_buffer_setup(struct videobuf_queue *q, unsigned int *count, unsigned int *size)
{
	int startindex = 0, i = 0;
	u32 phy_addr = 0, virt_addr = 0;
	struct atc_vout_device *vout = NULL;

	if ((NULL == q) || (NULL == q->priv_data) || (NULL == count) || (NULL == size)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s input params error\r\n", __func__);
		return -EINVAL;
	}

	vout = q->priv_data;
	if (V4L2_BUF_TYPE_VIDEO_OUTPUT != q->type) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d type %d\r\n", __func__, vout->vid, q->type);
		return -EINVAL;
	}

	if (V4L2_MEMORY_MMAP == vout->memory && *count < startindex) {
		*count = startindex;
	}

	if (V4L2_MEMORY_MMAP != vout->memory) {
		VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d memory is not V4L2_MEMORY_MMAP\r\n", __func__, vout->vid);
		return 0;
	}

	if (vout->format == VOUT_FMT_OSD) {
		*size = DEFAULT_BUF_SIZE;

		/* Check the size of the buffer */
		if (*size > ATC_VOUT_MAX_BUF_SIZE) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d size not support %d\r\n", __func__, vout->vid, *size);
			return -ENOMEM;
		}

		for (i = startindex; i < *count; i++) {
			vout->buffer_size = *size;

			virt_addr = atc_vout_alloc_osd_buffer(*size, &phy_addr);
			if (!virt_addr) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d error osd size %d\r\n", __func__, vout->vid, *size);
				return -ENOMEM;
			}

			vout->buf_virt_addr[i] = virt_addr;
			vout->buf_phy_addr[i] = phy_addr;
		}
	} else {
		/* Now allocated the V4L2 buffers */
		*size = DEFAULT_BUF_SIZE;

		for (i = startindex; i < *count; i++) {
			vout->buffer_size = *size;

			virt_addr = atc_vout_alloc_vdo_buffer(*size, &phy_addr);

			vout->buf_virt_addr[i] = virt_addr;
			vout->buf_phy_addr[i] = phy_addr;
		}
	}

	*count = vout->buffer_allocated = i;

	return 0;
}

/*
 * Free the V4L2 buffers additionally allocated than default
 * number of buffers
 */
static void atc_vout_free_extra_buffers(struct atc_vout_device *vout)
{
	int num_buffers = 0, i = 0;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return;
	}

	if (vout->format == VOUT_FMT_VIDEO) {
		for (i = num_buffers; i < vout->buffer_allocated; i++) {
			if (vout->buf_virt_addr[i]) {
				atc_vout_free_vdo_buffer(vout->buf_virt_addr[i], vout->buffer_size);
			}

			vout->buf_virt_addr[i] = 0;
			vout->buf_phy_addr[i] = 0;
		}
	} else {
		for (i = num_buffers; i < vout->buffer_allocated; i++) {
			if (vout->buf_virt_addr[i]) {
				atc_vout_free_osd_buffer(vout->buf_virt_addr[i], vout->buffer_size);
			}

			vout->buf_virt_addr[i] = 0;
			vout->buf_phy_addr[i] = 0;
		}
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d to do release osd buffer\r\n"
			, __func__, vout->vid);
	}

	vout->buffer_allocated = num_buffers;
}

/*
 * This function will be called when VIDIOC_QBUF ioctl is called.
 * It prepare buffers before give out for the display. This function
 * converts user space virtual address into physical address if userptr memory
 * exchange mechanism is used. If rotation is enabled, it copies entire
 * buffer into VRFB memory space before giving it to the DSS.
 */
static int atc_vout_buffer_prepare(struct videobuf_queue *q,
				   struct videobuf_buffer *vb,
				   enum v4l2_field field)
{
	struct atc_vout_device *vout = NULL;

	if ((NULL == q) || (NULL == q->priv_data) || (NULL == vb)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s input params error\r\n", __func__);
		return -EINVAL;
	}

	vout = q->priv_data;
	if (VIDEOBUF_NEEDS_INIT == vb->state) {
		vb->width = vout->pix.width;
		vb->height = vout->pix.height;
		vb->size = vb->width * vb->height * vout->bpp;
		vb->field = field;
	}

	vb->state = VIDEOBUF_PREPARED;

	/* if user pointer memory mechanism is used, get the physical
	 * address of the buffer
	 */
	if (V4L2_MEMORY_USERPTR == vb->memory) {
		if (0 == vb->baddr) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d vb->baddr error\r\n", __func__, vout->vid);
			return -EINVAL;
		}

		/* Physical address */
		vout->queued_buf_addr[vb->i] = (u8 *) atc_vout_uservirt_to_phys(vb->baddr);
	} else {
		vout->queued_buf_addr[vb->i] = (u8 *)vout->buf_phy_addr[vb->i];
	}

	return 0;
}

/*
 * Buffer queue function will be called from the videobuf layer when _QBUF
 * ioctl is called. It is used to enqueue buffer, which is ready to be
 * displayed.
 */
static void atc_vout_buffer_queue(struct videobuf_queue *q,
				  struct videobuf_buffer *vb)
{
	struct atc_vout_device *vout = NULL;

	if ((NULL == q) || (NULL == q->priv_data) || (NULL == vb)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s input params error\r\n", __func__);
		return;
	}

	vout = q->priv_data;
	/* Driver is also maintainig a queue. So enqueue buffer in the driver queue */
	list_add_tail(&vb->queue, &vout->dma_queue);
	vb->state = VIDEOBUF_QUEUED;
}

/*
 * Buffer release function is called from videobuf layer to release buffer
 * which are already allocated
 */
static void atc_vout_buffer_release(struct videobuf_queue *q,
				    struct videobuf_buffer *vb)
{
	if (NULL == vb) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vb is NULL\r\n", __func__);
		return;
	}

	vb->state = VIDEOBUF_NEEDS_INIT;
}

/*
 *  File operations
 */
static unsigned int atc_vout_poll(struct file *file,
				  struct poll_table_struct *wait)
{
	struct atc_vout_device *vout = NULL;
	struct videobuf_queue *q = NULL;

	if ((NULL == file) || (NULL == file->private_data)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s file or vout is NULL\r\n", __func__);
		return 0;
	}

	vout = file->private_data;
	q = &vout->vbq;

	return videobuf_poll_stream(file, q, wait);
}

static void atc_vout_vm_open(struct vm_area_struct *vma)
{
	struct atc_vout_device *vout = NULL;

	if ((NULL == vma) || (NULL == vma->vm_private_data)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vma is NULL\r\n", __func__);
		return;
	}

	vout = vma->vm_private_data;
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "vm_open [vma=%08lx-%08lx]\r\n", vma->vm_start, vma->vm_end);
	vout->mmap_count++;
}

static void atc_vout_vm_close(struct vm_area_struct *vma)
{
	struct atc_vout_device *vout = NULL;

	if ((NULL == vma) || (NULL == vma->vm_private_data)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vma is NULL\r\n", __func__);
		return;
	}

	vout = vma->vm_private_data;
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "vm_close [vma=%08lx-%08lx]\r\n", vma->vm_start, vma->vm_end);
	vout->mmap_count--;
}

static struct vm_operations_struct atc_vout_vm_ops = {
	.open	= atc_vout_vm_open,
	.close	= atc_vout_vm_close,
};

static int atc_vout_mmap(struct file *file, struct vm_area_struct *vma)
{
	int i;
	void *pos;
	unsigned long start = 0, size = 0;
	struct atc_vout_device *vout = NULL;
	struct videobuf_queue *q = NULL;

	if ((NULL == file) || (NULL == file->private_data) || (NULL == vma)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s input params error\r\n", __func__);
		return -EINVAL;
	}

	start =  vma->vm_start;
	size = vma->vm_end - vma->vm_start;
	vout = file->private_data;
	q = &vout->vbq;
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d pgoff=0x%lx, start=0x%lx, end=0x%lx\r\n", __func__
		, vout->vid, vma->vm_pgoff, vma->vm_start, vma->vm_end);

	/* look for the buffer to map */
	for (i = 0; i < VIDEO_MAX_FRAME; i++) {
		if (NULL == q->bufs[i]) {
			continue;
		}

		if (V4L2_MEMORY_MMAP != q->bufs[i]->memory) {
			continue;
		}

		if (q->bufs[i]->boff == (vma->vm_pgoff << PAGE_SHIFT)) {
			break;
		}
	}

	if (VIDEO_MAX_FRAME == i) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d offset invalid [offset=0x%lx]\r\n", __func__
			, vout->vid, (vma->vm_pgoff << PAGE_SHIFT));
		return -EINVAL;
	}

	/* Check the size of the buffer */
	if (size > vout->buffer_size) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d insufficient memory [%lu] [%u]\r\n", __func__
			, vout->vid, size, vout->buffer_size);
		return -ENOMEM;
	}

	q->bufs[i]->baddr = vma->vm_start;

	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_ops = &atc_vout_vm_ops;
	vma->vm_private_data = (void *) vout;

	if (vout->format == VOUT_FMT_OSD) {
		/*vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		vma->vm_pgoff = vout->buf_phy_addr[i] >> PAGE_SHIFT;
		if (remap_pfn_range(vma,
				    vma->vm_start,
				    vout->buf_phy_addr[i] >> PAGE_SHIFT,
				    vout->buffer_size, vma->vm_page_prot)) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d remap_pfn_range error\r\n", __func__, vout->vid);
			return -EAGAIN;
		}*/
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
		pos = (void *)vout->buf_virt_addr[i];
		vma->vm_pgoff = virt_to_phys((void *)pos) >> PAGE_SHIFT;

		while (size > 0) {
			unsigned long pfn;

			pfn = virt_to_phys((void *) pos) >> PAGE_SHIFT;
			if (remap_pfn_range(vma, start, pfn, PAGE_SIZE, PAGE_SHARED)) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d remap_pfn_range error\r\n", __func__, vout->vid);
				return -EAGAIN;
			}

			start += PAGE_SIZE;
			pos += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	} else {
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
		pos = (void *)vout->buf_virt_addr[i];
		vma->vm_pgoff = virt_to_phys((void *)pos) >> PAGE_SHIFT;

		while (size > 0) {
			unsigned long pfn;

			pfn = virt_to_phys((void *) pos) >> PAGE_SHIFT;
			if (remap_pfn_range(vma, start, pfn, PAGE_SIZE, PAGE_SHARED)) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d remap_pfn_range error\r\n", __func__, vout->vid);
				return -EAGAIN;
			}

			start += PAGE_SIZE;
			pos += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}

	vout->mmap_count++;
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "Exiting %s dev %d\r\n", __func__, vout->vid);

	return 0;
}

static int atc_vout_release(struct file *file)
{
	unsigned int ret = 0, idx = 0;
	struct videobuf_queue *q;
	struct atc_vout_device *vout = NULL;
	struct atcvideo_device *vid_dev = NULL;
	struct v4l2_pix_format *pix = NULL;

	if ((NULL == file) || (NULL == file->private_data)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s file or vout is NULL\r\n", __func__);
		return -EINVAL;
	}
	vout = file->private_data;
	vid_dev = vout->vid_dev;
	pix = &vout->pix;
	idx = vout->context;

	if ((NULL == vid_dev) || (NULL == pix)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d vid_dev or pix is NULL\r\n", __func__, vout->vid);
		return -EINVAL;
	}

	if (vout->enable) {
		ret = vidioc_overlay(file, vout, 0);
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d hide overlay %d\r\n", __func__, vout->vid, ret);
	}

	VOUT_PRINT(VOUT_LOG_LVL_DBG, "Entering %s dev %d\r\n", __func__, vout->vid);
	vout->prio = V4L2_PRIORITY_UNSET;

	q = &vout->vbq;

	/* Free all buffers */
	atc_vout_free_extra_buffers(vout);

	/* Even if apply changes fails we should continue
	   freeing allocated memory */
	if (vout->streaming) {
		vout->streaming = false;

		atc_dispc_unregister_isr(atc_vout_isr, vout);
		atc_vout_ioctl(vout,VIDIOC_STREAMOFF);

		videobuf_streamoff(q);
		videobuf_queue_cancel(q);
	}

	if (vout->mmap_count != 0) {
		vout->mmap_count = 0;
	}

	vout->opened -= 1;
	if (!vout->opened) {
		atc_vout_reset_video_data(vout);
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d set default params\r\n", __func__, vout->vid);
	}
	file->private_data = NULL;

	if (vout->buffer_allocated) {
		videobuf_mmap_free(q);
	}

	VOUT_PRINT(VOUT_LOG_LVL_DBG, "Exiting %s dev %d\r\n", __func__, vout->vid);
	return ret;
}

static int atc_vout_open(struct file *file)
{
	struct videobuf_queue *q;
	struct atc_vout_device *vout = NULL;

	vout = video_drvdata(file);
	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout device is NULL\r\n", __func__);
		return -ENODEV;
	}

	/* for now, we only support single open */
	if (vout->opened) {
		VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d open already\r\n", __func__, vout->vid);
		return -EBUSY;
	}

	vout->opened += 1;

	file->private_data = vout;
	vout->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	q = &vout->vbq;
	video_vbq_ops.buf_setup = atc_vout_buffer_setup;
	video_vbq_ops.buf_prepare = atc_vout_buffer_prepare;
	video_vbq_ops.buf_release = atc_vout_buffer_release;
	video_vbq_ops.buf_queue = atc_vout_buffer_queue;
	spin_lock_init(&vout->vbq_lock);

	videobuf_queue_dma_contig_init(q, &video_vbq_ops, q->dev,
				       &vout->vbq_lock, vout->type, V4L2_FIELD_NONE,
				       sizeof(struct videobuf_buffer), vout, NULL);

	VOUT_PRINT(VOUT_LOG_LVL_DBG, "Exiting %s dev %d\r\n", __func__, vout->vid);
	return 0;
}

int atc_vout_dump_buffer(struct atc_vout_device *vout, unsigned long buffer_addr, u32 options, u32 count)
{
	unsigned char vout_dumpfile[256] = "";
	struct file *fp = NULL;
	u32 pitch = 0;
	u32 plane_num = 0;
	void *plane0_va_addr = NULL;
	void *plane1_va_addr = NULL;
	u32 plane0_size = 0;
	u32 plane1_size = 0;
	u32 height = 0;
	int ret = 0;
	loff_t pos = 0;
	mm_segment_t fs = 0;
	struct OVERLAY_PARAM *prParam = NULL;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "vout:%p is NULL \r\n", vout);
		return (-EINVAL);
	}

	pitch = vout->pix.bytesperline / vout->bpp;
	height = vout->pix.height;
	if (vout->format == VOUT_FMT_VIDEO) {
		prParam = &vout->param;
		pitch = prParam->u4SrcWidth;
		height = prParam->u4SrcHeight;
		switch (vout->pix.pixelformat) {
			case v4l2_fourcc('A', 'V', '1', '2'):
			case v4l2_fourcc('A', 'M', '1', '2'):
				plane_num = 2;
				plane0_size = pitch * height;
				plane1_size = plane0_size >> 1;
				if (!pfn_valid(PHYS_PFN(prParam->u4PhysicalAddressY))) {
					plane0_va_addr = ioremap(prParam->u4PhysicalAddressY, plane0_size);
					VOUT_PRINT(VOUT_LOG_LVL_DBG, "u4PhysicalAddressY ioremap %x => %x \r\n", prParam->u4PhysicalAddressY, plane0_va_addr);
				} else {
					plane0_va_addr = phys_to_virt(prParam->u4PhysicalAddressY);
					VOUT_PRINT(VOUT_LOG_LVL_DBG, "u4PhysicalAddressY phys_to_virt %x => %x \r\n", prParam->u4PhysicalAddressY, plane0_va_addr);
				}
				if (NULL == plane0_va_addr) {
					VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d remap y error\r\n", __func__, vout->vid);
					ret = -ENOMEM;
					goto fail;
				}

				if (!pfn_valid(PHYS_PFN(prParam->u4PhysicalAddressC))) {
					plane1_va_addr = ioremap(prParam->u4PhysicalAddressC, plane1_size);
					VOUT_PRINT(VOUT_LOG_LVL_DBG, "u4PhysicalAddressC ioremap %x => %x \r\n", prParam->u4PhysicalAddressC, plane1_va_addr);
				} else {
					plane1_va_addr = phys_to_virt(prParam->u4PhysicalAddressC);
					VOUT_PRINT(VOUT_LOG_LVL_DBG, "u4PhysicalAddressC phys_to_virt %x => %x \r\n", prParam->u4PhysicalAddressC, plane1_va_addr);
				}
				if (NULL == plane1_va_addr) {
					VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d remap c error\r\n", __func__, vout->vid);
					if (!pfn_valid(PHYS_PFN(prParam->u4PhysicalAddressY))) {
						iounmap(plane0_va_addr);
					}
					ret = -ENOMEM;
					goto fail;
				}
				break;

			default:
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d fmt %x not support\r\n", __func__
					, vout->vid, vout->pix.pixelformat);
				goto fail;
		}
	} else if (vout->format == VOUT_FMT_OSD) {
		OSD_DATA_T *param = &vout->osd_param;
		pitch = param->u4Width;
		height = param->u4Height;
		plane_num = 1;
		plane0_size = pitch * height * vout->bpp;
		if (!pfn_valid(PHYS_PFN(param->u4BitmapPA))) {
			plane0_va_addr = ioremap(param->u4BitmapPA, plane0_size);
			VOUT_PRINT(VOUT_LOG_LVL_DBG, "u4BitmapPA ioremap %x => %x \r\n", param->u4BitmapPA, plane0_va_addr);
		} else {
			plane0_va_addr = phys_to_virt(param->u4BitmapPA);
			VOUT_PRINT(VOUT_LOG_LVL_DBG, "u4BitmapPA phys_to_virt %x => %x \r\n", param->u4BitmapPA, plane0_va_addr);
		}
		if (NULL == plane0_va_addr) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d remap osd bitmap error\r\n", __func__, vout->vid);
			ret = -ENOMEM;
			goto fail;
		}
	} else {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d fmt %x not support\r\n", __func__
			, vout->vid, vout->pix.pixelformat);
		goto fail;
	}

	sprintf(vout_dumpfile, "/media/sdcard/vout/vout%d-%dx%d(%dx%d)-%d-%c%c%c%c.raw",
			vout->vid, vout->pix.width, vout->pix.height, pitch, height, count,
			vout->pix.pixelformat, vout->pix.pixelformat >> 8, 
			vout->pix.pixelformat >> 16, vout->pix.pixelformat >> 24);
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s size: %d \r\n", vout_dumpfile, vout->pix.sizeimage);

	fp = filp_open(vout_dumpfile, O_CREAT | O_RDWR, 0777);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s open %s error:%d \n", __func__, vout_dumpfile, ret);
		goto fail;
	} else {
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s open %s ok, fp is %p \n", __func__, vout_dumpfile, fp);
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	if (plane_num == 1) {
		if (plane0_va_addr) {
			pos = fp->f_pos;
			ret = vfs_write(fp, (char __user *)plane0_va_addr, plane0_size, &pos);
			fp->f_pos = pos;

			iounmap(plane0_va_addr);
		}
	} else if (plane_num == 2) {
		if (plane0_va_addr) {
			pos = fp->f_pos;
			ret = vfs_write(fp, (char __user *)plane0_va_addr, plane0_size, &pos);
			fp->f_pos = pos;
		}

		if (plane1_va_addr) {
			pos = fp->f_pos;
			ret = vfs_write(fp, (char __user *)plane1_va_addr, plane1_size, &pos);
			fp->f_pos = pos;
		}

		if (!pfn_valid(PHYS_PFN(prParam->u4PhysicalAddressY))) {
			iounmap(plane0_va_addr);
		}
		if (!pfn_valid(PHYS_PFN(prParam->u4PhysicalAddressC))) {
			iounmap(plane1_va_addr);
		}
	}

	filp_close(fp, NULL);
	set_fs(fs);
	VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s %s write done\n", __func__, vout_dumpfile)

	return 0;

fail:
	return ret;
}

/*
 * V4L2 ioctls
 */
static int vidioc_querycap(struct file *file, void *fh,
			   struct v4l2_capability *cap)
{
	struct atc_vout_device *vout = fh;

	if ((NULL == vout) || ((NULL == vout->vfd)) || (NULL == cap)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout or cap is NULL\r\n", __func__);
		return -EINVAL;
	}

	strlcpy(cap->driver, VOUT_NAME, sizeof(cap->driver));
	strlcpy(cap->card, vout->vfd->name, sizeof(cap->card));
	cap->bus_info[0] = '\0';
	cap->capabilities = V4L2_CAP_STREAMING | V4L2_CAP_VIDEO_OUTPUT |
			    V4L2_CAP_VIDEO_OUTPUT_OVERLAY;

	return 0;
}

static int vidioc_g_fmt_vid_out(struct file *file, void *fh,
				struct v4l2_format *f)
{
	struct atc_vout_device *vout = fh;

	if ((NULL == vout) || (NULL == f)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or f is NULL\r\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	f->fmt.pix = vout->pix;
	mutex_unlock(&vout->lock);

	return 0;
}

static int vidioc_s_fmt_vid_out(struct file *file, void *fh,
				struct v4l2_format *f)
{
	struct atc_vout_device *vout = fh;
	int ret = 0, bpp = 0, format = 0;

	if ((NULL == vout) || (NULL == f)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or f is NULL\r\n", __func__);
		return -EINVAL;
	}

	switch (f->fmt.pix.pixelformat) {
	case v4l2_fourcc('A', 'M', '1', '2'):
	case v4l2_fourcc('A', 'V', '1', '2'):
		bpp = YC420_BPP;
		format = VOUT_FMT_VIDEO;
		break;
	case V4L2_PIX_FMT_ARGB32:
		bpp = RGB32_BPP;
		format = VOUT_FMT_OSD;
		break;
	case V4L2_PIX_FMT_RGB565:
		bpp = RGB565_BPP;
		format = VOUT_FMT_OSD;
		break;
	default:
		bpp = YC420_BPP;
		format = VOUT_FMT_VIDEO;
	}

	f->fmt.pix.sizeimage = f->fmt.pix.width * f->fmt.pix.height * bpp;

	mutex_lock(&vout->lock);
	if ((bpp != vout->bpp) || (format != vout->format)) {
		vout->bpp = bpp;
		vout->format = format;
		vout->stateflags |= VOUT_SET_SRC_PARAMS;
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d bpp = %d, format = %d\r\n", __func__, vout->vid
			, vout->bpp, vout->format);
	}

	if ((vout->pix.pixelformat != f->fmt.pix.pixelformat) || (vout->pix.width != f->fmt.pix.width)
	    || (vout->pix.height != f->fmt.pix.height)) {
		vout->pix = f->fmt.pix;
		vout->stateflags |= VOUT_SET_SRC_PARAMS;
		atc_vout_default_crop(&vout->pix, &vout->crop);
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d pixelformat = %x, height = %d,width = %d \r\n", __func__, vout->vid
			, vout->pix.pixelformat,vout->pix.height,vout->pix.width);
	}
	mutex_unlock(&vout->lock);

	return ret;
}

static int vidioc_s_fmt_vid_overlay(struct file *file, void *fh,
				    struct v4l2_format *f)
{
	int ret = 0, context = 0;
	struct atc_vout_device *vout = fh;
	struct v4l2_window *win = NULL;
	struct v4l2_rect *src_rect = NULL;
	struct v4l2_rect *dst_rect = NULL;

	if ((NULL == vout) || (NULL == f)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or f is NULL\r\n", __func__);
		return -EINVAL;
	}

	win = &f->fmt.win;
	src_rect = &win->w;

	mutex_lock(&vout->lock);
	context = vout->context;
	switch(context) {
	case VOUT_CONTEXT_FRONT:
		if (vout->win.field != win->field) {
			vout->win.field = win->field;
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front field %x\r\n", __func__, vout->vid
				, vout->win.field);
		}
		if (vout->win.chromakey != win->chromakey) {
			vout->win.chromakey = win->chromakey;
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front chromakey %x\r\n", __func__, vout->vid
				, vout->win.chromakey);
		}
		dst_rect = &vout->win.w;
		if ((dst_rect->top != src_rect->top) || (dst_rect->left != src_rect->left) ||
		    (dst_rect->width != src_rect->width) || (dst_rect->height != src_rect->height)) {
			memcpy(dst_rect, &win->w, sizeof(struct v4l2_rect));
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front update params (%d, %d, %d, %d)\r\n", __func__
				, vout->vid, dst_rect->top, dst_rect->left, dst_rect->width, dst_rect->height);
			vout->stateflags |= VOUT_SET_FRONT_DST_PARAMS;
		}
		break;
	case VOUT_CONTEXT_REAR:
		if (vout->extwin.field != win->field) {
			vout->extwin.field = win->field;
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear field %x\r\n", __func__, vout->vid
				, vout->extwin.field);
		}
		if (vout->extwin.chromakey != win->chromakey) {
			vout->extwin.chromakey = win->chromakey;
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear chromakey %x\r\n", __func__, vout->vid
				, vout->extwin.chromakey);
		}
		dst_rect = &vout->extwin.w;
		if ((dst_rect->top != src_rect->top) || (dst_rect->left != src_rect->left) ||
		    (dst_rect->width != src_rect->width) || (dst_rect->height != src_rect->height)) {
			memcpy(dst_rect, &win->w, sizeof(struct v4l2_rect));
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear update params (%d, %d, %d, %d)\r\n", __func__
				, vout->vid, dst_rect->top, dst_rect->left, dst_rect->width, dst_rect->height);
			vout->stateflags |= VOUT_SET_REAR_DST_PARAMS;
		}
		break;
	default:
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d not support context %d\r\n", __func__, vout->vid, context);
		mutex_unlock(&vout->lock);
		return -EINVAL;
	}
	mutex_unlock(&vout->lock);

	return ret;
}

static int vidioc_g_fmt_vid_overlay(struct file *file, void *fh,
				    struct v4l2_format *f)
{
	struct v4l2_window *win = NULL;
	struct atc_vout_device *vout = fh;

	if ((NULL == vout) || (NULL == f)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or f is NULL\r\n", __func__);
		return -EINVAL;
	}

	win = &f->fmt.win;

	mutex_lock(&vout->lock);
	switch (vout->context) {
	case VOUT_CONTEXT_FRONT:
		win->w = vout->win.w;
		win->field = vout->win.field;
		win->global_alpha = vout->win.global_alpha;
		break;
	case VOUT_CONTEXT_REAR:
		win->w = vout->extwin.w;
		win->field = vout->extwin.field;
		win->global_alpha = vout->extwin.global_alpha;
		break;
	default:
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d not support context %d\r\n", __func__
			, vout->vid, vout->context);
		mutex_unlock(&vout->lock);
		return -EINVAL;
	}
	mutex_unlock(&vout->lock);

	return 0;
}

static int vidioc_cropcap(struct file *file, void *fh,
			  struct v4l2_cropcap *cropcap)
{
	struct atc_vout_device *vout = fh;
	struct v4l2_pix_format *pix = NULL;

	if ((NULL == vout) || (NULL == cropcap)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or cropcap is NULL\r\n", __func__);
		return -EINVAL;
	}

	if (cropcap->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d type err %d\r\n", __func__, vout->vid, cropcap->type);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	pix = &vout->pix;
	/* Width and height are always even */
	cropcap->bounds.width = pix->width & ~1;
	cropcap->bounds.height = pix->height & ~1;

	atc_vout_default_crop(&vout->pix, &cropcap->defrect);
	cropcap->pixelaspect.numerator = 1;
	cropcap->pixelaspect.denominator = 1;
	mutex_unlock(&vout->lock);

	return 0;
}

static int vidioc_g_crop(struct file *file, void *fh, struct v4l2_crop *crop)
{
	struct atc_vout_device *vout = fh;

	if ((NULL == vout) || (NULL == crop)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or crop is NULL\r\n", __func__);
		return -EINVAL;
	}

	if (crop->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d type err %d\r\n", __func__, vout->vid, crop->type);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	crop->c = vout->crop;
	mutex_unlock(&vout->lock);

	return 0;
}

static int vidioc_s_crop(struct file *file, void *fh, const struct v4l2_crop *crop)
{
	struct atc_vout_device *vout = fh;
	struct v4l2_rect *dst_rect = NULL;
	int ret = 0;

	if ((NULL == vout) || (NULL == crop)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or crop is NULL\r\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	dst_rect = &vout->crop;
	if (((crop->c.left + crop->c.width) <= vout->pix.width) &&
		((crop->c.top + crop->c.height) <= vout->pix.height)) {
		if ((dst_rect->top != crop->c.top) || (dst_rect->left != crop->c.left) ||
		    (dst_rect->width != crop->c.width) || (dst_rect->height != crop->c.height)) {
			vout->crop = crop->c;
			vout->stateflags |= VOUT_SET_SRC_PARAMS;
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d update params (%d, %d, %d, %d)\r\n", __func__
				, vout->vid, dst_rect->left, dst_rect->top, dst_rect->width, dst_rect->height);
		}
	} else {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d update params (%d, %d, %d, %d) src %d %d\r\n", __func__
			, vout->vid, crop->c.left, crop->c.top, crop->c.width, crop->c.height
			, vout->pix.width, vout->pix.height);
		ret = -EINVAL;
	}
	mutex_unlock(&vout->lock);

	return ret;
}

static int vidioc_reqbufs(struct file *file, void *fh,
			  struct v4l2_requestbuffers *req)
{
	int ret = 0;
	unsigned int i, num_buffers = 0;
	struct atc_vout_device *vout = fh;
	struct videobuf_queue *q = NULL;

	if ((NULL == vout) || (NULL == req)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or req is NULL\r\n", __func__);
		return -EINVAL;
	}

	if (req->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d type err %d\r\n", __func__, vout->vid, req->type);
		return -EINVAL;
	}

	/* if memory is not mmp or userptr return error */
	if ((V4L2_MEMORY_MMAP != req->memory) &&
	    (V4L2_MEMORY_USERPTR != req->memory)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d memory err %d\r\n", __func__, vout->vid, req->memory);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);

	/* Cannot be requested when streaming is on */
	if (vout->streaming) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d no streaming\r\n", __func__, vout->vid);
		ret = -EBUSY;
		goto reqbuf_err;
	}

	q = &vout->vbq;
	/* If buffers are already allocated free them */
	if (q->bufs[0] && (V4L2_MEMORY_MMAP == q->bufs[0]->memory)) {
		if (vout->mmap_count) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d mmap_count %d\r\n", __func__
				, vout->vid, vout->mmap_count);
			ret = -EBUSY;
			goto reqbuf_err;
		}

		if (vout->format == VOUT_FMT_VIDEO) {
			for (i = num_buffers; i < vout->buffer_allocated; i++) {
				if (vout->buf_virt_addr[i]) {
					atc_vout_free_vdo_buffer(vout->buf_virt_addr[i], vout->buffer_size);
				}
				vout->buf_virt_addr[i] = 0;
				vout->buf_phy_addr[i] = 0;
			}
		} else {
			for (i = num_buffers; i < vout->buffer_allocated; i++) {
				if (vout->buf_virt_addr[i]) {
					atc_vout_free_osd_buffer(vout->buf_virt_addr[i], vout->buffer_size);
				}
				vout->buf_virt_addr[i] = 0;
				vout->buf_phy_addr[i] = 0;
			}
		}

		vout->buffer_allocated = num_buffers;
		videobuf_mmap_free(q);
	} else if (q->bufs[0] && (V4L2_MEMORY_USERPTR == q->bufs[0]->memory)) {
		if (vout->buffer_allocated) {
			videobuf_mmap_free(q);

			for (i = 0; i < vout->buffer_allocated; i++) {
				kfree(q->bufs[i]);
				q->bufs[i] = NULL;
			}

			vout->buffer_allocated = 0;
		}
	}

	/*store the memory type in data structure */
	vout->memory = req->memory;

	INIT_LIST_HEAD(&vout->dma_queue);

	mutex_unlock(&vout->lock);
	/* call videobuf_reqbufs api */
	ret = videobuf_reqbufs(q, req);
	mutex_lock(&vout->lock);
	if (ret < 0) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d ret %d\r\n", __func__, vout->vid, ret);
		goto reqbuf_err;
	}

	vout->buffer_allocated = req->count;

reqbuf_err:
	mutex_unlock(&vout->lock);

	return ret;
}

static int vidioc_querybuf(struct file *file, void *fh,
			   struct v4l2_buffer *b)
{
	struct atc_vout_device *vout = fh;

	if ((NULL == vout) || (NULL == b)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or b is NULL\r\n", __func__);
		return -EINVAL;
	}

	return videobuf_querybuf(&vout->vbq, b);
}

static int vidioc_qbuf(struct file *file, void *fh,
		       struct v4l2_buffer *buffer)
{
	struct atc_vout_device *vout = fh;
	struct videobuf_queue *q = NULL;
	int ret = -EINVAL;

	if ((NULL == vout) || (NULL == buffer)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout or buffer is NULL\r\n", __func__);
		return ret;
	}

	mutex_lock(&vout->lock);
	q = &vout->vbq;
	if ((V4L2_BUF_TYPE_VIDEO_OUTPUT != buffer->type) ||
	    (buffer->index >= vout->buffer_allocated) ||
	    (q->bufs[buffer->index]->memory != buffer->memory)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d buffer fail\r\n", __func__, vout->vid);
		goto done;
	}

	if (V4L2_MEMORY_USERPTR == buffer->memory) {
		if ((buffer->length < vout->pix.sizeimage) ||
		    (0 == buffer->m.userptr)) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d userptr fail\r\n", __func__, vout->vid);
			goto done;
		}
	}


	mutex_unlock(&vout->lock);
	ret = videobuf_qbuf(q, buffer);
	mutex_lock(&vout->lock);
	if (ret) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d videobuf_qbuf ret %d\r\n", __func__, vout->vid, ret);
		goto done;
	}

	if (!vout->datavalid) {
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d data enable\r\n", __func__, vout->vid);
		vout->datavalid = true;
	}

	ret = atc_vout_qbuf(vout, buffer->index);
	if (ret) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d atc_vout_qbuf ret %d\r\n", __func__, vout->vid, ret);
		goto done;
	}

	vout->qbufcnt++;

	if (dump_buffer_flag && vout->enable) {
		if (!dump_maxcount || (dump_count < dump_maxcount)) {
			VOUT_PRINT(VOUT_LOG_LVL_DBG, "dev %d dumpbuffer\r\n", vout->vid);
			atc_vout_dump_buffer(vout, NULL, dump_options, dump_count);
			dump_count++;
			if (!dump_maxcount && (DEFAULT_DUMP_MAX_COUNT == dump_count)) {
				dump_count = 0;
			} else if (dump_count == dump_maxcount) {
				dump_buffer_flag = 0;
				dump_count = 0;
			}
		}
	}

done:
	mutex_unlock(&vout->lock);

	return ret;
}

static int vidioc_dqbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct atc_vout_device *vout = fh;
	struct videobuf_queue *q = NULL;
	int ret;
	u32 addr;
	unsigned long size;
	struct videobuf_buffer *vb;

	if ((NULL == vout) || (NULL == b)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout or b is NULL\r\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	q = &vout->vbq;
	vb = q->bufs[b->index];
	if ((NULL == vb) || (!vout->streaming)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d vb is NULL and streaming %d\r\n", __func__, vout->vid
			, vout->streaming);
		mutex_unlock(&vout->lock);
		return -EINVAL;
	}

	/*VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s enter dev %d id %d count %d\r\n", __func__, vout->vid
		, b->index, vout->dqbufcnt);*/
	/* Add unlock mutex for dqbuf block mode*/
	mutex_unlock(&vout->lock);
	if (file->f_flags & O_NONBLOCK) {
		/* Call videobuf_dqbuf for non blocking mode */
		ret = videobuf_dqbuf(q, (struct v4l2_buffer *)b, 1);
	} else {
		/* Call videobuf_dqbuf for  blocking mode */
		ret = videobuf_dqbuf(q, (struct v4l2_buffer *)b, 0);
	}
	mutex_lock(&vout->lock);

	vout->dqbufcnt++;
	addr = (unsigned long) vout->buf_phy_addr[vb->i];
	size = (unsigned long) vb->size;
	dma_unmap_single(vout->vid_dev->v4l2_dev.dev,  addr, size, DMA_TO_DEVICE);

	if (ret) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d ret %d\r\n", __func__, vout->vid, ret);
	}/* else {
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d id %d count %d\r\n", __func__, vout->vid
			, b->index, vout->dqbufcnt);
	}*/
	mutex_unlock(&vout->lock);

	return ret;
}

static int vidioc_streamon(struct file *file, void *fh, enum v4l2_buf_type i)
{
	struct atc_vout_device *vout = fh;
	struct videobuf_queue *q = &vout->vbq;
	int ret = 0;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh is NULL\r\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);

	if (vout->streaming) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d streaming on already\r\n", __func__, vout->vid);
		ret = -EBUSY;
		goto streamon_err;
	}

	mutex_unlock(&vout->lock);
	ret = videobuf_streamon(q);
	mutex_lock(&vout->lock);
	if (ret) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d streaming on ret %d\r\n", __func__, vout->vid, ret);
		goto streamon_err;
	}

	if (list_empty(&vout->dma_queue)) {
		ret = -EIO;
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d queue empty\r\n", __func__, vout->vid);
		goto streamon_err1;
	}

	/* Get the next frame from the buffer queue */
	vout->next_frm = vout->cur_frm = list_entry(vout->dma_queue.next, struct videobuf_buffer, queue);
	/* Remove buffer from the buffer queue */
	list_del(&vout->cur_frm->queue);
	/* Mark state of the current frame to active */
	vout->cur_frm->state = VIDEOBUF_ACTIVE;
	/* Initialize field_id and started member */
	vout->field_id = 0;

	/* set flag here. Next QBUF will start DMA */
	vout->streaming = true;

	vout->first_int = 1;

	ret = atc_dispc_register_isr(atc_vout_isr, vout);

streamon_err1:
	if (ret) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d ret %d and call streamoff\r\n", __func__, vout->vid, ret);
		mutex_unlock(&vout->lock);
		ret = videobuf_streamoff(q);
		mutex_lock(&vout->lock);
	} else {
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d ret %d\r\n", __func__, vout->vid, ret);
	}

streamon_err:
	mutex_unlock(&vout->lock);

	return ret;
}

static int vidioc_streamoff(struct file *file, void *fh, enum v4l2_buf_type i)
{
	struct atc_vout_device *vout = fh;
	int ret = 0,devid = -1;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh is NULL\r\n", __func__);
		return -EINVAL;
	}

	if (!vout->streaming) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d no streaming\r\n", __func__, vout->vid);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	vout->streaming = false;
	vout->datavalid = false;

	ret = atc_dispc_unregister_isr(atc_vout_isr, vout);
	if (vout->format == VOUT_FMT_OSD) {

	} else {
		devid = atc_vout_get_backend_dev(vout, VOUT_CONTEXT_FRONT);
	        if ((devid >= 0) && (devid < MAX_VOUT_DEV)) {
			VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d exit runing vout %d\r\n", __func__, vout->vid, devid);
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d  not exit runing vout \r\n", __func__, vout->vid);
			if (atc_vout_can_use_hwdev(vout, VOUT_HW_VDP1)) {
				atc_vout_ioctl(vout,VIDIOC_STREAMOFF);
			}
		}
	}

	INIT_LIST_HEAD(&vout->dma_queue);
	mutex_unlock(&vout->lock);
	ret = videobuf_streamoff(&vout->vbq);
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d return %d\r\n", __func__, vout->vid, ret);

	return ret;
}

static int vidioc_s_fbuf(struct file *file, void *fh, const struct v4l2_framebuffer *fbuf)
{
	u32 enable = 0, key_type = 0;
	struct atc_vout_device *vout = fh;

	if ((NULL == vout) || (NULL == fbuf)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or fbuf is NULL\r\n", __func__);
		return -EINVAL;
	}

	if (fbuf->flags & V4L2_FBUF_FLAG_SRC_CHROMAKEY) {
		key_type = SRC_COLOR_KEY;
		enable = 1;
	} else if (fbuf->flags & V4L2_FBUF_FLAG_CHROMAKEY) {
		key_type = DST_COLOR_KEY;
		enable = 1;
	} else {
		enable = 0;
	}

	mutex_lock(&vout->lock);
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d enable %d, key %d, key 0x%x\r\n", __func__, vout->vid
		, enable, key_type, vout->win.chromakey);
	set_colorkey(key_type, vout->win.chromakey, enable);
	mutex_unlock(&vout->lock);

	return 0;
}

static int vidioc_g_fbuf(struct file *file, void *fh, struct v4l2_framebuffer *fbuf)
{
	if (NULL == fbuf) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or fbuf is NULL\r\n", __func__);
		return -EINVAL;
	}

	/* The video overlay must stay within the framebuffer and can't be
	   positioned independently. */
	fbuf->flags = V4L2_FBUF_FLAG_OVERLAY;
	fbuf->capability = V4L2_FBUF_CAP_LOCAL_ALPHA | V4L2_FBUF_CAP_CHROMAKEY | V4L2_FBUF_CAP_SRC_CHROMAKEY;
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s\r\n", __func__);

	return 0;
}

static int vidioc_g_ctrl(struct file *file, void *fh, struct v4l2_control *ctl)
{
	struct atc_vout_device *vout = fh;
	int ret = 0, idx = 0;

	if ((NULL == vout) || (NULL == ctl)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or ctl is NULL\r\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	idx = vout->context;
	switch (ctl->id) {
	case V4L2_CID_BRIGHTNESS: {
		ctl->value = vout->brightness[idx];
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d context %d brightness %d\r\n", __func__, vout->vid
			, idx, ctl->value);
		break;
	}
	case V4L2_CID_CONTRAST: {
		ctl->value = vout->contrast[idx];
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d context %d contrast %d\r\n", __func__, vout->vid
			, idx, ctl->value);
		break;
	}
	case V4L2_CID_SATURATION: {
		ctl->value = vout->saturation[idx];
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d context %d saturation %d\r\n", __func__, vout->vid
			, idx, ctl->value);
		break;
	}
	case V4L2_CID_HUE: {
		ctl->value = vout->hue[idx];
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d context %d hue %d\r\n", __func__, vout->vid
			, idx, ctl->value);
		break;
	}
	case V4L2_CID_YUVGAIN: {
		ctl->value = VOUT_MERGE_YUV_GAIN(vout->y_gain[idx], vout->u_gain[idx]
			, vout->v_gain[idx]);
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d context %d YUV gain %d\r\n", __func__, vout->vid
			, idx, ctl->value);
		break;
	}
	case V4L2_CID_CONTEXT: {
		ctl->value = vout->context;
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d context %d\r\n", __func__, vout->vid, ctl->value);
		break;
	}
	case V4L2_CID_OUTPUT: {
		ctl->value = vout->output;
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d output %d\r\n", __func__, vout->vid, ctl->value);
		break;
	}
	default: {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d not support id %d value %d\r\n", __func__
			, vout->vid, ctl->id, ctl->value);
		ret = -EINVAL;
		break;
	}
	}
	mutex_unlock(&vout->lock);

	return ret;
}

static int vidioc_s_ctrl(struct file *file, void *fh, struct v4l2_control *ctl)
{
	struct atc_vout_device *vout = fh;
	int ret = 0, idx = 0;

	if ((NULL == vout) || (NULL == ctl)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or ctl is NULL\r\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	idx = vout->context;
	switch (ctl->id) {
	case V4L2_CID_BRIGHTNESS: {
		if (ctl->value < VOUT_BRIGHTNESS_MAX) {
			if (vout->brightness[idx] != ctl->value) {
				vout->brightness[idx] = ctl->value;
				VcpSetBrightness(idx, ctl->value);
				VOUT_PRINT(VOUT_LOG_LVL_TRACE, "%s dev %d context %d brightness %d\r\n", __func__
					, idx, vout->vid, ctl->value);
			}
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d brightness %d\r\n", __func__, vout->vid, ctl->value);
			ret = -EINVAL;
		}
		break;
	}
	case V4L2_CID_CONTRAST: {
		if (ctl->value < VOUT_CONTRAST_MAX) {
			if (vout->contrast[idx] != ctl->value) {
				vout->contrast[idx] = ctl->value;
				VcpSetContrast(idx, ctl->value);
				VOUT_PRINT(VOUT_LOG_LVL_TRACE, "%s dev %d context %d contrast %d\r\n", __func__
					, idx, vout->vid, ctl->value);
			}
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d contrast %d\r\n", __func__, vout->vid, ctl->value);
			ret = -EINVAL;
		}
		break;
	}
	case V4L2_CID_SATURATION: {
		if (ctl->value < VOUT_SATURATION_MAX) {
			if (vout->saturation[idx] != ctl->value) {
				vout->saturation[idx] = ctl->value;
				VcpSetSaturation(idx, ctl->value);
				VOUT_PRINT(VOUT_LOG_LVL_TRACE, "%s dev %d context %d saturation %d\r\n", __func__
					, idx, vout->vid, ctl->value);
			}
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d saturation %d\r\n", __func__, vout->vid, ctl->value);
			ret = -EINVAL;
		}
		break;
	}
	case V4L2_CID_HUE: {
		if (ctl->value < VOUT_HUE_MAX) {
			if (vout->hue[idx] != ctl->value) {
				vout->hue[idx] = ctl->value;
				VcpSetHue(idx, ctl->value);
				VOUT_PRINT(VOUT_LOG_LVL_TRACE, "%s dev %d context %d hue %d\r\n", __func__
					, idx, vout->vid, ctl->value);
			}
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d hue %d\r\n", __func__, vout->vid, ctl->value);
			ret = -EINVAL;
		}
		break;
	}
	case V4L2_CID_YUVGAIN: {
		u32 y = VOUT_GET_Y_GAIN(ctl->value);
		u32 u = VOUT_GET_U_GAIN(ctl->value);
		u32 v = VOUT_GET_V_GAIN(ctl->value);

		if ((y < VOUT_GAIN_MAX) && (u < VOUT_GAIN_MAX) && (v < VOUT_GAIN_MAX)) {
			if ((vout->y_gain[idx] != y) && (vout->u_gain[idx] != u) && (vout->v_gain[idx] != v)) {
				vout->y_gain[idx] = y;
				vout->u_gain[idx] = u;
				vout->v_gain[idx] = v;
				VcpSetYGain(idx, y);
				VcpSetUGain(idx, u);
				VcpSetVGain(idx, v);
				VOUT_PRINT(VOUT_LOG_LVL_TRACE, "%s dev %d context %d YUV gain %d\r\n", __func__
					, idx, vout->vid, ctl->value);
			}
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d YUV gain %d\r\n", __func__, vout->vid, ctl->value);
			ret = -EINVAL;
		}
		break;
	}
	case V4L2_CID_OUTPUTBLACK: {
		if (ctl->value) {
			vout->stateflags |= VOUT_SET_BG_OUTPUT;
		} else {
			vout->stateflags &= ~VOUT_SET_BG_OUTPUT;
		}
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d OUTPUT BLACK %d\r\n", __func__, vout->vid, ctl->value);
		atc_vout_ioctl(vout, STIOC_SET_FMT_BLACK);
		break;
	}
	case V4L2_CID_ZORDER: {
		u32 src_plane = 0, dst_plane = 0, i = 0;
		u32 order = ctl->value, tmp = 0, value = 0;

		src_plane = vPmxHalGetPlaneOrder(0);
		value = (src_plane >> (4 * order)) & PMX_PLANE_MASK;

		if (value == order) {
			VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d not change org %d, new %d %x\r\n", __func__, vout->vid
				, (int)value, (int)order, (unsigned int)src_plane);
			break;
		}

		/*VOUT_PRINT(VOUT_LOG_LVL_DBG, "vidioc_s_ctrl -> V4L2_CID_SET_ZORDER, %d %d %x\r\n"
			, value, order, src_plane);*/

		for (i = 0; i < PMX_PLANE_MAX; i++) {
			if (PMX_PLANE_VIDEO == ((src_plane >> 4 * i) & PMX_PLANE_MASK)) {
				VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d find layer %d\r\n", __func__, vout->vid, i);
				break;
			}
		}

		if (i == PMX_PLANE_MAX) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d error no video layer\r\n", __func__, vout->vid);
			ret = -EINVAL;
			break;
		}

		tmp = (src_plane >> 4 * i) & PMX_PLANE_DST_MASK;
		dst_plane = src_plane;

		if (i < order) {
			for (; i < order; i++) {
				value = (src_plane >> (4 * (i + 1))) & PMX_PLANE_DST_MASK;
				dst_plane &= ~(PMX_PLANE_DST_MASK << (4 * i));
				dst_plane |= value << (4 * i);
			}
		} else {
			for (; i > order; i--) {
				value = (src_plane >> (4 * (i - 1))) & PMX_PLANE_DST_MASK;
				dst_plane &= ~(PMX_PLANE_DST_MASK << (4 * i));
				dst_plane |= value << (4 * i);
			}
		}

		dst_plane &= ~(PMX_PLANE_DST_MASK << (4 * i));
		dst_plane |= tmp << (4 * i);
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d get video layer %d, dst order %x\r\n", __func__, vout->vid
			, (int)tmp, (unsigned int)dst_plane);
		vPmxHalSetPlaneOrder(0, dst_plane);
		break;
	}
	case V4L2_CID_CONTEXT: {
		if (ctl->value <= VOUT_CONTEXT_REAR) {
			if (vout->context != ctl->value) {
				vout->context = ctl->value;
				VOUT_PRINT(VOUT_LOG_LVL_TRACE, "%s dev %d context %d\r\n", __func__
					, vout->vid, ctl->value);
			}
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d context %d\r\n", __func__, vout->vid, ctl->value);
			ret = -EINVAL;
		}
		break;
	}
	case V4L2_CID_OUTPUT: {
		if (ctl->value <= VOUT_OUTPUT_FRONTREAR) {
			if (vout->output != ctl->value) {
				ret = atc_vout_set_output(vout, ctl->value);
			}
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d output %d -> %d\r\n", __func__, vout->vid
				, vout->output, ctl->value);
			ret = -EINVAL;
		}
		break;
	}
	case V4L2_CID_COLOR_RANGE: {
		u32 range = ctl->value;

		if (vout->range == range) {
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d range not change old %u, new %u\r\n",
				__func__, vout->vid, vout->range, range);
			break;
		}

		vout->range = range;
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d new range value %u\n",
			__func__, vout->vid, vout->range);
		ret = atc_vout_ioctl(vout, STIOC_SET_COLOR_RANGE);
		break;
	}
	case V4L2_CID_COLOR_ENCODEING: {
		u32 enconding = ctl->value;

		if (vout->encoding == enconding) {
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d encoding not change old %u, new %u\r\n",
				__func__, vout->vid, vout->encoding, enconding);
			break;
		}

		vout->encoding = enconding;
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d new encoding value %u\n",
			__func__, vout->vid, vout->encoding);
		ret = atc_vout_ioctl(vout, STIOC_SET_COLOR_ENCODING);
		break;
	}

	default: {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d not support id %d value %d\r\n", __func__, vout->vid
			, ctl->id, ctl->value);
		ret = -EINVAL;
		break;
	}
	}
	mutex_unlock(&vout->lock);

	return ret;
}

int vidioc_overlay(struct file *file, void *fh, unsigned int on)
{
	struct atc_vout_device *vout = fh;
	int ret = 0;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or vid_dev is NULL\r\n", __func__);
		ret = -EINVAL;
		goto done;
	}

	VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s enter dev %d overlay %d data %d\r\n", __func__, vout->vid, on
		, vout->datavalid);
	mutex_lock(&vout->lock);
	if (on) {
		if (vout->enable) {
			VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d enable already\r\n", __func__, vout->vid);
			goto done;
		}
		vout->enable = true;
		if (!(vout->stateflags & VOUT_SET_PARAMS_MASK)) {
			vout->stateflags |= VOUT_SET_SRC_PARAMS;
		}
		if (vout->format == VOUT_FMT_OSD) {
			ret = atc_vout_osd_on(vout);
		} else {
			ret = atc_vout_show_video(vout);
		}
	} else {
		if (!vout->enable) {
			VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d disable but not enable yet\r\n", __func__, vout->vid);
			goto done;
		}
		if (vout->format == VOUT_FMT_OSD) {
			ret = atc_vout_osd_off(vout);
		} else {
			ret = atc_vout_hide_video(vout);
		}
		vout->enable = false;
		/*vout->datavalid = false;*/
	}
	VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d overlay %d data %d return %d\r\n", __func__, vout->vid, on
		, vout->datavalid, ret);

done:
	mutex_unlock(&vout->lock);

	return ret;
}

static int vidioc_g_priority(struct file *file, void *fh, enum v4l2_priority *p)
{
	struct atc_vout_device *vout = fh;

	if ((NULL == vout) || (NULL == p)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev input arg error\r\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	*p = vout->prio;
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d prio %d\r\n", __func__, vout->vid, *p);
	mutex_unlock(&vout->lock);

	return 0;
}

static int vidioc_s_priority(struct file *file, void *fh, enum v4l2_priority p)
{
	struct atc_vout_device *vout = fh;
	int ret = 0;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&vout->lock);
	if (p != vout->prio) {
		if ((V4L2_PRIORITY_RECORD == p) && (V4L2_PRIORITY_RECORD == atc_vout_prio_max(vout))) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d high priority already set\r\n", __func__, vout->vid);
			ret = -EBUSY;
		} else {
			if (p <= V4L2_PRIORITY_RECORD) {
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d from %d to %d\r\n", __func__
					, vout->vid, vout->prio, p);
				vout->prio = p;
			} else {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d priority %d is invalid\r\n", __func__
					, vout->vid, p);
				ret = -EINVAL;
			}
		}
	}
	mutex_unlock(&vout->lock);

	return ret;
}

static long vidioc_default(struct file *file, void *fh, bool valid_prio, unsigned int cmd, void *arg)
{
	struct atc_vout_device *vout = fh;
	long ret = -EINVAL;

	if ((NULL == vout) || (NULL == arg)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s fh or arg is NULL\r\n", __func__);
		return -EINVAL;
	}

	switch (cmd) {
	case VIDIOC_QUERY_VALID_DATA_RECT: {
		struct VOUT_PARAM *vout_param = NULL;
		struct v4l2_pix_format *pix = NULL;
		struct v4l2_rect *out = arg;
		void *y_va_addr, *c_va_addr;
		VIDEO_INFO_T vid_info;
		RECT_HV rect;
		u32 size;
		void *video_buf_y, *video_buf_c;

		mutex_lock(&vout->lock);
		if ((vout->format == VOUT_FMT_VIDEO) && (vout->datavalid) && (vout->cur_frm != vout->next_frm)) {
			pix = &vout->pix;

			vout_param = (struct VOUT_PARAM *)vout->buf_virt_addr[vout->cur_frm->i];
			if (NULL == vout_param) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d param error\r\n", __func__, vout->vid);
				goto done;
			}
			mutex_unlock(&vout->lock);

			switch (pix->pixelformat) {
			case v4l2_fourcc('A', 'V', '1', '2'):
				vid_info.u4Mode = MODE_LINE;
				break;
			case v4l2_fourcc('A', 'M', '1', '2'):
				vid_info.u4Mode = MODE_BLOCK;
				break;
			default:
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d fmt %x not support\r\n", __func__
					, vout->vid, vout->pix.pixelformat);
				goto done;
			}

			size = pix->width * pix->height;
			y_va_addr = ioremap(vout_param->y_phy_addr, size);
			if (NULL == y_va_addr) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d remap y error\r\n", __func__, vout->vid);
				ret = -ENOMEM;
				goto done;
			}
			c_va_addr = ioremap(vout_param->c_phy_addr, size >> 1);
			if (NULL == c_va_addr) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d remap c error\r\n", __func__, vout->vid);
				iounmap(y_va_addr);
				ret = -ENOMEM;
				goto done;
			}

			video_buf_y = kmalloc(size, 0);
			if(!video_buf_y){
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "kmalloc for video_buf_y fail!\n");
				iounmap(y_va_addr);
				iounmap(c_va_addr);
				ret = -ENOMEM;
				goto done;
			}

			video_buf_c = kmalloc(size >> 1, 0);
			if(!video_buf_c){
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "kmalloc for video_buf_c fail!\n");
				iounmap(y_va_addr);
				iounmap(c_va_addr);
				kfree(video_buf_y);
				ret = -ENOMEM;
				goto done;
			}

			memcpy(video_buf_y, y_va_addr, size);
			memcpy(video_buf_c, c_va_addr, size/2);
			/*Call get active rect api*/
			vid_info.u4YVaAddr = (u32)video_buf_y;
			vid_info.u4CVaAddr = (u32)video_buf_c;
			vid_info.u4Width = pix->width;
			vid_info.u4Height = pix->height;
			GR_GetActiveRect(vid_info, &rect);
			out->left = rect.left;
			out->top = rect.top;
			out->width = rect.right - rect.left;
			out->height = rect.bottom - rect.top;
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d y/c 0x%x 0x%x w/h %d %d type %d out (%d %d %d %d)\r\n"
				, __func__, vout->vid, (u32)vid_info.u4YVaAddr, (u32)vid_info.u4CVaAddr
				, vid_info.u4Width, vid_info.u4Height, vid_info.u4Mode, out->left, out->top
				, out->width, out->height);

			kfree(video_buf_y);
			kfree(video_buf_c);

			iounmap(y_va_addr);
			iounmap(c_va_addr);
			ret = 0;
		} else {
			mutex_unlock(&vout->lock);
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d cur_frm %x format %d error\r\n", __func__
				, vout->vid, (u32)vout->cur_frm, vout->format);
		}
		break;
	}
	case VIDIOC_SCREEN_PHY_SIZE: {
		unsigned int *value = (unsigned int *)arg;
		ret = atc_vout_ioctl(vout, STIOC_GET_PHY_ACTIVE);
		if (ret < 0) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s get disp physical size error, default 0\n", __func__);
			*value = 0;
			break;
		}
		*value |= vout->screen_size.width << 16;
		*value |= vout->screen_size.height;
		ret = 0;
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s screen resolution: %x\n", __func__, *value);
		break;
	}
	default:
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d cmd %x not support\r\n", __func__, vout->vid, cmd);
	}

done:

	return ret;
}

static const struct v4l2_ioctl_ops vout_ioctl_ops = {
	.vidioc_querycap              = vidioc_querycap,
	.vidioc_g_fmt_vid_out         = vidioc_g_fmt_vid_out,
	.vidioc_s_fmt_vid_out         = vidioc_s_fmt_vid_out,
	.vidioc_s_fbuf                = vidioc_s_fbuf,
	.vidioc_g_fbuf                = vidioc_g_fbuf,
	.vidioc_cropcap               = vidioc_cropcap,
	.vidioc_s_fmt_vid_out_overlay = vidioc_s_fmt_vid_overlay,
	.vidioc_g_fmt_vid_out_overlay = vidioc_g_fmt_vid_overlay,
	.vidioc_cropcap               = vidioc_cropcap,
	.vidioc_g_crop                = vidioc_g_crop,
	.vidioc_s_crop                = vidioc_s_crop,
	.vidioc_reqbufs               = vidioc_reqbufs,
	.vidioc_querybuf              = vidioc_querybuf,
	.vidioc_qbuf                  = vidioc_qbuf,
	.vidioc_dqbuf                 = vidioc_dqbuf,
	.vidioc_streamon              = vidioc_streamon,
	.vidioc_streamoff             = vidioc_streamoff,
	.vidioc_g_ctrl                = vidioc_g_ctrl,
	.vidioc_s_ctrl                = vidioc_s_ctrl,
	.vidioc_overlay               = vidioc_overlay,
	.vidioc_g_priority            = vidioc_g_priority,
	.vidioc_s_priority            = vidioc_s_priority,
	.vidioc_default               = vidioc_default,
};

static const struct v4l2_file_operations atc_vout_fops = {
	.owner		= THIS_MODULE,
	.poll		= atc_vout_poll,
	.unlocked_ioctl = video_ioctl2,
	.mmap		= atc_vout_mmap,
	.open		= atc_vout_open,
	.release	= atc_vout_release,
};

/* Init functions used during driver initialization */
/* Initial setup of video_data */
static int __init atc_vout_setup_video_data(struct atc_vout_device *vout)
{
	struct video_device *vfd;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return -EINVAL;
	}

	atc_vout_reset_video_data(vout);

	/* initialize the video_device struct */
	vfd = vout->vfd = video_device_alloc();
	if (!vfd) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d allocate video device struct fail\r\n", __func__, vout->vid);
		return -ENOMEM;
	}

	vfd->release = video_device_release;
	vfd->ioctl_ops = &vout_ioctl_ops;

	strlcpy(vfd->name, VOUT_NAME, sizeof(vfd->name));

	vfd->fops = &atc_vout_fops;
	vfd->v4l2_dev = &vout->vid_dev->v4l2_dev;
	vfd->vfl_dir = VFL_DIR_TX;
	mutex_init(&vout->lock);
	vfd->minor = -1;

	return 0;

}

/* Setup video buffers */
static int __init atc_vout_setup_video_bufs(struct platform_device *pdev,
					    int vid_num)
{
	u32 numbuffers;
	int ret = 0, i;
	struct atc_vout_device *vout = NULL;
	struct v4l2_device *v4l2_dev = NULL;
	struct atcvideo_device *vid_dev = NULL;

	if (NULL == pdev) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s pdev is NULL\r\n", __func__);
		return -EINVAL;
	}

	v4l2_dev = platform_get_drvdata(pdev);
	if (NULL == v4l2_dev) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s v4l2_dev error\r\n", __func__);
		return -EINVAL;
	}
	vid_dev = container_of(v4l2_dev, struct atcvideo_device, v4l2_dev);
	if (NULL == vid_dev) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vid_dev error\r\n", __func__);
		return -EINVAL;
	}
	vout = vid_dev->vouts[vid_num];
	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout error\r\n", __func__);
		return -EINVAL;
	}

	if (vout->format == VOUT_FMT_VIDEO) {
		numbuffers = DEFAULT_BUF_NUM;
		vout->buffer_size = DEFAULT_BUF_SIZE;

		for (i = 0; i < numbuffers; i++) {
			vout->buf_virt_addr[i] =
				atc_vout_alloc_vdo_buffer(vout->buffer_size, (u32 *)&vout->buf_phy_addr[i]);
			if (!vout->buf_virt_addr[i]) {
				numbuffers = i;
				ret = -ENOMEM;
				goto free_buffers;
			}
		}
	} else {
		numbuffers = DEFAULT_BUF_NUM;
		vout->buffer_size = DEFAULT_BUF_SIZE;

		for (i = 0; i < numbuffers; i++) {
			vout->buf_virt_addr[i] =
				atc_vout_alloc_osd_buffer(vout->buffer_size, (u32 *)&vout->buf_phy_addr[i]);
			if (!vout->buf_virt_addr[i]) {
				numbuffers = i;
				ret = -ENOMEM;
				goto free_buffers;
			}
		}
		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d not support osd format\r\n", __func__, vout->vid);
	}

	return ret;

free_buffers:
	VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d free_buffers\r\n", __func__, vout->vid);

	if (vout->format == VOUT_FMT_VIDEO) {
		for (i = 0; i < numbuffers; i++) {
			if (vout->buf_virt_addr[i]) {
				atc_vout_free_vdo_buffer(vout->buf_virt_addr[i], vout->buffer_size);
			}

			vout->buf_virt_addr[i] = 0;
			vout->buf_phy_addr[i] = 0;
		}
	} else {
		for (i = 0; i < numbuffers; i++) {
			if (vout->buf_virt_addr[i]) {
				atc_vout_free_osd_buffer(vout->buf_virt_addr[i], vout->buffer_size);
			}

			vout->buf_virt_addr[i] = 0;
			vout->buf_phy_addr[i] = 0;
		}
	}

	return ret;
}

/* Create video out devices */
static int __init atc_vout_create_video_devices(struct platform_device *pdev)
{
	int ret = 0, k = 0;
	struct v4l2_device *v4l2_dev = NULL;
	struct atcvideo_device *vid_dev = NULL;
	struct atc_vout_device *vout = NULL;
	struct video_device *vfd = NULL;

	if (NULL == pdev) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev is NULL\r\n", __func__);
		return -EINVAL;
	}
	v4l2_dev = platform_get_drvdata(pdev);
	if (NULL == v4l2_dev) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s v4l2_dev is NULL\r\n", __func__);
		return -EINVAL;
	}
	vid_dev = container_of(v4l2_dev, struct atcvideo_device, v4l2_dev);
	if (NULL == vid_dev) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vid_dev is NULL\r\n", __func__);
		return -EINVAL;
	}

	for (k = 0; k < MAX_VOUT_DEV; k++) {
		vout = kzalloc(sizeof(struct atc_vout_device), GFP_KERNEL);
		if (NULL == vout) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d could not allocate memory\r\n", __func__, k);
			return -ENOMEM;
		}

		vout->vid = k;
		vid_dev->vouts[k] = vout;
		vout->vid_dev = vid_dev;

		/* Setup the default configuration for the video devices */
		if (atc_vout_setup_video_data(vout) != 0) {
			ret = -ENOMEM;
			goto error;
		}

		/* Allocate default number of buffers for the video streaming */
		if (atc_vout_setup_video_bufs(pdev, k) != 0) {
			ret = -ENOMEM;
			goto error1;
		}

		/* Register the Video device with V4L2 */
		vfd = vout->vfd;
		if (video_register_device(vfd, VFL_TYPE_GRABBER, VOUT_DEV_BASE + k) < 0) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d register Video for Linux device fail\r\n"
				, __func__, vout->vid);
			vfd->minor = -1;
			ret = -ENODEV;
			goto error2;
		}

		video_set_drvdata(vfd, vout);

		VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d registered and initialized\r\n", __func__, vout->vid);

		continue;
error2:
		atc_vout_free_buffers(vout);
error1:
		video_device_release(vfd);
error:
		kfree(vout);
	}

	return ret;
}
/* Driver functions */
static void atc_vout_cleanup_device(struct atc_vout_device *vout)
{
	struct video_device *vfd;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return;
	}

	vfd = vout->vfd;

	if (vfd) {
		if (!video_is_registered(vfd)) {
			/*
			 * The device was never registered, so release the
			 * video_device struct directly.
			 */
			video_device_release(vfd);
		} else {
			/*
			 * The unregister function will release the video_device
			 * struct as well as unregistering it.
			 */
			video_unregister_device(vfd);
		}
	}

	atc_vout_free_buffers(vout);

	mutex_destroy(&vout->lock);

	kfree(vout);
}

static int atc_vout_remove(struct platform_device *pdev)
{
	struct v4l2_device *v4l2_dev = NULL;
	struct atcvideo_device *vid_dev = NULL;
	int k = 0;

	if (NULL == pdev) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev is NULL\r\n", __func__);
		return -EINVAL;
	}
	v4l2_dev = platform_get_drvdata(pdev);
	if (NULL == v4l2_dev) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s v4l2_dev is NULL\r\n", __func__);
		return -EINVAL;
	}
	vid_dev = container_of(v4l2_dev, struct atcvideo_device, v4l2_dev);
	if (NULL == vid_dev) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vid_dev is NULL\r\n", __func__);
		return -EINVAL;
	}

	v4l2_device_unregister(v4l2_dev);

	for (k = 0; k < MAX_VOUT_DEV; k++) {
		if (vid_dev->vouts[k]) {
			atc_vout_cleanup_device(vid_dev->vouts[k]);
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d is NULL\r\n", __func__, k);
		}
	}

	kfree(vid_dev);

	return 0;
}

static int atc_vout_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct atcvideo_device *vid_dev = NULL;
	struct reserved_mem *vm_mem;
	static void * va;

	MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);

	of_reserved_mem_device_init(&(pdev->dev));
	vm_mem = (struct reserved_mem *)(pdev->dev.cma_area);
	if (!vm_mem || (vm_mem->base == 0) || (vm_mem->size == 0)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "get memory failed %x\r\n", (unsigned int)vm_mem);
		goto probe_err;
	}
	//vbuf_va = ioremap(vm_mem->base, vm_mem->size);
	vbuf_va = (void *)phys_to_virt(vm_mem->base);
	if (vbuf_va == NULL) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "ioremap 1 failed %x\r\n", (unsigned int)vbuf_va);
		goto probe_err;
	}
	set_pool_param(vm_mem->base, (unsigned int)vbuf_va, vm_mem->size - VOUT_OSD_BUF_SIZE);
	osd_buf_pa = vm_mem->base + vm_mem->size - VOUT_OSD_BUF_SIZE;
	osd_buf_va = (u32)vbuf_va + vm_mem->size - VOUT_OSD_BUF_SIZE;
	VOUT_PRINT(VOUT_LOG_LVL_INFO, "get memory %s vdo 0x%x va %x size %d osd 0x%x va 0x%x size %d\r\n", vm_mem->name
		, vm_mem->base, (u32)vbuf_va, vm_mem->size / SZ_1M, osd_buf_pa, osd_buf_va, VOUT_OSD_BUF_SIZE / SZ_1M);

	vid_dev = kzalloc(sizeof(struct atcvideo_device), GFP_KERNEL);
	if (vid_dev == NULL) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s kzalloc fail\r\n", __func__);
		ret = -ENOMEM;
		goto probe_err;
	}

	if (v4l2_device_register(&pdev->dev, &vid_dev->v4l2_dev) < 0) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s v4l2_device_register failed\r\n", __func__);
		ret = -ENODEV;
		goto probe_err1;
	}
	mutex_init(&interface_lock);
	ret = atc_vout_create_video_devices(pdev);
	if (ret == 0) {
		return ret;
	}

	VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s atc_vout_create_video_devices failed ret %d\r\n", __func__, ret);
	v4l2_device_unregister(&vid_dev->v4l2_dev);

probe_err1:
	if (va) {
		iounmap(va);
	}

	kfree(vid_dev);
probe_err:

	return ret;
}

static const struct of_device_id vout_of_match[] = {
	{.compatible = "Autochips,vout",},
	{}
};

static struct platform_driver atc_vout_driver = {
	.driver = {
		.name = VOUT_NAME,
		.of_match_table = vout_of_match,
	},
	.probe = atc_vout_probe,
	.remove = atc_vout_remove,
};

static int debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;

	return 0;
}

static ssize_t debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	char s[128] = "";
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s \r\n", __func__);
	sprintf(s, "vout dumpbuffer flag:%d dump_options:%d dump_maxcount:%d path:/media/sdcard/vout/ \r\n",
		dump_buffer_flag, dump_options, dump_maxcount);

	/* following statement similar to
	 * return copy_to_user(ubuf, s, strlen(s)); */
	return simple_read_from_buffer(ubuf, count, ppos, s, strlen(s));
}

static ssize_t debug_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	const int debug_bufmax = 512 - 1;
	size_t ret;
	char cmd_buffer[512];
	VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s \r\n", __func__);

	ret = count;

	if (count > debug_bufmax)
		count = debug_bufmax;

	if (copy_from_user(&cmd_buffer, ubuf, count))
		return -EFAULT;

	cmd_buffer[count] = 0;

	if (strncmp(cmd_buffer, "dumpbuffer", 10) == 0) {
		ret = sscanf(cmd_buffer + 10, " %d %d %d", &dump_buffer_flag, &dump_options, &dump_maxcount);
		if (ret < 0) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s scanf input parameters error \r\n", __func__);
			return -EFAULT;
		}
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dump flag:%d options:%d maxcount:%d\r\n",
			__func__, dump_buffer_flag, dump_options, dump_maxcount);
		if (!dump_buffer_flag) {
			dump_options = 0;
			dump_maxcount = 0;
			dump_count = 0;
		}
	} else if (strncmp(cmd_buffer, "dumptime:", 9) == 0) {
		ret = sscanf(cmd_buffer + 9, "0x%x", &dump_time_flag);
		if (ret < 0) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s scanf input parameters error \r\n", __func__);
			return -EFAULT;
		}
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dump_time:%d \r\n", __func__, dump_time_flag);
	} else {
		VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s debug cmd can not be recognized! \r\n", __func__);
	}

	return count;
}

static const struct file_operations debug_fops = {
	.open = debug_open,
	.read = debug_read,
	.write = debug_write,
};

static int __init atc_vout_init(void)
{
	struct device_node *node = NULL;
	int ret = -ENOMEM;

	node = of_find_compatible_node(NULL, NULL, "Autochips,vout");
	if (!node) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s: get dts node fail\r\n", __func__);
		goto err;
	}

	ret = platform_driver_register(&atc_vout_driver);
	if (ret) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s: register driver failed %d\r\n", __func__, ret);
	}

	vout_debugfs = debugfs_create_file("vout-debug", S_IFREG | 0444, NULL, (void *)0, &debug_fops);
	if (NULL == vout_debugfs) {
		VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s debugfs_create_file fail \n", __func__);
	}

err:
	return ret;
}

static void atc_vout_cleanup(void)
{
	if (vbuf_va) {
		iounmap(vbuf_va);
	}
	platform_driver_unregister(&atc_vout_driver);
	VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s: unregister driver\r\n", __func__);
}

late_initcall(atc_vout_init);
module_exit(atc_vout_cleanup);
