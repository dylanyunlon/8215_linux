/*
 * Copyright (c) 2022 AutoChips Inc.
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

#ifndef _LINUX_IDC_DEV_H
#define _LINUX_IDC_DEV_H

#include <linux/types.h>

#define MAX_CHANNEL_NAME_LEN 63

#define IDC_DOMAIN_KM_CLUSTER 0
#define IDC_DOMAIN_KM_ADAS 1
#define IDC_DOMAIN_KM_IVI 2

#define IDC_KM_CLUSTER_DOMAIN_NAME "cluster"
#define IDC_KM_ADAS_DOMAIN_NAME "adas"
#define IDC_KM_IVI_DOMAIN_NAME "ivi"

#define IDC_F_DOMAIN_SHM 34

/*
 * This feature indicates that memory accesses by the driver and the
 * device are ordered in a way described by the platform.
 */
#define IDC_F_ORDER_PLATFORM 36

/* This marks a buffer as continuing via the next field. */
#define VRING_DESC_F_NEXT 1
/* This marks a buffer as write-only (otherwise read-only). */
#define VRING_DESC_F_WRITE 2
/* This means the buffer contains a list of buffer descriptors. */
#define VRING_DESC_F_INDIRECT 4

#define VRING_DESC_F_EVENT 8

#define VRING_DESC_F_META 16

/*
 * Mark a descriptor as available or used in packed ring.
 * Notice: they are defined as shifts instead of shifted values.
 */
#define VRING_DESC_F_AVAIL 7
#define VRING_DESC_F_USING 11
#define VRING_DESC_F_USED 15

/* The Host uses this in used->flags to advise the Guest: don't kick me when
 * you add a buffer.  It's unreliable, so it's simply an optimization.  Guest
 * will still kick if it's out of buffers.
 */
#define VRING_USED_F_NO_NOTIFY 1
/* The Guest uses this in avail->flags to advise the Host: don't interrupt me
 * when you consume a buffer.  It's unreliable, so it's simply an
 * optimization.
 */
#define VRING_AVAIL_F_NO_INTERRUPT 1

/* Enable events in packed ring. */
#define VRING_EVENT_FLAG_ENABLE 0x0
/* Disable events in packed ring. */
#define VRING_EVENT_FLAG_DISABLE 0x1
/*
 * Enable events for a specific descriptor in packed ring.
 * (as specified by Descriptor Ring Change Event Offset/Wrap Counter).
 * Only valid if VIRTIO_RING_F_EVENT_IDX has been negotiated.
 */
#define VRING_EVENT_FLAG_DESC 0x2

/*
 * Wrap counter bit shift in event suppression structure
 * of packed ring.
 */
#define VRING_EVENT_F_WRAP_CTR 15

/* We support indirect buffer descriptors */
#define IDC_RING_F_INDIRECT_DESC 28

/* The Guest publishes the used index for which it expects an interrupt
 * at the end of the avail ring. Host should ignore the avail->flags field.
 */
/* The Host publishes the avail index for which it expects a kick
 * at the end of the used ring. Guest should ignore the used->flags field.
 */
#define IDC_RING_F_EVENT_IDX 29

/* Alignment requirements for vring elements.
 * When using pre-virtio 1.0 layout, these fall out naturally.
 */
#define VRING_AVAIL_ALIGN_SIZE 2
#define VRING_USED_ALIGN_SIZE 4
#define VRING_DESC_ALIGN_SIZE 16

#define IDC_KM_EVENT_CONNECTED 8
#define IDC_KM_EVENT_DISCONNECTED 9
#define IDC_KM_EVENT_RAW_DATA 0
#define IDC_KM_EVENT_MESSAGE 1
#define IDC_KM_EVENT_DMA_BUFFER 2
#define IDC_KM_EVENT_GFX_DMA_BUFFER 3
#define IDC_KM_EVENT_PARCEL 4

#define IDC_KM_EVENT_REPLY 255

#define GFX_BUF_PRIV_DATA_SZ (8)

struct idc_event_header {
    uint32_t id;
    uint32_t domain;
    uint32_t event_sz;
    int32_t channel;
    uint64_t rsp_id;
    uint32_t data_id;
    uint32_t data_sz;
    uint8_t data[4];
};

