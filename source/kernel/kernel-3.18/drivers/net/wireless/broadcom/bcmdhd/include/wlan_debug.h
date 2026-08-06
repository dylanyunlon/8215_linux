/*
 * Copyright (c) 2025 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __WLAN_DEBUG_H
#define __WLAN_DEBUG_H

#include <linux/version.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <cnn_log.h>

#ifdef BCMPCIE
// non-zero: host error
extern int pcie_host_status_dump(int log_enable);
#define _pcie_host_status_dump pcie_host_status_dump
#else
#define _pcie_host_status_dump(a) 0
#endif

#define _readx(readx, addr) (_pcie_host_status_dump(0) ? 0 : readx(addr))
#define _readb(addr) _readx(readb, addr)
#define _readw(addr) _readx(readw, addr)
#define _readl(addr) _readx(readl, addr)
#ifdef CONFIG_64BIT
#define _readq(addr) _readx(readq, addr)
#endif

#define _writex(writex, val, addr) do { \
	if (!_pcie_host_status_dump(0)) { \
		writex(val, addr); \
	} \
} while (0)
#define _writeb(val, addr) _writex(writeb, val, addr)
#define _writew(val, addr) _writex(writew, val, addr)
#define _writel(val, addr) _writex(writel, val, addr)
#ifdef CONFIG_64BIT
#define _writeq(val, addr) _writex(writeq, val, addr)
#endif

#define _pci_read_config_dword(dev, where, val) \
	(_pcie_host_status_dump(0) ? 0 : pci_read_config_dword(dev, where, val))
#define _pci_write_config_dword(dev, where, val) \
	(_pcie_host_status_dump(0) ? 0 : pci_write_config_dword(dev, where, val))
#define _pci_save_state(dev) (_pcie_host_status_dump(0) ? 0 : pci_save_state(dev))
#define _pci_restore_state(dev) (_pcie_host_status_dump(0) ? 0 : pci_restore_state(dev))

#endif /* __WLAN_DEBUG_H */
