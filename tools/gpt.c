#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define SECTOR_SIZE 512

#define GPT_PART_NAME_LEN   (72 / sizeof(uint16_t))

/* Globally unique identifier */
struct gpt_guid {
	uint32_t   time_low;
	uint16_t   time_mid;
	uint16_t   time_hi_and_version;
	uint8_t    clock_seq_hi;
	uint8_t    clock_seq_low;
	uint8_t    node[6];
};


/* The GPT Partition entry array contains an array of GPT entries. */
struct gpt_entry {
	struct gpt_guid     type; /* purpose and type of the partition */
	struct gpt_guid     partition_guid;
	uint64_t            lba_start;
	uint64_t            lba_end;
	uint64_t            attrs;
	uint16_t            name[GPT_PART_NAME_LEN];
}  __attribute__ ((packed));

struct gpt_header {
	uint64_t            signature; /* header identification */
	uint32_t            revision; /* header version */
	uint32_t            size; /* in bytes */
	uint32_t            crc32; /* header CRC checksum */
	uint32_t            reserved1; /* must be 0 */
	uint64_t            my_lba; /* LBA of block that contains this struct (LBA 1) */
	uint64_t            alternative_lba; /* backup GPT header */
	uint64_t            first_usable_lba; /* first usable logical block for partitions */
	uint64_t            last_usable_lba; /* last usable logical block for partitions */
	struct gpt_guid     disk_guid; /* unique disk identifier */
	uint64_t            partition_entry_lba; /* LBA of start of partition entries array */
	uint32_t            npartition_entries; /* total partition entries - normally 128 */
	uint32_t            sizeof_partition_entry; /* bytes for each GUID pt */
	uint32_t            partition_entry_array_crc32; /* partition CRC checksum */
	uint8_t             reserved2[512 - 92]; /* must all be 0 */
} __attribute__ ((packed));

struct gpt_record {
	uint8_t             boot_indicator; /* unused by EFI, set to 0x80 for bootable */
	uint8_t             start_head; /* unused by EFI, pt start in CHS */
	uint8_t             start_sector; /* unused by EFI, pt start in CHS */
	uint8_t             start_track;
	uint8_t             os_type; /* EFI and legacy non-EFI OS types */
	uint8_t             end_head; /* unused by EFI, pt end in CHS */
	uint8_t             end_sector; /* unused by EFI, pt end in CHS */
	uint8_t             end_track; /* unused by EFI, pt end in CHS */
	uint32_t            starting_lba; /* used by EFI - start addr of the on disk pt */
	uint32_t            size_in_lba; /* used by EFI - size of pt in LBA */
} __attribute__ ((packed));


int main(int argc, char** argv) {
    if(argc < 0) {
        printf("missing args\n");
        exit(1);
    }

    int fd = open(argv[1], O_RDONLY, 0);

    char sector_buf[SECTOR_SIZE];

    if(lseek(fd, SECTOR_SIZE * 1, SEEK_SET) < 0) {
        printf("error \n");
        exit(1);
    }

    if(read(fd, sector_buf, SECTOR_SIZE) < SECTOR_SIZE) {
        printf("error \n");
        exit(1);
    }

    struct gpt_header* ptr = (void*)(sector_buf);

    printf("")
}