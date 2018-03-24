function count=writeraw(G, filename)
%writeraw - write RAW format grey scale image file to Disk
% Usuage  : writeraw(G, filename)
% G: image data matrix
% filename: file name of the file to write to disk
% count: return value, the elements written to file

 disp([' Write image data to'  filename ' ...']);
 fid=fopen(filename,'wb');
 if (fid==-1)
    error('can not open output image filem press CTRL-C to exit \n');
    pause
 end
G=permute(G, [2,1]); 
count=fwrite(fid,G, 'uchar');
fclose(fid);