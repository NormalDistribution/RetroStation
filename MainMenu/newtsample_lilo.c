#include <alloca.h>
#include <newt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "devices.h"
#include "install.h"
#include "lilo.h"
#include "log.h"
#include "run.h"
#include "windows.h"

#ifdef __i386__
static char * kernelPath = "/boot/vmlinuz";
#elif __sparc__
static char * kernelPath = "/boot/vmlinux.gz";
#else
#error unsupported architecture
#endif

static int mkinitrd(char * kernelVersion) {
    char * argv[] = { "/sbin/mkinitrd", "/boot/initrd", "--ifneeded", 
			kernelVersion, NULL };
    int rc;
    static alreadyHappened = 0;

    #ifdef __sparc__
	return 0;
    #endif

    if (alreadyHappened) return 0;

    if (!access("/mnt/boot/initrd", X_OK)) {
	logMessage("/mnt/boot/initrd exists -- moving to "
			"/mnt/boot/initrd.orig");
	rename("/mnt/boot/initrd", "/mnt/boot/initrd.orig");
    }

    if (loadModule("loop", DRIVER_OTHER, DRIVER_MINOR_NONE, NULL)) 
	return INST_ERROR;

    winStatus(32, 3, "LILO", "Creating initial ramdisk...");
    rc = runProgramRoot(RUN_LOG, "/mnt", "/sbin/mkinitrd", argv);
    newtPopWindow();

    removeModule("loop");

    if (rc) {
	unlink("/mnt/boot/initrd");
    } else {
	alreadyHappened = 1;
    }

    return rc;
}

#define SKIP_LILO 1000

#if defined(__i386__)
static int liloWhere(char * hdName, char * bootDevice, char ** where) {
    newtComponent form, okay, listbox, cancel, answer, skip;
    char * format = "/dev/%-7s    %s";
    char buf[200];
    void * which;

    newtOpenWindow(12, 6, 55, 11, "Lilo Installation");

    form = newtForm(NULL, NULL, 0);
    newtFormAddComponent(form, 
		newtLabel(1, 1, "Where do you want to install "
				"the bootloader?"));

    listbox = newtListbox(2, 3, 3, NEWT_LISTBOX_RETURNEXIT);
    sprintf(buf, format, hdName, 
            "Master Boot Record");
    newtListboxAddEntry(listbox, buf, (void *) 1);
    sprintf(buf, format, bootDevice, 
            "First sector of boot partition");
    newtListboxAddEntry(listbox, buf, (void *) 2);
    sprintf(buf, format, "fd0", 
            "First sector of a floppy disk");
    newtListboxAddEntry(listbox, buf, (void *) 3);

    okay = newtButton(6, 7, "Ok");
    skip = newtButton(22, 7, "Skip");
    cancel = newtButton(38, 7, "Cancel");
    newtFormAddComponents(form, listbox, okay, skip, cancel, NULL);

    answer = newtRunForm(form);

    which = newtListboxGetCurrent(listbox);
    
    newtFormDestroy(form);
    newtPopWindow();

    if (answer == cancel) return INST_CANCEL;
    if (answer == skip) return SKIP_LILO;

    switch ((int) which) {
      case 1: 		*where = hdName; break;
      case 2: 		*where = bootDevice; break;
      case 3: 		*where = "fd0"; break;
    }

    return 0;
}
#elif defined(__sparc__)
static int liloWhere(char * hdName, char * bootDevice, char ** where) {
    newtComponent text, yes, no, cancel, f, answer;
    int rc;

    newtOpenWindow(18, 6, 42, 10, "SILO Installation");

    text = newtTextbox(1, 1, 40, 3, NEWT_TEXTBOX_WRAP);
    newtTextboxSetText(text,
	    "Would you like to install or configure the SILO bootloader on "
	    "your system?");

    yes = newtButton(3, 6, " Yes ");
    no = newtButton(16, 6, "  No  ");
    cancel = newtButton(29, 6, "Cancel");

    f = newtForm(NULL, NULL, 0);
    newtFormAddComponents(f, text, yes, no, cancel, NULL);

    answer = newtRunForm(f);
    if (answer == f) 
	answer = newtFormGetCurrent(f);

    newtPopWindow();
    newtFormDestroy(f);

    if (answer == yes) {
        *where = bootDevice; 
	rc = 0;
    } else if (answer == cancel) {
	rc = INST_CANCEL;
	return INST_CANCEL;
    } else {
	rc = SKIP_LILO;
    }

    return rc;
}
#endif

