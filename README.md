# agon-projects

Part of the official Quark firmware for the Agon series of microcomputers

### What is the Agon

Agon is a modern, fully open-source, 8-bit microcomputer and microcontroller in one small, low-cost board. As a computer, it is a standalone device that requires no host PC: it puts out its own video (VGA), audio (2 identical mono channels), accepts a PS/2 keyboard and has its own mass-storage in the form of a µSD card.

https://www.thebyteattic.com/p/agon.html

### The Projects

Each project is a self-contained executable and can be compiled and tested using the Zilog ZDS tools and USB smart cable.

The resultant Intel Hex file can be converted to an AGON executable bin file by using a hex2bin utility. If the command is to be executed as a star command from the command line, then the bin file must be copied to a mos directory in the root directory of the SD card.

- `ASM`:
	- `Hello World 16`: A simple example of a Z80 executable (16-bit Z80)
	- `Hello World 24`: A simple example of an ADL executable (24-bit Z80)
	- `Memory Dump`: A memory hex-dump to screen utility 
- `C`:

### The MOS executable format

The MOS header is stored from bytes 64 in the executable and consists of the following:

- A three byte ASCII representation of the word MOS
- A single byte for the header version
- A single byte for the executable type: 0 = Z80, 1 = ADL

It is stored at offset 64 in order for there to be sufficient space for the Z80 in Z80 mode. You can see examples of these in each projects init.asm file.

### Prerequisites

Either add the mos/src folder to the include folder in the project settings or copy the latest [mos_api.inc](https://github.com/breakintoprogram/agon-mos/blob/main/src/mos_api.inc) from the MOS project into the project folder before build.

### Testing

- Each project is designed to assemble/compile in debug mode at address &040000.
- Once loaded, it can be executed using the MOS command `run`.
- Additional parameters can be specified after the command, for example: `run memdump &100`

### Creating MOS Binary Executable Files

Z80 executables can be generated directly from the hex file created in the `debug` folder of the project after assembly.

ADL executables first need to built using the `release` profile. These are not relocatable; the release build will assemble to the final location &0B0000. Create the bin file from the hex file in the `release` folder of the project. 

NB: The ZDS tools currently clear the top of ram during download, which is why we debug them at a lower memory location.

### Etiquette

Please do not issue pull requests or issues for this project; it is very much a work-in-progress.
I will review this policy once the code is approaching live status and I have time to collaborate more.

### Build

The eZ80 is programmed via the ZDI connector on the left-hand side of the board. This requires a Zilog USB Smart Cable (part number ZUSBSC00100ZACG) that can be purchased from online stockists such as Mouser or RS Components. Note that at time of writing (July 2022) there are lead times for this cable.

Important! Make sure you get that exact model of cable; there are variants for the Zilog Encore CPU that look similar, but are not compatible with the eZ80 CPU.

In addition to the cable, you will need to download the free ZDS II tools ([product ID SD00063](https://zilog.com/index.php?option=com_zcm&task=view&soft_id=38&Itemid=74)). Note that this is only available for Windows.

Any custom settings for Agon development is contained within the project files, so no further configuration will need to be done.

### Licenses

All project files are released under an MIT license, unless there is an accompanying license in the project folder

### Requirements

- [ZDS II tools](https://zilog.com/index.php?option=com_zcm&task=view&soft_id=38&Itemid=74)
- A hex to binary convertor, for example [hex2bin](https://hex2bin.sourceforge.net)

### Links

- [Zilog eZ80 User Manual](http://www.zilog.com/docs/um0077.pdf)
- [ZiLOG Developer Studio II User Manual](http://www.zilog.com/docs/devtools/um0144.pdf)