#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <linux/dma-mapping.h>
#include <media/videobuf-dma-contig.h>
#include <media/v4l2-device.h>
#include <media/atc/vdp_mdd.h>
#include <media/atc/display_inc.h>
#include <media/atc/cp.h>
#include <media/atc/drv_osd_if.h>
#include "atc_voutlib.h"
#include <drm/drm_fourcc.h>

MODULE_AUTHOR("Autochips Inc");
MODULE_DESCRIPTION("ATC Video library");
MODULE_LICENSE("GPL");

#define SIZE_ALIGN(x, n)  ((x + n - 1) / n * n)

static u32 obuf_free_offset;
static u32 obuf_cnt;

/* Return the default overlay cropping rectangle in crop given the image
 * size in pix and the video display size in fbuf.  The default
 * cropping rectangle is the largest rectangle no larger than the capture size
 * that will fit on the display.  The default cropping rectangle is centered in
 * the image.  All dimensions and offsets are rounded down to even numbers.
 */
void atc_vout_default_crop(struct v4l2_pix_format *pix, struct v4l2_rect *crop)
{
	crop->width = pix->width;
	crop->height = pix->height;
	crop->left = 0;
	crop->top = 0;
}

/* Given a new format in pix and fbuf,  crop and win
 * structures are initialized to default values. crop
 * is initialized to the largest window size that will fit on the display.  The
 * crop window is centered in the image. win is initialized to
 * the same size as crop and is centered on the display.
 * All sizes and offsets are constrained to be even numbers.
 */
void atc_vout_new_format(struct v4l2_pix_format *pix,
			 struct v4l2_framebuffer *fbuf, struct v4l2_rect *crop,
			 struct v4l2_window *win, struct v4l2_window *extwin)
{
	/* crop defines the preview source window in the image capture buffer */
	atc_vout_default_crop(pix, crop);

	/* win defines the preview target window on the display */
	win->w.width = fbuf->fmt.width;
	win->w.height = fbuf->fmt.height;
	win->w.left = 0;
	win->w.top = 0;

	extwin->w.width = 720;
	extwin->w.height = 480;
	extwin->w.left = 0;
	extwin->w.top = 0;
}

/*
 * Allocate buffers
 */
unsigned long atc_vout_alloc_vdo_buffer(u32 buf_size, u32 *phys_addr)
{
	u32 order, size;
	unsigned long virt_addr, addr;

	size = PAGE_ALIGN(buf_size);
	order = get_order(size);
	virt_addr = __get_free_pages(GFP_KERNEL, order);
	addr = virt_addr;

	if (virt_addr) {
		while (size > 0) {
			SetPageReserved(virt_to_page(addr));
			addr += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}

	*phys_addr = (u32) virt_to_phys((void *) virt_addr);
	return virt_addr;
}

unsigned long atc_vout_alloc_osd_buffer(u32 buf_size, u32 *phys_addr)
{
	return	atc_vout_alloc_vdo_buffer(buf_size,phys_addr);
}

unsigned long atc_vout_alloc_osd_buffer_reserverd_memory(u32 buf_size, u32 *phys_addr)
{
	unsigned long virt_addr = 0;

	if (((buf_size + obuf_free_offset) > VOUT_OSD_BUF_SIZE) || (obuf_cnt > MAX_OBUF_CNT)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s error size 0x%x free offset 0x%x cnt %d\r\n", __func__
			, buf_size, obuf_free_offset, obuf_cnt);
		goto done;
	}

	*phys_addr = osd_buf_pa + obuf_free_offset;
	virt_addr = osd_buf_va + obuf_free_offset;
	memset(virt_addr, 0x0, buf_size);

	obuf_free_offset += PAGE_ALIGN(buf_size);
	obuf_cnt++;

	VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s pa 0x%x va 0x%x size 0x%x free offset 0x%x cnt %d\r\n", __func__
		, *phys_addr, (u32)virt_addr, buf_size, obuf_free_offset, obuf_cnt);
done:

	return virt_addr;
}

/*
 * Free buffers
 */