static void editBootLabel(struct partition * item) {
    newtComponent form, entry, okay, cancel, clear, answer;
    char buf[50];
    char * entryValue;
 
    newtOpenWindow(10, 7, 50, 10, "Edit Boot Label");

    form = newtForm(NULL, NULL, 0);

    strcpy(buf,"Device      : /dev/");
    strcat(buf, item->device);
    newtFormAddComponent(form, newtLabel(1, 1, buf));
    newtFormAddComponent(form, newtLabel(1, 3, "Boot label :"));

    entry = newtEntry(17, 3, item->bootLabel, 20, &entryValue, 
		      NEWT_ENTRY_SCROLL | NEWT_ENTRY_RETURNEXIT);

    okay = newtButton(5, 6, "Ok");
    clear = newtButton(20, 6, "Clear");
    cancel = newtButton(35, 6, "Cancel");

    newtFormAddComponents(form, entry, okay, clear, cancel, NULL);

    do {
	answer = newtRunForm(form);

	if (answer == clear)
	    newtEntrySet(entry, "", 1);
    } while (answer == clear);

    if (answer != cancel) {
	if (item->bootLabel) free(item->bootLabel);

	if (strlen(entryValue))
	    item->bootLabel = strdup(entryValue);
	else
	    item->bootLabel = NULL;
    }

    newtPopWindow();
}	    

static int doinstallLilo(char * prefix, char * dev, char * rootdev,
			 struct partitionTable table,
			 char * append, char * kernelVersion, 
			 char * hdname, int linear) {
    char filename[100];
    FILE * f;
    char * argv[] = { "/mnt/sbin/lilo", NULL };
    int i;
    int rc;
    struct stat sb;
    int useInitrd = 0;
    struct partition * root = NULL;

    if (mkinitrd(kernelVersion))
	return INST_ERROR;

    if (!stat("/mnt/boot/initrd", &sb))
	useInitrd = 1;

    #ifdef __sparc__
	sprintf(filename, "%s/silo.conf", prefix);
    #else
	sprintf(filename, "%s/lilo.conf", prefix);
    #endif

    /* why not? */
    rename("/mnt/etc/lilo.conf", "/mnt/etc/lilo.conf.orig");
    rename("/mnt/etc/silo.conf", "/mnt/etc/silo.conf.orig");
    
    f = fopen(filename, "w");
    if (!f) {
	errorWindow("cannot create [ls]ilo config file: %s");
	return INST_ERROR;
    }

    logMessage("writing [sl]ilo config to %s", filename);

    for (i = 0; i < table.count; i++) {
	if (table.parts[i].bootLabel && 
		table.parts[i].type == PART_EXT2) {
	    root = table.parts + i;
	}
    }

    if (!root) {
	errorWindow("No ext2 partition is bootable");
	return INST_ERROR;
    }

    #ifdef __i386__
	fprintf(f, "boot=/dev/%s\n", dev);
	fprintf(f, "map=/boot/map\n");
	fprintf(f, "install=/boot/boot.b\n");
	fprintf(f, "prompt\n");
	if (linear) fprintf(f, "linear\n");
	fprintf(f, "timeout=50\n");
    #elif __sparc__
	fprintf(f, "timeout=50\n");
	fprintf(f, "partition=%s\n", root->device + 3);
	fprintf(f, "root=/dev/%s\n", root->device);
    #else
	#error "unsupported architecture";
    #endif

    for (i = 0; i < table.count; i++) {
	if (table.parts[i].bootLabel) {
	    if (table.parts[i].type == PART_EXT2) {
		fprintf(f, "image=%s\n", kernelPath);
		fprintf(f, "\tlabel=%s\n", table.parts[i].bootLabel);
		
		/* we can setup a /boot on the fresh system if we need to */
		if (!strcmp(table.parts[i].device, dev))
		    fprintf(f, "\troot=/dev/%s\n", rootdev);
		else
		    fprintf(f, "\troot=/dev/%s\n", table.parts[i].device);
		if (useInitrd)
		    fprintf(f, "\tinitrd=/boot/initrd\n");
		if (append) fprintf(f, "\tappend=\"%s\"\n", append);
		fprintf(f, "\tread-only\n");
	  #ifdef __i386__
	    } else {
		fprintf(f, "other=/dev/%s\n", table.parts[i].device);
		fprintf(f, "\tlabel=%s\n", table.parts[i].bootLabel);
		fprintf(f, "\ttable=/dev/%.3s\n", table.parts[i].device);
		if (strncmp(table.parts[i].device, hdname, 3)) 
		    fprintf(f, "\tloader=/boot/any_d.b\n");
	  #endif
	    }
	}
    }

    fclose(f);

    winStatus(35, 3, "Running", "Installing boot loader...");

    #ifdef __i386__
	rc = runProgramRoot(RUN_LOG, "/mnt", "sbin/lilo", argv);
    #elif __sparc__ 
	rc = doMount("/proc", "/mnt/proc", "proc", 0, 0);
	if (rc) {
	    newtPopWindow();
	    return rc;
	}
	rc = runProgramRoot(RUN_LOG, "/mnt", "sbin/silo", argv);
	umount("/mnt/proc");
    #else
	#error unsupported architectures
    #endif

    newtPopWindow();

    if (rc)
	return INST_ERROR;

    return 0;
}

