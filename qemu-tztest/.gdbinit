target remote :1234
set print pretty

define load_el3
file el3/el3.elf
add-symbol-file el3/el3.elf &_EL3_TEXT_BASE
end

define load_el1s
file el1/secure/el1_sec.elf
add-symbol-file el1/secure/el1_sec.elf &_EL1_S_TEXT_BASE
end

define load_el1ns
file el1/nonsecure/el1_nsec.elf
add-symbol-file el1/nonsecure/el1_nsec.elf &_EL1_NS_TEXT_BASE
end

define load_el0s
file el0/secure/el0_sec.elf
add-symbol-file el0/secure/el0_sec.elf &_EL0_S_TEXT_BASE
end

define load_el0ns
file el0/nonsecure/el0_nsec.elf
add-symbol-file el0/nonsecure/el0_nsec.elf &_EL0_NS_TEXT_BASE
end