struct idc_dma_buf_data {
    union {
        void *buf;
        int fd;
    } data;
    uint32_t size;
};

struct idc_gfx_buf_meta_data {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t format;
    uint32_t priv[GFX_BUF_PRIV_DATA_SZ];
};

struct idc_gfx_buf_data {
    struct idc_gfx_buf_meta_data meta;
    union {
        void *buf;
        int fd;
    } data;
    uint32_t size;
};

struct vring_desc_event {
    /* Descriptor Ring Change Event Offset/Wrap Counter. */
    __le16 off_wrap;
    /* Descriptor Ring Change Event Flags. */
    __le16 flags;
};

struct idc_allocation_data {
    char name[MAX_CHANNEL_NAME_LEN + 1];
    bool is_tx;
    int channel;
};

struct idc_name_data {
    int channel;
    char name[MAX_CHANNEL_NAME_LEN + 1];
};

struct idc_connection_data {
    char domain_name[MAX_CHANNEL_NAME_LEN + 1];
    int channel;
};

struct idc_disconnection_data {
    int channel;
};

struct idc_event_data {
    int channel;
    uint32_t id;
    uint32_t data_id;
    uint64_t rsp_id;
    uint8_t *data;
    uint32_t data_sz;
};

struct idc_buffer_data {
    int channel;
    uint64_t rsp_id;
    int fd;
    int size;

    int fence;
};

struct idc_gfx_buffer_data {
    int channel;
    uint64_t rsp_id;
    struct idc_gfx_buf_meta_data meta;
    int fd;
    int fence;
};

struct idc_events_data {
    uint32_t timeout;
    uint32_t capacity;
    void *data;
    uint32_t size;
};

struct idc_event_reply_data {
    int channel;
    uint64_t rsp_id;
    uint8_t *data;
    uint32_t data_sz;
};

struct idc_destroy_data {
    int channel;
};

/* The following is used with USED_EVENT_IDX and AVAIL_EVENT_IDX */
/* Assuming a given event_idx value from the other side, if
 * we have just incremented index from old to new_idx,
 * should we trigger an event?
 */
static inline int vring_need_event(__u16 event_idx, __u16 new_idx, __u16 old)
{
    /* Note: Xen has similar logic for notification hold-off
     * in include/xen/interface/io/ring.h with req_event and req_prod
     * corresponding to event_idx + 1 and new_idx respectively.
     * Note also that req_event and req_prod in Xen start at 1,
     * event indexes in virtio start at 0.
     */
    return (__u16)(new_idx - event_idx - 1) < (__u16)(new_idx - old);
}

#define IDC_IOC_MAGIC 'I'

#define IDC_IOC_NEW_CHANNEL _IOWR(IDC_IOC_MAGIC, 0, struct idc_allocation_data)
#define IDC_IOC_SET_NAME _IOW(IDC_IOC_MAGIC, 5, struct idc_name_data)
#define IDC_IOC_GET_NAME _IOWR(IDC_IOC_MAGIC, 6, struct idc_name_data)
#define IDC_IOC_CONNECT _IOWR(IDC_IOC_MAGIC, 10, struct idc_connection_data)
#define IDC_IOC_DISCONNECT _IOWR(IDC_IOC_MAGIC, 11, struct idc_disconnection_data)
#define IDC_IOC_POST_EVENT _IOWR(IDC_IOC_MAGIC, 15, struct idc_event_data)
#define IDC_IOC_POST_BUFFER _IOWR(IDC_IOC_MAGIC, 16, struct idc_buffer_data)
#define IDC_IOC_POST_GFX_BUFFER _IOWR(IDC_IOC_MAGIC, 17, struct idc_gfx_buffer_data)
#define IDC_IOC_READ_EVENTS _IOWR(IDC_IOC_MAGIC, 20, struct idc_events_data)
#define IDC_IOC_EVENT_REPLY _IOWR(IDC_IOC_MAGIC, 25, struct idc_event_reply_data)
#define IDC_IOC_DESTROY _IOWR(IDC_IOC_MAGIC, 30, struct idc_destroy_data)

#endif /* _LINUX_IDC_DEV_H */
