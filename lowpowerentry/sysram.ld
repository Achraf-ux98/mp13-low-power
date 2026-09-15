SECTION_PROLOGUE(.sysram_text,,)
{
	. = ALIGN(4);
	__sysram_text_start = .;
	KEEP(*(.sysram_text))
	KEEP(*(.sysram_text.*))
	. = ALIGN(4);
	__sysram_text_end = .;
} GROUP_DATA_LINK_IN(SYSRAM, ROMABLE_REGION)

__sysram_text_load_start = LOADADDR(.sysram_text);

SECTION_PROLOGUE(.sysram_data,,)
{
	. = ALIGN(4);
	__sysram_data_start = .;
	KEEP(*(.sysram_data))
	KEEP(*(.sysram_data.*))
	. = ALIGN(4);
	__sysram_data_end = .;
} GROUP_DATA_LINK_IN(SYSRAM, ROMABLE_REGION)

__sysram_data_load_start = LOADADDR(.sysram_data);

SECTION_PROLOGUE(.sysram_bss,(NOLOAD),)
{
	. = ALIGN(8);
	__sysram_bss_start = .;
	KEEP(*(.sysram_bss))
	KEEP(*(.sysram_bss.*))
	. = ALIGN(8);
	__sysram_bss_end = .;
} GROUP_DATA_LINK_IN(SYSRAM, SYSRAM)