/*
 *      uvc_driver.c  --  USB Video Class driver
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

/*
 * This driver aims to support video input and ouput devices compliant with the
 * 'USB Video Class' specification.
 *
 * The driver doesn't support the deprecated v4l1 interface. It implements the
 * mmap capture method only, and doesn't do any image format conversion in
 * software. If your user-space application doesn't support YUYV or MJPEG, fix
 * it :-). Please note that the MJPEG data have been stripped from their
 * Huffman tables (DHT marker), you will need to add it back if your JPEG
 * codec can't handle MJPEG data.
 */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/videodev2.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/pm_runtime.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>
#include <media/v4l2-common.h>

#include <mach/ac83xx.h>

#include "uvcvideo.h"

#define DRIVER_AUTHOR		"Laurent Pinchart " \
				"<laurent.pinchart@ideasonboard.com>"
#define DRIVER_DESC		"USB Video Class driver"

unsigned int uvc_clock_param = CLOCK_MONOTONIC;
unsigned int uvc_no_drop_param;
static unsigned int uvc_quirks_param = -1;
unsigned int uvc_trace_param = 0x3F;
unsigned int uvc_timeout_param = UVC_CTRL_STREAMING_TIMEOUT;

// Houston adds 2010/12/02 for Debug print
unsigned int DbgPrint_param = 0;

static int uvc_number = 0;
#define UVC_GENERIC_VIDEO_NUM_FRONT  20    // USB Bus 2 /dev/video20 固定映射
#define UVC_GENERIC_VIDEO_NUM_REAR   21    // USB Bus 1 /dev/video21 固定映射
#define UVC_MAX_DEVICES            2     // 最多支持2个设备

// 双路摄像头设备管理
static struct uvc_device *uvc_devices[UVC_MAX_DEVICES] = {NULL, NULL};
static int uvc_device_count = 0;

// 重新探测追踪
static int uvc_reprobe_count = 0;
static unsigned long uvc_last_probe_time = 0;
static unsigned long uvc_first_probe_time = 0;
/* ------------------------------------------------------------------------
 * Video formats
 */
void * __iomem DRAMC_BASE;

static struct uvc_format_desc uvc_fmts[] = {
	{
		.name		= "YUV 4:2:2 (YUYV)",
		.guid		= UVC_GUID_FORMAT_YUY2,
		.fcc		= V4L2_PIX_FMT_YUYV,
	},
	{
		.name		= "YUV 4:2:2 (YUYV)",
		.guid		= UVC_GUID_FORMAT_YUY2_ISIGHT,
		.fcc		= V4L2_PIX_FMT_YUYV,
	},
	{
		.name		= "YUV 4:2:0 (NV12)",
		.guid		= UVC_GUID_FORMAT_NV12,
		.fcc		= V4L2_PIX_FMT_NV12,
	},
	{
		.name		= "MJPEG",
		.guid		= UVC_GUID_FORMAT_MJPEG,
		.fcc		= V4L2_PIX_FMT_MJPEG,
	},
	{
		.name		= "YVU 4:2:0 (YV12)",
		.guid		= UVC_GUID_FORMAT_YV12,
		.fcc		= V4L2_PIX_FMT_YVU420,
	},
	{
		.name		= "YUV 4:2:0 (I420)",
		.guid		= UVC_GUID_FORMAT_I420,
		.fcc		= V4L2_PIX_FMT_YUV420,
	},
	{
		.name		= "YUV 4:2:0 (M420)",
		.guid		= UVC_GUID_FORMAT_M420,
		.fcc		= V4L2_PIX_FMT_M420,
	},
	{
		.name		= "YUV 4:2:2 (UYVY)",
		.guid		= UVC_GUID_FORMAT_UYVY,
		.fcc		= V4L2_PIX_FMT_UYVY,
	},
	{
		.name		= "Greyscale (8-bit)",
		.guid		= UVC_GUID_FORMAT_Y800,
		.fcc		= V4L2_PIX_FMT_GREY,
	},
	{
		.name		= "Greyscale (16-bit)",
		.guid		= UVC_GUID_FORMAT_Y16,
		.fcc		= V4L2_PIX_FMT_Y16,
	},
	{
		.name		= "RGB Bayer",
		.guid		= UVC_GUID_FORMAT_BY8,
		.fcc		= V4L2_PIX_FMT_SBGGR8,
	},
	{
		.name		= "RGB565",
		.guid		= UVC_GUID_FORMAT_RGBP,
		.fcc		= V4L2_PIX_FMT_RGB565,
	},
	{
		.name		= "H.264",
		.guid		= UVC_GUID_FORMAT_H264,
		.fcc		= V4L2_PIX_FMT_H264,
	},
};

/* ------------------------------------------------------------------------
 * Utility functions
 */

struct usb_host_endpoint *uvc_find_endpoint(struct usb_host_interface *alts,
		__u8 epaddr)
{
	struct usb_host_endpoint *ep;
	unsigned int i;

	for (i = 0; i < alts->desc.bNumEndpoints; ++i) {
		ep = &alts->endpoint[i];
		if (ep->desc.bEndpointAddress == epaddr)
			return ep;
	}

	return NULL;
}

static struct uvc_format_desc *uvc_format_by_guid(const __u8 guid[16])
{
	unsigned int len = ARRAY_SIZE(uvc_fmts);
	unsigned int i;

	for (i = 0; i < len; ++i) {
		if (memcmp(guid, uvc_fmts[i].guid, 16) == 0)
			return &uvc_fmts[i];
	}

	return NULL;
}

static __u32 uvc_colorspace(const __u8 primaries)
{
	static const __u8 colorprimaries[] = {
		0,
		V4L2_COLORSPACE_SRGB,
		V4L2_COLORSPACE_470_SYSTEM_M,
		V4L2_COLORSPACE_470_SYSTEM_BG,
		V4L2_COLORSPACE_SMPTE170M,
		V4L2_COLORSPACE_SMPTE240M,
	};

	if (primaries < ARRAY_SIZE(colorprimaries))
		return colorprimaries[primaries];

	return 0;
}

/* Simplify a fraction using a simple continued fraction decomposition. The
 * idea here is to convert fractions such as 333333/10000000 to 1/30 using
 * 32 bit arithmetic only. The algorithm is not perfect and relies upon two
 * arbitrary parameters to remove non-significative terms from the simple
 * continued fraction decomposition. Using 8 and 333 for n_terms and threshold
 * respectively seems to give nice results.
 */
void uvc_simplify_fraction(uint32_t *numerator, uint32_t *denominator,
		unsigned int n_terms, unsigned int threshold)
{
	uint32_t *an;
	uint32_t x, y, r;
	unsigned int i, n;

	an = kmalloc(n_terms * sizeof *an, GFP_KERNEL);
	if (an == NULL)
		return;

	/* Convert the fraction to a simple continued fraction. See
	 * http://mathforum.org/dr.math/faq/faq.fractions.html
	 * Stop if the current term is bigger than or equal to the given
	 * threshold.
	 */
	x = *numerator;
	y = *denominator;

	for (n = 0; n < n_terms && y != 0; ++n) {
		an[n] = x / y;
		if (an[n] >= threshold) {
			if (n < 2)
				n++;
			break;
		}

		r = x - an[n] * y;
		x = y;
		y = r;
	}

	/* Expand the simple continued fraction back to an integer fraction. */
	x = 0;
	y = 1;

	for (i = n; i > 0; --i) {
		r = y;
		y = an[i-1] * y + x;
		x = r;
	}

	*numerator = y;
	*denominator = x;
	kfree(an);
}

/* Convert a fraction to a frame interval in 100ns multiples. The idea here is
 * to compute numerator / denominator * 10000000 using 32 bit fixed point
 * arithmetic only.
 */
uint32_t uvc_fraction_to_interval(uint32_t numerator, uint32_t denominator)
{
	uint32_t multiplier;

	/* Saturate the result if the operation would overflow. */
	if (denominator == 0 ||
	    numerator/denominator >= ((uint32_t)-1)/10000000)
		return (uint32_t)-1;

	/* Divide both the denominator and the multiplier by two until
	 * numerator * multiplier doesn't overflow. If anyone knows a better
	 * algorithm please let me know.
	 */
	multiplier = 10000000;
	while (numerator > ((uint32_t)-1)/multiplier) {
		multiplier /= 2;
		denominator /= 2;
	}

	return denominator ? numerator * multiplier / denominator : 0;
}

/* ------------------------------------------------------------------------
 * Terminal and unit management
 */

struct uvc_entity *uvc_entity_by_id(struct uvc_device *dev, int id)
{
	struct uvc_entity *entity;

	list_for_each_entry(entity, &dev->entities, list) {
		if (entity->id == id)
			return entity;
	}

	return NULL;
}

static struct uvc_entity *uvc_entity_by_reference(struct uvc_device *dev,
	int id, struct uvc_entity *entity)
{
	unsigned int i;

	if (entity == NULL)
		entity = list_entry(&dev->entities, struct uvc_entity, list);

	list_for_each_entry_continue(entity, &dev->entities, list) {
		for (i = 0; i < entity->bNrInPins; ++i)
			if (entity->baSourceID[i] == id)
				return entity;
	}

	return NULL;
}

static struct uvc_streaming *uvc_stream_by_id(struct uvc_device *dev, int id)
{
	struct uvc_streaming *stream;

	list_for_each_entry(stream, &dev->streams, list) {
		if (stream->header.bTerminalLink == id)
			return stream;
	}

	return NULL;
}

/* ------------------------------------------------------------------------
 * Descriptors parsing
 */

/* ------------------------------------------------------------------------
 * Dynamic Power Management Functions
 */

/*
 * 动态调整USB电源管理策略
 * 根据设备使用情况和错误历史动态调整电源管理
 */
void uvc_adjust_power_management(struct uvc_device *dev, bool in_use)
{
	struct usb_device *udev = interface_to_usbdev(dev->intf);
	
	printk(KERN_INFO "[UVC_DYNAMIC_PM] Adjusting power management for device %s (in_use=%d)\n", 
		dev->name, in_use);
	
	// 禁用自动挂起但保持电源管理接口可访问
	printk(KERN_INFO "[UVC_DYNAMIC_PM] Disabling autosuspend while keeping PM interface accessible\n");
	
	// 始终禁用USB设备的自动挂起
	usb_disable_autosuspend(udev);
	
	// 确保设备始终保持活跃状态
	pm_runtime_set_active(&dev->intf->dev);
	pm_runtime_mark_last_busy(&dev->intf->dev);
	
	// 设置永不自动挂起，但不禁止电源管理访问
	pm_runtime_set_autosuspend_delay(&dev->intf->dev, -1);
	pm_runtime_dont_use_autosuspend(&dev->intf->dev);
	
	if (in_use) {
		printk(KERN_INFO "[UVC_DYNAMIC_PM] Device in use, maintaining active state\n");
		// 设备使用中时确保活跃
		pm_runtime_get_sync(&dev->intf->dev);
	} else {
		printk(KERN_INFO "[UVC_DYNAMIC_PM] Device idle, but keeping accessible\n");
		// 即使闲置也保持活跃状态，避免虚假断开
		pm_runtime_mark_last_busy(&dev->intf->dev);
	}
}

/*
 * 记录USB恢复失败事件，用于动态调整策略
 */
void uvc_record_resume_failure(struct uvc_device *dev, int error_code)
{
	if (!dev) return;
	
	dev->resume_failure_count++;
	dev->last_resume_error = error_code;
	
	printk(KERN_WARNING "[UVC_RESUME_FAILURE] Recording resume failure #%d for device %s (error=%d)\n", 
		dev->resume_failure_count, dev->name, error_code);
	
	// 注意：由于已完全禁用autosuspend，此函数主要用于日志记录
	printk(KERN_INFO "[UVC_RESUME_FAILURE] Autosuspend is globally disabled, no PM adjustment needed\n");
}