static int getBootLabels(struct partitionTable table, struct fstab fstab) {
    newtComponent f, okay, text, listbox, label, cancel, edit, answer;
    char buf[80];
    int i, j;
    int foundDos = 0;
    int mustAsk = 0;
    int * map;
    struct partition * curr;
    int * currNum;
    int count;

    f = newtForm(NULL, NULL, 0);
    text = newtTextbox(1, 1, 60, 4, NEWT_TEXTBOX_WRAP);
    newtTextboxSetText(text, 
		       "The boot manager Red Hat uses can boot other " 
                       "operating systems as well. You need to tell me " 
                       "what partitions you would like to be able to boot " 
                       "and what label you want to use for each of them.");


    sprintf(buf, "%-10s  %-25s %-18s", "Device", "Partition type", 
			"Boot label");
    label = newtLabel(1, 6, buf);

    listbox = newtListbox(1, 7, 7, NEWT_LISTBOX_RETURNEXIT);
    map = alloca(sizeof(int) * table.count);
    
    for (i = 0, count = 0; i < table.count; i++) {
	if (table.parts[i].type != PART_SWAP && 
	    table.parts[i].type != PART_IGNORE &&
#ifdef __sparc__
	    table.parts[i].type != PART_OTHER &&
#endif
	    (table.parts[i].type != PART_DOS || !foundDos)) {

	    if (table.parts[i].type == PART_DOS) {
		table.parts[i].bootLabel = strdup("dos");
		foundDos = 1;
	    }

	    if (table.parts[i].type == PART_EXT2) {
		for (j = 0; j < fstab.numEntries; j++) {
		    if (!strcmp(table.parts[i].device, fstab.entries[j].device))
			break;
		}

		if (j < fstab.numEntries && !table.parts[i].bootLabel) 
		    continue;
	    }

	    if (!table.parts[i].bootLabel ||
		strcmp(table.parts[i].bootLabel, "linux")) mustAsk = 1;

	    sprintf(buf, "/dev/%-5s  %-25s %-18s", table.parts[i].device, 
		    table.parts[i].tagName, table.parts[i].bootLabel ? 
			table.parts[i].bootLabel : "");
	    map[count] = i;
	    newtListboxAddEntry(listbox, buf, map + count++);
	} 
    }

    newtFormAddComponents(f, text, label, listbox, NULL);

    if (!mustAsk) {
	newtFormDestroy(f);
	return 0;
    }

    newtOpenWindow(8, 2, 64, 19, "Bootable Partitions");

    okay = newtButton(8, 15, "Ok");
    edit = newtButton(26, 15, "Edit");
    cancel = newtButton(44, 15, "Cancel");

    newtFormAddComponents(f, okay, edit, cancel, NULL);

    do { 
	answer = newtRunForm(f);
	if (answer == edit || answer == listbox) {
	    currNum = newtListboxGetCurrent(listbox);
	    curr = table.parts + *currNum;
	    editBootLabel(curr);
	    sprintf(buf, "/dev/%-5s  %-25s %-18s", curr->device, 
		    curr->tagName, 
		    curr->bootLabel ? curr->bootLabel : "");

	    newtListboxSetEntry(listbox, currNum - map, buf);
	}
    } while (answer == edit || answer == listbox);

    newtFormDestroy(f);
    newtPopWindow();

    if (answer == cancel)
	return INST_CANCEL;
    else
	return 0;
}

