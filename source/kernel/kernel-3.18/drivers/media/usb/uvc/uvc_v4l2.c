/*
 *      uvc_v4l2.c  --  USB Video Class driver - V4L2 API
 *
 *      Copyright (C) 2005-2010
 *          Laurent Pinchart (laurent.pinchart@ideasonboard.com)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,36)
#include <linux/compat.h>
#endif
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/videodev2.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/wait.h>
#include <linux/atomic.h>

#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>

#include "uvcvideo.h"

unsigned int is_yuyv_setting = 0;
/* ------------------------------------------------------------------------
 * UVC ioctls
 */
static int uvc_ioctl_ctrl_map(struct uvc_video_chain *chain,
	struct uvc_xu_control_mapping *xmap)
{
	struct uvc_control_mapping *map;
	unsigned int size;
	int ret;

	map = kzalloc(sizeof *map, GFP_KERNEL);
	if (map == NULL)
		return -ENOMEM;

	map->id = xmap->id;
	memcpy(map->name, xmap->name, sizeof map->name);
	memcpy(map->entity, xmap->entity, sizeof map->entity);
	map->selector = xmap->selector;
	map->size = xmap->size;
	map->offset = xmap->offset;
	map->v4l2_type = xmap->v4l2_type;
	map->data_type = xmap->data_type;

	switch (xmap->v4l2_type) {
	case V4L2_CTRL_TYPE_INTEGER:
	case V4L2_CTRL_TYPE_BOOLEAN:
	case V4L2_CTRL_TYPE_BUTTON:
		break;

	case V4L2_CTRL_TYPE_MENU:
		/* Prevent excessive memory consumption, as well as integer
		 * overflows.
		 */
		if (xmap->menu_count == 0 ||
		    xmap->menu_count > UVC_MAX_CONTROL_MENU_ENTRIES) {
			ret = -EINVAL;
			goto done;
		}

		size = xmap->menu_count * sizeof(*map->menu_info);
		map->menu_info = kmalloc(size, GFP_KERNEL);
		if (map->menu_info == NULL) {
			ret = -ENOMEM;
			goto done;
		}

		if (copy_from_user(map->menu_info, xmap->menu_info, size)) {
			ret = -EFAULT;
			goto done;
		}

		map->menu_count = xmap->menu_count;
		break;

	default:
		uvc_trace(UVC_TRACE_CONTROL, "Unsupported V4L2 control type "
			  "%u.\n", xmap->v4l2_type);
		ret = -ENOTTY;
		goto done;
	}

	ret = uvc_ctrl_add_mapping(chain, map);

done:
	kfree(map->menu_info);
	kfree(map);

	return ret;
}

/* ------------------------------------------------------------------------
 * V4L2 interface
 */

/*
 * Find the frame interval closest to the requested frame interval for the
 * given frame format and size. This should be done by the device as part of
 * the Video Probe and Commit negotiation, but some hardware don't implement
 * that feature.
 */
static __u32 uvc_try_frame_interval(struct uvc_frame *frame, __u32 interval)
{
	unsigned int i;

	if (frame->bFrameIntervalType) {
		__u32 best = -1, dist;

		for (i = 0; i < frame->bFrameIntervalType; ++i) {
			dist = interval > frame->dwFrameInterval[i]
			     ? interval - frame->dwFrameInterval[i]
			     : frame->dwFrameInterval[i] - interval;

			if (dist > best)
				break;

			best = dist;
		}

		interval = frame->dwFrameInterval[i-1];
	} else {
		const __u32 min = frame->dwFrameInterval[0];
		const __u32 max = frame->dwFrameInterval[1];
		const __u32 step = frame->dwFrameInterval[2];

		interval = min + (interval - min + step/2) / step * step;
		if (interval > max)
			interval = max;
	}

	return interval;
}

static int uvc_v4l2_try_format(struct uvc_streaming *stream,
	struct v4l2_format *fmt, struct uvc_streaming_control *probe,
	struct uvc_format **uvc_format, struct uvc_frame **uvc_frame)
{
	struct uvc_format *format = NULL;
	struct uvc_frame *frame = NULL;
	__u16 rw, rh;
	unsigned int d, maxd;
	unsigned int i;
	__u32 interval;
	int ret = 0;
	__u8 *fcc;

	if (fmt->type != stream->type)
		return -EINVAL;

	fcc = (__u8 *)&fmt->fmt.pix.pixelformat;
	uvc_trace(UVC_TRACE_FORMAT, "Trying format 0x%08x (%c%c%c%c): %ux%u.\n",
			fmt->fmt.pix.pixelformat,
			fcc[0], fcc[1], fcc[2], fcc[3],
			fmt->fmt.pix.width, fmt->fmt.pix.height);

    /*
     * For non-YUYV, the size of URB is less.
     * AC8317: 8x512 byte USB FIFO. if size of allocate is 800,
     * There is 2x512 FIFO is killed.
     * Bug fix. By Joshua @16/07/2015.
     */

	if (0x34363248 == fmt->fmt.pix.pixelformat) {
	    is_yuyv_setting = 1;
	} else {
	    is_yuyv_setting = 0;
	}

	/* Check if the hardware supports the requested format. */
	for (i = 0; i < stream->nformats; ++i) {
		format = &stream->format[i];
		if (format->fcc == fmt->fmt.pix.pixelformat)
			break;
	}

	if (format == NULL || format->fcc != fmt->fmt.pix.pixelformat) {
		uvc_trace(UVC_TRACE_FORMAT, "Unsupported format 0x%08x.\n",
				fmt->fmt.pix.pixelformat);
		return -EINVAL;
	}

	/* Find the closest image size. The distance between image sizes is
	 * the size in pixels of the non-overlapping regions between the
	 * requested size and the frame-specified size.
	 */
	rw = fmt->fmt.pix.width;
	rh = fmt->fmt.pix.height;
	maxd = (unsigned int)-1;

	for (i = 0; i < format->nframes; ++i) {
		__u16 w = format->frame[i].wWidth;
		__u16 h = format->frame[i].wHeight;

		d = min(w, rw) * min(h, rh);
		d = w*h + rw*rh - 2*d;
		if (d < maxd) {
			maxd = d;
			frame = &format->frame[i];
		}

		if (maxd == 0)
			break;
	}

	if (frame == NULL) {
		uvc_trace(UVC_TRACE_FORMAT, "Unsupported size %ux%u.\n",
				fmt->fmt.pix.width, fmt->fmt.pix.height);
		return -EINVAL;
	}

