#!/bin/bash
#
# Attract-Mode language utility script
#
# - applies a consistent format to language resource files
# - updates them accordingly when new strings are added to msg_template.txt
#
if [ ! -d config/language ]
then
	echo "Error, config/language subdirectory not found"
	exit 1
fi

echo "Updating languages:"
for f in config/language/*.msg
do
	echo -e "\t$f"
	cat config/language/msg_template.txt $f | gawk '
BEGIN {
	PROCINFO[ "sorted_in" ] ="@ind_str_asc"
	FS=";"
	OFS=""
	first_line="";
	second_line="";
	reading_template=1;
	reading_translated=1;
}
{
	if ( !reading_template )
	{
		if ( length( first_line ) == 0 )
			first_line = $0;
		else if ( length( second_line ) == 0 )
			second_line = $0;
	}

	if ( substr( $0, 1, 1 ) == "#" )
	{
		if ( $1 == "#@DONE_TEMPLATE" )
			reading_template=0;
		if ( $1 == "#@DONE_TRANSLATED" )
			reading_translated=0;
	}
	else
	{
		if ( substr( $1, 1, 1 ) == "_" )
		{
			if ( reading_template )
				templ_help[$1]=$2;
			else if ( reading_translated )
			{
				if ( $1 in templ_help )
				{
					templ_help[$1]=$2;
					templ_help_found[$1]=1;
				}
				else
					extra_help[$1]=$2;
			}
		}
		else
		{
			if ( reading_template )
				templ[$1]=$2;
			else if ( reading_translated )
			{
				if ( $1 in templ )
				{
					templ[$1]=$2;
					templ_found[$1]=1;
				}
				else
					extra[$1]=$2;
			}
		}
	}
}
END {
	print first_line;
	if ( length( second_line ) > 1 )
	{
		print second_line;
	}

	print "#";
	print "# Interface strings";
	print "#";

	for ( t in templ )
	{
		if (( length( t ) > 1 ) && ( t in templ_found ))
			print t,";",templ[t];
	}

	print "";
	print "#";
	print "# Help messages";
	print "#";

	for ( t in templ_help )
	{
		if (( length( t ) > 1 ) && ( t in templ_help_found ))
			print t,";",templ_help[t];
	}

	print "";
	print "#";
	print "# Extra strings provided by translation";
	print "#";

	for ( t in extra )
	{
		if ( length( t ) > 1 )
			print t,";",extra[t];
	}
	for ( t in extra_help )
	{
		if ( length( t ) > 1 )
			print t,";",extra_help[t];
	}

	print "";
	print "#@DONE_TRANSLATED"
	print "#";
	print "# Strings still needing translation";
	print "#";

	for ( t in templ )
	{
		if (( length( t ) > 1 ) && (!( t in templ_found )))
			print t,";",templ[t];
	}
	for ( t in templ_help )
	{
		if (( length( t ) > 1 ) && (!( t in templ_help_found )))
			print t,";",templ_help[t];
	}
}
' > config/language/temp.msg

	mv config/language/temp.msg $f
done
