#pragma once

#include "../../includes.h"

#define NR_IDE_CONTROLLERS 2  
/* Per ATA spec... */
#define NR_DEVICES_PER_CONTROLLER 2  

/* Position of each IDE controller. */
#define PRIMARY_IDE_CONTROLLER    0  
#define SECONDARY_IDE_CONTROLLER  1  

/* Base I/O ports of first and second ATA/IDE controllers.  These are standard values for a PC. */
#define PRIMARY_IDE_CONTROLLER_IOBASE   0x1F0
#define SECONDARY_IDE_CONTROLLER_IOBASE 0x170

/* Registers offsets. */
#define ATA_DATA        0
#define ATA_ERROR       1
#define ATA_NSECTOR     2
#define ATA_SECTOR      3
#define ATA_LCYL        4
#define ATA_HCYL        5
#define ATA_DRV_HEAD    6
#define ATA_STATUS      7
#define ATA_COMMAND     7
#define ATA_DEV_CTL     0x206

/* ATA protocol commands. */
#define ATA_IDENTIFY    0xEC
#define ATAPI_IDENTIFY  0xA1
#define ATA_READ_BLOCK  0x20
#define ATA_WRITE_BLOCK 0x30

/* Important bits in the status register of an ATA controller.
   See ATA/ATAPI-4 spec, section 7.15.6 */
#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_ERR  0x01

/* Important bits in the device control register.
   See ATA/ATAPI-4 spec, section 7.9.6 */
#define ATA_CTL_SRST    0x04
#define ATA_CTL_nIEN    0x02

/* Maximum timeout, in microseconds, for all commands (= 30 seconds) */
#define ATA_TIMEOUT 30000000

/* Operation types. */
#define IO_READ     0
#define IO_WRITE    1

/* Device position in the ATA chain. */
#define MASTER      0
#define SLAVE       1

/* The block size, in bytes. */
#define BLOCK_SIZE  512

/* The max. number of blocks this IDE driver can read/write in one operation. */
#define MAX_NBLOCKS 256

struct ide_device final {
	/* Pointer to the controller managing this device. */
	struct ide_controller *controller;

	/* Position of this device in the ATA chain (MASTER/SLAVE) */
	byte_t position : 1;

	/* Indicates whether this device was successfully identified.
	   If this bit is not set, the information included in this
	   structure below this point is not valid. */
	byte_t present : 1;

	/* Does this device support the PACKET command feature set? */
	byte_t atapi : 1;

	/* Does this device support LBA addressing? */
	byte_t lba : 1;

	/* Is DMA supported by this device? */
	byte_t dma : 1;

	/* General information about the device. */
	char model[40];
	char serial[20];
	char firmware[8];

	/* Disk geometry. */
	unsigned int cylinders;
	unsigned int heads;
	unsigned int sectors;
	unsigned int capacity;
};

struct ide_controller {
	/* Base I/O port:
	   0x1F0 for the 1st controller
	   0x170 for the 2nd controller */
	int iobase;

	/* List of devices attached to this controller. */
	struct ide_device devices[NR_DEVICES_PER_CONTROLLER];

	/* A controller can serve only one request at a time. This mutex
	   protects the controller while it's being used by another task. */
	struct kmutex *mutex;

	/* When issuing a request to the IDE controller, a task decrements
	   the value of this semaphore (DOWN). The IRQ handler increments it
	   when the I/O operation has completed. */
	struct ksema *io_sema;
};

/* Simplix supports up to 2 IDE controllers addressable via their standard
   I/O ports. A PC may have more than 2 IDE controllers and controllers may
   use different I/O ports. We don't handle these cases. */
static struct ide_controller controllers[NR_IDE_CONTROLLERS] = {
	{
		.iobase = PRIMARY_IDE_CONTROLLER_IOBASE,
		.devices = {
			{ .position = MASTER },
			{ .position = SLAVE }
		}
	},
	{
		.iobase = SECONDARY_IDE_CONTROLLER_IOBASE,
		.devices = {
			{ .position = MASTER },
			{ .position = SLAVE }
		}
	}
};