	/* Use the default frame interval. */
	interval = frame->dwDefaultFrameInterval;
	uvc_trace(UVC_TRACE_FORMAT, "Using default frame interval %u.%u us "
		"(%u.%u fps).\n", interval/10, interval%10, 10000000/interval,
		(100000000/interval)%10);

	/* Set the format index, frame index and frame interval. */
	memset(probe, 0, sizeof *probe);
	probe->bmHint = 1;	/* dwFrameInterval */
	probe->bFormatIndex = format->index;
	probe->bFrameIndex = frame->bFrameIndex;
	probe->dwFrameInterval = uvc_try_frame_interval(frame, interval);
	/* Some webcams stall the probe control set request when the
	 * dwMaxVideoFrameSize field is set to zero. The UVC specification
	 * clearly states that the field is read-only from the host, so this
	 * is a webcam bug. Set dwMaxVideoFrameSize to the value reported by
	 * the webcam to work around the problem.
	 *
	 * The workaround could probably be enabled for all webcams, so the
	 * quirk can be removed if needed. It's currently useful to detect
	 * webcam bugs and fix them before they hit the market (providing
	 * developers test their webcams with the Linux driver as well as with
	 * the Windows driver).
	 */
	mutex_lock(&stream->mutex);
	if (stream->dev->quirks & UVC_QUIRK_PROBE_EXTRAFIELDS)
		probe->dwMaxVideoFrameSize =
			stream->ctrl.dwMaxVideoFrameSize;

	/* Probe the device. */
	ret = uvc_probe_video(stream, probe);
	mutex_unlock(&stream->mutex);
	if (ret < 0)
		goto done;

	fmt->fmt.pix.width = frame->wWidth;
	fmt->fmt.pix.height = frame->wHeight;
	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = format->bpp * frame->wWidth / 8;
	fmt->fmt.pix.sizeimage = probe->dwMaxVideoFrameSize;
	fmt->fmt.pix.colorspace = format->colorspace;
	fmt->fmt.pix.priv = 0;

	// 添加分辨率调试信息
	uvc_printk(KERN_INFO, "[DEBUG_RESOLUTION] uvc_v4l2_get_format: Setting format %dx%d from frame->wWidth/wHeight for device %s\n",
		   frame->wWidth, frame->wHeight, 
		   stream->dev->udev->product ? stream->dev->udev->product : "Unknown");
	
	if (uvc_format != NULL)
		*uvc_format = format;
	if (uvc_frame != NULL)
		*uvc_frame = frame;

done:
	return ret;
}


static int uvc_v4l2_try_still_format(struct uvc_streaming *stream,
	struct v4l2_format *fmt, struct uvc_still_image_control *probe)
{
	struct uvc_format *format = NULL;
	struct uvc_still_frame *frame = NULL;
	__u16 rw, rh;
	unsigned int d, maxd;
	unsigned int i;
	int ret = 0;
	__u8 *fcc;

	if (fmt->type != stream->type)
		return -EINVAL;

	fcc = (__u8 *)&fmt->fmt.pix.pixelformat;
	uvc_trace(UVC_TRACE_FORMAT, "Trying still image format 0x%08x (%c%c%c%c): %ux%u.\n",
			fmt->fmt.pix.pixelformat,
			fcc[0], fcc[1], fcc[2], fcc[3],
			fmt->fmt.pix.width, fmt->fmt.pix.height);

	/* Check if the hardware supports the requested format. */
	for (i = 0; i < stream->nformats; ++i) {
		format = &stream->format[i];
		if (format->fcc == fmt->fmt.pix.pixelformat && format->nstillframes)
			break;
	}


	if (format == NULL || format->fcc != fmt->fmt.pix.pixelformat) {
		uvc_trace(UVC_TRACE_FORMAT, "Unsupported still image format 0x%08x.\n",
				fmt->fmt.pix.pixelformat);
		return -EINVAL;
	}

	/* Find the closest image size. The distance between image sizes is
	 * the size in pixels of the non-overlapping regions between the
	 * requested size and the frame-specified size.
	 */
	rw = fmt->fmt.pix.width;
	rh = fmt->fmt.pix.height;
	maxd = (unsigned int)-1;

	for (i = 0; i < format->nstillframes; ++i) {
		__u16 w = format->still_frame[i].wWidth;
		__u16 h = format->still_frame[i].wHeight;

		d = min(w, rw) * min(h, rh);
		d = w*h + rw*rh - 2*d;
		if (d < maxd) {
			maxd = d;
			frame = &format->still_frame[i];
		}

		if (maxd == 0)
			break;
	}

	if (frame == NULL) {
		uvc_trace(UVC_TRACE_FORMAT, "Unsupported still image frame size %ux%u.\n",
				fmt->fmt.pix.width, fmt->fmt.pix.height);
		return -EINVAL;
	}


	/* Set the format index, frame index */
	memset(probe, 0, sizeof *probe);

	probe->bFormatIndex = format->index;
	probe->bFrameIndex = frame->bFrameIndex;
	probe->bCompressionIndex = 0;	
	mutex_lock(&stream->mutex);


	/* Probe the device. */
	ret = uvc_probe_still(stream, probe);
	mutex_unlock(&stream->mutex);
	if (ret < 0)
		goto done;

	fmt->fmt.pix.width = frame->wWidth;
	fmt->fmt.pix.height = frame->wHeight;
	fmt->fmt.pix.sizeimage = probe->dwMaxVideoFrameSize;

done:
	return ret;
}

static int uvc_v4l2_get_format(struct uvc_streaming *stream,
	struct v4l2_format *fmt)
{
	struct uvc_format *format;
	struct uvc_frame *frame;
	int ret = 0;

	if (fmt->type != stream->type)
		return -EINVAL;

	mutex_lock(&stream->mutex);
	format = stream->cur_format;
	frame = stream->cur_frame;

	if (format == NULL || frame == NULL) {
		ret = -EINVAL;
		goto done;
	}

	fmt->fmt.pix.pixelformat = format->fcc;
	fmt->fmt.pix.width = frame->wWidth;
	fmt->fmt.pix.height = frame->wHeight;
	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = format->bpp * frame->wWidth / 8;
	fmt->fmt.pix.sizeimage = stream->ctrl.dwMaxVideoFrameSize;
	fmt->fmt.pix.colorspace = format->colorspace;
	fmt->fmt.pix.priv = 0;

done:
	mutex_unlock(&stream->mutex);
	return ret;
}

static int uvc_v4l2_set_format(struct uvc_streaming *stream,
	struct v4l2_format *fmt)
{
	struct uvc_streaming_control probe;
	struct uvc_format *format;
	struct uvc_frame *frame;
	int ret;

	if (fmt->type != stream->type)
		return -EINVAL;

