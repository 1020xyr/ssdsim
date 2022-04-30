#使用示例 sh run_cdf.sh op_read
echo "Hello World !"
#两句需分开写，不然第一个元素前面会出现$
base1_dir=/root/gitee/ssdsim_bak/ssdsim-master/result/test/4-17-1
base2_dir=/root/gitee/ssdsim_bak/ssdsim-master/result/test/4-17-3
base3_dir=/root/gitee/ssdsim_bak/ssdsim-master/result/test/4-17-1-1
gc_dir=/root/gitee/ssdsim_bak/ssdsim-master/result/test/4-17-2

base1_str=`ls $base1_dir`
base2_str=`ls $base2_dir`
base3_str=`ls $base3_dir`
gc_str=`ls $gc_dir`
#将字符串划分成数组
base1=($base1_str)
base2=($base2_str)
base3=($base3_str)
gc=($gc_str)
dir=op_picture-$1
mkdir -p $dir

echo ${base1[@]} 
echo ${base2[@]} 
echo ${base3[@]} 
echo ${gc[@]} 

len=${#gc[*]}
echo $len
for((i=0;i<$len;i++));
do 
    if echo "${gc[i]}" | grep -q -E 'out'
    then
	    echo "${gc[i]}"
        python3 op_write.py $base1_dir/${base1[i]} $base2_dir/${base2[i]}   $base3_dir/${base3[i]}  $gc_dir/${gc[i]} $dir/${gc[i]}_  ${base1[i]}
    fi
done