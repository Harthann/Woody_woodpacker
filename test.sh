#! /usr/bin/bash

ncycle=1
bin=ls

while getopts ":nb" option; do
   case $option in
      b) # display Help
         bin=$OPTARG;;
      n) # Enter a name
          echo $OPTARG
          ncycle=$(($OPTARG))
          echo "$ncycle";;
     \?) # Invalid option
         echo "Defaults settings";;
   esac
done

#   Copy bin into actual directory
if [ !-f "$bin0"]; then
    cp $(which $bin) $PWD/"$bin"0
fi

echo "Time to perform packing over $bin, $ncycle times"

#   Operate packing multiple times
counter=0
while [ $counter -lt $ncycle ]; do
    ./woody_woodpacker $bin$counter > /dev/null
    counter=`expr $counter + 1`
    echo $bin$counter
    mv woody $bin$counter
    rm $bin`expr $counter - 1`
done

echo "Packing done!"
