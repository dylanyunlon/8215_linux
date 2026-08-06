/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <media/v4l2-ioctl.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/dma-mapping.h>
#include <media/videobuf-dma-contig.h>
#include <media/videobuf2-vmalloc.h>
#include <generated/atc_project.h>
#include "x_ver.h"
#include "atc_capture.h"
#include "cvbs_hal.h"
#include "audio_hal.h"
#include "avm_hal.h"
#include "tvd_wch_mgr.h"
#include "ybrvga_hal.h"
//#include "digital_hal.h"
//#include "tvp_hal.h"
//#if defined(CONFIG_ATC_PRJ_ac823x_evb)
//#define ENABLE_HDMI
//#endif

#ifdef ENABLE_HDMI
#include "hdmi_hal.h"
#endif

static u32 input_type = 0U;

static struct avin_device avin_dev = {{NULL}};
static struct device *cap_dev;


const struct avin_channel_config_params avin_ch_params[] = {
	{
		.name = "NTSC_M",
		.width = 720,
		.height = 480,
		.frm_fmt = 0,
		.ycmux_mode = 1,
		.eav2sav = 268,
		.sav2eav = 1440,
		.vsize = 525,
		.capture_format = 0,
		.vbi_supported = 1,
		.hd_sd = 0,
		.stdid = V4L2_STD_525_60,
	},
	{
		.name = "PAL_BDGHIK",
		.width = 720,
		.height = 576,
		.frm_fmt = 0,
		.ycmux_mode = 1,
		.eav2sav = 280,
		.sav2eav = 1440,
		.vsize = 625,
		.capture_format = 0,
		.vbi_supported = 1,
		.hd_sd = 0,
		.stdid = V4L2_STD_625_50,
	}
};

#ifdef CONFIG_AVM_ENABLE
static int avin_get_devIdx(struct file *file, int32_t *pdevIdx)
{
	struct video_device *vdev = video_devdata(file);

	if (NULL == pdevIdx) {
		pr_err("[AVIN]%s: param pdevIdx is NULL!\r\n", __func__);
		return -1;
	}

	if (0 == strncmp(AVM_VIDEO_DEV0_NAME, vdev->dev.kobj.name,
		strlen(vdev->dev.kobj.name))) {
		*pdevIdx = 0;
	} else if (0 == strncmp(AVM_VIDEO_DEV1_NAME, vdev->dev.kobj.name,
		strlen(vdev->dev.kobj.name))) {
		*pdevIdx = 1;
	} else if (0 == strncmp(AVM_VIDEO_DEV2_NAME, vdev->dev.kobj.name,
		strlen(vdev->dev.kobj.name))) {
		*pdevIdx = 2;
	} else if (0 == strncmp(AVM_VIDEO_DEV3_NAME, vdev->dev.kobj.name,
		strlen(vdev->dev.kobj.name))) {
		*pdevIdx = 3;
	} else if (0 == strncmp(AVM_VIDEO_DEV4_NAME, vdev->dev.kobj.name,
		strlen(vdev->dev.kobj.name))) {
		*pdevIdx = 4;
	} else {
		*pdevIdx = -1;
	}

	return 0;
}
#endif

static inline struct avin_cap_buffer *to_avin_buffer(struct vb2_buffer *vb)
{
	return container_of(vb, struct avin_cap_buffer, vb);
}


/**
 * avin_buffer_queue_setup : Callback function for buffer setup.
 * @vq: vb2_queue ptr
 * @format: v4l2 format
 * @nbuffers: ptr to number of buffers requested by application
 * @nplanes:: contains number of distinct video planes needed to hold a frame
 * @sizes[]: contains the size (in bytes) of each plane.
 * @alloc_ctxs: ptr to allocation context
 *
 * This callback function is called when reqbuf() is called to adjust
 * the buffer count and buffer size
 */
static int avin_buffer_queue_setup(struct vb2_queue *vq,
			const struct v4l2_format *format,
			unsigned int *nbuffers, unsigned int *nplanes,
			unsigned int sizes[], void *alloc_ctxs[])
{
	struct channel_obj *ch = vb2_get_drv_priv(vq);
	struct common_obj *common;

	pr_debug("[AVIN]%s: enter\n", __func__);
	if (*nbuffers > VIDEO_MAX_FRAME) {
		pr_err("[AVIN]%s: request buf cnt(%d) failed!\n", __func__, *nbuffers);
		return -EINVAL;
	}
	common = &ch->common[AVIN_VIDEO_INDEX];

	switch (ch->input_type) {
	case AVIN_TYPE_CVBS_VIDEO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_CVBS_VIDEO\n", __func__);
		//cvbs_init_video(0);
		break;

	case AVIN_TYPE_BACKCAR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_BACKCAR\n", __func__);
		//cvbs_init_video(0);
		break;

	case AVIN_TYPE_CVBS_AUDIO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_CVBS_AUDIO\n", __func__);
		cvbs_init_audio(0);
		break;

	case AVIN_TYPE_DIGITAL_AUDIO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_DIGITAL_AUDIO\n", __func__);
		break;

	case AVIN_TYPE_DIGITAL_VIDEO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_DIGITAL_VIDEO\n", __func__);
		//digital_init_video(0);
		break;

#ifdef CONFIG_YBR_ENABLE
	case AVIN_TYPE_YPBPR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_YPBPR\n", __func__);
		ybrvga_init_video(0);
		break;
#endif

#ifdef ENABLE_HDMI
	case AVIN_TYPE_HDMI:
		pr_debug("[AVIN]%s: device AVIN_TYPE_HDMI\n", __func__);
		hdmi_init_video(0);
		hdmi_init_audio(0);
		break;
#endif

#ifdef CONFIG_AVM_ENABLE
	case AVIN_TYPE_AVM_FRONT:
	case AVIN_TYPE_AVM_REAR:
	case AVIN_TYPE_AVM_LEFT:
	case AVIN_TYPE_AVM_RIGHT:
	case AVIN_TYPE_AVM_SIGVIEW:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		g_avm_data[ch->input_type - AVIN_TYPE_AVM_FRONT].num_frames = *nbuffers;
		//avm_init_video(0);
		break;
#endif

	default:
		break;
	}

	/*if (fmt && fmt->fmt.pix.sizeimage < common->fmt.fmt.pix.sizeimage) {
		return -EINVAL;
	}*/

#ifdef CONFIG_AVM_ENABLE
	if (ch->input_type < AVIN_TYPE_AVM_FRONT) {
		*nbuffers = AVIN_CAPTURE_MAX_BUFFER;
	}
#else
	*nbuffers = AVIN_CAPTURE_MAX_BUFFER;
#endif

	*nplanes = 1;
	sizes[0] = common->alloc_size;
	alloc_ctxs[0] = common->alloc_ctx;

	return 0;
}


/**
 * avin_buffer_queue : Callback function to add buffer to DMA queue
 * @vb: ptr to vb2_buffer
 */
