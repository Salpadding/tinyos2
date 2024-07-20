ENTRY(_start)


SECTIONS
{
 . = HEADER_ENTRY,
 .header : {
    *(header.entry);
    *(header.entry32);
    *(header.text);
    *(header.data);
 }
 . = . + PM_ENTRY,
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