/*
 * The characters in the strings returned by the IDENTIFY command are
 * swapped (the spec mentions it) E.g 'eGenir c2143' => 'Generic 2143'
 * This function unscrambles them and wipes out any trailing garbage.
 */
static void fix_ide_string(char *s, int len)
{
	char c, *p = s, *end = s + (len & ~1);

	/* Swap characters. */
	while (p != end) {
		c = *p;
		*p = *(p + 1);
		*(p + 1) = c;
		p += 2;
	}

	/* Make sure we have a NULL byte at the end. Wipe out trailing garbage. */
	p = end - 1;
	*p-- = '\0';
	while (p-- != s) {
		c = *p;
		if (c > 32 && c < 127)
			break;
		*p = '\0';
	}
}

/*
 * Wait for the bits specified by the bitmask in the specified controller
 * status register to have the specified value, or for the specified timeout
 * (in microseconds) to expire. Returns whether or not the timeout expired.
 */
static byte_t wait_for_controller(struct ide_controller *controller,
	byte_t mask, byte_t value, unsigned long timeout)
{
	byte_t status;
	do {
		status = inb(controller->iobase + ATA_STATUS);
		udelay(1); /* Wait a little before trying again. */
	} while ((status & mask) != value && --timeout);
	return timeout;
}

/*
 * Resets the specified controller. Returns whether the controller was
 * successfully reset. See ATA/ATAPI-4 spec, section 9.3.
 */
static bool_t reset_controller(struct ide_controller *controller)
{
	int iobase = controller->iobase;

	/* Set the SRST bit in the control register. The spec says that the host
	   shall not begin polling the status register until at least 2 ms after
	   the SRST bit has been set. */
	outb(iobase + ATA_DEV_CTL, ATA_CTL_SRST); mdelay(2);

	/* The device is supposed to set the BSY flag within 400 ns of detecting
	   that the SRST bit has been set. */
	if (!wait_for_controller(controller, ATA_STATUS_BSY, ATA_STATUS_BSY, 1))
		return FALSE;

	/* The spec says that the device shall wait until the host resets the SRST
	   before proceeding any further with the reset sequence. */
	outb(iobase + ATA_DEV_CTL, 0);

	/* Wait at most 30 seconds for the BSY flag to be cleared. */
	if (!wait_for_controller(controller, ATA_STATUS_BSY, 0, ATA_TIMEOUT))
		return FALSE;

	return TRUE;
}

/*
 * Select the specified IDE device. Returns whether the operation was
 * successful. See ATA/ATAPI-4 spec, section 9.6.
 */
static bool_t select_device(struct ide_device *device)
{
	int iobase = device->controller->iobase;

	/* At this point, we must ensure that BSY = 0 and DRQ = 0.
	   See Device Selection Protocol, ATA/ATAPI-4 spec, section 9.6 */
	if ((inb(iobase + ATA_STATUS) & (ATA_STATUS_BSY | ATA_STATUS_DRQ)))
		return FALSE;

	/* Select the drive. The spec says to wait at least 400 ns before
	   reading the status register to ensure its content is valid. */
	outb(iobase + ATA_DRV_HEAD, 0xa0 | (device->position << 4)); udelay(1);

	/* By now, we should have BSY = 0 and DRQ = 0.
	   See Device Selection Protocol, ATA/ATAPI-4 spec, section 9.6. */
	if ((inb(iobase + ATA_STATUS) & (ATA_STATUS_BSY | ATA_STATUS_DRQ)))
		return FALSE;

	return TRUE;
}

/*
 * Tries to detect and identify the specified IDE device.
 * See http://www.osdev.org/wiki/ATA_PIO_Mode#IDENTIFY_command
 */