static void avin_buffer_queue(struct vb2_buffer *vb)
{
	struct channel_obj *ch = vb2_get_drv_priv(vb->vb2_queue);
	struct avin_cap_buffer *buf = to_avin_buffer(vb);
	struct common_obj *common;
	unsigned long flags;
#ifdef CONFIG_AVM_ENABLE
	int avmIdx = 0;
	int bufIdx = 0;
	void *vaddr;
#endif

	common = &ch->common[AVIN_VIDEO_INDEX];
	spin_lock_irqsave(&common->irqlock, flags);

#ifdef CONFIG_AVM_ENABLE
	if (ch->input_type >= AVIN_TYPE_AVM_FRONT) {
		//create buf list
		vaddr = vb2_plane_vaddr(vb, 0U);
		avmIdx = ch->input_type - AVIN_TYPE_AVM_FRONT;
		bufIdx = buf->vb.v4l2_buf.index;
		if (g_avm_data[avmIdx].memset_cnt < g_avm_data[avmIdx].num_frames) {
			int size = g_avm_data[avmIdx].pix.width *g_avm_data[avmIdx].pix.height;
			memset((u8 *)vaddr, 0x10, size);
			memset((u8 *)(vaddr + size), 0x80, size / 2);
			g_avm_data[avmIdx].memset_cnt++;
			pr_info("[AVIN]%s: ch(%d) memset buffer to black with cnt(%d)\n", __func__,
				avmIdx, g_avm_data[avmIdx].memset_cnt);
			g_avm_data[avmIdx].buffers[bufIdx].data = (u8 *)vaddr;
			g_avm_data[avmIdx].buffers[bufIdx].idx = bufIdx;
			g_avm_data[avmIdx].buffers[bufIdx].status = FRAME_EMPTY;
		} else {
			/*if (FRAME_READY == g_avm_data[avmIdx].buffers[bufIdx].status) {
				g_avm_data[avmIdx].buffers[bufIdx].status = FRAME_EMPTY;
			} else {
				pr_err("[AVIN]%s:ch(%d) queue the same buf(%d) addr(%08x) status(%d)!\n",
					__func__, avmIdx, bufIdx, (unsigned int)vaddr,
					g_avm_data[avmIdx].buffers[bufIdx].status);
				spin_unlock_irqrestore(&common->irqlock, flags);
				return;
			}*/
		}
	}
#endif

	/* add the buffer to the DMA queue */
	list_add_tail(&buf->list, &common->dma_queue);
	spin_unlock_irqrestore(&common->irqlock, flags);
}

/*
 * vb2 uses these to release the mutex when waiting in dqbuf.  I'm
 * not actually sure we need to do this (I'm not sure that vb2_dqbuf() needs
 * to be called with the mutex held), but better safe than sorry. IKnow heihei
 */
static void avin_wait_prepare(struct vb2_queue *vq)
{
	struct channel_obj *ch = vb2_get_drv_priv(vq);
	struct common_obj *common = &ch->common[AVIN_VIDEO_INDEX];

	mutex_unlock(&common->lock);
}

static void avin_wait_finish(struct vb2_queue *vq)
{
	struct channel_obj *ch = vb2_get_drv_priv(vq);
	struct common_obj *common = &ch->common[AVIN_VIDEO_INDEX];

	mutex_lock(&common->lock);
}


/**
 * avin_start_streaming : Starts the DMA engine for streaming
 * @vb: ptr to vb2_buffer
 * @count: number of buffers
 */
static int avin_start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct channel_obj *ch = vb2_get_drv_priv(vq);
	struct common_obj *common = &ch->common[AVIN_VIDEO_INDEX];
	struct avin_cap_buffer *buf, *tmp;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&common->irqlock, flags);

	/* Initialize field_id */
	ch->field_id = 0;
	/*cvbs_config_addr(ch, ret);*/
	/* Get the next frame from the buffer queue */
	common->cur_frm = common->next_frm = list_entry(common->dma_queue.next,
		struct avin_cap_buffer, list);
	/* Remove buffer from the buffer queue */
	list_del(&common->cur_frm->list);
	spin_unlock_irqrestore(&common->irqlock, flags);

	/**
	 * Set interrupt for both the fields in CVBS Register enable channel in
	 * CVBS register
	 */
	pr_debug("[AVIN]%s: input_type(%d) input_idx(%d)\n",
		__func__, ch->input_type, ch->input_idx);
	switch (ch->input_type) {
	case AVIN_TYPE_CVBS_VIDEO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_CVBS_VIDEO\n", __func__);
		if (cvbs_start_video(TVD_APP_ID_AVIN, ch->input_idx)) {
			pr_err("[AVIN]%s: cvbs_start_video failed!\n", __func__);
			ret = -EFAULT;
			goto err;
		}
		input_type = ch->input_type;
		break;

	case AVIN_TYPE_BACKCAR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_BACKCAR\n", __func__);
		if (cvbs_start_video(TVD_APP_ID_BACKCAR, ch->input_idx)) {
			pr_err("[AVIN]%s: cvbs_start_video failed!\n", __func__);
			ret = -EFAULT;
			goto err;
		}
		break;

	case AVIN_TYPE_CVBS_AUDIO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_CVBS_AUDIO\n", __func__);
		if (!cvbs_start_audio(ch->input_idx)) {
			pr_err("[AVIN]%s: cvbs_start_audio failed!\n", __func__);
			ret = -EFAULT;
			goto err;
		}
		break;

	case AVIN_TYPE_DIGITAL_AUDIO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_DIGITAL_AUDIO\n", __func__);
		if (!start_digitalAud(1)) {
			pr_err("[AVIN]%s: start_digitalAud failed!\n", __func__);
			ret = -EFAULT;
			goto err;
		}
		break;

	case AVIN_TYPE_DIGITAL_VIDEO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_DIGITAL_VIDEO\n", __func__);
		/*if(digital_start_video(ch->input_idx)) {
			pr_err("[AVIN]%s: digital_start_video failed!\n", __func__);
			ret = -EFAULT;
			goto err;
		}*/
		input_type = ch->input_type;
		break;

#ifdef CONFIG_YBR_ENABLE
	case AVIN_TYPE_YPBPR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_YBRVGA\n", __func__);
		if(ybrvga_start_video(ch->input_idx)) {
			pr_err("[AVIN]%s: ybrvga_start_video failed!\n", __func__);
			ret = -EFAULT;
			goto err;
		}
		input_type = ch->input_type;
		break;
#endif

#ifdef ENABLE_HDMI
	case AVIN_TYPE_HDMI:
		pr_debug("[AVIN]%s: device AVIN_TYPE_HDMI\n", __func__);
		input_type = ch->input_type;
		hdmi_start_video(0);
		hdmi_start_audio(0);
		break;
#endif

#ifdef CONFIG_AVM_ENABLE
	case AVIN_TYPE_AVM_FRONT:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		ret = avm_start_video(1);
		if (ret) {
			pr_err("[AVIN]%s: avm_start_video(1) failed(%d)\r\n",__func__, ret);
			goto err;
		}
		input_type = ch->input_type;
		break;
	case AVIN_TYPE_AVM_REAR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		ret = avm_start_video(0);
		if (ret) {
			pr_err("[AVIN]%s: avm_start_video(0) failed(%d)\r\n",__func__, ret);
			goto err;
		}
		input_type = ch->input_type;
		break;
	case AVIN_TYPE_AVM_LEFT:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		ret = avm_start_video(2);
		if (ret) {
			pr_err("[AVIN]%s: avm_start_video(2) failed(%d)\r\n",__func__, ret);
			goto err;
		}
		input_type = ch->input_type;
		break;
	case AVIN_TYPE_AVM_RIGHT:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		ret = avm_start_video(3);
		
		if (ret) {
			pr_err("[AVIN]%s: avm_start_video(3) failed(%d)\r\n",__func__, ret);
			goto err;
		}
		input_type = ch->input_type;
		break;
	case AVIN_TYPE_AVM_SIGVIEW:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		ret = avm_start_video(4);
		
		if (ret) {
			pr_err("[AVIN]%s: avm_start_video(4) failed(%d)\r\n",__func__, ret);
			goto err;
		}
		input_type = ch->input_type;
		break;
#endif

	default:
		break;
	}

	ch->state = AVIN_CHANNEL_STATE_STARTED;
	return ret;