/* ------------------------------------------------------------------------
 * Descriptors parsing
 */

static int uvc_parse_format(struct uvc_device *dev,
	struct uvc_streaming *streaming, struct uvc_format *format,
	__u32 **intervals, unsigned char *buffer, int buflen)
{
	struct usb_interface *intf = streaming->intf;
	struct usb_host_interface *alts = intf->cur_altsetting;
	struct uvc_format_desc *fmtdesc;
	struct uvc_frame *frame;
	const unsigned char *start = buffer;
	unsigned int interval;
	unsigned int i, n;
	__u8 ftype;

	format->type = buffer[2];
	format->index = buffer[3];

	switch (buffer[2]) {
	case UVC_VS_FORMAT_UNCOMPRESSED:
	case UVC_VS_FORMAT_FRAME_BASED:
		n = buffer[2] == UVC_VS_FORMAT_UNCOMPRESSED ? 27 : 28;
		if (buflen < n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
			       "interface %d FORMAT error\n",
			       dev->udev->devnum,
			       alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		/* Find the format descriptor from its GUID. */
		fmtdesc = uvc_format_by_guid(&buffer[5]);

		if (fmtdesc != NULL) {
			strlcpy(format->name, fmtdesc->name,
				sizeof format->name);
			format->fcc = fmtdesc->fcc;
		} else {
			uvc_printk(KERN_INFO, "Unknown video format %pUl\n",
				&buffer[5]);
			snprintf(format->name, sizeof(format->name), "%pUl\n",
				&buffer[5]);
			format->fcc = 0;
		}

		format->bpp = buffer[21];
		if (buffer[2] == UVC_VS_FORMAT_UNCOMPRESSED) {
			ftype = UVC_VS_FRAME_UNCOMPRESSED;
		} else {
			ftype = UVC_VS_FRAME_FRAME_BASED;
			if (buffer[27])
				format->flags = UVC_FMT_FLAG_COMPRESSED;
		}
		break;

	case UVC_VS_FORMAT_MJPEG:
		if (buflen < 11) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
			       "interface %d FORMAT error\n",
			       dev->udev->devnum,
			       alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		strlcpy(format->name, "MJPEG", sizeof format->name);
		format->fcc = V4L2_PIX_FMT_MJPEG;
		format->flags = UVC_FMT_FLAG_COMPRESSED;
		format->bpp = 0;
		ftype = UVC_VS_FRAME_MJPEG;
		break;

	case UVC_VS_FORMAT_DV:
		if (buflen < 9) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
			       "interface %d FORMAT error\n",
			       dev->udev->devnum,
			       alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		switch (buffer[8] & 0x7f) {
		case 0:
			strlcpy(format->name, "SD-DV", sizeof format->name);
			break;
		case 1:
			strlcpy(format->name, "SDL-DV", sizeof format->name);
			break;
		case 2:
			strlcpy(format->name, "HD-DV", sizeof format->name);
			break;
		default:
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
			       "interface %d: unknown DV format %u\n",
			       dev->udev->devnum,
			       alts->desc.bInterfaceNumber, buffer[8]);
			return -EINVAL;
		}

		strlcat(format->name, buffer[8] & (1 << 7) ? " 60Hz" : " 50Hz",
			sizeof format->name);

		format->fcc = V4L2_PIX_FMT_DV;
		format->flags = UVC_FMT_FLAG_COMPRESSED | UVC_FMT_FLAG_STREAM;
		format->bpp = 0;
		ftype = 0;

		/* Create a dummy frame descriptor. */
		frame = &format->frame[0];
		memset(&format->frame[0], 0, sizeof format->frame[0]);
		frame->bFrameIntervalType = 1;
		frame->dwDefaultFrameInterval = 1;
		frame->dwFrameInterval = *intervals;
		*(*intervals)++ = 1;
		format->nframes = 1;
		break;

	case UVC_VS_FORMAT_MPEG2TS:
	case UVC_VS_FORMAT_STREAM_BASED:
		/* Not supported yet. */
	default:
		uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
		       "interface %d unsupported format %u\n",
		       dev->udev->devnum, alts->desc.bInterfaceNumber,
		       buffer[2]);
		return -EINVAL;
	}

	uvc_trace(UVC_TRACE_DESCR, "Found format %s.\n", format->name);

	buflen -= buffer[0];
	buffer += buffer[0];

	/* Parse the frame descriptors. Only uncompressed, MJPEG and frame
	 * based formats have frame descriptors.
	 */
	while (buflen > 2 && buffer[1] == USB_DT_CS_INTERFACE &&
	       buffer[2] == ftype) {
		frame = &format->frame[format->nframes];
		if (ftype != UVC_VS_FRAME_FRAME_BASED)
			n = buflen > 25 ? buffer[25] : 0;
		else
			n = buflen > 21 ? buffer[21] : 0;

		n = n ? n : 3;

		if (buflen < 26 + 4*n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
			       "interface %d FRAME error\n", dev->udev->devnum,
			       alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		frame->bFrameIndex = buffer[3];
		frame->bmCapabilities = buffer[4];
		frame->wWidth = get_unaligned_le16(&buffer[5]);
		frame->wHeight = get_unaligned_le16(&buffer[7]);
		frame->dwMinBitRate = get_unaligned_le32(&buffer[9]);
		frame->dwMaxBitRate = get_unaligned_le32(&buffer[13]);
		
		// 添加分辨率解析调试信息
		uvc_printk(KERN_INFO, "[DEBUG_RESOLUTION] USB descriptor parsing: Found frame %dx%d for device %s (devpath: %s)\n",
			   frame->wWidth, frame->wHeight, 
			   dev->udev->product ? dev->udev->product : "Unknown",
			   dev->udev->devpath);
		
		if (ftype != UVC_VS_FRAME_FRAME_BASED) {
			frame->dwMaxVideoFrameBufferSize =
				get_unaligned_le32(&buffer[17]);
			frame->dwDefaultFrameInterval =
				get_unaligned_le32(&buffer[21]);
			frame->bFrameIntervalType = buffer[25];
		} else {
			frame->dwMaxVideoFrameBufferSize = 0;
			frame->dwDefaultFrameInterval =
				get_unaligned_le32(&buffer[17]);
			frame->bFrameIntervalType = buffer[21];
		}
		frame->dwFrameInterval = *intervals;

		/* Several UVC chipsets screw up dwMaxVideoFrameBufferSize
		 * completely. Observed behaviours range from setting the
		 * value to 1.1x the actual frame size to hardwiring the
		 * 16 low bits to 0. This results in a higher than necessary
		 * memory usage as well as a wrong image size information. For
		 * uncompressed formats this can be fixed by computing the
		 * value from the frame size.
		 */
		if (!(format->flags & UVC_FMT_FLAG_COMPRESSED))
			frame->dwMaxVideoFrameBufferSize = format->bpp
				* frame->wWidth * frame->wHeight / 8;

		/* Some bogus devices report dwMinFrameInterval equal to
		 * dwMaxFrameInterval and have dwFrameIntervalStep set to
		 * zero. Setting all null intervals to 1 fixes the problem and
		 * some other divisions by zero that could happen.
		 */
		for (i = 0; i < n; ++i) {
			interval = get_unaligned_le32(&buffer[26+4*i]);
			*(*intervals)++ = interval ? interval : 1;
		}

		/* Make sure that the default frame interval stays between
		 * the boundaries.
		 */
		n -= frame->bFrameIntervalType ? 1 : 2;
		frame->dwDefaultFrameInterval =
			min(frame->dwFrameInterval[n],
			    max(frame->dwFrameInterval[0],
				frame->dwDefaultFrameInterval));

		if (dev->quirks & UVC_QUIRK_RESTRICT_FRAME_RATE) {
			frame->bFrameIntervalType = 1;
			frame->dwFrameInterval[0] =
				frame->dwDefaultFrameInterval;
		}

		uvc_trace(UVC_TRACE_DESCR, "- %ux%u (%u.%u fps)\n",
			frame->wWidth, frame->wHeight,
			10000000/frame->dwDefaultFrameInterval,
			(100000000/frame->dwDefaultFrameInterval)%10);

		format->nframes++;
		buflen -= buffer[0];
		buffer += buffer[0];
	}

	if (buflen > 2 && buffer[1] == USB_DT_CS_INTERFACE &&
	    buffer[2] == UVC_VS_STILL_IMAGE_FRAME) {
	    	n = buflen > 4 ? buffer[4] : 0;
	    if(buflen< 10+4*n)
	    {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
			       "interface %d still frame error\n", dev->udev->devnum,
			       alts->desc.bInterfaceNumber);
	    	return -EINVAL;
	    }
    	format->nstillframes = buffer[4];
		for(i = 0; i <n; i++)
		{
			format->still_frame[i].bFrameIndex = i+1;
		    format->still_frame[i].wWidth = get_unaligned_le16(&buffer[5+4*i]);
			format->still_frame[i].wHeight= get_unaligned_le16(&buffer[7+4*i]);
			uvc_trace(UVC_TRACE_DESCR, "-still %d %ux%u\n", format->still_frame[i].bFrameIndex,
					format->still_frame[i].wWidth, format->still_frame[i].wHeight);
		}
		buflen -= buffer[0];
		buffer += buffer[0];
	}

	if (buflen > 2 && buffer[1] == USB_DT_CS_INTERFACE &&
	    buffer[2] == UVC_VS_COLORFORMAT) {
		if (buflen < 6) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
			       "interface %d COLORFORMAT error\n",
			       dev->udev->devnum,
			       alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		format->colorspace = uvc_colorspace(buffer[3]);

		buflen -= buffer[0];
		buffer += buffer[0];
	}

	return buffer - start;
}

static int uvc_parse_streaming(struct uvc_device *dev,
	struct usb_interface *intf)
{
	struct uvc_streaming *streaming = NULL;
	struct uvc_format *format;
	struct uvc_frame *frame;
	struct uvc_still_frame *stillframe;
	struct usb_host_interface *alts = &intf->altsetting[0];
	unsigned char *_buffer, *buffer = alts->extra;
	int _buflen, buflen = alts->extralen;
	unsigned int nformats = 0, nframes = 0, nintervals = 0, nstillframes = 0;
	unsigned int size, i, n, p;
	__u32 *interval;
	__u16 psize;
	int ret = -EINVAL;

	if (intf->cur_altsetting->desc.bInterfaceSubClass
		!= UVC_SC_VIDEOSTREAMING) {
		uvc_trace(UVC_TRACE_DESCR, "device %d interface %d isn't a "
			"video streaming interface\n", dev->udev->devnum,
			intf->altsetting[0].desc.bInterfaceNumber);
		return -EINVAL;
	}

	if (usb_driver_claim_interface(&uvc_driver.driver, intf, dev)) {
		uvc_trace(UVC_TRACE_DESCR, "device %d interface %d is already "
			"claimed\n", dev->udev->devnum,
			intf->altsetting[0].desc.bInterfaceNumber);
		return -EINVAL;
	}

	streaming = kzalloc(sizeof *streaming, GFP_KERNEL);
	if (streaming == NULL) {
		usb_driver_release_interface(&uvc_driver.driver, intf);
		return -EINVAL;
	}

	mutex_init(&streaming->mutex);
	streaming->dev = dev;
	streaming->intf = usb_get_intf(intf);
	streaming->intfnum = intf->cur_altsetting->desc.bInterfaceNumber;

