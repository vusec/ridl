dl=https://www.cs.vu.nl/~herbertb/download/ridlers/files/js-shell.tar.gz
hash="a6d6ef4985fe79868fa9bd2c3aca9fd9"

for x in ./*;
do
	md5=$(md5sum $x | cut -f 1 -d " ")
	if [[ $md5 == $hash ]]; then
		echo "js-shell already downloaded ($x)"
		exit
	fi
done

file=js-shell.tar.gz
echo Downloading...
wget $dl 
tar -xzf $file
rm $file