err:
	spin_lock_irqsave(&common->irqlock, flags);
	if (common->cur_frm == common->next_frm) {
		vb2_buffer_done(&common->cur_frm->vb, VB2_BUF_STATE_QUEUED);
	} else {
		if (common->cur_frm != NULL)
			vb2_buffer_done(&common->cur_frm->vb, VB2_BUF_STATE_QUEUED);
		if (common->next_frm != NULL)
			vb2_buffer_done(&common->next_frm->vb, VB2_BUF_STATE_QUEUED);
	}
	list_for_each_entry_safe(buf, tmp, &common->dma_queue, list) {
		list_del(&buf->list);
		vb2_buffer_done(&buf->vb, VB2_BUF_STATE_QUEUED);
	}
	spin_unlock_irqrestore(&common->irqlock, flags);

	return ret;
}

/**
 * avin_stop_streaming : Stop the DMA engine
 * @vq: ptr to vb2_queue
 *
 * This callback stops the DMA engine and any remaining buffers
 * in the DMA queue are released.
 */
static void avin_stop_streaming(struct vb2_queue *vq)
{
	struct channel_obj *ch = vb2_get_drv_priv(vq);
	struct common_obj *common;
	unsigned long flags;

	pr_debug("[AVIN]%s: enter\n", __func__);
	common = &ch->common[AVIN_VIDEO_INDEX];
	switch (ch->input_type) {
	case AVIN_TYPE_CVBS_VIDEO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_CVBS_VIDEO\n", __func__);
		cvbs_stop_video(TVD_APP_ID_AVIN, ch->input_idx);
		break;

	case AVIN_TYPE_BACKCAR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_BACKCAR\n", __func__);
		cvbs_stop_video(TVD_APP_ID_BACKCAR, ch->input_idx);
		break;

	case AVIN_TYPE_CVBS_AUDIO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_CVBS_AUDIO\n", __func__);
		cvbs_stop_audio(ch->input_idx);
		break;

	case AVIN_TYPE_DIGITAL_AUDIO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_DIGTAL_AUDIO\n", __func__);
		stop_digitalAud(1);
		break;

	case AVIN_TYPE_DIGITAL_VIDEO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_DIGITAL_VIDEO\n", __func__);
		//digital_stop_video();
		break;

#ifdef CONFIG_YBR_ENABLE
	case AVIN_TYPE_YPBPR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_YBRVGA\n", __func__);
		ybrvga_stop_video();
		break;
#endif

#ifdef ENABLE_HDMI
	case AVIN_TYPE_HDMI:
		pr_debug("[AVIN]%s: device AVIN_TYPE_HDMI\n", __func__);
		hdmi_stop_video(0);
		hdmi_stop_audio(0);
		break;
#endif

#ifdef CONFIG_AVM_ENABLE
	case AVIN_TYPE_AVM_FRONT:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		avm_stop_video(1);
		g_avm_data[0].streaming = 0;
		g_avm_data[0].memset_cnt = 0;
		g_avm_data[0].cur_frames = 0;
		break;
	case AVIN_TYPE_AVM_REAR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		avm_stop_video(0);
		g_avm_data[1].streaming = 0;
		g_avm_data[1].memset_cnt = 0;
		g_avm_data[1].cur_frames = 0;
		break;
	case AVIN_TYPE_AVM_LEFT:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		avm_stop_video(2);
		g_avm_data[2].streaming = 0;
		g_avm_data[2].memset_cnt = 0;
		g_avm_data[2].cur_frames = 0;
		break;
	case AVIN_TYPE_AVM_RIGHT:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		avm_stop_video(3);
		g_avm_data[3].streaming = 0;
		g_avm_data[3].memset_cnt = 0;
		g_avm_data[3].cur_frames = 0;
		break;
	case AVIN_TYPE_AVM_SIGVIEW:
		pr_debug("[AVIN]%s: device AVIN_TYPE_AVM\n", __func__);
		avm_stop_video(4);
		g_avm_data[4].streaming = 0;
		g_avm_data[4].memset_cnt = 0;
		g_avm_data[4].cur_frames = 0;
		break;
#endif

	default:
		break;
	}
	//input_type = 0U;
	/* release all active buffers */
	spin_lock_irqsave(&common->irqlock, flags);

	if (common->cur_frm == common->next_frm) {
		vb2_buffer_done(&common->cur_frm->vb, VB2_BUF_STATE_ERROR);
	} else {
		if (common->cur_frm != NULL)
			vb2_buffer_done(&common->cur_frm->vb, VB2_BUF_STATE_ERROR);

		if (common->next_frm != NULL)
			vb2_buffer_done(&common->next_frm->vb, VB2_BUF_STATE_ERROR);
	}

	while (!list_empty(&common->dma_queue)) {
		common->next_frm = list_entry(common->dma_queue.next, struct avin_cap_buffer, list);
		list_del(&common->next_frm->list);
		vb2_buffer_done(&common->next_frm->vb, VB2_BUF_STATE_ERROR);
	}

	spin_unlock_irqrestore(&common->irqlock, flags);
	ch->state = AVIN_CHANNEL_STATE_STOPPED;
	pr_debug("[AVIN]%s: success!\n", __func__);
}


static struct vb2_ops video_qops = {
	.queue_setup = avin_buffer_queue_setup,
	.start_streaming = avin_start_streaming,
	.stop_streaming = avin_stop_streaming,
	.buf_queue = avin_buffer_queue,
	.wait_prepare = avin_wait_prepare,
	.wait_finish = avin_wait_finish,
};

/**
 * avin_schedule_next_buffer: set next buffer address for capture
 * @common : ptr to common channel object
 *
 * This function will get next buffer from the dma queue and
 * set the buffer address in the avin register for capture.
 * the buffer is marked active
 */
static void avin_schedule_next_buffer2(struct common_obj *common)
{
	common->next_frm = list_entry(common->dma_queue.next, struct avin_cap_buffer, list);
	/* Remove that buffer from the buffer queue */
	list_del(&common->next_frm->list);
	/* Make curFrm pointing to nextFrm */
	common->cur_frm = common->next_frm;
}

/**
 * avin_process_buffer_complete: process a completed buffer
 * @common: ptr to common channel object
 *
 * This function time stamp the buffer and mark it as DONE. It also
 * wake up any process waiting on the QUEUE and set the next buffer
 * as current
 */
static void avin_process_buffer_complete(struct common_obj *common, const struct capture_priv *data)
{
	void *addr = 0;
	struct capture_priv *cap_param;

	addr = vb2_plane_vaddr(&common->cur_frm->vb, 0U);

#ifdef CONFIG_AVM_ENABLE
	if (common->alloc_size == sizeof(struct capture_priv)) {
		cap_param = (struct capture_priv *)addr;
		memcpy(cap_param, data, sizeof(struct capture_priv));
	} else {
		/*struct framebuf *pBuf = (struct framebuf *)data;
		if (addr != pBuf->data) {
			pr_err("[AVIN]%s: addr(0x%lx) != data(0x%lx)\n", __func__,
				(unsigned long)addr, (unsigned long)pBuf->data);
		} else {
			pBuf->status = FRAME_READY;
		}*/

		struct avmbuf *avmBuf = (struct avmbuf *)data;
		pr_debug("[AVIN]%s:add.y=0x%lx, add.c=0x%lx\n", __func__, avmBuf->yAddr, avmBuf->cAddr);
		memcpy(addr, avmBuf->yAddr, avmBuf->width*avmBuf->height);
		memcpy(addr+avmBuf->width*avmBuf->height, avmBuf->cAddr, avmBuf->width*avmBuf->height/2);
	}
#else
	cap_param = (struct capture_priv *)addr;
	memcpy(cap_param, data, sizeof(struct capture_priv));
#endif

	/*v4l2_get_timestamp(&common->cur_frm->vb.v4l2_buf.timestamp);*/
	vb2_buffer_done(&common->cur_frm->vb, VB2_BUF_STATE_DONE);
}

