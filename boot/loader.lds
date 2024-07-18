ENTRY(_start)
SECTIONS
{

 . = 0x500,
 .text : {
  *(entry);
  *(.text);
  _text_end = . ;
 }
 .data : {
   *(.data);
   *(.rodata*);
 }
}