void atc_vout_free_vdo_buffer(unsigned long virtaddr, u32 buf_size)
{
	u32 order, size;
	unsigned long addr = virtaddr;

	size = PAGE_ALIGN(buf_size);
	order = get_order(size);

	while (size > 0) {
		ClearPageReserved(virt_to_page(addr));
		addr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	free_pages((unsigned long) virtaddr, order);
}
void atc_vout_free_osd_buffer(unsigned long virtaddr, u32 buf_size)
{
	atc_vout_free_vdo_buffer(virtaddr,buf_size);
}
/*
*
*/
void atc_vout_reset_cp(struct atc_vout_device *vout, u32 idx)
{
	if ((NULL == vout) || (idx > VOUT_HW_VDP2)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return;
	}

	VOUT_PRINT(VOUT_LOG_LVL_DBG, "%s dev %d context %d\r\n", __func__, vout->vid, vout->context);
	VcpReset(idx);
	vout->brightness[idx] = VcpGetBrightness(idx);
	vout->contrast[idx] = VcpGetContrast(idx);
	vout->saturation[idx] = VcpGetSaturation(idx);
	vout->hue[idx] = VcpGetHue(idx);
	vout->y_gain[idx] = VcpGetYGain(idx);
	vout->u_gain[idx] = VcpGetUGain(idx);
	vout->v_gain[idx] = VcpGetVGain(idx);
	vout->bgcolor[idx] = 0x808010;
}

enum v4l2_priority atc_vout_prio_max(struct atc_vout_device *vout)
{
	struct atcvideo_device *vid_dev = vout->vid_dev;
	enum v4l2_priority max = V4L2_PRIORITY_UNSET;
	int i = 0;

	for (i = 0; i < MAX_VOUT_DEV; i++) {
		if ((vout->format == vid_dev->vouts[i]->format) && (vid_dev->vouts[i]->prio > max)) {
			max = vid_dev->vouts[i]->prio;
		}
	}

	return max;
}

u32 atc_vout_get_output(struct atc_vout_device *vout, bool show)
{
	u32 output;

	output = vout->output;
	if (output != vout->orgout) {
		if (show) {
			if (output == VOUT_OUTPUT_NONE) {
				vout->orgout = vout->output;
				output = VOUT_OUTPUT_NONE;
				goto done;
			}

			switch (vout->orgout) {
			case VOUT_OUTPUT_FRONT:
				if ((output == VOUT_OUTPUT_REAR) || (output == VOUT_OUTPUT_FRONTREAR)) {
					vout->stateflags |= VOUT_SET_REAR_DST_PARAMS;
				} else {
					VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d error show %d -> %d\r\n", __func__
						, vout->vid, vout->orgout, output);
					output = VOUT_OUTPUT_NONE;
				}
				break;
			case VOUT_OUTPUT_REAR:
				if ((output == VOUT_OUTPUT_FRONT) || (output == VOUT_OUTPUT_FRONTREAR)) {
					vout->stateflags |= VOUT_SET_FRONT_DST_PARAMS;
				} else {
					VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d error show %d -> %d\r\n", __func__
						, vout->vid, vout->orgout, output);
					output = VOUT_OUTPUT_NONE;
				}
				break;
			case VOUT_OUTPUT_NONE:
				if (output == VOUT_OUTPUT_FRONT) {
					output = VOUT_OUTPUT_FRONT;
					vout->stateflags |= VOUT_SET_FRONT_DST_PARAMS;
				} else if (output == VOUT_OUTPUT_REAR) {
					output = VOUT_OUTPUT_REAR;
					vout->stateflags |= VOUT_SET_REAR_DST_PARAMS;
				} else if (output == VOUT_OUTPUT_FRONTREAR) {
					output = VOUT_OUTPUT_FRONT | VOUT_OUTPUT_REAR;
					vout->stateflags |= VOUT_OUTPUT_FRONT | VOUT_SET_REAR_DST_PARAMS;
				} else {
					VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d error show %d -> %d\r\n", __func__
						, vout->vid, vout->orgout, output);
					output = VOUT_OUTPUT_NONE;
				}
				break;
			default:
				VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d error show %d -> %d\r\n", __func__, vout->vid
					, vout->orgout, output);
				output = VOUT_OUTPUT_NONE;
				}
		} else {
			switch (vout->orgout) {
			case VOUT_OUTPUT_FRONT:
				if ((output == VOUT_OUTPUT_NONE) || (output == VOUT_OUTPUT_REAR)) {
					output = VOUT_OUTPUT_FRONT;
				} else {
					VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d hide %d -> %d\r\n", __func__
						, vout->vid, vout->orgout, output);
					output = VOUT_OUTPUT_NONE;
				}
				break;
			case VOUT_OUTPUT_REAR:
				if ((output == VOUT_OUTPUT_NONE) | (output == VOUT_OUTPUT_FRONT)) {
					output = VOUT_OUTPUT_REAR;
				} else {
					VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d hide %d -> %d\r\n", __func__
						, vout->vid, vout->orgout, output);
					output = VOUT_OUTPUT_NONE;
				}
				break;
			case VOUT_OUTPUT_FRONTREAR:
				if (output == VOUT_OUTPUT_NONE) {
					output = VOUT_OUTPUT_FRONT | VOUT_OUTPUT_REAR;
				} else if (output == VOUT_OUTPUT_FRONT) {
					output = VOUT_OUTPUT_REAR;
				} else if (output == VOUT_OUTPUT_REAR) {
					output = VOUT_OUTPUT_FRONT;
				} else {
					VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d hide %d -> %d\r\n", __func__
						, vout->vid, vout->orgout, output);
					output = VOUT_OUTPUT_NONE;
				}
				break;
			default:
				VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d hide %d -> %d\r\n", __func__, vout->vid
					, vout->orgout, output);
				output = VOUT_OUTPUT_NONE;
			}
		}
	}
done:

	return output;
}

static int is_yuv_format(u32 format)
{
	switch (format) {
		case DRM_FORMAT_YUYV:
		case DRM_FORMAT_YVYU:
		case DRM_FORMAT_UYVY:
		case DRM_FORMAT_VYUY:
		case DRM_FORMAT_NV12:
		case DRM_FORMAT_NV21:
		case DRM_FORMAT_NV16:
		case DRM_FORMAT_NV61:
		case DRM_FORMAT_NV24:
		case DRM_FORMAT_NV42:
		case DRM_FORMAT_YUV420:
		case DRM_FORMAT_YVU420:
		case DRM_FORMAT_YUV422:
		case DRM_FORMAT_YVU422:
		case DRM_FORMAT_YUV444:
		case DRM_FORMAT_YVU444:
		case v4l2_fourcc('A', 'V', '1', '2'):
		case v4l2_fourcc('A', 'M', '1', '2'):
			return 1;
		default:
			return 0;
	}

	return 0;
}

