echo "start shell"
for trace in `ls trace`
do
	filename=trace/${trace}
	echo "$filename"
	./sum $filename
	echo "done"
done
