To Debug:
    - Flash it and have program running with stlink attached
    - open OPENODC: openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
    - new terminal: gdb-multiarch build/Timer.elf
    - target extended-remote :3333
    - Add breakpoints and go



TODO:
    [D]: Make correct alternating SPRINT/REST Play sequence. 
    []: Add functionality to set seconds for SPRINT with butttons.
    []: Create seconds sound so the user knows where they are in the logic structure.
    []: Create a schematic on EasyEda
    []: Create Sound functions to replace // Announce comments
    []: Remove delay from PlayBuzzer() 