int avin_buffer_complete(enum avin_device_type device_type, const struct capture_priv *data)
{
	struct common_obj *common;
	unsigned long flags;
	common = &(avin_dev.dev[device_type]->common[0]);
	/* skip If streaming is not started in this channel */
	/* Check the field format */
	/* Progressive mode */
	spin_lock_irqsave(&common->irqlock, flags);
	if (list_empty(&common->dma_queue)) {
		spin_unlock_irqrestore(&common->irqlock, flags);
		//spin_unlock_irqsave(&common->irqlock);
		pr_info("[AVIN]%s: not have empty buffer\n",__func__);
		return -1;
	}
	avin_process_buffer_complete(common, data);
	avin_schedule_next_buffer2(common);
	spin_unlock_irqrestore(&common->irqlock, flags);
	return 0;
}


/**
 * avin_update_std_info() - update standard related info
 * @ch: ptr to channel object
 *
 * For a given standard selected by application, update values
 * in the device data structures
 */
static int avin_update_std_info(struct channel_obj *ch)
{
	struct common_obj *common = &ch->common[AVIN_VIDEO_INDEX];
	struct avin_params *avinparams = &ch->avinparams;
	const struct avin_channel_config_params *config;
	struct avin_channel_config_params *std_info = &avinparams->std_info;
	struct video_obj *vid_ch = &ch->video;
	int index;
	const unsigned int avin_ch_params_count = ARRAY_SIZE(avin_ch_params);;

	pr_debug("[AVIN]%s: enter\n", __func__);
	for (index = 0; index < avin_ch_params_count; index++) {
		config = &avin_ch_params[index];
		if (config->hd_sd == 0) {
			pr_debug("[AVIN]%s: SD format\n", __func__);
			if (config->stdid & vid_ch->stdid) {
				memcpy(std_info, config, sizeof(*config));
				break;
			}
		} else {
			pr_debug("[AVIN]%s: HD format\n", __func__);
			if (!memcmp(&config->dv_timings, &vid_ch->dv_timings,
				sizeof(vid_ch->dv_timings))) {
				memcpy(std_info, config, sizeof(*config));
				break;
			}
		}
	}

	/* standard not found */
	if (index == avin_ch_params_count) {
		return -EINVAL;
	}

	common->fmt.fmt.pix.width = std_info->width;
	common->width = std_info->width;
	common->fmt.fmt.pix.height = std_info->height;
	common->height = std_info->height;
	common->fmt.fmt.pix.sizeimage = common->height * common->width * 3 / 2;
	common->fmt.fmt.pix.bytesperline = std_info->width;
	avinparams->video_params.hpitch = std_info->width;
	avinparams->video_params.storage_mode = std_info->frm_fmt;

	if (vid_ch->stdid) {
		common->fmt.fmt.pix.colorspace = V4L2_COLORSPACE_SMPTE170M;
	} else {
		common->fmt.fmt.pix.colorspace = V4L2_COLORSPACE_REC709;
	}

	if (ch->avinparams.std_info.frm_fmt) {
		common->fmt.fmt.pix.field = V4L2_FIELD_NONE;
	} else {
		common->fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	}

	if (ch->avinparams.iface.if_type == AVIN_IF_RAW_BAYER) {
		common->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR8;
	} else {
		common->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	}

	common->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	return 0;
}

/**
 * avin_get_default_field() - Get default field type based on interface
 * @avin_params - ptr to avin params
 */
static inline enum v4l2_field avin_get_default_field(
	const struct avin_interface *iface)
{
	return (iface->if_type == AVIN_IF_RAW_BAYER) ? V4L2_FIELD_NONE : V4L2_FIELD_INTERLACED;
}


/**
 * avin_set_input() - Select an input
 * @avin_cfg - global config ptr
 * @ch - channel
 * @_index - Given input index from application
 *
 * Select the given input.
 */
static int avin_set_input(struct avin_capture_config *avin_cfg,
							struct channel_obj *ch, int index)
{
	/*struct cvbs_capture_chan_config *chan_cfg =*/
	/*		  &cvbs_cfg->chan_config[ch->channel_id];*/

	cvbs_init_audio(index);

	/*ch->input_idx = index;*/
	/* copy interface parameters to cvbs */
	/*ch->cvbsparams.iface = chan_cfg->cvbs_if;*/
	/* update tvnorms from the sub device input info */
	/*ch->video_dev->tvnorms = chan_cfg->inputs[index].input.std;*/
	return 0;
}

/**
 * avin_querystd() - querystd handler
 * @file: file ptr
 * @priv: file handle
 * @std_id: ptr to std id
 *
 * This function is called to detect standard at the selected input
 */
static int avin_querystd(struct file *file, void *priv, v4l2_std_id *std_id)
{
	int ret = 0;
#ifdef CONFIG_AVM_ENABLE
	int32_t idx = 0;
#endif
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);

	pr_debug("[AVIN]%s: enter\n", __func__);
#ifdef CONFIG_AVM_ENABLE
	ret = avin_get_devIdx(file, &idx);
	if (ret) {
		pr_err("[AVIN]%s: avin_get_devIdx failed(%d)\r\n", __func__, ret);
		return -EFAULT;
	}
	if (idx >= 0) {
		ret = avm_get_std((uint8_t)idx, std_id);
		if (ret) {
			pr_err("[AVIN]%s: tvp_get_std failed(%d)\r\n", __func__, ret);
			return -EFAULT;
		}
		ch->video.stdid = *std_id;
	} else {
#endif
		/* Call querystd function of decoder device */
		ret = v4l2_subdev_call(ch->sd, video, querystd, std_id);
		if ((ret == -ENOIOCTLCMD) || (ret == -ENODEV)) {
			return -ENODATA;
		}
		if (ret) {
			pr_debug("[AVIN]%s: Failed to query standard for sub devices!\n", __func__);
			return ret;
		}
#ifdef CONFIG_AVM_ENABLE
	}
#endif

	return 0;
}

/**
 * avin_g_std() - get STD handler
 * @file: file ptr
 * @priv: file handle
 * @std_id: ptr to std id
 */
static int avin_g_std(struct file *file, void *priv, v4l2_std_id *std)
{
	struct avin_capture_config *config = cap_dev->platform_data;
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);
	struct avin_capture_chan_config *chan_cfg;
	struct v4l2_input input;

	pr_debug("[AVIN]%s: enter\n", __func__);
	if (config->chan_config[ch->channel_id].inputs == NULL) {
		return -ENODATA;
	}

	chan_cfg = &config->chan_config[ch->channel_id];
	input = chan_cfg->inputs[ch->input_idx].input;

	if (input.capabilities != V4L2_IN_CAP_STD) {
		return -ENODATA;
	}

	*std = ch->video.stdid;

	return 0;
}