	/* The Pico iMage webcam has its class-specific interface descriptors
	 * after the endpoint descriptors.
	 */
	if (buflen == 0) {
		for (i = 0; i < alts->desc.bNumEndpoints; ++i) {
			struct usb_host_endpoint *ep = &alts->endpoint[i];

			if (ep->extralen == 0)
				continue;

			if (ep->extralen > 2 &&
			    ep->extra[1] == USB_DT_CS_INTERFACE) {
				uvc_trace(UVC_TRACE_DESCR, "trying extra data "
					"from endpoint %u.\n", i);
				buffer = alts->endpoint[i].extra;
				buflen = alts->endpoint[i].extralen;
				break;
			}
		}
	}

	/* Skip the standard interface descriptors. */
	while (buflen > 2 && buffer[1] != USB_DT_CS_INTERFACE) {
		buflen -= buffer[0];
		buffer += buffer[0];
	}

	if (buflen <= 2) {
		uvc_trace(UVC_TRACE_DESCR, "no class-specific streaming "
			"interface descriptors found.\n");
		goto error;
	}

	/* Parse the header descriptor. */
	switch (buffer[2]) {
	case UVC_VS_OUTPUT_HEADER:
		streaming->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		size = 9;
		break;

	case UVC_VS_INPUT_HEADER:
		streaming->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		size = 13;
		break;

	default:
		uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming interface "
			"%d HEADER descriptor not found.\n", dev->udev->devnum,
			alts->desc.bInterfaceNumber);
		goto error;
	}

	p = buflen >= 4 ? buffer[3] : 0;
	n = buflen >= size ? buffer[size-1] : 0;

	if (buflen < size + p*n) {
		uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
			"interface %d HEADER descriptor is invalid.\n",
			dev->udev->devnum, alts->desc.bInterfaceNumber);
		goto error;
	}

	streaming->header.bNumFormats = p;
	streaming->header.bEndpointAddress = buffer[6];
	if (buffer[2] == UVC_VS_INPUT_HEADER) {
		streaming->header.bmInfo = buffer[7];
		streaming->header.bTerminalLink = buffer[8];
		streaming->header.bStillCaptureMethod = buffer[9];
		streaming->header.bTriggerSupport = buffer[10];
		streaming->header.bTriggerUsage = buffer[11];
	} else {
		streaming->header.bTerminalLink = buffer[7];
	}
	streaming->header.bControlSize = n;

	streaming->header.bmaControls = kmemdup(&buffer[size], p * n,
						GFP_KERNEL);
	if (streaming->header.bmaControls == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	buflen -= buffer[0];
	buffer += buffer[0];

	_buffer = buffer;
	_buflen = buflen;

	/* Count the format and frame descriptors. */
	while (_buflen > 2 && _buffer[1] == USB_DT_CS_INTERFACE) {
		switch (_buffer[2]) {
		case UVC_VS_FORMAT_UNCOMPRESSED:
		case UVC_VS_FORMAT_MJPEG:
		case UVC_VS_FORMAT_FRAME_BASED:
			nformats++;
			break;

		case UVC_VS_FORMAT_DV:
			/* DV format has no frame descriptor. We will create a
			 * dummy frame descriptor with a dummy frame interval.
			 */
			nformats++;
			nframes++;
			nintervals++;
			break;

		case UVC_VS_FORMAT_MPEG2TS:
		case UVC_VS_FORMAT_STREAM_BASED:
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
				"interface %d FORMAT %u is not supported.\n",
				dev->udev->devnum,
				alts->desc.bInterfaceNumber, _buffer[2]);
			break;

		case UVC_VS_FRAME_UNCOMPRESSED:
		case UVC_VS_FRAME_MJPEG:
			nframes++;
			if (_buflen > 25)
				nintervals += _buffer[25] ? _buffer[25] : 3;
			break;

		case UVC_VS_FRAME_FRAME_BASED:
			nframes++;
			if (_buflen > 21)
				nintervals += _buffer[21] ? _buffer[21] : 3;
			break;

		case UVC_VS_STILL_IMAGE_FRAME:
			if (_buflen > 4)
				nstillframes += _buffer[4] ? _buffer[4] : 0;
		}

		_buflen -= _buffer[0];
		_buffer += _buffer[0];
	}

	if (nformats == 0) {
		uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming interface "
			"%d has no supported formats defined.\n",
			dev->udev->devnum, alts->desc.bInterfaceNumber);
		goto error;
	}

	size = nformats * sizeof *format + nframes * sizeof *frame
	     + nintervals * sizeof *interval + nstillframes* sizeof *stillframe;
	format = kzalloc(size, GFP_KERNEL);
	if (format == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	frame = (struct uvc_frame *)&format[nformats];
	interval = (__u32 *)&frame[nframes];
	stillframe = (struct uvc_still_frame*)&interval[nintervals];

	streaming->format = format;
	streaming->nformats = nformats;

	/* Parse the format descriptors. */
	while (buflen > 2 && buffer[1] == USB_DT_CS_INTERFACE) {
		switch (buffer[2]) {
		case UVC_VS_FORMAT_UNCOMPRESSED:
		case UVC_VS_FORMAT_MJPEG:
		case UVC_VS_FORMAT_DV:
		case UVC_VS_FORMAT_FRAME_BASED:
			format->frame = frame;
			format->still_frame = stillframe;
			ret = uvc_parse_format(dev, streaming, format,
				&interval, buffer, buflen);
			if (ret < 0)
				goto error;

			frame += format->nframes;
			stillframe += format->nstillframes;
			format++;

			buflen -= ret;
			buffer += ret;
			continue;

		default:
			break;
		}

		buflen -= buffer[0];
		buffer += buffer[0];
	}

	if (buflen)
		uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming interface "
			"%d has %u bytes of trailing descriptor garbage.\n",
			dev->udev->devnum, alts->desc.bInterfaceNumber, buflen);

	/* Parse the alternate settings to find the maximum bandwidth. */
	for (i = 0; i < intf->num_altsetting; ++i) {
		struct usb_host_endpoint *ep;
		alts = &intf->altsetting[i];
		ep = uvc_find_endpoint(alts,
				streaming->header.bEndpointAddress);
		if (ep == NULL)
			continue;

		psize = le16_to_cpu(ep->desc.wMaxPacketSize);
		psize = (psize & 0x07ff) * (1 + ((psize >> 11) & 3));
		if (psize > streaming->maxpsize)
			streaming->maxpsize = psize;
	}

	list_add_tail(&streaming->list, &dev->streams);
	return 0;

error:
	usb_driver_release_interface(&uvc_driver.driver, intf);
	usb_put_intf(intf);
	kfree(streaming->format);
	kfree(streaming->header.bmaControls);
	kfree(streaming);
	return ret;
}

static struct uvc_entity *uvc_alloc_entity(u16 type, u8 id,
		unsigned int num_pads, unsigned int extra_size)
{
	struct uvc_entity *entity;
	unsigned int num_inputs;
	unsigned int size;
	unsigned int i;

	extra_size = ALIGN(extra_size, sizeof(*entity->pads));
	num_inputs = (type & UVC_TERM_OUTPUT) ? num_pads : num_pads - 1;
	size = sizeof(*entity) + extra_size + sizeof(*entity->pads) * num_pads
	     + num_inputs;
	entity = kzalloc(size, GFP_KERNEL);
	if (entity == NULL)
		return NULL;

	entity->id = id;
	entity->type = type;

	entity->num_links = 0;
	entity->num_pads = num_pads;
	entity->pads = ((void *)(entity + 1)) + extra_size;

	for (i = 0; i < num_inputs; ++i)
		entity->pads[i].flags = MEDIA_PAD_FL_SINK;
	if (!UVC_ENTITY_IS_OTERM(entity))
		entity->pads[num_pads-1].flags = MEDIA_PAD_FL_SOURCE;

	entity->bNrInPins = num_inputs;
	entity->baSourceID = (__u8 *)(&entity->pads[num_pads]);

	return entity;
}

/* Parse vendor-specific extensions. */
static int uvc_parse_vendor_control(struct uvc_device *dev,
	const unsigned char *buffer, int buflen)
{
	struct usb_device *udev = dev->udev;
	struct usb_host_interface *alts = dev->intf->cur_altsetting;
	struct uvc_entity *unit;
	unsigned int n, p;
	int handled = 0;

	switch (le16_to_cpu(dev->udev->descriptor.idVendor)) {
	case 0x046d:		/* Logitech */
		if (buffer[1] != 0x41 || buffer[2] != 0x01)
			break;

		/* Logitech implements several vendor specific functions
		 * through vendor specific extension units (LXU).
		 *
		 * The LXU descriptors are similar to XU descriptors
		 * (see "USB Device Video Class for Video Devices", section
		 * 3.7.2.6 "Extension Unit Descriptor") with the following
		 * differences:
		 *
		 * ----------------------------------------------------------
		 * 0		bLength		1	 Number
		 *	Size of this descriptor, in bytes: 24+p+n*2
		 * ----------------------------------------------------------
		 * 23+p+n	bmControlsType	N	Bitmap
		 * 	Individual bits in the set are defined:
		 * 	0: Absolute
		 * 	1: Relative
		 *
		 * 	This bitset is mapped exactly the same as bmControls.
		 * ----------------------------------------------------------
		 * 23+p+n*2	bReserved	1	Boolean
		 * ----------------------------------------------------------
		 * 24+p+n*2	iExtension	1	Index
		 *	Index of a string descriptor that describes this
		 *	extension unit.
		 * ----------------------------------------------------------
		 */
		p = buflen >= 22 ? buffer[21] : 0;
		n = buflen >= 25 + p ? buffer[22+p] : 0;

		if (buflen < 25 + p + 2*n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d EXTENSION_UNIT error\n",
				udev->devnum, alts->desc.bInterfaceNumber);
			break;
		}

		unit = uvc_alloc_entity(UVC_VC_EXTENSION_UNIT, buffer[3],
					p + 1, 2*n);
		if (unit == NULL)
			return -ENOMEM;

		memcpy(unit->extension.guidExtensionCode, &buffer[4], 16);
		unit->extension.bNumControls = buffer[20];
		memcpy(unit->baSourceID, &buffer[22], p);
		unit->extension.bControlSize = buffer[22+p];
		unit->extension.bmControls = (__u8 *)unit + sizeof(*unit);
		unit->extension.bmControlsType = (__u8 *)unit + sizeof(*unit)
					       + n;
		memcpy(unit->extension.bmControls, &buffer[23+p], 2*n);

		if (buffer[24+p+2*n] != 0)
			usb_string(udev, buffer[24+p+2*n], unit->name,
				   sizeof unit->name);
		else
			sprintf(unit->name, "Extension %u", buffer[3]);

		list_add_tail(&unit->list, &dev->entities);
		handled = 1;
		break;
	}

	return handled;
}