/*
* Call video ioctl in fbdev driver
*/
int atc_vout_ioctl(struct atc_vout_device *vout, u32 code)
{
	int ret = -EINVAL;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return ret;
	}

	switch (code) {
	case VIDIOC_QBUF:
	case VIDIOC_STREAMOFF:
	case VIDIOC_OVERLAY: {
		struct OVERLAY_PARAM *prParam = &vout->param;
		mutex_lock(&interface_lock);
		ret = VDP_IOControl(code, (void *)prParam, NULL);
		mutex_unlock(&interface_lock);
		if (!ret && (vout->stateflags & VOUT_SET_PARAMS_MASK)) {
			vout->stateflags &= ~VOUT_SET_PARAMS_MASK;
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d %d %d %d (%d %d %d %d) (%d %d %d %d) %x %x %x %x\r\n"
				, __func__, vout->vid, prParam->u4Idx, prParam->u4SrcWidth, prParam->u4SrcHeight
				, (unsigned int)prParam->rSrcRect.left, (unsigned int)prParam->rSrcRect.top
				, (unsigned int)prParam->rSrcRect.right, (unsigned int)prParam->rSrcRect.bottom
				, (unsigned int)prParam->rDstRect.left, (unsigned int)prParam->rDstRect.top
				, (unsigned int)prParam->rDstRect.right, (unsigned int)prParam->rDstRect.bottom
				, prParam->u4PhysicalAddressY, prParam->u4PhysicalAddressC
				, prParam->u4Flags, vout->stateflags);
		}
		break;
	}
	case STIOC_SET_FMT_BLACK: {
		struct FMT_BG_PARAM fmt;

		fmt.u4Idx = vout->context;
		fmt.u4Color = vout->bgcolor[vout->context];
		fmt.fgEnable = (vout->stateflags & VOUT_SET_BG_OUTPUT)? true: false;
		ret = VDP_IOControl(STIOC_SET_FMT_BLACK, &fmt, NULL);
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d set black hw %d color 0x%x enable %d ret %d\r\n", __func__
			, vout->vid, fmt.u4Idx, fmt.u4Color, fmt.fgEnable, ret);
		break;
	}
	case STIOC_SET_COLOR_RANGE: {
		u32 flags = 0;
		if (is_yuv_format(vout->pix.pixelformat)) {
			if (vout->range == COLOR_YCBCR_FULL_RANGE) {
				flags = OVERLAY_FLAG_YUV_WIDE;
			} else {
				flags = OVERLAY_FLAG_YUV_NARROW;
			}
			ret = VDP_IOControl(STIOC_SET_COLOR_RANGE, &flags, NULL);
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d set color_encoding %d color_range %d ret %d\r\n", __func__
				, vout->vid, vout->encoding, vout->range, ret);
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d format %c%c%c%c is not YUV format\r\n", __func__, vout->vid,
				vout->pix.pixelformat & 0xFF, (vout->pix.pixelformat >> 8) & 0xFF,
				(vout->pix.pixelformat >> 16) & 0xFF, (vout->pix.pixelformat >> 24) & 0xFF);
			ret = 0;
		}
		break;
	}
	case STIOC_SET_COLOR_ENCODING: {
		u32 flags = 0;
		if (is_yuv_format(vout->pix.pixelformat)) {
			if (vout->encoding == COLOR_YCBCR_BT709) {
				flags = OVERLAY_FLAG_YUV_BT709;
			} else {
				flags == OVERLAY_FLAG_YUV_BT601;
			}
			ret = VDP_IOControl(STIOC_SET_COLOR_ENCODING, &flags, NULL);
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d set color_encoding %d color_range %d ret %d\r\n", __func__
				, vout->vid, vout->encoding, vout->range, ret);
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_WARN, "%s dev %d format %c%c%c%c is not YUV format\r\n", __func__, vout->vid,
				vout->pix.pixelformat & 0xFF, (vout->pix.pixelformat >> 8) & 0xFF,
				(vout->pix.pixelformat >> 16) & 0xFF, (vout->pix.pixelformat >> 24) & 0xFF);
			ret = 0;
		}

		break;
	}
	case STIOC_GET_PHY_ACTIVE: {
		ret = VDP_IOControl(STIOC_GET_PHY_ACTIVE, NULL, &(vout->screen_size));
		VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d get screen size (%dmm x %dmm) ret %d\r\n", __func__
			, vout->vid, vout->screen_size.width, vout->screen_size.height, ret);
		break;
	}
	default:
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d code is not support\r\n", __func__, vout->vid);
		break;
	}

	if (ret) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d code %x return %d\r\n", __func__, vout->vid, code, ret);
	}

	return ret;
}

/*
* Check video devices is show or not and input priority higher than it
*/
int atc_vout_get_backend_dev(struct atc_vout_device *vout, u32 context)
{
	struct atcvideo_device *vid_dev = NULL;
	struct atc_vout_device *tmp = NULL;
	enum v4l2_priority prio = V4L2_PRIORITY_UNSET;
	u32 format = VOUT_FMT_UNKNOWN;
	int ret = -EINVAL, i = 0;

	if ((NULL == vout) || (context > VOUT_CONTEXT_REAR)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s input arg error\r\n", __func__);
		return ret;
	}

	vid_dev = vout->vid_dev;
	format = vout->format;

	for (i = 0; i < MAX_VOUT_DEV; i++) {
		if (i == vout->vid) {
			continue;
		}
		tmp = vid_dev->vouts[i];
		if (tmp->enable && tmp->datavalid && (tmp->format == format)) {
			if ((VOUT_CONTEXT_FRONT == context) && (tmp->output & VOUT_OUTPUT_FRONT)) {
				if (prio < tmp->prio) {
					prio = tmp->prio;
					ret = i;
				}
			} else if ((VOUT_CONTEXT_REAR == context) && (tmp->output & VOUT_OUTPUT_REAR)) {
				if (prio < tmp->prio) {
					prio = tmp->prio;
					ret = i;
				}
			}
		}
	}

	return ret;
}