/**
 * avin_s_std() - set STD handler
 * @file: file ptr
 * @priv: file handle
 * @std_id: ptr to std id
 */
static int avin_s_std(struct file *file, void *priv, v4l2_std_id std_id)
{
	struct avin_capture_config *config = cap_dev->platform_data;
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);
	struct common_obj *common = &ch->common[AVIN_VIDEO_INDEX];
	struct avin_capture_chan_config *chan_cfg;
	struct v4l2_input input;
	int ret;

	pr_debug("[AVIN]%s: enter\n", __func__);
	if (config->chan_config[ch->channel_id].inputs == NULL) {
		return -ENODATA;
	}

	chan_cfg = &config->chan_config[ch->channel_id];
	input = chan_cfg->inputs[ch->input_idx].input;

	if (input.capabilities != V4L2_IN_CAP_STD) {
		return -ENODATA;
	}

	if (vb2_is_busy(&common->buffer_queue)) {
		return -EBUSY;
	}

	/* Call encoder subdevice function to set the standard */
	ch->video.stdid = std_id;
	memset(&ch->video.dv_timings, 0, sizeof(ch->video.dv_timings));

	/* Get the information about the standard */
	if (avin_update_std_info(ch)) {
		pr_err("[AVIN]%s: avin_update_std_info failed!\n", __func__);
		return -EINVAL;
	}

	/* set standard in the sub device */
	ret = v4l2_subdev_call(ch->sd, video, s_std, std_id);

	if (ret && (ret != -ENOIOCTLCMD) && (ret != -ENODEV)) {
		pr_debug("[AVIN]%s: Failed to set standard for sub devices!\n", __func__);
		return ret;
	}

	return 0;
}

/**
 * avin_enum_input() - ENUMINPUT handler
 * @file: file ptr
 * @priv: file handle
 * @input: ptr to input structure
 */
static int avin_enum_input(struct file *file, void *priv,
			   struct v4l2_input *input)
{
	struct avin_capture_config *config = cap_dev->platform_data;
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);
	struct avin_capture_chan_config *chan_cfg;

	chan_cfg = &config->chan_config[ch->channel_id];

	if (input->index >= chan_cfg->input_count) {
		return -EINVAL;
	}

	memcpy(input, &chan_cfg->inputs[input->index].input, sizeof(*input));

	return 0;
}

/**
 * avin_g_input() - Get INPUT handler
 * @file: file ptr
 * @priv: file handle
 * @index: ptr to input index
 */
static int avin_g_input(struct file *file, void *priv, unsigned int *index)
{
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);

	*index = ch->input_idx;

	return 0;
}

/**
 * avin_s_input() - Set INPUT handler
 * @file: file ptr
 * @priv: file handle
 * @index: input index
 */
static int avin_s_input(struct file *file, void *priv, unsigned int index)
{
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);

	pr_debug("[AVIN]%s: enter with index(%d)\n", __func__, index);
	if ((ch->input_type != AVIN_TYPE_HDMI) && (index > CVBS_MAX_PORT_NUM)) {
		pr_err("[AVIN]%s: index(%d) error!\n", __func__, index);
		return -EINVAL;
	}

	if (ch->state != AVIN_CHANNEL_STATE_STARTED) {
		ch->input_idx = index;
		pr_debug("[AVIN]%s: not streaming just setting!\n", __func__);
		return 0;
	}

	switch (ch->input_type) {
	case AVIN_TYPE_CVBS_VIDEO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_CVBS_VIDEO\n", __func__);
		if (cvbs_select_video(TVD_APP_ID_AVIN, index)) {
			pr_err("[AVIN]%s: cvbs_select_video failed!\n", __func__);
			return -1;
		}
		break;

	case AVIN_TYPE_BACKCAR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_BACKCAR\n", __func__);
		if (cvbs_select_video(TVD_APP_ID_BACKCAR, index)) {
			pr_err("[AVIN]%s: cvbs_select_video failed!\n", __func__);
			return -1;
		}
		break;

	case AVIN_TYPE_CVBS_AUDIO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_CVBS_AUDIO\n", __func__);
		if (cvbs_select_audio(index)) {
			pr_err("[AVIN]%s: cvbs_select_audio failed!\n", __func__);
			return -1;
		}
		break;

	case AVIN_TYPE_DIGITAL_AUDIO:
	case AVIN_TYPE_DIGITAL_VIDEO:
		pr_debug("[AVIN]%s: device AVIN_TYPE_DIGITAL_VIDEO/AUDIO\n", __func__);
		break;

#ifdef CONFIG_YBR_ENABLE
	case AVIN_TYPE_YPBPR:
		pr_debug("[AVIN]%s: device AVIN_TYPE_YBRVGA\n", __func__);
		if(ybrvga_select_video(index)) {
			pr_debug("[AVIN]%s: ybrvga_select_video failed!\n", __func__);
			return -1;
		}
		break;
#endif

#ifdef ENABLE_HDMI
	case AVIN_TYPE_HDMI:
		pr_debug("[AVIN]%s: send RCP Key to MHL Driver\n", __func__);
		sendRcp(index);
		break;
#endif

	case AVIN_TYPE_AVM_SIGVIEW:
		pr_debug("[AVIN]%s: device AVM set input \n", __func__);
		avm_set_input(index);
		break;
		
	default:
		break;
	}

	pr_debug("[AVIN]%s: success!\n", __func__);
	ch->input_idx = index;

	return 0;
}

/**
 * avin_enum_fmt_vid_cap() - ENUM_FMT handler
 * @file: file ptr
 * @priv: file handle
 * @index: input index
 */
static int avin_enum_fmt_vid_cap(struct file *file, void  *priv, struct v4l2_fmtdesc *fmt)
{
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);

	if (fmt->index != 0) {
		pr_err("[AVIN]%s: Invalid format index(%d)!\n", __func__, fmt->index);
		return -EINVAL;
	}

	/* Fill in the information about format */
	if (ch->avinparams.iface.if_type == AVIN_IF_RAW_BAYER) {
		fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		strcpy(fmt->description, "Raw Mode -Bayer Pattern GrRBGb");
		fmt->pixelformat = V4L2_PIX_FMT_SBGGR8;
	} else {
		fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		strcpy(fmt->description, "YCbCr4:2:2 YC Planar");
		fmt->pixelformat = V4L2_PIX_FMT_YUV420;
	}

	return 0;
}

/**
 * avin_try_fmt_vid_cap() - TRY_FMT handler
 * @file: file ptr
 * @priv: file handle
 * @fmt: ptr to v4l2 format structure
 */
static int avin_try_fmt_vid_cap(struct file *file, void *priv, struct v4l2_format *format)
{
#ifdef CONFIG_AVM_ENABLE
	int ret = 0;
	int32_t idx = 0;
#endif
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);
	struct v4l2_pix_format *pixfmt = &format->fmt.pix;
	struct common_obj *common = &(ch->common[AVIN_VIDEO_INDEX]);
	struct avin_params *avin_params = &ch->avinparams;