static int uvc_parse_standard_control(struct uvc_device *dev,
	const unsigned char *buffer, int buflen)
{
	struct usb_device *udev = dev->udev;
	struct uvc_entity *unit, *term;
	struct usb_interface *intf;
	struct usb_host_interface *alts = dev->intf->cur_altsetting;
	unsigned int i, n, p, len;
	__u16 type;

	switch (buffer[2]) {
	case UVC_VC_HEADER:
		n = buflen >= 12 ? buffer[11] : 0;

		if (buflen < 12 || buflen < 12 + n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d HEADER error\n", udev->devnum,
				alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		dev->uvc_version = get_unaligned_le16(&buffer[3]);
		dev->clock_frequency = get_unaligned_le32(&buffer[7]);

		/* Parse all USB Video Streaming interfaces. */
		for (i = 0; i < n; ++i) {
			intf = usb_ifnum_to_if(udev, buffer[12+i]);
			if (intf == NULL) {
				uvc_trace(UVC_TRACE_DESCR, "device %d "
					"interface %d doesn't exists\n",
					udev->devnum, i);
				continue;
			}

			uvc_parse_streaming(dev, intf);
		}
		break;

	case UVC_VC_INPUT_TERMINAL:
		if (buflen < 8) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d INPUT_TERMINAL error\n",
				udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		/* Make sure the terminal type MSB is not null, otherwise it
		 * could be confused with a unit.
		 */
		type = get_unaligned_le16(&buffer[4]);
		if ((type & 0xff00) == 0) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d INPUT_TERMINAL %d has invalid "
				"type 0x%04x, skipping\n", udev->devnum,
				alts->desc.bInterfaceNumber,
				buffer[3], type);
			return 0;
		}

		n = 0;
		p = 0;
		len = 8;

		if (type == UVC_ITT_CAMERA) {
			n = buflen >= 15 ? buffer[14] : 0;
			len = 15;

		} else if (type == UVC_ITT_MEDIA_TRANSPORT_INPUT) {
			n = buflen >= 9 ? buffer[8] : 0;
			p = buflen >= 10 + n ? buffer[9+n] : 0;
			len = 10;
		}

		if (buflen < len + n + p) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d INPUT_TERMINAL error\n",
				udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		term = uvc_alloc_entity(type | UVC_TERM_INPUT, buffer[3],
					1, n + p);
		if (term == NULL)
			return -ENOMEM;

		if (UVC_ENTITY_TYPE(term) == UVC_ITT_CAMERA) {
			term->camera.bControlSize = n;
			term->camera.bmControls = (__u8 *)term + sizeof *term;
			term->camera.wObjectiveFocalLengthMin =
				get_unaligned_le16(&buffer[8]);
			term->camera.wObjectiveFocalLengthMax =
				get_unaligned_le16(&buffer[10]);
			term->camera.wOcularFocalLength =
				get_unaligned_le16(&buffer[12]);
			memcpy(term->camera.bmControls, &buffer[15], n);
		} else if (UVC_ENTITY_TYPE(term) ==
			   UVC_ITT_MEDIA_TRANSPORT_INPUT) {
			term->media.bControlSize = n;
			term->media.bmControls = (__u8 *)term + sizeof *term;
			term->media.bTransportModeSize = p;
			term->media.bmTransportModes = (__u8 *)term
						     + sizeof *term + n;
			memcpy(term->media.bmControls, &buffer[9], n);
			memcpy(term->media.bmTransportModes, &buffer[10+n], p);
		}

		if (buffer[7] != 0)
			usb_string(udev, buffer[7], term->name,
				   sizeof term->name);
		else if (UVC_ENTITY_TYPE(term) == UVC_ITT_CAMERA)
			sprintf(term->name, "Camera %u", buffer[3]);
		else if (UVC_ENTITY_TYPE(term) == UVC_ITT_MEDIA_TRANSPORT_INPUT)
			sprintf(term->name, "Media %u", buffer[3]);
		else
			sprintf(term->name, "Input %u", buffer[3]);

		list_add_tail(&term->list, &dev->entities);
		break;

	case UVC_VC_OUTPUT_TERMINAL:
		if (buflen < 9) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d OUTPUT_TERMINAL error\n",
				udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		/* Make sure the terminal type MSB is not null, otherwise it
		 * could be confused with a unit.
		 */
		type = get_unaligned_le16(&buffer[4]);
		if ((type & 0xff00) == 0) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d OUTPUT_TERMINAL %d has invalid "
				"type 0x%04x, skipping\n", udev->devnum,
				alts->desc.bInterfaceNumber, buffer[3], type);
			return 0;
		}

		term = uvc_alloc_entity(type | UVC_TERM_OUTPUT, buffer[3],
					1, 0);
		if (term == NULL)
			return -ENOMEM;

		memcpy(term->baSourceID, &buffer[7], 1);

		if (buffer[8] != 0)
			usb_string(udev, buffer[8], term->name,
				   sizeof term->name);
		else
			sprintf(term->name, "Output %u", buffer[3]);

		list_add_tail(&term->list, &dev->entities);
		break;

	case UVC_VC_SELECTOR_UNIT:
		p = buflen >= 5 ? buffer[4] : 0;

		if (buflen < 5 || buflen < 6 + p) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d SELECTOR_UNIT error\n",
				udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		unit = uvc_alloc_entity(buffer[2], buffer[3], p + 1, 0);
		if (unit == NULL)
			return -ENOMEM;

		memcpy(unit->baSourceID, &buffer[5], p);

		if (buffer[5+p] != 0)
			usb_string(udev, buffer[5+p], unit->name,
				   sizeof unit->name);
		else
			sprintf(unit->name, "Selector %u", buffer[3]);

		list_add_tail(&unit->list, &dev->entities);
		break;

	case UVC_VC_PROCESSING_UNIT:
		n = buflen >= 8 ? buffer[7] : 0;
		p = dev->uvc_version >= 0x0110 ? 10 : 9;

		if (buflen < p + n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d PROCESSING_UNIT error\n",
				udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		unit = uvc_alloc_entity(buffer[2], buffer[3], 2, n);
		if (unit == NULL)
			return -ENOMEM;

		memcpy(unit->baSourceID, &buffer[4], 1);
		unit->processing.wMaxMultiplier =
			get_unaligned_le16(&buffer[5]);
		unit->processing.bControlSize = buffer[7];
		unit->processing.bmControls = (__u8 *)unit + sizeof *unit;
		memcpy(unit->processing.bmControls, &buffer[8], n);
		if (dev->uvc_version >= 0x0110)
			unit->processing.bmVideoStandards = buffer[9+n];

		if (buffer[8+n] != 0)
			usb_string(udev, buffer[8+n], unit->name,
				   sizeof unit->name);
		else
			sprintf(unit->name, "Processing %u", buffer[3]);

		list_add_tail(&unit->list, &dev->entities);
		break;

	case UVC_VC_EXTENSION_UNIT:
		p = buflen >= 22 ? buffer[21] : 0;
		n = buflen >= 24 + p ? buffer[22+p] : 0;

		if (buflen < 24 + p + n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
				"interface %d EXTENSION_UNIT error\n",
				udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		unit = uvc_alloc_entity(buffer[2], buffer[3], p + 1, n);
		if (unit == NULL)
			return -ENOMEM;

		memcpy(unit->extension.guidExtensionCode, &buffer[4], 16);
		unit->extension.bNumControls = buffer[20];
		memcpy(unit->baSourceID, &buffer[22], p);
		unit->extension.bControlSize = buffer[22+p];
		unit->extension.bmControls = (__u8 *)unit + sizeof *unit;
		memcpy(unit->extension.bmControls, &buffer[23+p], n);

		if (buffer[23+p+n] != 0)
			usb_string(udev, buffer[23+p+n], unit->name,
				   sizeof unit->name);
		else
			sprintf(unit->name, "Extension %u", buffer[3]);

		list_add_tail(&unit->list, &dev->entities);
		break;

	default:
		uvc_trace(UVC_TRACE_DESCR, "Found an unknown CS_INTERFACE "
			"descriptor (%u)\n", buffer[2]);
		break;
	}

	return 0;
}

static int uvc_parse_control(struct uvc_device *dev)
{
	struct usb_host_interface *alts = dev->intf->cur_altsetting;
	unsigned char *buffer = alts->extra;
	int buflen = alts->extralen;
	int ret;

	/* Parse the default alternate setting only, as the UVC specification
	 * defines a single alternate setting, the default alternate setting
	 * zero.
	 */

	while (buflen > 2) {
		if (uvc_parse_vendor_control(dev, buffer, buflen) ||
		    buffer[1] != USB_DT_CS_INTERFACE)
			goto next_descriptor;

		if ((ret = uvc_parse_standard_control(dev, buffer, buflen)) < 0)
			return ret;

next_descriptor:
		buflen -= buffer[0];
		buffer += buffer[0];
	}

	/* Check if the optional status endpoint is present. Built-in iSight
	 * webcams have an interrupt endpoint but spit proprietary data that
	 * don't conform to the UVC status endpoint messages. Don't try to
	 * handle the interrupt endpoint for those cameras.
	 */
	if (alts->desc.bNumEndpoints == 1 &&
	    !(dev->quirks & UVC_QUIRK_BUILTIN_ISIGHT)) {
		struct usb_host_endpoint *ep = &alts->endpoint[0];
		struct usb_endpoint_descriptor *desc = &ep->desc;

		if (usb_endpoint_is_int_in(desc) &&
		    le16_to_cpu(desc->wMaxPacketSize) >= 8 &&
		    desc->bInterval != 0) {
			uvc_trace(UVC_TRACE_DESCR, "Found a Status endpoint "
				"(addr %02x).\n", desc->bEndpointAddress);
			dev->int_ep = ep;
		}
	}

	return 0;
}

/* ------------------------------------------------------------------------
 * UVC device scan
 */

/*
 * Scan the UVC descriptors to locate a chain starting at an Output Terminal
 * and containing the following units:
 *
 * - one or more Output Terminals (USB Streaming or Display)
 * - zero or one Processing Unit
 * - zero, one or more single-input Selector Units
 * - zero or one multiple-input Selector Units, provided all inputs are
 *   connected to input terminals
 * - zero, one or mode single-input Extension Units
 * - one or more Input Terminals (Camera, External or USB Streaming)
 *
 * The terminal and units must match on of the following structures:
 *
 * ITT_*(0) -> +---------+    +---------+    +---------+ -> TT_STREAMING(0)
 * ...         | SU{0,1} | -> | PU{0,1} | -> | XU{0,n} |    ...
 * ITT_*(n) -> +---------+    +---------+    +---------+ -> TT_STREAMING(n)
 *
 *                 +---------+    +---------+ -> OTT_*(0)
 * TT_STREAMING -> | PU{0,1} | -> | XU{0,n} |    ...
 *                 +---------+    +---------+ -> OTT_*(n)
 *
 * The Processing Unit and Extension Units can be in any order. Additional
 * Extension Units connected to the main chain as single-unit branches are
 * also supported. Single-input Selector Units are ignored.
 */
static int uvc_scan_chain_entity(struct uvc_video_chain *chain,
	struct uvc_entity *entity)
{
	switch (UVC_ENTITY_TYPE(entity)) {
	case UVC_VC_EXTENSION_UNIT:
		if (uvc_trace_param & UVC_TRACE_PROBE)
			printk(" <- XU %d", entity->id);

		if (entity->bNrInPins != 1) {
			uvc_trace(UVC_TRACE_DESCR, "Extension unit %d has more "
				"than 1 input pin.\n", entity->id);
			return -1;
		}

		break;

	case UVC_VC_PROCESSING_UNIT:
		if (uvc_trace_param & UVC_TRACE_PROBE)
			printk(" <- PU %d", entity->id);

		if (chain->processing != NULL) {
			uvc_trace(UVC_TRACE_DESCR, "Found multiple "
				"Processing Units in chain.\n");
			return -1;
		}

		chain->processing = entity;
		break;

	case UVC_VC_SELECTOR_UNIT:
		if (uvc_trace_param & UVC_TRACE_PROBE)
			printk(" <- SU %d", entity->id);

		/* Single-input selector units are ignored. */
		if (entity->bNrInPins == 1)
			break;

		if (chain->selector != NULL) {
			uvc_trace(UVC_TRACE_DESCR, "Found multiple Selector "
				"Units in chain.\n");
			return -1;
		}

		chain->selector = entity;
		break;

	case UVC_ITT_VENDOR_SPECIFIC:
	case UVC_ITT_CAMERA:
	case UVC_ITT_MEDIA_TRANSPORT_INPUT:
		if (uvc_trace_param & UVC_TRACE_PROBE)
			printk(" <- IT %d\n", entity->id);

		break;

	case UVC_OTT_VENDOR_SPECIFIC:
	case UVC_OTT_DISPLAY:
	case UVC_OTT_MEDIA_TRANSPORT_OUTPUT:
		if (uvc_trace_param & UVC_TRACE_PROBE)
			printk(" OT %d", entity->id);

		break;

	case UVC_TT_STREAMING:
		if (UVC_ENTITY_IS_ITERM(entity)) {
			if (uvc_trace_param & UVC_TRACE_PROBE)
				printk(" <- IT %d\n", entity->id);
		} else {
			if (uvc_trace_param & UVC_TRACE_PROBE)
				printk(" OT %d", entity->id);
		}

		break;

	default:
		uvc_trace(UVC_TRACE_DESCR, "Unsupported entity type "
			"0x%04x found in chain.\n", UVC_ENTITY_TYPE(entity));
		return -1;
	}

