ENTRY(_start)
SECTIONS
{
 . = 0x1000,
 .header : {
    *(header.entry);
    *(header.entry32);
    *(header.text);
    *(header.data);
 }
 . = . + 0x10000,
 .text : {
  *(.text);
  _etext = . ;
 }
 .data : {
   *(.data);
   *(.rodata*);
 }
 .bss : {
 *(.bss);
 }
}
