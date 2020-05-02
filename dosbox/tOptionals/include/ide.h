/*
 * IDE ATA/ATAPI emulation
 */

#ifndef DOSBOX_IDE_H
#define DOSBOX_IDE_H

#define MAX_IDE_CONTROLLERS 	8

extern const char *ide_names[MAX_IDE_CONTROLLERS];
extern void (*ide_inits[MAX_IDE_CONTROLLERS])(Section *);

void IDE_Auto(signed char &index,bool &slave);
void IDE_CDROM_Attach(signed char index,bool slave,unsigned char drive_index);
void IDE_CDROM_Detach(unsigned char drive_index);
void IDE_Hard_Disk_Attach(signed char index,bool slave,unsigned char bios_drive_index);
void IDE_Hard_Disk_Detach(unsigned char bios_drive_index);
void IDE_ResetDiskByBIOS(unsigned char disk);

/* From VGA.cpp */
static inline bool is_power_of_2(Bitu val) {
	return (val != 0) && ((val&(val-1)) == 0);
	/* To explain: if val is a power of 2, then only one bit is set.
	 * Decrementing val would change that one bit to 0, and all bits to the right to 1.
	 * Example:
	 *
	 * Power of 2: val = 1024
	 *
	 *      1024 = 0000 0100 0000 0000
	 *  AND 1023 = 0000 0011 1111 1111
	 *  ------------------------------
	 *         0 = 0000 0000 0000 0000
	 *
	 * Non-power of 2: val = 713
	 *
	 *       713 = 0000 0010 1100 1001
	 *   AND 712 = 0000 0010 1100 1000
	 *  ------------------------------
	 *       712 = 0000 0010 1100 1000
	 *
	 * See how that works?
	 *
	 * For more info see https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2*/
}

#endif