	list_add_tail(&entity->chain, &chain->entities);
	return 0;
}

static int uvc_scan_chain_forward(struct uvc_video_chain *chain,
	struct uvc_entity *entity, struct uvc_entity *prev)
{
	struct uvc_entity *forward;
	int found;

	/* Forward scan */
	forward = NULL;
	found = 0;

	while (1) {
		forward = uvc_entity_by_reference(chain->dev, entity->id,
			forward);
		if (forward == NULL)
			break;
		if (forward == prev)
			continue;

		switch (UVC_ENTITY_TYPE(forward)) {
		case UVC_VC_EXTENSION_UNIT:
			if (forward->bNrInPins != 1) {
				uvc_trace(UVC_TRACE_DESCR, "Extension unit %d "
					  "has more than 1 input pin.\n",
					  entity->id);
				return -EINVAL;
			}

			list_add_tail(&forward->chain, &chain->entities);
			if (uvc_trace_param & UVC_TRACE_PROBE) {
				if (!found)
					printk(" (->");

				printk(" XU %d", forward->id);
				found = 1;
			}
			break;

		case UVC_OTT_VENDOR_SPECIFIC:
		case UVC_OTT_DISPLAY:
		case UVC_OTT_MEDIA_TRANSPORT_OUTPUT:
		case UVC_TT_STREAMING:
			if (UVC_ENTITY_IS_ITERM(forward)) {
				uvc_trace(UVC_TRACE_DESCR, "Unsupported input "
					"terminal %u.\n", forward->id);
				return -EINVAL;
			}

			list_add_tail(&forward->chain, &chain->entities);
			if (uvc_trace_param & UVC_TRACE_PROBE) {
				if (!found)
					printk(" (->");

				printk(" OT %d", forward->id);
				found = 1;
			}
			break;
		}
	}
	if (found)
		printk(")");

	return 0;
}

static int uvc_scan_chain_backward(struct uvc_video_chain *chain,
	struct uvc_entity **_entity)
{
	struct uvc_entity *entity = *_entity;
	struct uvc_entity *term;
	int id = -EINVAL, i;

	switch (UVC_ENTITY_TYPE(entity)) {
	case UVC_VC_EXTENSION_UNIT:
	case UVC_VC_PROCESSING_UNIT:
		id = entity->baSourceID[0];
		break;

	case UVC_VC_SELECTOR_UNIT:
		/* Single-input selector units are ignored. */
		if (entity->bNrInPins == 1) {
			id = entity->baSourceID[0];
			break;
		}

		if (uvc_trace_param & UVC_TRACE_PROBE)
			printk(" <- IT");

		chain->selector = entity;
		for (i = 0; i < entity->bNrInPins; ++i) {
			id = entity->baSourceID[i];
			term = uvc_entity_by_id(chain->dev, id);
			if (term == NULL || !UVC_ENTITY_IS_ITERM(term)) {
				uvc_trace(UVC_TRACE_DESCR, "Selector unit %d "
					"input %d isn't connected to an "
					"input terminal\n", entity->id, i);
				return -1;
			}

			if (uvc_trace_param & UVC_TRACE_PROBE)
				printk(" %d", term->id);

			list_add_tail(&term->chain, &chain->entities);
			uvc_scan_chain_forward(chain, term, entity);
		}

		if (uvc_trace_param & UVC_TRACE_PROBE)
			printk("\n");

		id = 0;
		break;

	case UVC_ITT_VENDOR_SPECIFIC:
	case UVC_ITT_CAMERA:
	case UVC_ITT_MEDIA_TRANSPORT_INPUT:
	case UVC_OTT_VENDOR_SPECIFIC:
	case UVC_OTT_DISPLAY:
	case UVC_OTT_MEDIA_TRANSPORT_OUTPUT:
	case UVC_TT_STREAMING:
		id = UVC_ENTITY_IS_OTERM(entity) ? entity->baSourceID[0] : 0;
		break;
	}

	if (id <= 0) {
		*_entity = NULL;
		return id;
	}

	entity = uvc_entity_by_id(chain->dev, id);
	if (entity == NULL) {
		uvc_trace(UVC_TRACE_DESCR, "Found reference to "
			"unknown entity %d.\n", id);
		return -EINVAL;
	}

	*_entity = entity;
	return 0;
}

static int uvc_scan_chain(struct uvc_video_chain *chain,
			  struct uvc_entity *term)
{
	struct uvc_entity *entity, *prev;

	uvc_trace(UVC_TRACE_PROBE, "Scanning UVC chain:");

	entity = term;
	prev = NULL;

	while (entity != NULL) {
		/* Entity must not be part of an existing chain */
		if (entity->chain.next || entity->chain.prev) {
			uvc_trace(UVC_TRACE_DESCR, "Found reference to "
				"entity %d already in chain.\n", entity->id);
			return -EINVAL;
		}

		/* Process entity */
		if (uvc_scan_chain_entity(chain, entity) < 0)
			return -EINVAL;

		/* Forward scan */
		if (uvc_scan_chain_forward(chain, entity, prev) < 0)
			return -EINVAL;

		/* Backward scan */
		prev = entity;
		if (uvc_scan_chain_backward(chain, &entity) < 0)
			return -EINVAL;
	}

	return 0;
}

static unsigned int uvc_print_terms(struct list_head *terms, u16 dir,
		char *buffer)
{
	struct uvc_entity *term;
	unsigned int nterms = 0;
	char *p = buffer;

	list_for_each_entry(term, terms, chain) {
		if (!UVC_ENTITY_IS_TERM(term) ||
		    UVC_TERM_DIRECTION(term) != dir)
			continue;

		if (nterms)
			p += sprintf(p, ",");
		if (++nterms >= 4) {
			p += sprintf(p, "...");
			break;
		}
		p += sprintf(p, "%u", term->id);
	}

	return p - buffer;
}

static const char *uvc_print_chain(struct uvc_video_chain *chain)
{
	static char buffer[43];
	char *p = buffer;

	p += uvc_print_terms(&chain->entities, UVC_TERM_INPUT, p);
	p += sprintf(p, " -> ");
	uvc_print_terms(&chain->entities, UVC_TERM_OUTPUT, p);

	return buffer;
}

/*
 * Scan the device for video chains and register video devices.
 *
 * Chains are scanned starting at their output terminals and walked backwards.
 */
static int uvc_scan_device(struct uvc_device *dev)
{
	struct uvc_video_chain *chain;
	struct uvc_entity *term;

	list_for_each_entry(term, &dev->entities, list) {
		if (!UVC_ENTITY_IS_OTERM(term))
			continue;

		/* If the terminal is already included in a chain, skip it.
		 * This can happen for chains that have multiple output
		 * terminals, where all output terminals beside the first one
		 * will be inserted in the chain in forward scans.
		 */
		if (term->chain.next || term->chain.prev)
			continue;

		chain = kzalloc(sizeof(*chain), GFP_KERNEL);
		if (chain == NULL)
			return -ENOMEM;

		INIT_LIST_HEAD(&chain->entities);
		mutex_init(&chain->ctrl_mutex);
		chain->dev = dev;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,13)
		v4l2_prio_init(&chain->prio);
#endif

		if (uvc_scan_chain(chain, term) < 0) {
			kfree(chain);
			continue;
		}

		uvc_trace(UVC_TRACE_PROBE, "Found a valid video chain (%s).\n",
			  uvc_print_chain(chain));

		list_add_tail(&chain->list, &dev->chains);
	}

	if (list_empty(&dev->chains)) {
		uvc_printk(KERN_INFO, "No valid video chain found.\n");
		return -1;
	}

	return 0;
}

/* ------------------------------------------------------------------------
 * Video device registration and unregistration
 */

/*
 * Delete the UVC device.
 *
 * Called by the kernel when the last reference to the uvc_device structure
 * is released.
 *
 * As this function is called after or during disconnect(), all URBs have
 * already been canceled by the USB core. There is no need to kill the
 * interrupt URB manually.
 */
static void uvc_delete(struct uvc_device *dev)
{
	struct list_head *p, *n;

	uvc_status_cleanup(dev);
	uvc_ctrl_cleanup_device(dev);

	usb_put_intf(dev->intf);
	usb_put_dev(dev->udev);

	if (dev->vdev.dev)
		v4l2_device_unregister(&dev->vdev);
#ifdef CONFIG_MEDIA_CONTROLLER
	if (media_devnode_is_registered(&dev->mdev.devnode))
		media_device_unregister(&dev->mdev);
#endif

	list_for_each_safe(p, n, &dev->chains) {
		struct uvc_video_chain *chain;
		chain = list_entry(p, struct uvc_video_chain, list);
		kfree(chain);
	}

	list_for_each_safe(p, n, &dev->entities) {
		struct uvc_entity *entity;
		entity = list_entry(p, struct uvc_entity, list);
#ifdef CONFIG_MEDIA_CONTROLLER
		uvc_mc_cleanup_entity(entity);
#endif
		if (entity->vdev) {
			video_device_release(entity->vdev);
			entity->vdev = NULL;
		}
		kfree(entity);
	}

	list_for_each_safe(p, n, &dev->streams) {
		struct uvc_streaming *streaming;
		streaming = list_entry(p, struct uvc_streaming, list);
		usb_driver_release_interface(&uvc_driver.driver,
			streaming->intf);
		usb_put_intf(streaming->intf);
		kfree(streaming->format);
		kfree(streaming->header.bmaControls);
		kfree(streaming);
	}

	kfree(dev);
}

static void uvc_release(struct video_device *vdev)
{
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	struct uvc_device *dev = stream->dev;

	/* Decrement the registered streams count and delete the device when it
	 * reaches zero.
	 */
	if (atomic_dec_and_test(&dev->nstreams))
		uvc_delete(dev);
}

/*
 * Unregister the video devices.
 */
static void uvc_unregister_video(struct uvc_device *dev)
{
	struct uvc_streaming *stream;

	/* Unregistering all video devices might result in uvc_delete() being
	 * called from inside the loop if there's no open file handle. To avoid
	 * that, increment the stream count before iterating over the streams
	 * and decrement it when done.
	 */
	atomic_inc(&dev->nstreams);

	list_for_each_entry(stream, &dev->streams, list) {
		if (stream->vdev == NULL)
			continue;

		video_unregister_device(stream->vdev);
		stream->vdev = NULL;

		uvc_debugfs_cleanup_stream(stream);
	}

	/* Decrement the stream count and call uvc_delete explicitly if there
	 * are no stream left.
	 */
	if (atomic_dec_and_test(&dev->nstreams))
		uvc_delete(dev);
}

/*
 * 强制获取指定的video设备号
 * 如果指定的设备号被占用，尝试释放并重新分配
 */
/*
 * 根据USB总线确定固定的video设备号
 * 根据USB总线进行固定映射：
 * USB Bus 2 -> video20 固定映射
 * USB Bus 1 -> video21 固定映射
 */