static void identify_ide_device(struct ide_device *device)
{
	int i, iobase = device->controller->iobase;
	byte_t status, cl, ch, cmd;
	uint16_t info[256];

	device->present = FALSE;

	/* Before we can do anything, we need to check whether we have an ATA
	   controller. By default, in bochs (and in most PCs), the first two
	   controllers are enabled. You can easily disable a controller in bochs
	   by adding the following line to .bochsrc (X = 0, 1, 2 or 3)
	   ataX: enabled=0
	   The best way to detect the presence of an ATA controller at a known
	   port is to write a value to (for example) its Sector Count register
	   and check that the value "sticks". */
	outb(iobase + ATA_NSECTOR, 0xab);
	if (inb(iobase + ATA_NSECTOR) != 0xab)
		return;

	/* Reset the controller. This step is apparently required (although not
	   by the ATA/ATAPI-4 spec) to get the correct device signature after the
	   drive has been selected. */
	reset_controller(device->controller);

	/* Execute device selection protocol. See ATA/ATAPI-4 spec, section 9.7 */
	if (!select_device(device))
		return;

	/* See ATA/ATAPI-4 spec, section 8.12.5.2 and 9.1. I also looked at bochs'
	   BIOS listing (rombios.c) as the spec does not seem to be respected. */
	if (inb(iobase + ATA_NSECTOR) == 0x01 &&
		inb(iobase + ATA_SECTOR) == 0x01) {
		cl = inb(iobase + ATA_LCYL);
		ch = inb(iobase + ATA_HCYL);
		status = inb(iobase + ATA_STATUS);
		if (cl == 0x14 && ch == 0xeb) {
			/* This device implements the PACKET Command feature set. */
			device->present = TRUE;
			device->atapi = TRUE;
		}
		else if (cl == 0 && ch == 0 && status != 0) {
			/* This device does not implement the PACKET Command feature set. */
			device->present = TRUE;
		}
	}

	if (!device->present)
		return;

	cmd = device->atapi ? ATAPI_IDENTIFY : ATA_IDENTIFY;

	/* Send the IDENTIFY (PACKET) DEVICE command. */
	outb(iobase + ATA_COMMAND, cmd); udelay(1);

	/* See ATA/ATAPI-4 spec, section 9.7 */
	if (!wait_for_controller(device->controller,
		ATA_STATUS_BSY | ATA_STATUS_DRQ | ATA_STATUS_ERR,
		ATA_STATUS_DRQ, ATA_TIMEOUT)) {
		device->present = FALSE;
		return;
	}

	/* The IDENTIFY command succeded. Collect the data. */
	for (i = 0; i < 256; i++)
		info[i] = inw(iobase + ATA_DATA);

	device->lba = (info[49] >> 9) & 1;
	device->dma = (info[49] >> 8) & 1;

	device->cylinders = (unsigned int)info[1];
	device->heads = (unsigned int)info[3];
	device->sectors = (unsigned int)info[6];

	/* Here, we simplified things a bit.
	   See ATA/ATAPI-4 spec, Annexe B for more information. */
	if (device->lba) {
		device->capacity = (unsigned int)info[60];
	}
	else {
		device->capacity = device->heads * device->sectors * device->cylinders;
	}

	/* Copy and massage the information that is useful to us. */
	memcpy(device->model, &info[27], 40);
	memcpy(device->serial, &info[10], 20);
	memcpy(device->firmware, &info[23], 8);

	fix_ide_string(device->model, 40);
	fix_ide_string(device->serial, 20);
	fix_ide_string(device->firmware, 8);
}

/*
 * Returns the IDE device associated with the specified minor number.
 */
static struct ide_device *get_ide_device(unsigned int minor)
{
	struct ide_controller *controller;
	struct ide_device *device;

	ASSERT(minor < NR_IDE_CONTROLLERS * NR_DEVICES_PER_CONTROLLER);

