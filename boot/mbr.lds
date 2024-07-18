OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)
SECTIONS
{

 . = (0x7c00),
 .text :
 {
  *(entry);
  *(.text);
  _text_end = . ;
 }
 .data : {
  *(.rodata*);
  *(.data);
  . = _start + 510;
  SHORT(0xAA55);
 }
/DISCARD/ : { * }
}
