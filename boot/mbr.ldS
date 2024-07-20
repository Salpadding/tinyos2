OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

MEMORY
{
    MBR(wx): ORIGIN = 0x7c00, LENGTH = 512
}

SECTIONS
{
 .text :
 {
  *(entry);
  *(.text);
 } > MBR

 .data : {
  *(.rodata*);
  *(.data);
  . = _start + 510;
  SHORT(0xAA55);
 } > MBR
}