	controller = &controllers[minor / NR_DEVICES_PER_CONTROLLER];
	device = &controller->devices[minor % NR_DEVICES_PER_CONTROLLER];
	return device;
}

/*
 * Generic read/write function.
 */
static unsigned int ide_read_write_blocks(unsigned int minor, offset_t block,
	unsigned int nblocks, void *buffer, int type)
{
	struct ide_device *device;
	struct ide_controller *controller;
	uint16_t *buf = (uint16_t *)buffer;
	byte_t sc, cl, ch, hd, cmd;
	int iobase, i;

	device = get_ide_device(minor);
	if (!device->present)
		return 0;

	if (!nblocks)
		return 0;

	if (nblocks > MAX_NBLOCKS)
		nblocks = MAX_NBLOCKS;

	if (block + nblocks > device->capacity)
		return 0;

	controller = device->controller;
	iobase = controller->iobase;

	/* Protect our resource (the IDE controller) */
	kmutex_lock(controller->mutex);

	/* Either the controller was not being used, or the prior I/O operation
	   just completed and we were awoken by the call to kmutex_unlock below. */

	/* Execute device selection protocol. See ATA/ATAPI-4 spec, section 9.7 */
	if (!select_device(device)) {
		kmutex_unlock(controller->mutex);
		return 0;
	}

	if (device->lba) {
		sc = block & 0xff;
		cl = (block >> 8) & 0xff;
		ch = (block >> 16) & 0xff;
		hd = (block >> 24) & 0xf;
	}
	else {
		/* See http://en.wikipedia.org/wiki/CHS_conversion */
		int cyl = block / (device->heads * device->sectors);
		int tmp = block % (device->heads * device->sectors);
		sc = tmp % device->sectors + 1;
		cl = cyl & 0xff;
		ch = (cyl >> 8) & 0xff;
		hd = tmp / device->sectors;
	}

	cmd = type == IO_READ ? ATA_READ_BLOCK : ATA_WRITE_BLOCK;

	/* See ATA/ATAPI-4 spec, section 8.27.4 */
	outb(iobase + ATA_NSECTOR, nblocks);
	outb(iobase + ATA_SECTOR, sc);
	outb(iobase + ATA_LCYL, cl);
	outb(iobase + ATA_HCYL, ch);
	outb(iobase + ATA_DRV_HEAD, (device->lba << 6) | (device->position << 4) | hd);
	outb(iobase + ATA_COMMAND, cmd);

	/* The host shall wait at least 400 ns before reading the Status register.
	   See PIO data in/out protocol in ATA/ATAPI-4 spec. */
	udelay(1);

	/* Wait at most 30 seconds for the BSY flag to be cleared. */
	if (!wait_for_controller(controller, ATA_STATUS_BSY, 0, ATA_TIMEOUT)) {
		kmutex_unlock(controller->mutex);
		return 0;
	}

	/* Did the device report an error? */
	if (inb(iobase + ATA_STATUS) & ATA_STATUS_ERR) {
		kmutex_unlock(controller->mutex);
		return 0;
	}

	if (type == IO_WRITE) {
		/* Transfer the data to the controller. */
		for (i = nblocks * 256; --i >= 0;)
			outw(iobase + ATA_DATA, *buf++);
	}

	/* Go to sleep until the IRQ handler wakes us up. Note: on Bochs, the IRQ
	   is raised before we even reach this line! This is OK, and in that case,
	   this line will not make us go to sleep (the semaphore will have been
	   incremented by the IRQ handler prior to reaching this line) */
	ksema_down(controller->io_sema);

	/* Either the device completed the operation very quickly,
	   or we went to sleep and just got woken up by the IRQ handler. */

	/* Did the device report an error? */
	if (inb(iobase + ATA_STATUS) & ATA_STATUS_ERR) {
		kmutex_unlock(controller->mutex);
		return 0;
	}

	if (type == IO_READ) {
		/* Copy the data to the destination buffer. */
		for (i = nblocks * 256; --i >= 0;)
			*buf++ = inw(iobase + ATA_DATA);
	}

	kmutex_unlock(controller->mutex);

	return nblocks;
}
/*
 * Read the specified block from the specified device, and copy its content
 * to the destination buffer. The work is delegated to ide_read_write_blocks.
 */