/*
*
*/
int atc_vout_hide_video(struct atc_vout_device *vout)
{
	struct atc_vout_device *tmpdev;
	struct OVERLAY_PARAM *prParam = NULL;
	int ret = 0, devid = MAX_VOUT_DEV;
	u32 output = VOUT_OUTPUT_NONE;

	if ((NULL == vout) || (NULL == vout->vid_dev)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		ret = -EINVAL;
		goto done;
	}

	output = atc_vout_get_output(vout, false);
	if (VOUT_OUTPUT_NONE == output) {
		goto done;
	}

	prParam = &vout->param;
	if (output & VOUT_OUTPUT_FRONT) {
		tmpdev = vout->vid_dev->hwdevs[VOUT_HW_VDP1];
		if (tmpdev != vout) { /* vout device is not showing hw device, onlay update sw params*/
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front disable SW resource\r\n", __func__, vout->vid);
		} else {
			prParam->u4Idx = VOUT_HW_VDP1;
			//ret = atc_vout_ioctl(vout, VIDIOC_STREAMOFF);
			ret = atc_vout_ioctl(vout, VIDIOC_OVERLAY);
			if (ret) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d front error %d\r\n", __func__, vout->vid, ret);
				goto done;
			} else {
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front hide %d\r\n", __func__, vout->vid
					, prParam->u4Idx);
			}
			/* check and show backend device if need*/
			devid = atc_vout_get_backend_dev(vout, VOUT_CONTEXT_FRONT);
			if ((devid >= 0) && (devid < MAX_VOUT_DEV)) {
				ret = atc_vout_ioctl(vout, VIDIOC_STREAMOFF);
				tmpdev = vout->vid_dev->vouts[devid];
				prParam = &tmpdev->param;
				prParam->u4Flags &= ~VDP_FLIP_ADDRESS;
				prParam->u4Flags |= VDP_UPDATE_OVERLAY;
				prParam->u4Idx = VOUT_HW_VDP1;
				prParam->rDstRect.left = tmpdev->win.w.left;
				prParam->rDstRect.top = tmpdev->win.w.top;
				prParam->rDstRect.right = tmpdev->win.w.left + tmpdev->win.w.width;
				prParam->rDstRect.bottom = tmpdev->win.w.top + tmpdev->win.w.height;
				ret = atc_vout_ioctl(tmpdev, VIDIOC_QBUF);
				if (ret) {
					VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d show front backend dev %d error\r\n"
						, __func__, vout->vid, tmpdev->vid);
					goto done;
				}
				/* Relate backend sw device with hw device*/
				vout->vid_dev->hwdevs[VOUT_HW_VDP1] = tmpdev;
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front show backend %d dev\r\n", __func__
					, vout->vid, tmpdev->vid);
			} else {
				vout->vid_dev->hwdevs[VOUT_HW_VDP1] = NULL;
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d release front hw\r\n", __func__, vout->vid);
			}
		}
	}

	if (output & VOUT_OUTPUT_REAR) {
		tmpdev = vout->vid_dev->hwdevs[VOUT_HW_VDP2];
		if (tmpdev != vout) { /* vout device is not showing hw device, onlay update sw params*/
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear disable SW resource\r\n", __func__, vout->vid);
		} else {
			prParam->u4Idx = VOUT_HW_VDP2;
			ret = atc_vout_ioctl(vout, VIDIOC_STREAMOFF);
			if (ret) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d rear error %d\r\n", __func__, vout->vid, ret);
				goto done;
			} else {
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear hide %d\r\n", __func__, vout->vid
					, prParam->u4Idx);
			}
			/* check and show backend device if need*/
			devid = atc_vout_get_backend_dev(vout, VOUT_CONTEXT_REAR);
			if ((devid >= 0) && (devid < MAX_VOUT_DEV)) {
				tmpdev = vout->vid_dev->vouts[devid];
				prParam = &tmpdev->param;
				prParam->u4Flags &= ~VDP_FLIP_ADDRESS;
				prParam->u4Flags |= VDP_UPDATE_OVERLAY;
				prParam->u4Idx = VOUT_HW_VDP2;
				prParam->rDstRect.left = tmpdev->extwin.w.left;
				prParam->rDstRect.top = tmpdev->extwin.w.top;
				prParam->rDstRect.right = tmpdev->extwin.w.left + tmpdev->extwin.w.width;
				prParam->rDstRect.bottom = tmpdev->extwin.w.top + tmpdev->extwin.w.height;
				ret = atc_vout_ioctl(tmpdev, VIDIOC_QBUF);
				if (ret) {
					VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d show rear backend dev %d error\r\n"
						, __func__, vout->vid, tmpdev->vid);
					goto done;
				}
				vout->vid_dev->hwdevs[VOUT_HW_VDP2] = tmpdev;
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear show backend %d dev\r\n", __func__
					, vout->vid, tmpdev->vid);
			} else {
				TurnOnTve(0, false);
				vout->vid_dev->hwdevs[VOUT_HW_VDP2] = NULL;
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d release rear hw\r\n", __func__, vout->vid);
			}
		}
	}

	if (vout->output != vout->orgout) {
		vout->orgout = vout->output;
	}

done:
	return ret;
}

/*
*
*/
bool atc_vout_can_use_hwdev(struct atc_vout_device *vout, enum vout_hw_ovls hwovl)
{
	struct atc_vout_device *hwdev;
	bool ret = false;

	if ((NULL == vout) || (NULL == vout->vid_dev)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return ret;
	}

	hwdev = vout->vid_dev->hwdevs[hwovl];
	/* Check hw is in use or not, and can not preempt hw resource*/
	if ((NULL == hwdev) || (vout == hwdev)) {
		ret = true;
	} else if (vout->prio > hwdev->prio) {
		ret = atc_vout_ioctl(hwdev, VIDIOC_OVERLAY);
		vout->vid_dev->hwdevs[hwovl] = vout;
		ret = true;
	}

	return ret;
}