	ret = uvc_v4l2_try_format(stream, fmt, &probe, &format, &frame);
	if (ret < 0)
		return ret;

	mutex_lock(&stream->mutex);

	if (uvc_queue_allocated(&stream->queue)) {
		ret = -EBUSY;
		goto done;
	}

	memcpy(&stream->ctrl, &probe, sizeof probe);
	stream->cur_format = format;
	stream->cur_frame = frame;

done:
	mutex_unlock(&stream->mutex);
	return ret;
}

static int uvc_v4l2_set_still_format(struct uvc_streaming *stream,
	struct v4l2_format *still_fmt)
{
	struct uvc_still_image_control probe;
	int ret;

	if (still_fmt->type != stream->type)
		return -EINVAL;

	ret = uvc_v4l2_try_still_format(stream, still_fmt, &probe);
	if (ret < 0)
		return ret;

	mutex_lock(&stream->mutex);

	if (uvc_still_queue_allocated(&stream->still_queue)) 
		uvc_free_still_buffers(&stream->still_queue);

	memcpy(&stream->still_ctrl, &probe, sizeof probe);

	mutex_unlock(&stream->mutex);
	return ret;
}

static int uvc_v4l2_get_streamparm(struct uvc_streaming *stream,
		struct v4l2_streamparm *parm)
{
	uint32_t numerator, denominator;

	if (parm->type != stream->type)
		return -EINVAL;

	mutex_lock(&stream->mutex);
	numerator = stream->ctrl.dwFrameInterval;
	mutex_unlock(&stream->mutex);

	denominator = 10000000;
	uvc_simplify_fraction(&numerator, &denominator, 8, 333);

	memset(parm, 0, sizeof *parm);
	parm->type = stream->type;

	if (stream->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		parm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
		parm->parm.capture.capturemode = 0;
		parm->parm.capture.timeperframe.numerator = numerator;
		parm->parm.capture.timeperframe.denominator = denominator;
		parm->parm.capture.extendedmode = 0;
		parm->parm.capture.readbuffers = 0;
	} else {
		parm->parm.output.capability = V4L2_CAP_TIMEPERFRAME;
		parm->parm.output.outputmode = 0;
		parm->parm.output.timeperframe.numerator = numerator;
		parm->parm.output.timeperframe.denominator = denominator;
	}

	return 0;
}

static int uvc_v4l2_set_streamparm(struct uvc_streaming *stream,
		struct v4l2_streamparm *parm)
{
	struct uvc_streaming_control probe;
	struct v4l2_fract timeperframe;
	uint32_t interval;
	int ret;

	if (parm->type != stream->type)
		return -EINVAL;

