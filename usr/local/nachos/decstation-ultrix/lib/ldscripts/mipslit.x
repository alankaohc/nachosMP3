OUTPUT_FORMAT("ecoff-littlemips", "ecoff-bigmips",
	      "ecoff-littlemips")
 SEARCH_DIR(/usr/local/nachos/decstation-ultrix/lib);
ENTRY(__start)
SECTIONS
{
  . = 0x400000 + SIZEOF_HEADERS;
  .text : {
     _ftext = . ;
    *(.init)
     eprol  =  .;
    *(.text)
    PROVIDE (__runtime_reloc_start = .);
    *(.rel.sdata)
    PROVIDE (__runtime_reloc_stop = .);
    *(.fini)
     etext  =  .;
     _etext  =  .;
  }
  . = 0x10000000;
  .rdata : {
    *(.rdata)
  }
   _fdata = ALIGN(16);
  .data : {
    *(.data)
    CONSTRUCTORS
  }
   _gp = ALIGN(16) + 0x8000;
  .lit8 : {
    *(.lit8)
  }
  .lit4 : {
    *(.lit4)
  }
  .sdata : {
    *(.sdata)
  }
   edata  =  .;
   _edata  =  .;
   _fbss = .;
  .sbss : {
    *(.sbss)
    *(.scommon)
  }
  .bss : {
    *(.bss)
    *(COMMON)
  }
   end = .;
   _end = .;
}
