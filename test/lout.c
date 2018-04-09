#define LOUT_NO_INLINE
#include "../lout.h"

int main()
{
	lout xml, *XML = &xml;
	int Err = 0;
	if(LoutFileOpen(XML, "Lout.xml", 0))
	{
		LoutComment(XML, "Test Comment");
		LoutPush(XML, "tag");

			LoutPushAttr(XML, "a", "href='https://github.com/azmr/lout'");
				LoutPrint(XML, "Lout GitHub page");
			LoutPop(XML);

			LoutPush(XML, "p");
				LoutPrint(XML, "Some text");
				LoutEmpty(XML, "br");
				LoutPrint(XML, "Some more text");
			LoutPop(XML);
		LoutComment(XML, "Test... \nMultiline...\nComment");
		LoutPop(XML);

		LoutFileClose(XML);
		if(Err) { fprintf(stderr, "%d errors while exporting!\n", Err); }

	}
	else
	{
		fprintf(stderr, "Couldn't open %s\n", "Lout.xml");
	}
	return 0;
}