static int uvc_get_fixed_video_number_by_port(struct usb_device *udev)
{
	struct usb_device *parent;
	int port_number = -1;
	int bus_number = -1;
	int fixed_number = -1;
	const char *device_path;
	
	if (!udev) {
		uvc_printk(KERN_ERR, "[UVC_PORT_DEBUG] NULL USB device\n");
		return -1;
	}
	
	// 获取父设备(USB HUB)
	parent = udev->parent;
	if (!parent) {
		uvc_printk(KERN_INFO, "[UVC_PORT_DEBUG] Device has no parent (root hub)\n");
		return -1;
	}
	
	// 获取端口号和总线号
	port_number = udev->portnum;
	bus_number = udev->bus->busnum;
	device_path = dev_name(&udev->dev);
	
	uvc_printk(KERN_INFO, "[UVC_PORT_DEBUG] USB device info:\n");
	uvc_printk(KERN_INFO, "[UVC_PORT_DEBUG]   Bus: %d, Port: %d\n", bus_number, port_number);
	uvc_printk(KERN_INFO, "[UVC_PORT_DEBUG]   Device path: %s\n", device_path);
	
	// 根据USB总线进行固定映射：
	// USB Bus 2 -> video20 固定映射
	// USB Bus 1 -> video21 固定映射
	if (bus_number == 2) {
		fixed_number = 20;  // USB Bus 2 -> video20 固定映射
		uvc_printk(KERN_INFO, "[UVC_PORT_DEBUG] Bus 2 mapped to video20 (Front Camera)\n");
	} else if (bus_number == 1) {
		fixed_number = 21;  // USB Bus 1 -> video21 固定映射
		uvc_printk(KERN_INFO, "[UVC_PORT_DEBUG] Bus 1 mapped to video21 (Rear Camera)\n");
	} else {
		uvc_printk(KERN_INFO, "[UVC_PORT_DEBUG] Unknown bus %d, using auto-allocation\n", bus_number);
		fixed_number = -1;  // 自动分配
	}
	
	uvc_printk(KERN_INFO, "[UVC_PORT_DEBUG] Final mapping: Bus%d -> video%d\n", 
		   bus_number, fixed_number);
	
	return fixed_number;
}

/*
 * 强制注册video设备到指定的设备号，必要时清理已存在的设备
 */
static int uvc_force_register_at_number(struct video_device *vdev, int target_number)
{
	int ret;
	
	if (!vdev || target_number < 0 || target_number >= 256) {
		uvc_printk(KERN_ERR, "[UVC_FORCE_REG] Invalid parameters: vdev=%p, number=%d\n", 
			   vdev, target_number);
		return -EINVAL;
	}
	
	uvc_printk(KERN_INFO, "[UVC_FORCE_REG] Attempting to register at video%d\n", target_number);
	uvc_printk(KERN_INFO, "[UVC_FORCE_REG] Device name: %s\n", vdev->name ? vdev->name : "Unknown");
	
	// 尝试注册到目标设备号，让内核处理冲突检查
	ret = video_register_device(vdev, VFL_TYPE_GRABBER, target_number);
	
	if (ret == 0) {
		if (vdev->num == target_number) {
			uvc_printk(KERN_INFO, "[UVC_FORCE_REG] Successfully registered as video%d\n", vdev->num);
			return 0;
		} else {
			uvc_printk(KERN_WARNING, "[UVC_FORCE_REG] Registered as video%d instead of video%d\n", 
				   vdev->num, target_number);
			// 如果系统分配了不同的设备号，我们接受它
			return 0;
		}
	} else {
		uvc_printk(KERN_ERR, "[UVC_FORCE_REG] Failed to register at video%d: %d\n", 
			   target_number, ret);
		
		// 如果指定设备号失败，尝试自动分配
		uvc_printk(KERN_INFO, "[UVC_FORCE_REG] Trying auto-allocation as fallback\n");
		ret = video_register_device(vdev, VFL_TYPE_GRABBER, -1);
		if (ret == 0) {
			uvc_printk(KERN_INFO, "[UVC_FORCE_REG] Fallback successful: video%d\n", vdev->num);
		} else {
			uvc_printk(KERN_ERR, "[UVC_FORCE_REG] Auto-allocation also failed: %d\n", ret);
		}
		
		return ret;
	}
}

static int uvc_force_video_number(struct video_device *vdev, int desired_number)
{
	// 注意：desired_number参数在新的端口映射模式下将被忽略
	// 实际的设备号由USB端口位置决定
	
	// 安全检查：验证vdev指针
	if (!vdev) {
		uvc_printk(KERN_ERR, "[UVC_DEBUG] NULL video device pointer\n");
		return -EINVAL;
	}
	
	// 从video_device中获取关联的UVC设备
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	if (!stream || !stream->dev || !stream->dev->udev) {
		uvc_printk(KERN_ERR, "[UVC_DEBUG] Cannot access USB device from video device\n");
		// 回退到原始的期望设备号
		return uvc_force_register_at_number(vdev, desired_number);
	}
	
	// 根据USB端口获取固定的设备号
	int fixed_number = uvc_get_fixed_video_number_by_port(stream->dev->udev);
	
	if (fixed_number >= 0) {
		// 使用基于端口的固定设备号
		uvc_printk(KERN_INFO, "[UVC_DEBUG] Using port-based fixed number: video%d\n", fixed_number);
		return uvc_force_register_at_number(vdev, fixed_number);
	} else {
		// 端口检测失败，使用原始的期望设备号
		uvc_printk(KERN_INFO, "[UVC_DEBUG] Port detection failed, using desired number: video%d\n", desired_number);
		return uvc_force_register_at_number(vdev, desired_number);
	}
}

static int uvc_register_video(struct uvc_device *dev,
		struct uvc_streaming *stream)
{
	struct video_device *vdev;
	int ret;

	uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] === VIDEO REGISTER START ===\n");
	uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] Device: %p, Stream: %p\n", dev, stream);
	uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] Target video number: %d\n", uvc_number);

	/* Initialize the streaming interface with default streaming
	 * parameters.
	 */
	ret = uvc_video_init(stream);
	if (ret < 0) {
		uvc_printk(KERN_ERR, "Failed to initialize the device "
			"(%d).\n", ret);
		return ret;
	}

	uvc_debugfs_init_stream(stream);

	/* Register the device with V4L. */
	uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] Allocating video device\n");
	vdev = video_device_alloc();
	if (vdev == NULL) {
		uvc_printk(KERN_ERR, "[UVC_REGISTER_DEBUG] Failed to allocate video device (%d).\n",
			   ret);
		return -ENOMEM;
	}
	uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] Video device allocated at %p\n", vdev);

	/* We already hold a reference to dev->udev. The video device will be
	 * unregistered before the reference is released, so we don't need to
	 * get another one.
	 */
	vdev->v4l2_dev = &dev->vdev;
	vdev->fops = &uvc_fops;
	vdev->release = uvc_release;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,13)
	vdev->prio = &stream->chain->prio;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
	set_bit(V4L2_FL_USE_FH_PRIO, &vdev->flags);
#endif
	if (stream->type == V4L2_BUF_TYPE_VIDEO_OUTPUT)
		vdev->vfl_dir = VFL_DIR_TX;
#endif
	strlcpy(vdev->name, dev->name, sizeof vdev->name);

	/* Set the driver data before calling video_register_device, otherwise
	 * uvc_v4l2_open might race us.
	 */
	stream->vdev = vdev;
	video_set_drvdata(vdev, stream);

	// 获取基于USB端口的固定设备号 - 只为实际存在的设备分配
	int port_based_number = -1;
	if (dev && dev->udev) {
		port_based_number = uvc_get_fixed_video_number_by_port(dev->udev);
		uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] Port-based number: %d\n", port_based_number);
	}
	
	// 使用固定映射（如果可用）或传统分配
	int target_number = (port_based_number >= 0) ? port_based_number : uvc_number;
	
	uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] Target number: %d (port-based: %s)\n", 
		   target_number, (port_based_number >= 0) ? "YES" : "NO");
	
	ret = uvc_force_video_number(vdev, target_number);
	
	if (ret < 0) {
		uvc_printk(KERN_ERR, "Failed to register video device (%d).\n", ret);
		stream->vdev = NULL;
		video_device_release(vdev);
		return ret;
	}
	
	// 获取实际分配的设备号
	int actual_number = vdev->num;
	
	// 如果使用了端口映射，不需要更新全局uvc_number
	// 如果使用了传统分配，更新uvc_number避免冲突
	if (port_based_number < 0) {
		// 传统分配模式：更新uvc_number为下一个可用的设备号
		if (actual_number >= uvc_number) {
			uvc_number = actual_number + 1;
		}
		uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] Traditional mode: Next uvc_number set to: %d\n", uvc_number);
	} else {
		uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] Port-based mode: Fixed mapping maintained\n");
	}
	
	uvc_printk(KERN_INFO, "[UVC_REGISTER_DEBUG] Video device registered as video%d (port-based: %s)\n", 
		   actual_number, port_based_number >= 0 ? "YES" : "NO");

	atomic_inc(&dev->nstreams);
	return 0;
}

/*
 * Register all video devices in all chains.
 */
static int uvc_register_terms(struct uvc_device *dev,
	struct uvc_video_chain *chain)
{
	struct uvc_streaming *stream;
	struct uvc_entity *term;
	int ret;

	list_for_each_entry(term, &chain->entities, chain) {
		if (UVC_ENTITY_TYPE(term) != UVC_TT_STREAMING)
			continue;

		stream = uvc_stream_by_id(dev, term->id);
		if (stream == NULL) {
			uvc_printk(KERN_INFO, "No streaming interface found "
				   "for terminal %u.", term->id);
			continue;
		}

		stream->chain = chain;
		ret = uvc_register_video(dev, stream);
		if (ret < 0)
			return ret;

		term->vdev = stream->vdev;
	}

	return 0;
}

static int uvc_register_chains(struct uvc_device *dev)
{
	struct uvc_video_chain *chain;
	int ret;

	list_for_each_entry(chain, &dev->chains, list) {
		ret = uvc_register_terms(dev, chain);
		if (ret < 0)
			return ret;

#ifdef CONFIG_MEDIA_CONTROLLER
		ret = uvc_mc_register_entities(chain);
		if (ret < 0) {
			uvc_printk(KERN_INFO, "Failed to register entites "
				"(%d).\n", ret);
		}
#endif
	}

	return 0;
}

/* ------------------------------------------------------------------------
 * USB probe, disconnect, suspend and resume
 */

