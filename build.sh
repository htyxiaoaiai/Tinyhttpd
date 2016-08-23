make clean;
gcc -o httpd httpd.c -pthread;
make cgi;
make output;