static unsigned int ide_read_blocks(unsigned int minor, offset_t block,
	unsigned int nblocks, void *buffer)
{
	return ide_read_write_blocks(minor, block, nblocks, buffer, IO_READ);
}
/*
 * Write the content of the source buffer in the specified block of the
 * specified device. The work is delegated to ide_read_write_blocks.
 */
static unsigned int ide_write_blocks(unsigned int minor, offset_t block,
	unsigned int nblocks, void *buffer)
{
	return ide_read_write_blocks(minor, block, nblocks, buffer, IO_WRITE);
}

static void handle_ide_controller_interrupt(uint32_t esp,
struct ide_controller *controller)
{
	/* This wakes up the task waiting for the I/O operation to complete. */
	ksema_up(controller->io_sema);
}

static void handle_primary_ide_controller_interrupt(uint32_t esp)
{
	handle_ide_controller_interrupt(esp, &controllers[PRIMARY_IDE_CONTROLLER]);
}
static void handle_secondary_ide_controller_interrupt(uint32_t esp)
{
	handle_ide_controller_interrupt(esp, &controllers[SECONDARY_IDE_CONTROLLER]);
}

/*
 * Detect IDE devices and register IRQ handlers.
 * This function is called at boot time only!
 */
void init_ide_devices(void)
{
	int i, j;
	struct ide_device *device;
	struct ide_controller *controller;
	char msg[256];

	if (register_blkdev_class(BLKDEV_IDE_DISK_MAJOR, "IDE Hard Disk Driver",
		&ide_read_blocks, &ide_write_blocks) != S_OK)
		return;

	for (i = 0; i < NR_IDE_CONTROLLERS; i++) {
		/* Initialize the controller structure. */
		controller = &controllers[i];
		controller->mutex = kmutex_init();
		controller->io_sema = ksema_init(0);

		/* Detect and identify IDE devices attached to this controller. */
		for (j = 0; j < NR_DEVICES_PER_CONTROLLER; j++) {

			/* Initialize the device structure. */
			device = &controller->devices[j];
			device->controller = controller;
			identify_ide_device(device);

			if (!device->present || device->atapi)
				continue;

			/* Show the device information on the screen.
			   Since we don't support ATAPI devices, let's not list them! */
			snprintf(msg, 256,
				"%s [%u-%u]: %s (%u/%u/%u - %u sectors) LBA:%s - DMA:%s\n",
				device->atapi ? "CD-ROM" : "Hard Disk",
				i, device->position,
				device->model,
				device->cylinders,
				device->heads,
				device->sectors,
				device->capacity,
				device->lba ? "YES" : "NO",
				device->dma ? "YES" : "NO");
			gfx_putstring(msg);

			/* Register the device with the block device subsystem. */
			register_blkdev_instance(BLKDEV_IDE_DISK_MAJOR,
				i * NR_DEVICES_PER_CONTROLLER + j,
				msg, BLOCK_SIZE, device->capacity);
		}
	}

	/* Register an IRQ handler for each IDE controller. Even if a controller is
	   not present, or if no device is attached to it, this shouldn't hurt. */
	irq_set_handler(IRQ_PRIMARY_IDE, handle_primary_ide_controller_interrupt);
	irq_set_handler(IRQ_SECONDARY_IDE, handle_secondary_ide_controller_interrupt);

	/* Enable IRQ lines. */
	enable_irq_line(IRQ_PRIMARY_IDE);
	enable_irq_line(IRQ_SECONDARY_IDE);
}