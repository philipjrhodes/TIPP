#!/bin/csh
foreach folderShare ( /data0 /data1 /data2 )
	ls $folderShare
	rm -rf $folderShare/$1
	mkdir $folderShare/$1

	if ($folderShare == /data0) then
		mkdir $folderShare/$1/rawPointData
		rbox $2 n D2 O0.5 > $folderShare/$1/rawPointData/mydata.ver 
		../convertFile/qHullConvert2DFile1 $folderShare/$1/rawPointData/
		rm $folderShare/$1/rawPointData/mydata.ver
	endif
end

