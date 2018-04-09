/* TODO:
 * - Keep single-contents tags on same line -> newline before tags instead of after?
 * - have multiple types of failure?
 * - Flags
 *     - newlines
 *     - indenting
 *     - sanitising?
 */

#ifndef LOUT_H
#include <stdio.h>

#ifndef LOUT_NO_INLINE
#define lout_inline inline
#else
#define lout_inline
#endif/*LOUT_NO_INLINE*/

typedef unsigned int uint;

typedef struct lout_tag
{
	char *Name;
	char *Attributes;
	int SelfClosing;
} lout_tag;

typedef struct lout_stack
{
	lout_tag Tags[1000];
	uint Len;
	uint Cap;
} lout_stack;
#define LOUT_NUM_TAGS (sizeof(((lout_stack *)0)->Tags) / sizeof(*((lout_stack *)0)->Tags))

typedef struct lout
{
	// TODO: union with string
	FILE *File;
	lout_stack Stack;
	uint Flags;
} lout;

// fills in XML file for Lout to use
// returns success
int LoutFileOpen(lout *Lout, char *Filename, uint Flags)
{
	Lout->File = fopen(Filename, "w");
	Lout->Flags = Flags;
	Lout->Stack.Cap = LOUT_NUM_TAGS;
	int Result = !!Lout->File;
	return Result;
}

// returns success
int LoutFileClose(lout *Lout)
{
	int Result = ! fclose(Lout->File);
	return Result;
}

#define LOUT_INDENT 4
lout_inline int LoutIndent(lout *Lout, int Depth)
{
	int Indent = Depth * LOUT_INDENT;
	int Result = fprintf(Lout->File, "%*s", Indent, "");
	return Result;
}

/* returns number of chars printed. Negative numbers indicate errors */
int LoutPushTag(lout *Lout, lout_tag Tag)
{
	char Empty = '\0';
	int Result = 0;
	char *Close = Tag.SelfClosing ? " /" : "";
	LoutIndent(Lout, Lout->Stack.Len);
	if(Tag.Attributes) { Result = fprintf(Lout->File, "<%s %s%s>\n", Tag.Name, Tag.Attributes, Close); }
	else              { Result = fprintf(Lout->File, "<%s%s>\n",    Tag.Name, Close); }

	if(! Tag.SelfClosing && Lout->Stack.Len + 1 < Lout->Stack.Cap && Result)
	{ Lout->Stack.Tags[++Lout->Stack.Len] = Tag; }
	else
	{ Result = -1; }

	return Result;
}

lout_inline int LoutPushAttr(lout *Lout, char *TagName, char *Attributes)
{ lout_tag Tag = { TagName, Attributes }; return LoutPushTag(Lout, Tag); }

lout_inline int LoutPush(lout *Lout, char *TagName)
{ lout_tag Tag = { TagName }; return LoutPushTag(Lout, Tag); }

lout_inline int LoutEmptyAttr(lout *Lout, char *TagName, char *Attributes)
{ lout_tag Tag = { TagName, Attributes, 1}; return LoutPushTag(Lout, Tag); }

lout_inline int LoutEmpty(lout *Lout, char *TagName)
{ lout_tag Tag = { TagName, 0, 1 }; return LoutPushTag(Lout, Tag); }

// Returns number of remaining items in stack
int LoutPop(lout *Lout)
{
	int Result = 0;
	int Len = Lout->Stack.Len;
	if(Len > 0)
	{
		LoutIndent(Lout, Len - 1);
		lout_tag Tag = Lout->Stack.Tags[Len];
		Result = fprintf(Lout->File, "</%s>\n", Tag.Name);

		if(Result)
		{ Result = --Lout->Stack.Len; }
		else
		{ Result = -1; }
	}
	return Result;
}

int LoutPrint(lout *Lout, char *String)
{
	int Result = 0;
	LoutIndent(Lout, Lout->Stack.Len);
	Result = fprintf(Lout->File, "%s\n", String);
	return Result;
}

int LoutComment(lout *Lout, char *String)
{
	int Result = 0;
	LoutIndent(Lout, Lout->Stack.Len);
	Result = fprintf(Lout->File, "<!-- %s -->\n", String);
	return Result;
}

#define LOUT_H
#endif