static int uvc_probe(struct usb_interface *intf,
		     const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct uvc_device *dev;
	int ret;

	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] === UVC PROBE START ===\n");
	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Interface: %p, USB Device: %p\n", intf, udev);
	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Current device count: %d\n", uvc_device_count);
	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] USB Device ID: %04x:%04x\n", 
		   le16_to_cpu(udev->descriptor.idVendor), le16_to_cpu(udev->descriptor.idProduct));
	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] USB Device Path: %s\n", udev->devpath);
	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Interface Number: %d\n", intf->cur_altsetting->desc.bInterfaceNumber);
	
	// 添加USB端口检测信息
	int expected_video_number = uvc_get_fixed_video_number_by_port(udev);
	if (expected_video_number >= 0) {
		uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Port-based mapping: This device should be video%d\n", expected_video_number);
	} else {
		uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Port detection failed, will use traditional numbering\n");
	}
	
	// 追踪重新探测
	unsigned long current_time = jiffies;
	if (uvc_first_probe_time == 0) {
		uvc_first_probe_time = current_time;
		uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] FIRST PROBE - Recording timestamp\n");
	} else {
		uvc_reprobe_count++;
		unsigned long time_since_first = (current_time - uvc_first_probe_time) * 1000 / HZ;
		unsigned long time_since_last = (current_time - uvc_last_probe_time) * 1000 / HZ;
		uvc_printk(KERN_WARNING, "[UVC_PROBE_DEBUG] REPROBE #%d DETECTED!\n", uvc_reprobe_count);
		uvc_printk(KERN_WARNING, "[UVC_PROBE_DEBUG] Time since first probe: %lu ms\n", time_since_first);
		uvc_printk(KERN_WARNING, "[UVC_PROBE_DEBUG] Time since last probe: %lu ms\n", time_since_last);
		uvc_printk(KERN_WARNING, "[UVC_PROBE_DEBUG] Call stack trace:\n");
		dump_stack();
	}
	uvc_last_probe_time = current_time;
	
	// 检查是否为重复调用
	if (uvc_device_count > 0) {
		uvc_printk(KERN_WARNING, "[UVC_PROBE_DEBUG] DEVICE_COUNT_WARNING! Current count: %d\n", uvc_device_count);
	}

	DRAMC_BASE = ioremap(0xF0006000, 0x100);
	if(!DRAMC_BASE)
		uvc_trace(UVC_TRACE_PROBE, "ioremap failed.\n");

	if (id->idVendor && id->idProduct) {
		uvc_trace(UVC_TRACE_PROBE, "Probing known UVC device %s "
				"(%04x:%04x)\n", udev->devpath, id->idVendor,
				id->idProduct);
	} else {
		uvc_trace(UVC_TRACE_PROBE, "Probing generic UVC device %s\n",
				udev->devpath);
	}

	if (uvc_device_count >= UVC_MAX_DEVICES) {
		uvc_trace(UVC_TRACE_PROBE, "Maximum UVC devices (%d) reached.\n", UVC_MAX_DEVICES);
		return -ENODEV;
	}

	// 使用基于USB总线的智能设备分配
	int port_based_number = uvc_get_fixed_video_number_by_port(udev);
	if (port_based_number > 0) {
		uvc_number = port_based_number;
		uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Using port-based assignment: video%d\n", uvc_number);
	} else {
		// 后备方案：按设备计数分配
		if (uvc_device_count == 0) {
			uvc_number = UVC_GENERIC_VIDEO_NUM_FRONT;  // 后备：默认分配到 video20
			uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Fallback: Assigning device to video%d\n", uvc_number);
		} else {
			uvc_number = UVC_GENERIC_VIDEO_NUM_REAR;   // 后备：默认分配到 video21
			uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Fallback: Assigning device to video%d\n", uvc_number);
		}
	}
	/* Allocate memory for the device and initialize it. */
	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Allocating device structure\n");
	if ((dev = kzalloc(sizeof *dev, GFP_KERNEL)) == NULL) {
		uvc_printk(KERN_ERR, "[UVC_PROBE_DEBUG] Failed to allocate device structure\n");
		return -ENOMEM;
	}
	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Device structure allocated at %p\n", dev);

	INIT_LIST_HEAD(&dev->entities);
	INIT_LIST_HEAD(&dev->chains);
	INIT_LIST_HEAD(&dev->streams);
	atomic_set(&dev->nstreams, 0);
	atomic_set(&dev->users, 0);
	atomic_set(&dev->nmappings, 0);

	dev->udev = usb_get_dev(udev);
	dev->intf = usb_get_intf(intf);
	dev->intfnum = intf->cur_altsetting->desc.bInterfaceNumber;
	dev->quirks = (uvc_quirks_param == -1)
		    ? id->driver_info : uvc_quirks_param;

	if (udev->product != NULL)
		strlcpy(dev->name, udev->product, sizeof dev->name);
	else
		snprintf(dev->name, sizeof dev->name,
			"UVC Camera (%04x:%04x)",
			le16_to_cpu(udev->descriptor.idVendor),
			le16_to_cpu(udev->descriptor.idProduct));

	/* Parse the Video Class control descriptor. */
	if (uvc_parse_control(dev) < 0) {
		uvc_trace(UVC_TRACE_PROBE, "Unable to parse UVC "
			"descriptors.\n");
		goto error;
	}

	uvc_printk(KERN_INFO, "Found UVC %u.%02x device %s (%04x:%04x)\n",
		dev->uvc_version >> 8, dev->uvc_version & 0xff,
		udev->product ? udev->product : "<unnamed>",
		le16_to_cpu(udev->descriptor.idVendor),
		le16_to_cpu(udev->descriptor.idProduct));

	if (dev->quirks != id->driver_info) {
		uvc_printk(KERN_INFO, "Forcing device quirks to 0x%x by module "
			"parameter for testing purpose.\n", dev->quirks);
		uvc_printk(KERN_INFO, "Please report required quirks to the "
			"linux-uvc-devel mailing list.\n");
	}

	/* Register the media and V4L2 devices. */
#ifdef CONFIG_MEDIA_CONTROLLER
	dev->mdev.dev = &intf->dev;
	strlcpy(dev->mdev.model, dev->name, sizeof(dev->mdev.model));
	if (udev->serial)
		strlcpy(dev->mdev.serial, udev->serial,
			sizeof(dev->mdev.serial));
	strcpy(dev->mdev.bus_info, udev->devpath);
	dev->mdev.hw_revision = le16_to_cpu(udev->descriptor.bcdDevice);
	dev->mdev.driver_version = LINUX_VERSION_CODE;
	if (media_device_register(&dev->mdev) < 0)
		goto error;

	dev->vdev.mdev = &dev->mdev;
#endif
	if (v4l2_device_register(&intf->dev, &dev->vdev) < 0)
		goto error;

	/* Initialize controls. */
	if (uvc_ctrl_init_device(dev) < 0)
		goto error;

	/* Scan the device for video chains. */
	if (uvc_scan_device(dev) < 0)
		goto error;

	/* Register video device nodes. */
	if (uvc_register_chains(dev) < 0)
		goto error;

	/* Save our data pointer in the interface data. */
	usb_set_intfdata(intf, dev);

	/* Initialize the interrupt URB. */
	if ((ret = uvc_status_init(dev)) < 0) {
		uvc_printk(KERN_INFO, "Unable to initialize the status "
			"endpoint (%d), status interrupt will not be "
			"supported.\n", ret);
	}

	// 初始化动态电源管理相关字段
	dev->resume_failure_count = 0;
	dev->last_resume_error = 0;
	dev->last_activity = jiffies;
	dev->power_management_disabled = false;

	// 通用USB摄像头初始化 - 移除SONIX特定逻辑
	dev->generic_chip = dev->valid_device = 0;
	dev->generic_chip = 1;  // 设置为通用摄像头
	uvc_printk(KERN_INFO, "Generic USB camera detected\n");

	uvc_trace(UVC_TRACE_PROBE, "UVC device initialized.\n");

	// 保存设备到全局数组
	uvc_devices[uvc_device_count] = dev;
	uvc_device_count++;

	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] === UVC PROBE SUCCESS ===\n");
	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Device registered successfully, new count: %d\n", uvc_device_count);
	uvc_printk(KERN_INFO, "[UVC_PROBE_DEBUG] Assigned video device number: %d\n", uvc_number);

	// USB智能电源管理配置 - 平衡稳定性和功耗
	
	// 配置最小化电源管理 - 防止虚假断开但保持设备可访问
	
	// 策略1: 完全禁用USB设备的自动挂起
	usb_disable_autosuspend(udev);
	
	// 策略2: 启用运行时电源管理以保持接口可访问
	pm_runtime_enable(&intf->dev);
	pm_runtime_enable(&udev->dev);
	
	// 策略3: 设置设备为活跃状态，确保立即可用
	pm_runtime_set_active(&intf->dev);  
	pm_runtime_set_active(&udev->dev);
	
	// 策略4: 获取电源管理引用，防止意外挂起但允许正常访问
	pm_runtime_get(&intf->dev);
	pm_runtime_get(&udev->dev);
	
	// 方案5: 简化的USB恢复配置 (适配内核3.18)
	
	// 标记设备为关键设备，优先保持活跃
	device_set_wakeup_capable(&udev->dev, false);  // 禁用唤醒功能，避免电源状态切换
	
	return 0;

error:
	uvc_unregister_video(dev);
	return -ENODEV;
}

static void uvc_disconnect(struct usb_interface *intf)
{
	struct uvc_device *dev = usb_get_intfdata(intf);

	uvc_printk(KERN_WARNING, "[UVC_DISCONNECT_DEBUG] === UVC DISCONNECT START ===\n");
	uvc_printk(KERN_WARNING, "[UVC_DISCONNECT_DEBUG] Interface: %p, Device: %p\n", intf, dev);
	uvc_printk(KERN_WARNING, "[UVC_DISCONNECT_DEBUG] Current device count: %d\n", uvc_device_count);
	
	if (dev && dev->udev) {
		uvc_printk(KERN_WARNING, "[UVC_DISCONNECT_DEBUG] USB Device: %04x:%04x (%s)\n", 
			   le16_to_cpu(dev->udev->descriptor.idVendor),
			   le16_to_cpu(dev->udev->descriptor.idProduct),
			   dev->udev->product ? dev->udev->product : "Unknown");
	}

	/* Set the USB interface data to NULL. This can be done outside the
	 * lock, as there's no other reader.
	 */
	usb_set_intfdata(intf, NULL);

	if (intf->cur_altsetting->desc.bInterfaceSubClass ==
	    UVC_SC_VIDEOSTREAMING) {
		return;
	}

	// 从全局数组中移除设备
	int i;
	for (i = 0; i < UVC_MAX_DEVICES; i++) {
		if (uvc_devices[i] == dev) {
			uvc_devices[i] = NULL;
			uvc_device_count--;
			uvc_printk(KERN_WARNING, "[UVC_DISCONNECT_DEBUG] Device removed from slot %d, remaining: %d\n", 
				i, uvc_device_count);
			break;
		}
	}

	if (dev) {
		dev->state |= UVC_DEV_DISCONNECTED;
	}

	uvc_unregister_video(dev);
}

static int uvc_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct uvc_device *dev = usb_get_intfdata(intf);
	struct uvc_streaming *stream;

	uvc_printk(KERN_WARNING, "[UVC_SUSPEND_DEBUG] === UVC SUSPEND START ===\n");
	uvc_printk(KERN_WARNING, "[UVC_SUSPEND_DEBUG] Interface: %p, Device: %p\n", intf, dev);
	uvc_printk(KERN_WARNING, "[UVC_SUSPEND_DEBUG] PM message event: %d\n", message.event);
	
	if (dev && dev->udev) {
		uvc_printk(KERN_WARNING, "[UVC_SUSPEND_DEBUG] USB Device: %04x:%04x (%s)\n", 
			   le16_to_cpu(dev->udev->descriptor.idVendor),
			   le16_to_cpu(dev->udev->descriptor.idProduct),
			   dev->udev->product ? dev->udev->product : "Unknown");
	}

	uvc_trace(UVC_TRACE_SUSPEND, "Suspending interface %u\n",
		intf->cur_altsetting->desc.bInterfaceNumber);

	/* Controls are cached on the fly so they don't need to be saved. */
	if (intf->cur_altsetting->desc.bInterfaceSubClass ==
	    UVC_SC_VIDEOCONTROL)
		return uvc_status_suspend(dev);

	list_for_each_entry(stream, &dev->streams, list) {
		if (stream->intf == intf)
			return uvc_video_suspend(stream);
	}

	uvc_trace(UVC_TRACE_SUSPEND, "Suspend: video streaming USB interface "
			"mismatch.\n");
	return -EINVAL;
}