#ifdef CONFIG_AVM_ENABLE
	ret = avin_get_devIdx(file, &idx);
	if (ret) {
		pr_err("[AVIN]%s: avin_get_devIdx failed(%d)\r\n", __func__, ret);
		return -EFAULT;
	}
	if (idx >= 0) {
		if (pixfmt->pixelformat != V4L2_PIX_FMT_NV12) {
			pr_err("[AVIN][%s]%s: pixelformat is not V4L2_PIX_FMT_NV12, so error!\r\n",
				vdev->dev.kobj.name, __func__);
			return -EINVAL;
		}
		if ((pixfmt->width >= 720) && (pixfmt->height >= 576)) {
			pixfmt->width = 720;
			pixfmt->height = 576;
			BUFFER_WIDTH = 720;
			BUFFER_HEIGHT= 576;
		} else if ((pixfmt->width >= 720) && (pixfmt->height >= 480)) {
			pixfmt->width = 720;
			pixfmt->height = 480;
			BUFFER_WIDTH = 720;
			BUFFER_HEIGHT= 480;
		} else if ((pixfmt->width >= 368) && (pixfmt->height >= 288)) {
			pixfmt->width = 368;
			pixfmt->height = 288;
			BUFFER_WIDTH = 368;
			BUFFER_HEIGHT= 288;
		} else if ((pixfmt->width >= 368) && (pixfmt->height >= 240)) {
			pixfmt->width = 368;
			pixfmt->height = 240;
			BUFFER_WIDTH = 368;
			BUFFER_HEIGHT= 240;
		} else {
			pr_err("[AVIN][%s]%s: width(%d) height(%d) error with std(N/P)!\r\n",
				vdev->dev.kobj.name, __func__, pixfmt->width, pixfmt->height);
			return -EINVAL;
		}
		
		pixfmt->field = V4L2_FIELD_NONE;
		pixfmt->bytesperline = pixfmt->width;
		pixfmt->sizeimage = pixfmt->width * pixfmt->height * 3 / 2;
		pixfmt->colorspace = V4L2_COLORSPACE_SMPTE170M;
		pixfmt->priv = 0;
		common->alloc_size = pixfmt->sizeimage;
		memcpy(&(common->fmt.fmt.pix), pixfmt, sizeof(struct v4l2_pix_format));
		memcpy(&(g_avm_data[ch->input_type - AVIN_TYPE_AVM_FRONT].pix), pixfmt, sizeof(struct v4l2_pix_format));
	} else {
#endif
		if (avin_params->iface.if_type == AVIN_IF_RAW_BAYER) {
			if (pixfmt->pixelformat != V4L2_PIX_FMT_SBGGR8) {
				pixfmt->pixelformat = V4L2_PIX_FMT_SBGGR8;
			}
		} else {
			if (pixfmt->pixelformat != V4L2_PIX_FMT_YUV420) {
				pixfmt->pixelformat = V4L2_PIX_FMT_YUV420;
			}
		}
		common->fmt.fmt.pix.pixelformat = pixfmt->pixelformat;
		avin_update_std_info(ch);
		pixfmt->field = common->fmt.fmt.pix.field;
		pixfmt->colorspace = common->fmt.fmt.pix.colorspace;
		pixfmt->bytesperline = common->fmt.fmt.pix.width;
		pixfmt->width = common->fmt.fmt.pix.width;
		pixfmt->height = common->fmt.fmt.pix.height;
		pixfmt->sizeimage = pixfmt->bytesperline * pixfmt->height * 3 / 2;
		pixfmt->priv = 0;
#ifdef CONFIG_AVM_ENABLE
	}
#endif

	return 0;
}


/**
 * avin_g_fmt_vid_cap() - Set INPUT handler
 * @file: file ptr
 * @priv: file handle
 * @fmt: ptr to v4l2 format structure
 */
static int avin_g_fmt_vid_cap(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);
	struct common_obj *common = &ch->common[AVIN_VIDEO_INDEX];

	pr_debug("[AVIN]%s: enter\n", __func__);
	/* Check the validity of the buffer type */
	if (common->fmt.type != format->type) {
		return -EINVAL;
	}

	/* Fill in the information about format */
	*format = common->fmt;

	return 0;
}

/**
 * avin_s_fmt_vid_cap() - Set FMT handler
 * @file: file ptr
 * @priv: file handle
 * @fmt: ptr to v4l2 format structure
 */
static int avin_s_fmt_vid_cap(struct file *file, void *priv, struct v4l2_format *format)
{
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);
	struct common_obj *common = &ch->common[AVIN_VIDEO_INDEX];
	int ret;

	if (vb2_is_busy(&common->buffer_queue)) {
		return -EBUSY;
	}

	ret = avin_try_fmt_vid_cap(file, priv, format);
	if (ret) {
		return ret;
	}

	/* store the format in the channel object */
	common->fmt = *format;

	return 0;
}

/**
 * avin_querycap() - QUERYCAP handler
 * @file: file ptr
 * @priv: file handle
 * @cap: ptr to v4l2_capability structure
 */
static int avin_querycap(struct file *file, void *priv, struct v4l2_capability *cap)
{
	strcpy(cap->driver, AVIN_DRIVER_NAME);
	strcpy(cap->card, AVIN_DRIVER_NAME);
	cap->version = KERNEL_VERSION(1, 0, 0);
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING | V4L2_CAP_READWRITE;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;
	snprintf(cap->bus_info, sizeof(cap->bus_info), "platform:%s", dev_name(cap_dev));

	return 0;
}


/*
 * avin_log_status() - Status information
 * @file: file ptr
 * @priv: file handle
 *
 * Returns zero.
 */
static int avin_log_status(struct file *filep, void *priv)
{
	/* status for sub devices */
	v4l2_device_call_all(&avin_dev.v4l2_dev, 0, core, log_status);

	return 0;
}

static int avin_s_ctrl(struct file *file, void *priv, struct v4l2_control *a)
{
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);

	pr_debug("[AVIN]%s: enter with id(%d) value(%d)\n", __func__, a->id, a->value);

#ifdef CONFIG_YBR_ENABLE
	if(a->id == V4L2_CID_AUTOGAIN) {
		ybrvga_set_auto();
	}
#endif

	if(a->id == V4L2_CID_SET_FMT) {
		/*if (digital_select_video(a->value)) {
			pr_err("[AVIN]%s: digital_select_video failed!\r\n", __func__);
			return -EFAULT;
		}*/
	}

	if(a->id == V4L2_CID_SET_MIRROR) {
		switch (ch->input_type) {
		case AVIN_TYPE_CVBS_VIDEO:
			pr_debug("[AVIN]%s: device AVIN_TYPE_CVBS_VIDEO\n", __func__);
			if (cvbs_set_mirror(TVD_APP_ID_AVIN, a->value)) {
				pr_err("[AVIN]%s: set cvbs mirror failed!\r\n", __func__);
				return -EFAULT;
			}
			break;

		case AVIN_TYPE_BACKCAR:
			pr_debug("[AVIN]%s: device AVIN_TYPE_BACKCAR\n", __func__);
			if (cvbs_set_mirror(TVD_APP_ID_BACKCAR, a->value)) {
				pr_err("[AVIN]%s: set backcar mirror failed!\r\n", __func__);
				return -EFAULT;
			}
			break;

		default:
			pr_err("[AVIN]%s: no way to set mirror with device(%d)\n",
				__func__, ch->input_type);
			break;
		}
	}

	return 0;
}

