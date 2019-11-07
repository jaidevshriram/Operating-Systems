if [ "$#" -ne 1 ];
then
    make clean && make qemu SCHEDULER=RR
    exit 1
fi

make clean && make qemu SCHEDULER=$1