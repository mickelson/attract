#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <xf86drm.h>

int main( int argc, char *argv[] )
{
	int ret=-1;

	if ( argc < 3 )
		printf( "Usage: %s <set|drop> <fd>\n", argv[0] );
	else if ( strcmp( argv[1], "drop" ) == 0 )
		ret = drmDropMaster( atoi( argv[2] ) );
	else if ( strcmp( argv[1], "set" ) == 0 )
		ret = drmSetMaster( atoi( argv[2] ) );

	return ret;
}