int atc_vout_show_video(struct atc_vout_device *vout)
{
	struct OVERLAY_PARAM *prParam = NULL;
	int ret = 0;
	u32 output = VOUT_OUTPUT_NONE;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		ret = -EINVAL;
		goto done;
	}

	output = atc_vout_get_output(vout, true);
	if (VOUT_OUTPUT_NONE == output) {
		goto done;
	}

	prParam = &vout->param;
	if (vout->stateflags & VOUT_SET_SRC_PARAMS) {
		prParam->u4Flags &= ~VDP_FLIP_ADDRESS;
		prParam->u4Flags |= VDP_UPDATE_OVERLAY;
	} else {
		prParam->u4Flags &= ~VDP_UPDATE_OVERLAY;
		prParam->u4Flags |= VDP_FLIP_ADDRESS;
	}

	if (output & VOUT_OUTPUT_FRONT) {
		if ((vout->stateflags & VOUT_SET_FRONT_DST_PARAMS) && (prParam->u4Flags & VDP_FLIP_ADDRESS)) {
			prParam->u4Flags &= ~VDP_FLIP_ADDRESS;
			prParam->u4Flags |= VDP_UPDATE_OVERLAY;
		}

		if (vout->enable && vout->datavalid) {
			if (atc_vout_can_use_hwdev(vout, VOUT_HW_VDP1)) {
				prParam->u4Idx = VOUT_HW_VDP1;
				prParam->rDstRect.left = vout->win.w.left;
				prParam->rDstRect.top = vout->win.w.top;
				prParam->rDstRect.right = vout->win.w.left + vout->win.w.width;
				prParam->rDstRect.bottom = vout->win.w.top + vout->win.w.height;
				ret = atc_vout_ioctl(vout, VIDIOC_QBUF);
				if (ret) {
					VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d front error\r\n", __func__, vout->vid);
					goto done;
				}
				/* Relate sw device with hw device*/
				if (vout->vid_dev->hwdevs[VOUT_HW_VDP1] == NULL) {
					VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d relate front hw\r\n"
						, __func__, vout->vid);
					vout->vid_dev->hwdevs[VOUT_HW_VDP1] = vout;
				}
			} else { /*Is not showing device, and only update params*/
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front SW resource update\r\n"
					, __func__, vout->vid);
			}
		} else if (vout->enable) {
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front wait streaming %d qbuf %d\r\n"
				, __func__, vout->vid, vout->streaming, vout->datavalid);
		}
	}

	if (output & VOUT_OUTPUT_REAR) {
		if ((vout->stateflags & VOUT_SET_REAR_DST_PARAMS) && (prParam->u4Flags & VDP_FLIP_ADDRESS)) {
			prParam->u4Flags &= ~VDP_FLIP_ADDRESS;
			prParam->u4Flags |= VDP_UPDATE_OVERLAY;
		}

		if (vout->enable && vout->datavalid) {
			if (atc_vout_can_use_hwdev(vout, VOUT_HW_VDP2)) {
				prParam->u4Idx = VOUT_HW_VDP2;
				prParam->rDstRect.left = vout->extwin.w.left;
				prParam->rDstRect.top = vout->extwin.w.top;
				prParam->rDstRect.right = vout->extwin.w.left + vout->extwin.w.width;
				prParam->rDstRect.bottom = vout->extwin.w.top + vout->extwin.w.height;
				ret = atc_vout_ioctl(vout, VIDIOC_QBUF);
				if (ret) {
					VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d rear error\r\n", __func__, vout->vid);
					goto done;
				}
				TurnOnTve(0, true);
				/* Relate sw device with hw device*/
				if (vout->vid_dev->hwdevs[VOUT_HW_VDP2] == NULL) {
					VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d relate rear hw\r\n"
						, __func__, vout->vid);
					vout->vid_dev->hwdevs[VOUT_HW_VDP2] = vout;
				}
			} else { /*Is not showing device, and only update params*/
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear SW resource update\r\n"
					, __func__, vout->vid);
			}
		} else if (vout->enable) {
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear wait streaming %d qbuf %d\r\n"
				, __func__, vout->vid, vout->streaming, vout->datavalid);
		}
	}

	if (vout->output != vout->orgout) {
		vout->orgout = vout->output;
	}

done:
	return ret;
}

static u32 pixfmt_to_colormode(u32 pixfmt)
{
	u32 colormode = OSD_CM_INVALID_VALUE;

	switch (pixfmt) {
	case V4L2_PIX_FMT_RGB565:
		colormode = OSD_CM_RGB565_DIRECT16;
		break;
	case V4L2_PIX_FMT_ARGB32:
		colormode = OSD_CM_ARGB8888_DIRECT32;
		break;
	case V4L2_PIX_FMT_PAL8:
		colormode = OSD_CM_RGB_CLUT8;
		break;
	}

	return colormode;
}