static int getAppendLine(char ** line, int * linear) {
    newtComponent form, text, entry, okay, cancel, answer;
    newtComponent linearCheck;
    char * result = NULL;
    char linearChar = (*linear) ? '*' : ' ';
    int buttonLine = 9;

    #ifdef __sparc__
	newtOpenWindow(12, 5, 55, 13, "Silo Installation");
    #else
	/* this is bigger on the Intel to leave room for the linear checkbox */
	newtOpenWindow(12, 5, 55, 15, "Lilo Installation");
    #endif

    form = newtForm(NULL, NULL, 0);
    text = newtTextbox(1, 1, 53, 5, NEWT_TEXTBOX_WRAP);
    newtTextboxSetText(text, 
		       "A few systems will need to pass special options "
		       "to the kernel at boot time for the system to function "
		       "properly. If you need to pass boot options to the "
		       "kernel, enter them now. If you don't need any or "
		       "aren't sure, leave this blank.");

    entry = newtEntry(1, 7, *line, 48, &result, 
			NEWT_ENTRY_SCROLL | NEWT_ENTRY_RETURNEXIT);

    #ifndef __sparc__
	buttonLine = 11;
	linearCheck = newtCheckbox(1, 9, 
			       "Use linear mode (needed for some SCSI drives)",
			       linearChar, NULL, &linearChar);
    #endif

    okay = newtButton(12, buttonLine, "Ok");
    cancel = newtButton(35, buttonLine, "Cancel");

    newtFormAddComponents(form, text, entry, NULL);
    #ifndef __sparc__
	newtFormAddComponent(form, linearCheck);
    #endif
    newtFormAddComponents(form, okay, cancel, NULL);
    newtFormSetCurrent(form, okay);

    answer = newtRunForm(form);

    newtPopWindow();

    if (answer == cancel) {
	newtFormDestroy(form);
	return INST_CANCEL;
    }

    *linear = linearChar != ' ';

    if (!strlen(result)) 
	*line = NULL;
    else 
	*line = strdup(result);

    newtFormDestroy(form);

    return 0;
}

#define LILO_WHERE 2
#define LILO_LABELS 3
#define LILO_INSTALL 4
#define LILO_APPEND 5
#define LILO_DONE 20
    
int installLilo(char * prefix, struct partitionTable table, 
	        struct fstab fstab, char * kernelVersion) {
    char * rootDevice, * bootDevice = NULL;
    char * hdName;
    char * where;
    char * append = NULL;
    char * chptr = NULL;
    int i;
    int rc;
    int stage = LILO_WHERE;
    static int linear = 0;

    hdName = alloca(4);
    strncpy(hdName, table.parts[0].device, 3);
    hdName[3] = '\0';
    
    for (i = 0; i < fstab.numEntries; i++) {
	if (!strcmp(fstab.entries[i].mntpoint, "/boot")) break;
    }

    if (i < fstab.numEntries) 
	bootDevice = fstab.entries[i].device;

    for (i = 0; i < fstab.numEntries; i++) {
	if (!strcmp(fstab.entries[i].mntpoint, "/")) break;
    }

    rootDevice = fstab.entries[i].device;
    if (!bootDevice) {
	bootDevice = rootDevice;
    }

    for (i = 0; i < table.count; i++) {
	if (!strcmp(table.parts[i].device, bootDevice)) {
	    table.parts[i].bootLabel = strdup("linux");
	    break;
	}
    }

    while (stage != LILO_DONE) {
	switch (stage) {
	  case LILO_WHERE:
	    rc = liloWhere(hdName, bootDevice, &where);
            if (rc == SKIP_LILO ) return 0;
	    if (rc) return rc;
	    stage = LILO_APPEND;
	    break;

	  case LILO_APPEND:
	    chptr = append;
	    rc = getAppendLine(&chptr, &linear);

	    if (rc == INST_ERROR) return INST_ERROR;
	    if (rc == INST_CANCEL)
		stage = LILO_WHERE;
	    else {
		stage = LILO_LABELS;

		if (append) free(append);
		if (chptr) {
		    append = alloca(strlen(chptr) + 1);
		    strcpy(append, chptr);
		    free(chptr);
		} else {
		    append = NULL;
		}
	    }

	    break;

	  case LILO_LABELS:
	    rc = getBootLabels(table, fstab);
	    if (rc == INST_ERROR) return INST_ERROR;
	    if (rc == INST_CANCEL)
		stage = LILO_APPEND;
	    else
		stage = LILO_INSTALL;
	    break;

	  case LILO_INSTALL:
	    rc = doinstallLilo(prefix, where, rootDevice, table, append, 
				kernelVersion, hdName, linear);
	    if (rc == INST_ERROR) return INST_ERROR;
	    stage = LILO_DONE;
	    break;
	}
    }

    return 0;
}