static int avin_g_ctrl(struct file * file,void * fh,struct v4l2_control * ctrl)
{
	u32 error_code = 0;

	pr_debug("[AVIN]%s: enter with id(%d)!\n", __func__, ctrl->id);
	switch(ctrl->id) {
	case V4L2_CID_GET_SIGNAL_MESSAGE:
		error_code = cvbs_get_signal_status(&ctrl->value);
		break;
		
#ifdef ENABLE_HDMI
	case V4L2_CID_GET_HDMI_MHL_DEVICE:
		ctrl->value = hdmi_getDeviceType();
		break;
		
	case V4L2_CID_GET_ORIENTATION:
		ctrl->value = (__s32)hdmi_getOrientation();
		break;

	case V4L2_CID_GET_HDMI_MHL_VDORECT:
		break;

	case V4L2_CID_GET_HDMI_MHL_VDORECT0:
	case V4L2_CID_GET_HDMI_MHL_VDORECT1:
	case V4L2_CID_GET_HDMI_MHL_VDORECT2:
	case V4L2_CID_GET_HDMI_MHL_VDORECT3:
		getVideoRect(ctrl->id, &ctrl->value);
		break;
		
	case V4L2_CID_GET_HDMI_MHL_SIGNAL:
		ctrl->value = hdmi_getSignalStatus();
		break;
#endif

	default:
		break;
	}
	if(0 != error_code) {
		pr_debug("[AVIN]%s: failed!\n", __func__);
		return -1;
	} else {
		pr_debug("[AVIN]%s: success!\n", __func__);
		return 0;
	}
}

static int avin_open(struct file *file)
{
#ifdef CONFIG_AVM_ENABLE
	int ret = 0;
	int32_t idx = 0;
	struct video_device *vdev = video_devdata(file);

	ret = avin_get_devIdx(file, &idx);
	if (ret) {
		pr_err("[AVIN][%s]%s: avin_get_devIdx failed(%d)\r\n",
			vdev->dev.kobj.name, __func__, ret);
		return -EFAULT;
	}
	if (idx >= 0) {
		memset(&g_avm_data[idx], 0, sizeof(struct avm_data));
	}
#endif

	return v4l2_fh_open(file);
}
static int avin_release(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct channel_obj *ch = video_get_drvdata(vdev);

#ifdef CONFIG_AVM_ENABLE
	int ret = 0;
	int32_t idx = 0;

	ret = avin_get_devIdx(file, &idx);
	if (ret) {
		pr_err("[AVIN][%s]%s: avin_get_devIdx failed(%d)\r\n",
			vdev->dev.kobj.name, __func__, ret);
		return -EFAULT;
	}
	
#endif

	ch->input_idx = 1;

	return vb2_fop_release(file);
}



/* avin capture ioctl operations */
static const struct v4l2_ioctl_ops avin_ioctl_ops = {
	.vidioc_querycap			= &avin_querycap,
	.vidioc_enum_fmt_vid_cap	= &avin_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap		= &avin_g_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap		= &avin_s_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap 	= &avin_try_fmt_vid_cap,
	.vidioc_enum_input			= &avin_enum_input,
	.vidioc_s_input				= &avin_s_input,
	.vidioc_g_input				= &avin_g_input,

	.vidioc_reqbufs				= vb2_ioctl_reqbufs,
	.vidioc_create_bufs			= vb2_ioctl_create_bufs,
	.vidioc_querybuf			= vb2_ioctl_querybuf,
	.vidioc_qbuf				= vb2_ioctl_qbuf,
	.vidioc_dqbuf				= vb2_ioctl_dqbuf,
	.vidioc_expbuf				= vb2_ioctl_expbuf,
	.vidioc_streamon			= vb2_ioctl_streamon,
	.vidioc_streamoff			= vb2_ioctl_streamoff,

	.vidioc_querystd			= &avin_querystd,
	.vidioc_s_std				= &avin_s_std,
	.vidioc_g_std				= &avin_g_std,
	.vidioc_log_status			= &avin_log_status,
	.vidioc_s_ctrl				= &avin_s_ctrl,
	.vidioc_g_ctrl				= &avin_g_ctrl,
};

/* avin file operations */
static struct v4l2_file_operations avin_fops = {
	.owner = THIS_MODULE,
	.open = avin_open,
	.release = avin_release,
	.unlocked_ioctl = video_ioctl2,
	.mmap = vb2_fop_mmap,
	.poll = vb2_fop_poll
};

/**
 * initialize_avin() - Initialize avin data structures
 *
 * Allocate memory for data structures and initialize them
 */
static int initialize_avin(void)
{
	int err, i, j;
	int free_channel_objects_index;

	/* Allocate memory for multi channel objects */
	for (i = 0; i < AVIN_TYPE_MAX; i++) {
		avin_dev.dev[i] = kzalloc(sizeof(*avin_dev.dev[i]), GFP_KERNEL);
		/* If memory allocation fails, return error */
		if (!avin_dev.dev[i]) {
			free_channel_objects_index = i;
			err = -ENOMEM;
			goto avin_init_free_channel_objects;
		}
	}

	return 0;

avin_init_free_channel_objects:
	for (j = 0; j < free_channel_objects_index; j++) {
		kfree(avin_dev.dev[j]);
	}

	return err;
}


static int avin_probe_complete(void)
{
	struct common_obj *common;
	struct video_device *vdev;
	struct channel_obj *ch;
	struct vb2_queue *q;
	int i, j, err, k;

	for (j = 0; j < AVIN_TYPE_MAX; j++) {
		ch = avin_dev.dev[j];
		ch->channel_id = j;
		ch->input_type = j;
		ch->input_idx = 1;
		common = &(ch->common[AVIN_VIDEO_INDEX]);
		common->alloc_size = sizeof(struct capture_priv);
		spin_lock_init(&common->irqlock);
		mutex_init(&common->lock);

		/* select input 0 */
		err = avin_set_input(avin_dev.config, ch, 0);

		if (err) {
			goto probe_out;
		}

		/* set initial format */
		ch->video.stdid = V4L2_STD_525_60;
		memset(&ch->video.dv_timings, 0, sizeof(ch->video.dv_timings));
		avin_update_std_info(ch);

		/* Initialize vb2 queue */
		q = &common->buffer_queue;
		q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
		q->drv_priv = ch;
		q->ops = &video_qops;
		/*q->mem_ops = &vb2_dma_contig_memops;*/

		q->mem_ops = &vb2_vmalloc_memops;
		q->buf_struct_size = sizeof(struct avin_cap_buffer);
		q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
		q->min_buffers_needed = 1;
		q->lock = &common->lock;
		err = vb2_queue_init(q);
		if (err) {
			pr_err("[AVIN]%s: vb2_queue_init failed!\n", __func__);
			goto probe_out;
		}

		/*common->alloc_ctx = vb2_dma_contig_init_ctx(cap_dev);*/
		if (IS_ERR(common->alloc_ctx)) {
			pr_err("[AVIN]%s: failed to get the context!\n", __func__);
			err = PTR_ERR(common->alloc_ctx);
			goto probe_out;
		}

		INIT_LIST_HEAD(&common->dma_queue);

		/* Initialize the video_device structure */
		vdev = ch->video_dev;

		strlcpy(vdev->name, AVIN_DRIVER_NAME, sizeof(vdev->name));
		pr_debug("[AVIN]%s: vdev(%p) name(%s)!\n", __func__, vdev, vdev->name);
		vdev->release = video_device_release;
		vdev->fops = &avin_fops;
		vdev->ioctl_ops = &avin_ioctl_ops;
		vdev->v4l2_dev = &avin_dev.v4l2_dev;
		/*vdev->vfl_dir = VFL_DIR_RX;*/
		vdev->queue = q;
		vdev->lock = &common->lock;
		video_set_drvdata(ch->video_dev, ch);
#ifdef CONFIG_AVM_ENABLE
		if (j >= AVIN_TYPE_AVM_FRONT) {
			err = video_register_device(vdev, VFL_TYPE_GRABBER,
				(AVIN_DEVICE_ID_AVM_BASE + j - AVIN_TYPE_AVM_FRONT));
		} else {
#endif
			err = video_register_device(vdev, VFL_TYPE_GRABBER,
				(AVIN_DEVICE_ID_BASE + j));
#ifdef CONFIG_AVM_ENABLE
		}
#endif
		if (err) {
			goto probe_out;
		}
	}

	pr_debug("[AVIN]%s: initialized success!\n", __func__);

	return 0;

probe_out:
	for (k = 0; k < j; k++) {
		/* Get the pointer to the channel object */
		ch = avin_dev.dev[k];
		common = &ch->common[k];
		vb2_dma_contig_cleanup_ctx(common->alloc_ctx);
		/* Unregister video device */
		video_unregister_device(ch->video_dev);
	}

	kfree(avin_dev.sd);

	for (i = 0; i < AVIN_TYPE_MAX; i++) {
		ch = avin_dev.dev[i];
		/* Note: does nothing if ch->video_dev == NULL */
		video_device_release(ch->video_dev);
	}

	v4l2_device_unregister(&avin_dev.v4l2_dev);

	return err;
}