int atc_vout_osd_on(struct atc_vout_device *vout)
{
	OSD_DATA_T *param = NULL;
	int ret = 0, i = 0, loop_cnt = 0;
	u32 rgn = 0, output = VOUT_OUTPUT_NONE;
	bool need_init = false;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return -EINVAL;
	}

	output = atc_vout_get_output(vout, true);
	if (VOUT_OUTPUT_NONE == output) {
		goto done;
	} else if (VOUT_OUTPUT_FRONTREAR == output) {
		loop_cnt = 2;
	} else {
		loop_cnt = 1;
	}

	param = &vout->osd_param;
	for (i = 0; i < loop_cnt; i++) {
		if (output & VOUT_OUTPUT_FRONT) {
			output &= ~VOUT_OUTPUT_FRONT;
			param->rDestRect.top = vout->win.w.top;
			param->rDestRect.left = vout->win.w.left;
			param->rDestRect.bottom = vout->win.w.top + vout->win.w.height;
			param->rDestRect.right = vout->win.w.left + vout->win.w.width;
			if (atc_vout_can_use_hwdev(vout, VOUT_HW_OSDF1)) {
				param->u4BlockID = BOOT_ANIMATION_OSD_PLANE;
				if (vout->stateflags & (VOUT_SET_SRC_PARAMS | VOUT_SET_FRONT_DST_PARAMS)) {
					need_init = true;
				}
			} else {
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front OSD SW resource update\r\n"
					, __func__, vout->vid);
				continue;
			}
		} else if (output & VOUT_OUTPUT_REAR) {
			output &= ~VOUT_OUTPUT_REAR;
			param->rDestRect.top = vout->extwin.w.top;
			param->rDestRect.left = vout->extwin.w.left;
			param->rDestRect.bottom = vout->extwin.w.top + vout->extwin.w.height;
			param->rDestRect.right = vout->extwin.w.left + vout->extwin.w.width;
			if (atc_vout_can_use_hwdev(vout, VOUT_HW_OSDR1)) {
				param->u4BlockID = OSD_PLANE_8;
				if (vout->stateflags & (VOUT_SET_SRC_PARAMS | VOUT_SET_REAR_DST_PARAMS)) {
					need_init = true;
				}
			} else {
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear OSD SW resource update\r\n"
					, __func__, vout->vid);
				continue;
			}
		}

		if (vout->enable && vout->datavalid) {
			u32 plane = param->u4BlockID;
			u32 dst_width = param->rDestRect.right - param->rDestRect.left;
			u32 dst_height = param->rDestRect.bottom - param->rDestRect.top;

			if (need_init) {
				rgn  = GetPlaneRgn(plane);
				if (rgn < OSD_MAX_NUM_RGN) {
					OSD_RGN_Delete(rgn);
					SetPlaneRgn(plane, INVALID_RGN);
				}
				vout->pix.bytesperline = param->u4Width * vout->bpp;
				OSD_BASE_SetOsdPosition(plane, param->rDestRect.left, param->rDestRect.top);
				/*Set Scale disable*/
				OSD_SC_Scale(plane, FALSE, dst_width, dst_height
					, dst_width, dst_height);
				OSD_RGN_LIST_DetachAll(plane);
				/*Set pic byte lines is src width *4 */
				OSD_RGN_Create(&rgn, param->u4Width, param->u4Height, (void *)param->u4BitmapPA
					, param->u4PixelFormat, vout->pix.bytesperline, 0, 0, dst_width, dst_height);
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s width %d height %d addr %x format %x,dst_width %d dst_height %d\r\n"
					, __func__, param->u4Width,param->u4Height,(void *)param->u4BitmapPA,param->u4PixelFormat,dst_width,dst_height);
				OSD_RGN_Insert(rgn, plane);
				SetPlaneRgn(plane, rgn);
				i4OsdPlaneFlipTo(plane, plane);
				i4OsdPlaneEnble(plane, TRUE);
			} else {
				rgn  = GetPlaneRgn(plane);
				OSD_RGN_Set(rgn, OSD_RGN_BMP_ADDR, param->u4BitmapPA);
			}

			if (plane == BOOT_ANIMATION_OSD_PLANE) {
				if (vout->vid_dev->hwdevs[VOUT_HW_OSDF1] == NULL) {
					VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d relate front osd hw\r\n"
						, __func__, vout->vid);
					vout->vid_dev->hwdevs[VOUT_HW_OSDF1] = vout;
				}
			} else {
				if (vout->vid_dev->hwdevs[VOUT_HW_OSDR1] == NULL) {
					VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d relate rear osd hw\r\n"
						, __func__, vout->vid);
					vout->vid_dev->hwdevs[VOUT_HW_OSDR1] = vout;
				}
			}
		} else if (vout->enable) {
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d output %d wait streaming %d qbuf %d\r\n"
				, __func__, vout->vid, vout->output, vout->streaming, vout->datavalid);
		}

	}

	if (vout->stateflags & VOUT_SET_PARAMS_MASK) {
		vout->stateflags &= ~VOUT_SET_PARAMS_MASK;
	}

	if (vout->output != vout->orgout) {
		vout->orgout = vout->output;
	}
done:

	return ret;
}

