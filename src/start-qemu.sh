# /bin/bash
qemu-system-x86_64 \
    -m 4096m \
    -drive if=pflash,format=raw,readonly=on,file=../OVMF_CODE.fd \
    -drive if=pflash,format=raw,readonly=on,file=../OVMF_VARS.fd \
    -drive format=raw,file=fat:rw:../esp \
    -gdb tcp::1234
#    -serial stdio \
#    -nodefaults \
#    -net none \