	if (parm->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
		timeperframe = parm->parm.capture.timeperframe;
	else
		timeperframe = parm->parm.output.timeperframe;

	interval = uvc_fraction_to_interval(timeperframe.numerator,
		timeperframe.denominator);
	uvc_trace(UVC_TRACE_FORMAT, "Setting frame interval to %u/%u (%u).\n",
		timeperframe.numerator, timeperframe.denominator, interval);

	mutex_lock(&stream->mutex);

	if (uvc_queue_streaming(&stream->queue)) {
		mutex_unlock(&stream->mutex);
		return -EBUSY;
	}

	memcpy(&probe, &stream->ctrl, sizeof probe);
	probe.dwFrameInterval =
		uvc_try_frame_interval(stream->cur_frame, interval);

	/* Probe the device with the new settings. */
	ret = uvc_probe_video(stream, &probe);
	if (ret < 0) {
		mutex_unlock(&stream->mutex);
		return ret;
	}

	memcpy(&stream->ctrl, &probe, sizeof probe);
	mutex_unlock(&stream->mutex);

	/* Return the actual frame period. */
	timeperframe.numerator = probe.dwFrameInterval;
	timeperframe.denominator = 10000000;
	uvc_simplify_fraction(&timeperframe.numerator,
		&timeperframe.denominator, 8, 333);

	if (parm->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {   
		parm->parm.capture.timeperframe = timeperframe;
        //yiling 2014-04-18 transfer dwMaxPayloadTransferSize to AP
        parm->parm.capture.reserved[0] = probe.dwMaxPayloadTransferSize;
    }    
	else
		parm->parm.output.timeperframe = timeperframe;

	return 0;
}

/* ------------------------------------------------------------------------
 * Privilege management
 */

/*
 * Privilege management is the multiple-open implementation basis. The current
 * implementation is completely transparent for the end-user and doesn't
 * require explicit use of the VIDIOC_G_PRIORITY and VIDIOC_S_PRIORITY ioctls.
 * Those ioctls enable finer control on the device (by making possible for a
 * user to request exclusive access to a device), but are not mature yet.
 * Switching to the V4L2 priority mechanism might be considered in the future
 * if this situation changes.
 *
 * Each open instance of a UVC device can either be in a privileged or
 * unprivileged state. Only a single instance can be in a privileged state at
 * a given time. Trying to perform an operation that requires privileges will
 * automatically acquire the required privileges if possible, or return -EBUSY
 * otherwise. Privileges are dismissed when closing the instance or when
 * freeing the video buffers using VIDIOC_REQBUFS.
 *
 * Operations that require privileges are:
 *
 * - VIDIOC_S_INPUT
 * - VIDIOC_S_PARM
 * - VIDIOC_S_FMT
 * - VIDIOC_REQBUFS
 */
static int uvc_acquire_privileges(struct uvc_fh *handle)
{
	/* Always succeed if the handle is already privileged. */
	if (handle->state == UVC_HANDLE_ACTIVE)
		return 0;

	/* Check if the device already has a privileged handle. */
	if (atomic_inc_return(&handle->stream->active) != 1) {
		atomic_dec(&handle->stream->active);
		return -EBUSY;
	}

	handle->state = UVC_HANDLE_ACTIVE;
	return 0;
}

static void uvc_dismiss_privileges(struct uvc_fh *handle)
{
	if (handle->state == UVC_HANDLE_ACTIVE)
		atomic_dec(&handle->stream->active);

	handle->state = UVC_HANDLE_PASSIVE;
}

static int uvc_has_privileges(struct uvc_fh *handle)
{
	return handle->state == UVC_HANDLE_ACTIVE;
}

/* ------------------------------------------------------------------------
 * V4L2 file operations
 */

static int uvc_v4l2_open(struct file *file)
{
	struct uvc_streaming *stream;
	struct uvc_fh *handle;
	int ret = 0;

	uvc_trace(UVC_TRACE_CALLS, "uvc_v4l2_open\n");
	
	stream = video_drvdata(file);

	if (stream->dev->state & UVC_DEV_DISCONNECTED) {
		return -ENODEV;
	}

	// 关键问题分析: USB设备恢复失败导致的 usb_autopm_get_interface 失败
	
	// 添加USB设备恢复状态的详细检查
	struct usb_device *udev = interface_to_usbdev(stream->dev->intf);
	
	// 检查设备是否处于异常状态，需要恢复
	bool need_recovery = false;
	if (udev->state == USB_STATE_SUSPENDED) {
		need_recovery = true;
	} else if (udev->state != USB_STATE_CONFIGURED) {
		need_recovery = true;
	} else if (!udev->can_submit) {
		need_recovery = true;
	}
	
	// 如果设备需要恢复，尝试恢复操作
	if (need_recovery) {
		
		// 步骤1: 尝试唤醒设备
		int wake_ret = pm_runtime_resume(&stream->dev->intf->dev);
		
		// 步骤2: 如果唤醒失败，尝试重置配置
		if (wake_ret < 0 || udev->state != USB_STATE_CONFIGURED) {
			int reset_ret = usb_reset_configuration(udev);
			
			if (reset_ret < 0) {
				reset_ret = usb_reset_device(udev);
				
				if (reset_ret < 0) {
					return -ENODEV;
				}
			}
		}
		
		// 等待设备稳定
		msleep(500);
	}
	
	// 使用增强的重试机制获取USB接口
	int retry_count = 0;
	const int max_retries = 5;  // 增加重试次数
	
	do {
		ret = usb_autopm_get_interface(stream->dev->intf);
		if (ret == 0) {
			break;
		}
		
		// 分析具体的错误类型和USB恢复失败的关系
		switch (ret) {
		case -ENODEV:
			// USB device disconnected - likely resume failure
			break;
		case -ESHUTDOWN:
			// USB interface shutting down
			break;
		case -EBUSY:
		{
			// USB interface busy - analyzing specific cause
			
			// 专门的EBUSY错误分析和处理
			struct usb_interface *intf = stream->dev->intf;
			struct usb_device *udev = interface_to_usbdev(intf);
			
			// 检查具体的EBUSY原因
			if (intf->dev.power.runtime_status == RPM_SUSPENDING) {
				// 等待挂起完成，然后强制恢复
				msleep(300 + retry_count * 200); // 递增等待时间
				pm_runtime_get_sync(&intf->dev);
				pm_runtime_mark_last_busy(&intf->dev);
			} else if (intf->dev.power.runtime_status == RPM_SUSPENDED) {
				// 强制恢复挂起的接口
				pm_runtime_get_sync(&intf->dev);
				pm_runtime_set_active(&intf->dev);
			} else {
				// 通用EBUSY恢复：重置设备状态
				pm_runtime_set_active(&intf->dev);
				pm_runtime_get_noresume(&intf->dev);
			}
		}
			break;
		case -EAGAIN:
			// USB power management temporarily unavailable
			break;
		default:
			// Unknown USB power management error
			break;
		}
		
		// 对于恢复相关的错误，不要立即放弃
		if (ret == -ENODEV) {
			// 最后一次恢复尝试
			if (retry_count == 0) {
				usb_queue_reset_device(stream->dev->intf);
				msleep(1000);  // 等待更长时间
			} else {
				break;
			}
		}
		
		// 根据错误类型调整重试延迟
		int delay_ms = 200;
		if (ret == -EBUSY) {
			delay_ms = 500;  // EBUSY可能需要更长的等待时间
		}
		
		if (retry_count < max_retries - 1) {
			msleep(delay_ms);
		}
		retry_count++;
		
	} while (retry_count < max_retries);
	
	if (ret < 0) {
		// 记录恢复失败事件，用于动态调整电源管理策略
		uvc_record_resume_failure(stream->dev, ret);
		
		return ret;
	}

	/* Create the device handle. */
	handle = kzalloc(sizeof *handle, GFP_KERNEL);
	if (handle == NULL) {
		usb_autopm_put_interface(stream->dev->intf);
		return -ENOMEM;
	}

	// 关键竞争点2: 用户计数原子增加
	
	if (atomic_inc_return(&stream->dev->users) == 1) {
		
		// 关键竞争点3: 状态启动
		ret = uvc_status_start(stream->dev);
		if (ret < 0) {
			usb_autopm_put_interface(stream->dev->intf);
			atomic_dec(&stream->dev->users);
			kfree(handle);
			return ret;
		}
	}

	handle->chain = stream->chain;
	handle->stream = stream;
	handle->state = UVC_HANDLE_PASSIVE;
	file->private_data = handle;

	// 设备成功打开，调整电源管理策略
	uvc_adjust_power_management(stream->dev, true);

	return 0;
}

static int uvc_v4l2_release(struct file *file)
{
	struct uvc_fh *handle = file->private_data;
	struct uvc_streaming *stream = handle->stream;

	uvc_trace(UVC_TRACE_CALLS, "uvc_v4l2_release\n");

	/* Only free resources if this is a privileged handle. */
	if (uvc_has_privileges(handle)) {
		uvc_video_enable(stream, 0);
		uvc_free_buffers(&stream->queue);
	}

	/* Release the file handle. */
	uvc_dismiss_privileges(handle);
	kfree(handle);
	file->private_data = NULL;

	int remaining_users = atomic_dec_return(&stream->dev->users);
	
	if (remaining_users == 0) {
		uvc_status_stop(stream->dev);
		
		// 设备不再使用，调整电源管理策略为节能模式
		uvc_adjust_power_management(stream->dev, false);
		printk(KERN_INFO "[UVC_DEBUG] uvc_v4l2_release: Applied power-saving management for idle device\n");
	}

	printk(KERN_ALERT "[UVC_DEBUG] uvc_v4l2_release: About to put autopm interface for %s\n", 
		video_device_node_name(stream->vdev));
	usb_autopm_put_interface(stream->dev->intf);
	printk(KERN_ALERT "[UVC_DEBUG] uvc_v4l2_release: Release completed for %s\n", 
		video_device_node_name(stream->vdev));
	return 0;
}

static long uvc_v4l2_do_ioctl(struct file *file, unsigned int cmd, void *arg)
{
	struct video_device *vdev = video_devdata(file);
	struct uvc_fh *handle = file->private_data;
	struct uvc_video_chain *chain = handle->chain;
	struct uvc_streaming *stream = handle->stream;
	long ret = 0;

	switch (cmd) {
	/* Query capabilities */
	case VIDIOC_QUERYCAP:
	{
		struct v4l2_capability *cap = arg;

		memset(cap, 0, sizeof *cap);
		strlcpy(cap->driver, "uvcvideo", sizeof cap->driver);
		strlcpy(cap->card, vdev->name, sizeof cap->card);
		usb_make_path(stream->dev->udev,
			      cap->bus_info, sizeof(cap->bus_info));
		cap->version = LINUX_VERSION_CODE;
		if (stream->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
			cap->capabilities = V4L2_CAP_VIDEO_CAPTURE
					  | V4L2_CAP_STREAMING;
		else
			cap->capabilities = V4L2_CAP_VIDEO_OUTPUT
					  | V4L2_CAP_STREAMING;
		break;
	}

	/* Get, Set & Query control */
	case VIDIOC_QUERYCTRL:
		return uvc_query_v4l2_ctrl(chain, arg);

	case VIDIOC_G_CTRL:
	{
		struct v4l2_control *ctrl = arg;
		struct v4l2_ext_control xctrl;

		memset(&xctrl, 0, sizeof xctrl);
		xctrl.id = ctrl->id;

		ret = uvc_ctrl_begin(chain);
		if (ret < 0)
			return ret;

		ret = uvc_ctrl_get(chain, &xctrl);
		uvc_ctrl_rollback(chain);
		if (ret >= 0)
			ctrl->value = xctrl.value;
		break;
	}

	case VIDIOC_S_CTRL:
	{
		struct v4l2_control *ctrl = arg;
		struct v4l2_ext_control xctrl;

		memset(&xctrl, 0, sizeof xctrl);
		xctrl.id = ctrl->id;
		xctrl.value = ctrl->value;

		ret = uvc_ctrl_begin(chain);
		if (ret < 0)
			return ret;

		ret = uvc_ctrl_set(chain, &xctrl);
		if (ret < 0) {
			uvc_ctrl_rollback(chain);
			return ret;
		}
		ret = uvc_ctrl_commit(chain);
		if (ret == 0)
			ctrl->value = xctrl.value;
		break;
	}

	case VIDIOC_QUERYMENU:
		return uvc_query_v4l2_menu(chain, arg);

	case VIDIOC_G_EXT_CTRLS:
	{
		struct v4l2_ext_controls *ctrls = arg;
		struct v4l2_ext_control *ctrl = ctrls->controls;
		unsigned int i;

		ret = uvc_ctrl_begin(chain);
		if (ret < 0)
			return ret;

		for (i = 0; i < ctrls->count; ++ctrl, ++i) {
			ret = uvc_ctrl_get(chain, ctrl);
			if (ret < 0) {
				uvc_ctrl_rollback(chain);
				ctrls->error_idx = i;
				return ret;
			}
		}
		ctrls->error_idx = 0;
		ret = uvc_ctrl_rollback(chain);
		break;
	}

	case VIDIOC_S_EXT_CTRLS:
	case VIDIOC_TRY_EXT_CTRLS:
	{
		struct v4l2_ext_controls *ctrls = arg;
		struct v4l2_ext_control *ctrl = ctrls->controls;
		unsigned int i;

		ret = uvc_ctrl_begin(chain);
		if (ret < 0)
			return ret;

		for (i = 0; i < ctrls->count; ++ctrl, ++i) {
			ret = uvc_ctrl_set(chain, ctrl);
			if (ret < 0) {
				uvc_ctrl_rollback(chain);
				ctrls->error_idx = i;
				return ret;
			}
		}

		ctrls->error_idx = 0;

		if (cmd == VIDIOC_S_EXT_CTRLS)
			ret = uvc_ctrl_commit(chain);
		else
			ret = uvc_ctrl_rollback(chain);
		break;
	}

	/* Get, Set & Enum input */
	case VIDIOC_ENUMINPUT:
	{
		const struct uvc_entity *selector = chain->selector;
		struct v4l2_input *input = arg;
		struct uvc_entity *iterm = NULL;
		u32 index = input->index;
		int pin = 0;

		if (selector == NULL ||
		    (chain->dev->quirks & UVC_QUIRK_IGNORE_SELECTOR_UNIT)) {
			if (index != 0)
				return -EINVAL;
			list_for_each_entry(iterm, &chain->entities, chain) {
				if (UVC_ENTITY_IS_ITERM(iterm))
					break;
			}
			pin = iterm->id;
		} else if (index < selector->bNrInPins) {
			pin = selector->baSourceID[index];
			list_for_each_entry(iterm, &chain->entities, chain) {
				if (!UVC_ENTITY_IS_ITERM(iterm))
					continue;
				if (iterm->id == pin)
					break;
			}
		}

		if (iterm == NULL || iterm->id != pin)
			return -EINVAL;

		memset(input, 0, sizeof *input);
		input->index = index;
		strlcpy(input->name, iterm->name, sizeof input->name);
		if (UVC_ENTITY_TYPE(iterm) == UVC_ITT_CAMERA)
			input->type = V4L2_INPUT_TYPE_CAMERA;
		break;
	}

	case VIDIOC_G_INPUT:
	{
		u8 input;

		if (chain->selector == NULL ||
		    (chain->dev->quirks & UVC_QUIRK_IGNORE_SELECTOR_UNIT)) {
			*(int *)arg = 0;
			break;
		}

		ret = uvc_query_ctrl(chain->dev, UVC_GET_CUR,
			chain->selector->id, chain->dev->intfnum,
			UVC_SU_INPUT_SELECT_CONTROL, &input, 1);
		if (ret < 0)
			return ret;

		*(int *)arg = input - 1;
		break;
	}

	case VIDIOC_S_INPUT:
	{
		u32 input = *(u32 *)arg + 1;

		if ((ret = uvc_acquire_privileges(handle)) < 0)
			return ret;

		if (chain->selector == NULL ||
		    (chain->dev->quirks & UVC_QUIRK_IGNORE_SELECTOR_UNIT)) {
			if (input != 1)
				return -EINVAL;
			break;
		}

		if (input == 0 || input > chain->selector->bNrInPins)
			return -EINVAL;

		return uvc_query_ctrl(chain->dev, UVC_SET_CUR,
			chain->selector->id, chain->dev->intfnum,
			UVC_SU_INPUT_SELECT_CONTROL, &input, 1);
	}

	/* Try, Get, Set & Enum format */
	case VIDIOC_ENUM_FMT:
	{
		struct v4l2_fmtdesc *fmt = arg;
		struct uvc_format *format;
		enum v4l2_buf_type type = fmt->type;
		__u32 index = fmt->index;

		if (fmt->type != stream->type ||
		    fmt->index >= stream->nformats)
			return -EINVAL;

		memset(fmt, 0, sizeof(*fmt));
		fmt->index = index;
		fmt->type = type;

		format = &stream->format[fmt->index];
		fmt->flags = 0;
		if (format->flags & UVC_FMT_FLAG_COMPRESSED)
			fmt->flags |= V4L2_FMT_FLAG_COMPRESSED;
		strlcpy(fmt->description, format->name,
			sizeof fmt->description);
		fmt->description[sizeof fmt->description - 1] = 0;
		fmt->pixelformat = format->fcc;
		break;
	}

	case VIDIOC_TRY_FMT:
	{
		struct uvc_streaming_control probe;

		return uvc_v4l2_try_format(stream, arg, &probe, NULL, NULL);
	}

	case VIDIOC_S_FMT:
		if ((ret = uvc_acquire_privileges(handle)) < 0)
			return ret;

		return uvc_v4l2_set_format(stream, arg);

	case VIDIOC_G_FMT:
		return uvc_v4l2_get_format(stream, arg);

	case UVCIOC_STILL_S_FMT:	//still image
		if (!uvc_has_privileges(handle))
			return -EBUSY;
		return uvc_v4l2_set_still_format(stream,arg);

	/* Frame size enumeration */
	case VIDIOC_ENUM_FRAMESIZES:
	{
		struct v4l2_frmsizeenum *fsize = arg;
		struct uvc_format *format = NULL;
		struct uvc_frame *frame;
		int i;

		/* Look for the given pixel format */
		for (i = 0; i < stream->nformats; i++) {
			if (stream->format[i].fcc ==
					fsize->pixel_format) {
				format = &stream->format[i];
				break;
			}
		}
		if (format == NULL)
			return -EINVAL;

		if (fsize->index >= format->nframes)
			return -EINVAL;

		frame = &format->frame[fsize->index];
		fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		fsize->discrete.width = frame->wWidth;
		fsize->discrete.height = frame->wHeight;
		break;
	}

	/* Frame interval enumeration */
	case VIDIOC_ENUM_FRAMEINTERVALS:
	{
		struct v4l2_frmivalenum *fival = arg;
		struct uvc_format *format = NULL;
		struct uvc_frame *frame = NULL;
		int i;

		/* Look for the given pixel format and frame size */
		for (i = 0; i < stream->nformats; i++) {
			if (stream->format[i].fcc ==
					fival->pixel_format) {
				format = &stream->format[i];
				break;
			}
		}
		if (format == NULL)
			return -EINVAL;

		for (i = 0; i < format->nframes; i++) {
			if (format->frame[i].wWidth == fival->width &&
			    format->frame[i].wHeight == fival->height) {
				frame = &format->frame[i];
				break;
			}
		}
		if (frame == NULL)
			return -EINVAL;

		if (frame->bFrameIntervalType) {
			if (fival->index >= frame->bFrameIntervalType)
				return -EINVAL;

			fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
			fival->discrete.numerator =
				frame->dwFrameInterval[fival->index];
			fival->discrete.denominator = 10000000;
			uvc_simplify_fraction(&fival->discrete.numerator,
				&fival->discrete.denominator, 8, 333);
		} else {
			fival->type = V4L2_FRMIVAL_TYPE_STEPWISE;
			fival->stepwise.min.numerator =
				frame->dwFrameInterval[0];
			fival->stepwise.min.denominator = 10000000;
			fival->stepwise.max.numerator =
				frame->dwFrameInterval[1];
			fival->stepwise.max.denominator = 10000000;
			fival->stepwise.step.numerator =
				frame->dwFrameInterval[2];
			fival->stepwise.step.denominator = 10000000;
			uvc_simplify_fraction(&fival->stepwise.min.numerator,
				&fival->stepwise.min.denominator, 8, 333);
			uvc_simplify_fraction(&fival->stepwise.max.numerator,
				&fival->stepwise.max.denominator, 8, 333);
			uvc_simplify_fraction(&fival->stepwise.step.numerator,
				&fival->stepwise.step.denominator, 8, 333);
		}
		break;
	}

	/* Get & Set streaming parameters */
	case VIDIOC_G_PARM:
		return uvc_v4l2_get_streamparm(stream, arg);

	case VIDIOC_S_PARM:
		if ((ret = uvc_acquire_privileges(handle)) < 0)
			return ret;

		return uvc_v4l2_set_streamparm(stream, arg);

	/* Cropping and scaling */
	case VIDIOC_CROPCAP:
	{
		struct v4l2_cropcap *ccap = arg;

		if (ccap->type != stream->type)
			return -EINVAL;

		ccap->bounds.left = 0;
		ccap->bounds.top = 0;

		mutex_lock(&stream->mutex);
		ccap->bounds.width = stream->cur_frame->wWidth;
		ccap->bounds.height = stream->cur_frame->wHeight;
		mutex_unlock(&stream->mutex);

		ccap->defrect = ccap->bounds;

		ccap->pixelaspect.numerator = 1;
		ccap->pixelaspect.denominator = 1;
		break;
	}

	case VIDIOC_G_CROP:
	case VIDIOC_S_CROP:
		return -EINVAL;

	/* Buffers & streaming */
	case VIDIOC_REQBUFS:
		if ((ret = uvc_acquire_privileges(handle)) < 0)
			return ret;

		mutex_lock(&stream->mutex);
		ret = uvc_alloc_buffers(&stream->queue, arg);
		mutex_unlock(&stream->mutex);
		if (ret < 0)
			return ret;

		if (ret == 0)
			uvc_dismiss_privileges(handle);

		ret = 0;
		break;

	case UVCIOC_STILL_REQBUF:
		if (!uvc_has_privileges(handle))
			return -EBUSY;

		mutex_lock(&stream->mutex);
		ret = uvc_alloc_still_buffer(&stream->still_queue, arg);
		mutex_unlock(&stream->mutex);
		if (ret < 0)
			return ret;
		break;
	case VIDIOC_QUERYBUF:
	{
		struct v4l2_buffer *buf = arg;

		if (!uvc_has_privileges(handle))
			return -EBUSY;

		return uvc_query_buffer(&stream->queue, buf);
	}

	case UVCIOC_STILL_QUERYBUF:
	{
		struct v4l2_buffer *buf = arg;

		if (!uvc_has_privileges(handle))
			return -EBUSY;
		return uvc_query_still_buffer(&stream->still_queue, buf);
	}
	case VIDIOC_QBUF:
		if (!uvc_has_privileges(handle))
			return -EBUSY;

		return uvc_queue_buffer(&stream->queue, arg);

	case VIDIOC_DQBUF:
	{	// yiling 2013-12-11 SN9C292A/SN9C291B remove unnecessary frame data +++
		int ret = 0, counter = 4;
		bool IsH264_16X16 = 0, IsMJ_MarkerErr = 0; 
		struct v4l2_buffer *buf = arg;
		
		if (!uvc_has_privileges(handle))
			return -EBUSY;
		ret = uvc_dequeue_buffer(&stream->queue, buf, file->f_flags & O_NONBLOCK);


			
		IsH264_16X16 = stream->cur_format->fcc == V4L2_PIX_FMT_H264 && buf->reserved == 0x00100010;
		IsMJ_MarkerErr = stream->cur_format->fcc == V4L2_PIX_FMT_MJPEG && (buf->reserved&0x80000000);
		
		// 通用USB摄像头 - 移除SONIX特定帧检查逻辑
		// 对于通用USB摄像头，直接返回缓冲区数据
		buf->reserved &= (~0x80000000);	// 清除标记错误位
		return ret;
	}	// yiling 2013-12-11 SN9C292A/SN9C291B remove unnecessary frame data ---

	case UVCIOC_STILL_GET_FRAME:
		if (!uvc_has_privileges(handle))
			return -EBUSY;
		return uvc_still_image_get_frame(stream, arg, file->f_flags & O_NONBLOCK);
	
	case VIDIOC_STREAMON:
	{
		int *type = arg;
		unsigned int temp;

		if (*type != stream->type)
			return -EINVAL;

		if (!uvc_has_privileges(handle))
			return -EBUSY;

		mutex_lock(&stream->mutex);

		if(stream->cur_format->fcc == V4L2_PIX_FMT_YUYV) {
			DRAMC_WRITE32(0x8fa132e6, 0x10);
			DRAMC_WRITE32(0xfb54dc79, 0x14);
			temp = DRAMC_READ32(0x6c);
			temp = temp | 1;
			DRAMC_WRITE32(temp, 0x6c);
		}

		ret = uvc_video_enable(stream, 1);
		mutex_unlock(&stream->mutex);
		if (ret < 0)
			return ret;
		break;
	}

	case VIDIOC_STREAMOFF:
	{
		int *type = arg;
		unsigned int temp;

		if (*type != stream->type)
			return -EINVAL;

		if (!uvc_has_privileges(handle))
			return -EBUSY;

		if (stream->cur_format->fcc == V4L2_PIX_FMT_YUYV) {
			DRAMC_WRITE32(0x7f91e2d5, 0x10);
			DRAMC_WRITE32(0xfa43cb68, 0x14);
			temp = DRAMC_READ32(0x6c);
			temp = temp | 1;
			DRAMC_WRITE32(temp, 0x6c);
		}

		return uvc_video_enable(stream, 0);
	}

	/* Analog video standards make no sense for digital cameras. */
	case VIDIOC_ENUMSTD:
	case VIDIOC_QUERYSTD:
	case VIDIOC_G_STD:
	case VIDIOC_S_STD:

	case VIDIOC_OVERLAY:

	case VIDIOC_ENUMAUDIO:
	case VIDIOC_ENUMAUDOUT:

	case VIDIOC_ENUMOUTPUT:
		uvc_trace(UVC_TRACE_IOCTL, "Unsupported ioctl 0x%08x\n", cmd);
		return -EINVAL;

	case UVCIOC_CTRL_MAP:
		return uvc_ioctl_ctrl_map(chain, arg);

	case UVCIOC_CTRL_QUERY:
		/* Memory access debugging - XU Control Query entry */
		printk(KERN_ALERT "[UVC_MEM_DEBUG] uvc_v4l2_do_ioctl: UVCIOC_CTRL_QUERY - PID=%d COMM=%s TID=%ld TGID=%d\n", 
			current->pid, current->comm, (long)current->pid, current->tgid);
		printk(KERN_ALERT "[UVC_MEM_DEBUG] uvc_v4l2_do_ioctl: About to call uvc_xu_ctrl_query (potential crash point)\n");
		
		return uvc_xu_ctrl_query(chain, arg);

	default:
		uvc_trace(UVC_TRACE_IOCTL, "Unknown ioctl 0x%08x\n", cmd);
		return -EINVAL;
	}

	return ret;
}

static long uvc_v4l2_ioctl(struct file *file,
		     unsigned int cmd, unsigned long arg)
{
	if (uvc_trace_param & UVC_TRACE_IOCTL) {
		uvc_printk(KERN_DEBUG, "uvc_v4l2_ioctl(");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,13)
		v4l_printk_ioctl(NULL, cmd);
#else	
		v4l_printk_ioctl(cmd);
#endif		
		printk(")\n");
	}

	return video_usercopy(file, cmd, arg, uvc_v4l2_do_ioctl);
}
#ifdef CONFIG_COMPAT
struct uvc_xu_control_mapping32 {
	__u32 id;
	__u8 name[32];
	__u8 entity[16];
	__u8 selector;

	__u8 size;
	__u8 offset;
	__u32 v4l2_type;
	__u32 data_type;

	compat_caddr_t menu_info;
	__u32 menu_count;

	__u32 reserved[4];
};

static int uvc_v4l2_get_xu_mapping(struct uvc_xu_control_mapping *kp,
			const struct uvc_xu_control_mapping32 __user *up)
{
	struct uvc_menu_info __user *umenus;
	struct uvc_menu_info __user *kmenus;
	compat_caddr_t p;

	if (!access_ok(VERIFY_READ, up, sizeof(*up)) ||
	    __copy_from_user(kp, up, offsetof(typeof(*up), menu_info)) ||
	    __get_user(kp->menu_count, &up->menu_count))
		return -EFAULT;

	memset(kp->reserved, 0, sizeof(kp->reserved));

	if (kp->menu_count == 0) {
		kp->menu_info = NULL;
		return 0;
	}

	if (__get_user(p, &up->menu_info))
		return -EFAULT;
	umenus = compat_ptr(p);
	if (!access_ok(VERIFY_READ, umenus, kp->menu_count * sizeof(*umenus)))
		return -EFAULT;

	kmenus = compat_alloc_user_space(kp->menu_count * sizeof(*kmenus));
	if (kmenus == NULL)
		return -EFAULT;
	kp->menu_info = kmenus;

	if (copy_in_user(kmenus, umenus, kp->menu_count * sizeof(*umenus)))
		return -EFAULT;

	return 0;
}

static int uvc_v4l2_put_xu_mapping(const struct uvc_xu_control_mapping *kp,
			struct uvc_xu_control_mapping32 __user *up)
{
	struct uvc_menu_info __user *umenus;
	struct uvc_menu_info __user *kmenus = kp->menu_info;
	compat_caddr_t p;

	if (!access_ok(VERIFY_WRITE, up, sizeof(*up)) ||
	    __copy_to_user(up, kp, offsetof(typeof(*up), menu_info)) ||
	    __put_user(kp->menu_count, &up->menu_count))
		return -EFAULT;

	if (__clear_user(up->reserved, sizeof(up->reserved)))
		return -EFAULT;

	if (kp->menu_count == 0)
		return 0;

	if (get_user(p, &up->menu_info))
		return -EFAULT;
	umenus = compat_ptr(p);

	if (copy_in_user(umenus, kmenus, kp->menu_count * sizeof(*umenus)))
		return -EFAULT;

	return 0;
}

struct uvc_xu_control_query32 {
	__u8 unit;
	__u8 selector;
	__u8 query;
	__u16 size;
	compat_caddr_t data;
};

static int uvc_v4l2_get_xu_query(struct uvc_xu_control_query *kp,
			const struct uvc_xu_control_query32 __user *up)
{
	u8 __user *udata;
	u8 __user *kdata;
	compat_caddr_t p;

	if (!access_ok(VERIFY_READ, up, sizeof(*up)) ||
	    __copy_from_user(kp, up, offsetof(typeof(*up), data)))
		return -EFAULT;

	if (kp->size == 0) {
		kp->data = NULL;
		return 0;
	}

	if (__get_user(p, &up->data))
		return -EFAULT;
	udata = compat_ptr(p);
	if (!access_ok(VERIFY_READ, udata, kp->size))
		return -EFAULT;

	kdata = compat_alloc_user_space(kp->size);
	if (kdata == NULL)
		return -EFAULT;
	kp->data = kdata;

	if (copy_in_user(kdata, udata, kp->size))
		return -EFAULT;

	return 0;
}

static int uvc_v4l2_put_xu_query(const struct uvc_xu_control_query *kp,
			struct uvc_xu_control_query32 __user *up)
{
	u8 __user *udata;
	u8 __user *kdata = kp->data;
	compat_caddr_t p;

	if (!access_ok(VERIFY_WRITE, up, sizeof(*up)) ||
	    __copy_to_user(up, kp, offsetof(typeof(*up), data)))
		return -EFAULT;

	if (kp->size == 0)
		return 0;

	if (get_user(p, &up->data))
		return -EFAULT;
	udata = compat_ptr(p);
	if (!access_ok(VERIFY_READ, udata, kp->size))
		return -EFAULT;

	if (copy_in_user(udata, kdata, kp->size))
		return -EFAULT;

	return 0;
}

#define UVCIOC_CTRL_MAP32	_IOWR('u', 0x20, struct uvc_xu_control_mapping32)
#define UVCIOC_CTRL_QUERY32	_IOWR('u', 0x21, struct uvc_xu_control_query32)

static long uvc_v4l2_compat_ioctl32(struct file *file,
		     unsigned int cmd, unsigned long arg)
{
	union {
		struct uvc_xu_control_mapping xmap;
		struct uvc_xu_control_query xqry;
	} karg;
	void __user *up = compat_ptr(arg);
	mm_segment_t old_fs;
	long ret;

	switch (cmd) {
	case UVCIOC_CTRL_MAP32:
		cmd = UVCIOC_CTRL_MAP;
		ret = uvc_v4l2_get_xu_mapping(&karg.xmap, up);
		break;

	case UVCIOC_CTRL_QUERY32:
		cmd = UVCIOC_CTRL_QUERY;
		ret = uvc_v4l2_get_xu_query(&karg.xqry, up);
		break;

	default:
		return -ENOIOCTLCMD;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = uvc_v4l2_ioctl(file, cmd, (unsigned long)&karg);
	set_fs(old_fs);

	if (ret < 0)
		return ret;

	switch (cmd) {
	case UVCIOC_CTRL_MAP:
		ret = uvc_v4l2_put_xu_mapping(&karg.xmap, up);
		break;

	case UVCIOC_CTRL_QUERY:
		ret = uvc_v4l2_put_xu_query(&karg.xqry, up);
		break;
	}

	return ret;
}
#endif

static ssize_t uvc_v4l2_read(struct file *file, char __user *data,
		    size_t count, loff_t *ppos)
{
	uvc_trace(UVC_TRACE_CALLS, "uvc_v4l2_read: not implemented.\n");
	return -EINVAL;
}

static int uvc_v4l2_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct uvc_fh *handle = file->private_data;
	struct uvc_streaming *stream = handle->stream;

	uvc_trace(UVC_TRACE_CALLS, "uvc_v4l2_mmap\n");
	if(vma->vm_pgoff == (STILL_IMAGE_MMAP_OFFSET>>PAGE_SHIFT))
	{
		uvc_trace(UVC_TRACE_CALLS, "uvc_v4l2_mmap:still\n");
		vma->vm_pgoff -= (STILL_IMAGE_MMAP_OFFSET>>PAGE_SHIFT);
		uvc_trace(UVC_TRACE_CALLS, "uvc_v4l2_mmap(still) off=%lx\n", vma->vm_pgoff);
		return uvc_still_queue_mmap(&stream->still_queue, vma);
	}
	else
		return uvc_queue_mmap(&stream->queue, vma);
}

static unsigned int uvc_v4l2_poll(struct file *file, poll_table *wait)
{
	struct uvc_fh *handle = file->private_data;
	struct uvc_streaming *stream = handle->stream;

	uvc_trace(UVC_TRACE_CALLS, "uvc_v4l2_poll\n");

	return uvc_queue_poll(&stream->queue, file, wait);
}

#ifndef CONFIG_MMU
static unsigned long uvc_v4l2_get_unmapped_area(struct file *file,
		unsigned long addr, unsigned long len, unsigned long pgoff,
		unsigned long flags)
{
	struct uvc_fh *handle = file->private_data;
	struct uvc_streaming *stream = handle->stream;

	uvc_trace(UVC_TRACE_CALLS, "uvc_v4l2_get_unmapped_area\n");

	return uvc_queue_get_unmapped_area(&stream->queue, pgoff);
}
#endif

const struct v4l2_file_operations uvc_fops = {
	.owner		= THIS_MODULE,
	.open		= uvc_v4l2_open,
	.release	= uvc_v4l2_release,
	.unlocked_ioctl	= uvc_v4l2_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32	= uvc_v4l2_compat_ioctl32,
#endif
	.read		= uvc_v4l2_read,
	.mmap		= uvc_v4l2_mmap,
	.poll		= uvc_v4l2_poll,
#ifndef CONFIG_MMU
	.get_unmapped_area = uvc_v4l2_get_unmapped_area,
#endif
};