int atc_vout_osd_off(struct atc_vout_device *vout)
{
	struct atc_vout_device *tmpdev;
	OSD_DATA_T *param = NULL;
	int ret = 0, i = 0, loop_cnt = 0, devid = MAX_VOUT_DEV;
	u32 plane = 0, rgn = 0, output = VOUT_OUTPUT_NONE, hwdev = 0;

	if (NULL == vout) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		return -EINVAL;
	}

	output = atc_vout_get_output(vout, false);
	if (output == VOUT_OUTPUT_NONE) {
		goto done;
	} else if (output == VOUT_OUTPUT_FRONTREAR) {
		loop_cnt = 2;
	} else {
		loop_cnt = 1;
	}

	param = &vout->osd_param;
	for (i = 0; i < loop_cnt; i++) {
		if (output & VOUT_OUTPUT_FRONT) {
			output &= ~VOUT_OUTPUT_FRONT;
			tmpdev = vout->vid_dev->hwdevs[VOUT_HW_OSDF1];
			if (tmpdev != vout) {
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d front OSD SW resource update\r\n"
					, __func__, vout->vid);
				continue;
			} else {
				param->u4BlockID = BOOT_ANIMATION_OSD_PLANE;
			}
		} else if (output & VOUT_OUTPUT_REAR) {
			output &= ~VOUT_OUTPUT_REAR;
			tmpdev = vout->vid_dev->hwdevs[VOUT_HW_OSDR1];
			if (tmpdev != vout) {
				VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d rear OSD SW resource update\r\n"
					, __func__, vout->vid);
				continue;
			} else {
				param->u4BlockID = OSD_PLANE_8;
			}
		}
		plane = param->u4BlockID;
		i4OsdPlaneEnble(plane, FALSE);
		rgn  = GetPlaneRgn(plane);
		if (rgn < OSD_MAX_NUM_RGN)
		{
			OSD_RGN_Delete(rgn);
			SetPlaneRgn(plane, INVALID_RGN);
		}

		/* check and show backend device if need*/
		if (plane == BOOT_ANIMATION_OSD_PLANE) {
			hwdev = VOUT_HW_OSDF1;
			devid = atc_vout_get_backend_dev(vout, VOUT_CONTEXT_FRONT);
		} else {
			hwdev = VOUT_HW_OSDR1;
			devid = atc_vout_get_backend_dev(vout, VOUT_CONTEXT_REAR);
		}
		if ((devid >= 0) && (devid < MAX_VOUT_DEV)) {
			u32 dst_width, dst_height;

			tmpdev = vout->vid_dev->vouts[devid];
			param = &tmpdev->osd_param;
			dst_width = param->rDestRect.right - param->rDestRect.left;
			dst_height = param->rDestRect.bottom - param->rDestRect.top;
			tmpdev->pix.bytesperline = param->u4Width * tmpdev->bpp;
			rgn  = GetPlaneRgn(plane);
			if (rgn < OSD_MAX_NUM_RGN) {
				OSD_RGN_Delete(rgn);
				SetPlaneRgn(plane, INVALID_RGN);
			}
			OSD_BASE_SetOsdPosition(plane, param->rDestRect.left, param->rDestRect.top);
			OSD_SC_Scale(plane, FALSE, dst_height, dst_height
				, dst_width, dst_height);
			OSD_RGN_LIST_DetachAll(plane);
			OSD_RGN_Create(&rgn, param->u4Width, param->u4Height, (void *)param->u4BitmapPA
				, param->u4PixelFormat, tmpdev->pix.bytesperline, 0, 0, dst_width, dst_height);
			OSD_RGN_Insert(rgn, plane);
			SetPlaneRgn(plane, rgn);
			i4OsdPlaneFlipTo(plane, plane);
			i4OsdPlaneEnble(plane, TRUE);
			vout->vid_dev->hwdevs[hwdev] = tmpdev;
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d output %d show OSD backend %d dev\r\n", __func__
				, vout->vid, i, tmpdev->vid);
		} else {
			if (hwdev == VOUT_HW_OSDR1) {
				TurnOnTve(0, false);
			}
			vout->vid_dev->hwdevs[hwdev] = NULL;
			VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d output %d release OSD hw\r\n", __func__, vout->vid, i);
		}
	}

	if (vout->output != vout->orgout) {
		vout->orgout = vout->output;
	}
done:
	return ret;
}