static int __uvc_resume(struct usb_interface *intf, int reset)
{
	struct uvc_device *dev = usb_get_intfdata(intf);
	struct uvc_streaming *stream;

	uvc_printk(KERN_WARNING, "[UVC_RESUME_DEBUG] === UVC RESUME START ===\n");
	uvc_printk(KERN_WARNING, "[UVC_RESUME_DEBUG] Interface: %p, Device: %p, Reset: %d\n", intf, dev, reset);
	
	if (dev && dev->udev) {
		uvc_printk(KERN_WARNING, "[UVC_RESUME_DEBUG] USB Device: %04x:%04x (%s)\n", 
			   le16_to_cpu(dev->udev->descriptor.idVendor),
			   le16_to_cpu(dev->udev->descriptor.idProduct),
			   dev->udev->product ? dev->udev->product : "Unknown");
	}

	uvc_trace(UVC_TRACE_SUSPEND, "Resuming interface %u\n",
		intf->cur_altsetting->desc.bInterfaceNumber);

	if (intf->cur_altsetting->desc.bInterfaceSubClass ==
	    UVC_SC_VIDEOCONTROL) {
		if (reset) {
			int ret = uvc_ctrl_resume_device(dev);

			if (ret < 0)
				return ret;
		}

		return uvc_status_resume(dev);
	}

	list_for_each_entry(stream, &dev->streams, list) {
		if (stream->intf == intf)
			return uvc_video_resume(stream, reset);
	}

	uvc_trace(UVC_TRACE_SUSPEND, "Resume: video streaming USB interface "
			"mismatch.\n");
	return -EINVAL;
}

static int uvc_resume(struct usb_interface *intf)
{
	return __uvc_resume(intf, 0);
}

static int uvc_reset_resume(struct usb_interface *intf)
{
	return __uvc_resume(intf, 1);
}

/* ------------------------------------------------------------------------
 * Module parameters
 */

static int uvc_clock_param_get(char *buffer, struct kernel_param *kp)
{
	if (uvc_clock_param == CLOCK_MONOTONIC)
		return sprintf(buffer, "CLOCK_MONOTONIC");
	else
		return sprintf(buffer, "CLOCK_REALTIME");
}

static int uvc_clock_param_set(const char *val, struct kernel_param *kp)
{
	if (strncasecmp(val, "clock_", strlen("clock_")) == 0)
		val += strlen("clock_");

	if (strcasecmp(val, "monotonic") == 0)
		uvc_clock_param = CLOCK_MONOTONIC;
	else if (strcasecmp(val, "realtime") == 0)
		uvc_clock_param = CLOCK_REALTIME;
	else
		return -EINVAL;

	return 0;
}

module_param_call(clock, uvc_clock_param_set, uvc_clock_param_get,
		  &uvc_clock_param, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(clock, "Video buffers timestamp clock");
module_param_named(nodrop, uvc_no_drop_param, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(nodrop, "Don't drop incomplete frames");
module_param_named(quirks, uvc_quirks_param, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(quirks, "Forced device quirks");
module_param_named(trace, uvc_trace_param, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(trace, "Trace level bitmask");
module_param_named(timeout, uvc_timeout_param, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(timeout, "Streaming control requests timeout");

/* ------------------------------------------------------------------------
 * Driver initialization and cleanup
 */

/*
 * The Logitech cameras listed below have their interface class set to
 * VENDOR_SPEC because they don't announce themselves as UVC devices, even
 * though they are compliant.
 */
static struct usb_device_id uvc_ids[] = {
	/* LogiLink Wireless Webcam */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x0416,
	  .idProduct		= 0xa91a,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX },
	/* Genius eFace 2025 */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x0458,
	  .idProduct		= 0x706e,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX },
	/* Microsoft Lifecam NX-6000 */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x045e,
	  .idProduct		= 0x00f8,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX },
	/* Microsoft Lifecam VX-7000 */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x045e,
	  .idProduct		= 0x0723,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX },
	/* Logitech Quickcam Fusion */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x046d,
	  .idProduct		= 0x08c1,
	  .bInterfaceClass	= USB_CLASS_VENDOR_SPEC,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0 },
	/* Logitech Quickcam Orbit MP */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x046d,
	  .idProduct		= 0x08c2,
	  .bInterfaceClass	= USB_CLASS_VENDOR_SPEC,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0 },
	/* Logitech Quickcam Pro for Notebook */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x046d,
	  .idProduct		= 0x08c3,
	  .bInterfaceClass	= USB_CLASS_VENDOR_SPEC,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0 },
	/* Logitech Quickcam Pro 5000 */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x046d,
	  .idProduct		= 0x08c5,
	  .bInterfaceClass	= USB_CLASS_VENDOR_SPEC,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0 },
	/* Logitech Quickcam OEM Dell Notebook */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x046d,
	  .idProduct		= 0x08c6,
	  .bInterfaceClass	= USB_CLASS_VENDOR_SPEC,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0 },
	/* Logitech Quickcam OEM Cisco VT Camera II */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x046d,
	  .idProduct		= 0x08c7,
	  .bInterfaceClass	= USB_CLASS_VENDOR_SPEC,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0 },
	/* Chicony CNF7129 (Asus EEE 100HE) */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x04f2,
	  .idProduct		= 0xb071,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_RESTRICT_FRAME_RATE },
	/* Alcor Micro AU3820 (Future Boy PC USB Webcam) */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x058f,
	  .idProduct		= 0x3820,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX },
	/* Apple Built-In iSight */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x05ac,
	  .idProduct		= 0x8501,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info 		= UVC_QUIRK_PROBE_MINMAX
				| UVC_QUIRK_BUILTIN_ISIGHT },
	/* Foxlink ("HP Webcam" on HP Mini 5103) */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x05c8,
	  .idProduct		= 0x0403,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_FIX_BANDWIDTH },
	/* Genesys Logic USB 2.0 PC Camera */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x05e3,
	  .idProduct		= 0x0505,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* Hercules Classic Silver */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x06f8,
	  .idProduct		= 0x300c,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_FIX_BANDWIDTH },
	/* ViMicro Vega */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x0ac8,
	  .idProduct		= 0x332d,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_FIX_BANDWIDTH },
	/* ViMicro - Minoru3D */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x0ac8,
	  .idProduct		= 0x3410,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_FIX_BANDWIDTH },
	/* ViMicro Venus - Minoru3D */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x0ac8,
	  .idProduct		= 0x3420,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_FIX_BANDWIDTH },
	/* MT6227 */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x0e8d,
	  .idProduct		= 0x0004,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX
				| UVC_QUIRK_PROBE_DEF },
	/* IMC Networks (Medion Akoya) */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x13d3,
	  .idProduct		= 0x5103,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* JMicron USB2.0 XGA WebCam */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x152d,
	  .idProduct		= 0x0310,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX },
	/* Syntek (HP Spartan) */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x174f,
	  .idProduct		= 0x5212,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* Syntek (Samsung Q310) */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x174f,
	  .idProduct		= 0x5931,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* Syntek (Packard Bell EasyNote MX52 */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x174f,
	  .idProduct		= 0x8a12,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* Syntek (Asus F9SG) */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x174f,
	  .idProduct		= 0x8a31,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* Syntek (Asus U3S) */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x174f,
	  .idProduct		= 0x8a33,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* Syntek (JAOtech Smart Terminal) */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x174f,
	  .idProduct		= 0x8a34,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* Miricle 307K */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x17dc,
	  .idProduct		= 0x0202,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* Lenovo Thinkpad SL400/SL500 */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x17ef,
	  .idProduct		= 0x480b,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STREAM_NO_FID },
	/* Aveo Technology USB 2.0 Camera */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x1871,
	  .idProduct		= 0x0306,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX
				| UVC_QUIRK_PROBE_EXTRAFIELDS },
	/* Ecamm Pico iMage */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x18cd,
	  .idProduct		= 0xcafe,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_EXTRAFIELDS },
	/* Manta MM-353 Plako */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x18ec,
	  .idProduct		= 0x3188,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX },
	/* FSC WebCam V30S */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x18ec,
	  .idProduct		= 0x3288,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX },
	/* Arkmicro unbranded */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x18ec,
	  .idProduct		= 0x3290,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_DEF },
	/* The Imaging Source USB CCD cameras */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x199e,
	  .idProduct		= 0x8102,
	  .bInterfaceClass	= USB_CLASS_VENDOR_SPEC,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0 },
	/* Bodelin ProScopeHR */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_DEV_HI
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x19ab,
	  .idProduct		= 0x1000,
	  .bcdDevice_hi		= 0x0126,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_STATUS_INTERVAL },
	/* MSI StarCam 370i */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x1b3b,
	  .idProduct		= 0x2951,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX },
	/* SiGma Micro USB Web Camera */
	{ .match_flags		= USB_DEVICE_ID_MATCH_DEVICE
				| USB_DEVICE_ID_MATCH_INT_INFO,
	  .idVendor		= 0x1c4f,
	  .idProduct		= 0x3000,
	  .bInterfaceClass	= USB_CLASS_VIDEO,
	  .bInterfaceSubClass	= 1,
	  .bInterfaceProtocol	= 0,
	  .driver_info		= UVC_QUIRK_PROBE_MINMAX
				| UVC_QUIRK_IGNORE_SELECTOR_UNIT },
	/* Generic USB Video Class */
	{ USB_INTERFACE_INFO(USB_CLASS_VIDEO, 1, 0) },
	{}
};

MODULE_DEVICE_TABLE(usb, uvc_ids);

/* ------------------------------------------------------------------------
 * Dual Camera Support Functions
 */

struct uvc_device *uvc_get_device_by_index(int index)
{
	if (index >= 0 && index < UVC_MAX_DEVICES)
		return uvc_devices[index];
	return NULL;
}

int uvc_get_device_count(void)
{
	return uvc_device_count;
}

struct uvc_device *uvc_get_front_device(void)
{
	return uvc_get_device_by_index(0);
}

struct uvc_device *uvc_get_rear_device(void)
{
	return uvc_get_device_by_index(1);
}

EXPORT_SYMBOL(uvc_get_device_by_index);
EXPORT_SYMBOL(uvc_get_device_count);
EXPORT_SYMBOL(uvc_get_front_device);
EXPORT_SYMBOL(uvc_get_rear_device);

struct uvc_driver uvc_driver = {
	.driver = {
		.name		= "uvcvideo",
		.probe		= uvc_probe,
		.disconnect	= uvc_disconnect,
		.suspend	= uvc_suspend,
		.resume		= uvc_resume,
		.reset_resume	= uvc_reset_resume,
		.id_table	= uvc_ids,
		.supports_autosuspend = 0,  // 完全禁用自动挂起支持
	},
};

static int __init uvc_init(void)
{
	int ret;

	// 初始化时重置设备计数器和数组
	uvc_device_count = 0;
	memset(uvc_devices, 0, sizeof(uvc_devices));
	
	// 全局声明：此驱动完全禁用USB自动挂起功能

	uvc_debugfs_init();

	ret = usb_register(&uvc_driver.driver);
	if (ret < 0) {
		uvc_debugfs_cleanup();
		return ret;
	}

	printk(KERN_INFO DRIVER_DESC " (" DRIVER_VERSION DRIVER_KERNEL_VERSION ")\n");
	return 0;
}

static void __exit uvc_cleanup(void)
{
	// 在模块卸载前，强制清理所有UVC设备
	int i;
	for (i = 0; i < UVC_MAX_DEVICES; i++) {
		if (uvc_devices[i] != NULL) {
			uvc_unregister_video(uvc_devices[i]);
			uvc_devices[i] = NULL;
		}
	}
	
	usb_deregister(&uvc_driver.driver);
	uvc_debugfs_cleanup();
	
	// 清理时重置设备计数器和数组
	uvc_device_count = 0;
	memset(uvc_devices, 0, sizeof(uvc_devices));
	
	// 给系统一点时间清理V4L2注册表
	msleep(100);
	printk(KERN_ALERT "[UVC_LIFECYCLE] uvc_cleanup: V4L2 cleanup delay completed\n");
}

module_init(uvc_init);
module_exit(uvc_cleanup);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