/**
 * avin_probe : This function probes the avin capture driver
 * @pdev: platform device pointer
 *
 * This creates device entries by register itself to the V4L2 driver and
 * initializes fields of each channel objects
 */
static __init int avin_probe(struct platform_device *pdev)
{
	int i, j, err;
	struct channel_obj *ch;
	struct video_device *vfd;

	pr_debug("[AVIN]%s: enter\n", __func__);

	cap_dev = &pdev->dev;

	err = initialize_avin();
	if (err) {
		pr_err("[AVIN]%s: initialize_avin failed!\n", __func__);
		return err;
	}

	err = v4l2_device_register(cap_dev, &avin_dev.v4l2_dev);
	if (err) {
		pr_err("[AVIN]%s: v4l2_device_register failed!\n", __func__);
		return err;
	}

	for (i = 0; i < AVIN_TYPE_MAX; i++) {
		/* Get the pointer to the channel object */
		ch = avin_dev.dev[i];
		/* Allocate memory for video device */
		vfd = video_device_alloc();
		if (NULL == vfd) {
			for (j = 0; j < i; j++) {
				ch = avin_dev.dev[j];
				video_device_release(ch->video_dev);
			}
			pr_err("[AVIN]%s: video_device_alloc failed!\n", __func__);
			err = -ENOMEM;
			return err;
		}

		/* Set video_dev to the video device */
		ch->video_dev = vfd;
	}

	avin_probe_complete();
	pr_info("[AVIN]%s: success!\n", __func__);

	return 0;
}


/**
 * avin_remove() - driver remove handler
 * @device: ptr to platform device structure
 *
 * The vidoe device is unregistered
 */
static int avin_remove(struct platform_device *device)
{
	struct common_obj *common;
	struct channel_obj *ch;
	int i;

	v4l2_device_unregister(&avin_dev.v4l2_dev);

	kfree(avin_dev.sd);

	/* un-register device */
	for (i = 0; i < AVIN_TYPE_MAX; i++) {
		/* Get the pointer to the channel object */
		ch = avin_dev.dev[i];
		common = &ch->common[AVIN_VIDEO_INDEX];
		/*vb2_dma_contig_cleanup_ctx(common->alloc_ctx);*/
		/* Unregister video device */
		video_unregister_device(ch->video_dev);
		kfree(avin_dev.dev[i]);
	}

	return 0;
}

#ifdef CONFIG_PM_SLEEP
/**
 * avin_suspend: avin device suspend
 */
static int avin_suspend(struct device *dev)
{

	struct common_obj *common;
	struct channel_obj *ch;
	int i;

	for (i = 0; i < AVIN_TYPE_MAX; i++) {
		/* Get the pointer to the channel object */
		ch = avin_dev.dev[i];
		common = &ch->common[AVIN_VIDEO_INDEX];

		if (!vb2_start_streaming_called(&common->buffer_queue)) {
			continue;
		}
	}

	return 0;
}

/*
 * avin_resume: avin device suspend
 */
static int avin_resume(struct device *dev)
{
	struct common_obj *common;
	struct channel_obj *ch;
	int i;

	for (i = 0; i < AVIN_TYPE_MAX; i++) {
		/* Get the pointer to the channel object */
		ch = avin_dev.dev[i];
		common = &ch->common[AVIN_VIDEO_INDEX];

		if (!vb2_start_streaming_called(&common->buffer_queue)) {
			continue;
		}
	}

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(avin_pm_ops, avin_suspend, avin_resume);

static __refdata struct platform_driver avin_driver = {
	.driver = {
		.name = AVIN_DRIVER_NAME,
		.owner = THIS_MODULE,
		.pm = &avin_pm_ops,
	},
	.probe = avin_probe,
	.remove = avin_remove,
};

#define TV_STD_ALL (V4L2_STD_NTSC | V4L2_STD_PAL)


static const struct avin_input avin_ch0_inputs[] = {
	{
		.input = {
			.index = 0,
			.name = "S-Video",
			.type = V4L2_INPUT_TYPE_CAMERA,
			.capabilities = V4L2_IN_CAP_STD,
			.std = TV_STD_ALL,
		},
	},
};

static struct avin_capture_config avin_capture_cfg = {
	/*
	.setup_input_path = setup_vpif_input_path,
	.setup_input_channel_mode = setup_vpif_input_channel_mode,
	*/
	.chan_config[0] = {
		.inputs = avin_ch0_inputs,
		.input_count = ARRAY_SIZE(avin_ch0_inputs),
	},

};

static struct platform_device avin_capture_dev = {
	.name = AVIN_DRIVER_NAME,
	.id = -1,
	.dev = {
		.platform_data = &avin_capture_cfg,
	},
};

static int __init avin_module_init(void)
{
	int ret = 0;

	MOD_VERSION_INFO(AVIN_MOD_NAME, AVIN_VER_MAIN, AVIN_VER_MINOR, AVIN_VER_REV);
	pr_info("[AVIN]%s: enter\n", __func__);
	ret = platform_device_register(&avin_capture_dev);
	if (ret) {
		pr_err("[AVIN]%s: platform_device_register failed(%d)!\n", __func__, ret);
		return ret;
	}

	ret = platform_driver_register(&avin_driver);
	if (ret) {
		pr_err("[AVIN]%s: platform_driver_register failed(%d)!\n", __func__, ret);
		platform_device_unregister(&avin_capture_dev);
		return ret;
	}

	cvbs_init();
	TWMgr_init();
	avm_init();
	ybrvga_init();
#ifdef CONFIG_AVM_ENABLE
	/*ret = tvp_init(TVP_SLAVE_ADDR);
	if (ret) {
		pr_err("[AVIN]%s: tvp_init failed(%d)\r\n", __func__, ret);
		return ret;
	}*/
#endif

	pr_info("[AVIN]%s: success!\n", __func__);

	return 0;
}

static void __exit avin_module_exit(void)
{
#ifdef CONFIG_AVM_ENABLE
	//tvp_deinit();
#endif
	platform_driver_unregister(&avin_driver);
	platform_device_unregister(&avin_capture_dev);
}

module_init(avin_module_init);
module_exit(avin_module_exit);


MODULE_DESCRIPTION("Autochips Audio/Video Port Interface driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(AVIN_DRIVER_VERSION);