extern void log_boot(char *str);
static int g_first_frame = 0;
/*
* Fill video ioctl params and call video ioctl function
*/
int atc_vout_qbuf(struct atc_vout_device *vout, u32 index)
{
	struct videobuf_buffer *vb = NULL;
	int ret = -EINVAL;

	if ((NULL == vout) || (NULL == vout->vid_dev)) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s vout is NULL\r\n", __func__);
		goto error;
	}

	vb = vout->vbq.bufs[index];
	if (vout->format == VOUT_FMT_VIDEO) {
		struct VOUT_PARAM *vout_param = (struct VOUT_PARAM *)vout->buf_virt_addr[vb->i];
		struct OVERLAY_PARAM *prParam = &vout->param;

		if (NULL == vout_param) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d vout_param is NULL\r\n", __func__, vout->vid);
			goto error;
		}

		if ((vout_param->disp_flags & VOUT_BG_OUTPUT_EN) && !(vout->stateflags & VOUT_SET_BG_OUTPUT)) {
			vout->stateflags |= VOUT_SET_BG_OUTPUT;
			atc_vout_ioctl(vout, STIOC_SET_FMT_BLACK);
			goto error;
		}

		if (vout_param->y_phy_addr && vout_param->c_phy_addr) {
			prParam->u4PhysicalAddressY = vout_param->y_phy_addr;
			prParam->u4PhysicalAddressC = vout_param->c_phy_addr;
			/*VOUT_PRINT(VOUT_LOG_LVL_INFO, "%s dev %d Y/C buffer count 0x%x 0x%x 0x%x id %d\r\n", __func__
				, vout->vid, vout->qbufcnt, vout_param->y_phy_addr, vout_param->c_phy_addr, vb->i);*/
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d Y/C buffer 0x%x 0x%x id %d\r\n", __func__, vout->vid
				, vout_param->y_phy_addr, vout_param->c_phy_addr, vb->i);
			goto error;
		}

		if (vout->pix.pixelformat == v4l2_fourcc('A', 'V', '1', '2')) {
			prParam->u4Flags |= VDP_SCANLINE_MODE;
		} else if (vout->pix.pixelformat == v4l2_fourcc('A', 'M', '1', '2')) {
			prParam->u4Flags &= ~VDP_SCANLINE_MODE;
		} else {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d fmt %x not support\r\n", __func__, vout->vid
				, vout->pix.pixelformat);
			goto error;
		}

		if (vout_param->disp_flags & VOUT_DI_FUN_EN) {
			prParam->u4Flags |= VDP_ENABLE_DEINT;
		} else {
			prParam->u4Flags &= ~VDP_ENABLE_DEINT;
		}

		if (vout_param->disp_flags & VOUT_DI_PD) {
			prParam->u4Flags |= VDP_ENABLE_PDDI;
		} else {
			prParam->u4Flags &= ~VDP_ENABLE_PDDI;
		}

		if (vout_param->disp_flags & VOUT_DI_SRC_MASK) {
			prParam->fgProgSrc = false;
		} else {
			prParam->fgProgSrc = true;
		}

		prParam->fgTopFiledFirst = vout_param->disp_flags & VOUT_DI_TOP_FLD_FIRST;
		prParam->fgRepeatFirstField = vout_param->disp_flags & VOUT_DI_RPT_FIRST_FLD;
		prParam->fgProgSeq = vout_param->disp_flags & VOUT_DI_PROG_SEQ;

		if (vout_param->disp_flags & VOUT_DI_SEEK_LOCATE) {
			prParam->u4Flags |= VDP_SEEK_LOCATE;
		} else {
			prParam->u4Flags &= ~VDP_SEEK_LOCATE;
		}

		if (vout_param->disp_flags & VOUT_DI_FF_RW) {
			prParam->u4Flags |= VDP_PLAY_FF_RW;
		} else {
			prParam->u4Flags &= ~VDP_PLAY_FF_RW;
		}

		if (vout_param->duration >= 1500) {
			prParam->u4Duration = vout_param->duration;
		} else {
			prParam->u4Duration = 3000;
		}

		if (vout->stateflags & VOUT_SET_PARAMS_MASK) {
			prParam->u4SrcWidth = SIZE_ALIGN(vout->pix.width, 16);
			prParam->u4SrcHeight = SIZE_ALIGN(vout->pix.height, 32);
			prParam->rSrcRect.left = vout->crop.left;
			prParam->rSrcRect.top = vout->crop.top;
			prParam->rSrcRect.right = vout->crop.left + vout->crop.width;
			prParam->rSrcRect.bottom = vout->crop.top + vout->crop.height;
		}

		ret = atc_vout_show_video(vout);
		if (ret) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d show video error %d\r\n", __func__, vout->vid, ret);
			goto error;
		}

		if (!(vout_param->disp_flags & VOUT_BG_OUTPUT_EN) && (vout->stateflags & VOUT_SET_BG_OUTPUT)) {
			vout->stateflags &= ~VOUT_SET_BG_OUTPUT;
			atc_vout_ioctl(vout, STIOC_SET_FMT_BLACK);
		}
	} else if (vout->format == VOUT_FMT_OSD) {
		OSD_DATA_T *param = &vout->osd_param;
		struct OSD_PARAM *osd_param = (struct OSD_PARAM *)vout->buf_virt_addr[vb->i];

		if (vout->stateflags & VOUT_SET_PARAMS_MASK) {
			param->u4PixelFormat = pixfmt_to_colormode(vout->pix.pixelformat);
			if (param->u4PixelFormat == OSD_CM_INVALID_VALUE) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d osd error %d\r\n", __func__, vout->vid
					, param->u4PixelFormat);
				goto error;
			}
			param->u4Width = vout->pix.width;
			param->u4Height = vout->pix.height;
		}
		param->u4BitmapPA = osd_param->buf_phy_addr;
		ret = atc_vout_osd_on(vout);
	}

	if (!g_first_frame) {
		g_first_frame = 1;
		log_boot("first frame qbuf\n");
	}

error:
	return ret;
}

/*
* set video output params and call video ioctl function
*/
int atc_vout_set_output(struct atc_vout_device *vout, u32 output)
{
	int ret = 0;
	u32 orgout = vout->output;

	if (is_fr_follow_on()) {
		VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d error for mirror on\r\n", __func__, vout->vid);
		return -EBUSY;
	}

	vout->output = output;
	VOUT_PRINT(VOUT_LOG_LVL_TRACE, "%s dev %d output %d -> %d\r\n", __func__
		, vout->vid, orgout, vout->output);

	if ((vout->output == VOUT_OUTPUT_NONE) || (orgout == VOUT_OUTPUT_FRONTREAR)) {
		/* Front Rear -> Front, Front Rear -> Rear*/
		/* Front Rear -> None, Front -> None, Rear -> None*/
		vout->orgout = orgout;
		ret = atc_vout_hide_video(vout);
		if (ret) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d output %d -> %d error\r\n"
				, __func__, vout->vid, orgout, vout->output);
			ret = -EINVAL;
		}
	} else if ((vout->output == VOUT_OUTPUT_FRONTREAR) ||
		(orgout == VOUT_OUTPUT_NONE)) {
		/* None -> Front,  None-> Rear*/
		vout->orgout = orgout;
		ret = atc_vout_show_video(vout);
		if (ret) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d output %d -> %d error\r\n"
				, __func__, vout->vid, orgout, vout->output);
			ret = -EINVAL;
		}
	} else {
		/* Front -> Rear, Rear -> Front*/
		vout->orgout = orgout;
		ret = atc_vout_hide_video(vout);
		if (ret) {
			VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d output %d -> %d error\r\n"
				, __func__, vout->vid, orgout, vout->output);
			ret = -EINVAL;
		} else {
			vout->orgout = orgout;
			ret = atc_vout_show_video(vout);
			if (ret) {
				VOUT_PRINT(VOUT_LOG_LVL_ERR, "%s dev %d output %d -> %d error\r\n"
					, __func__, vout->vid, orgout, vout->output);
				ret = -EINVAL;
			}
		}
	}

	return ret;
}

