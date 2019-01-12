/* TODO:
 * - Keep single-contents tags on same line -> newline before tags instead of after?
 * - have multiple types of failure?
 * - declarations? (no slash at end)
 * - better multi-line comments
 * - prefixes? e.g. for office xml
 * - current depth vs printed chars?
 * - any point in a separate lout_stack struct?
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

#ifndef LOUT_STACK_DEPTH
#define LOUT_STACK_DEPTH 256
#endif/*LOUT_STACK_DEPTH*/

typedef unsigned int uint;

typedef struct lout_tag
{
	char *Name;
	char *Attributes;
	int SelfClosing;
} lout_tag;

typedef struct lout_stack
{
	// TODO: SOA
	lout_tag Tags[LOUT_STACK_DEPTH];
	uint Depth;
	uint Cap;
} lout_stack;

typedef struct lout
{
	// TODO: union with string
	FILE *File;
	uint Flags;
	lout_stack Stack;
} lout;

// fills in XML file for Lout to use
// returns success
// TODO: add open mode e.g allow "a"
int LoutFileOpen(lout *Lout, char *Filename, uint Flags)
{
	Lout->File = fopen(Filename, "w");
	Lout->Flags = Flags;
	Lout->Stack.Cap = LOUT_STACK_DEPTH;
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
	/* char Empty = '\0'; */
	int Result = 0;
	char *Close = Tag.SelfClosing ? " /" : "";
	LoutIndent(Lout, Lout->Stack.Depth);
	if(Tag.Attributes) { Result = fprintf(Lout->File, "<%s %s%s>\n", Tag.Name, Tag.Attributes, Close); }
	else              { Result = fprintf(Lout->File, "<%s%s>\n",    Tag.Name, Close); }

	if(! Tag.SelfClosing && Lout->Stack.Depth + 1 < Lout->Stack.Cap && Result)
	{ Lout->Stack.Tags[++Lout->Stack.Depth] = Tag; }
	else
	{ Result = -1; }

	return Result;
}

#define loutLINE1(x) x##__LINE__
#define loutLINE2(x) loutLINE1(x)
#define loutLINE(x)  loutLINE2(x)
#define loutFIRST(a, ...) a
// TODO: make this work
#define LoutScoped(fn, ...) \
	for(int loutLINE(LoutN) = 0, Lout ## fn (__VA_ARGS__); \
		! loutLINE(LoutN)++; \
		LoutPop(loutFIRST(__VA_ARGS__)))

// TODO: formattable attributes version
lout_inline int LoutPushAttr(lout *Lout, char *TagName, char *Attributes)
{ lout_tag Tag = { TagName, Attributes }; return LoutPushTag(Lout, Tag); }

lout_inline int LoutPush(lout *Lout, char *TagName)
{ lout_tag Tag = { TagName }; return LoutPushTag(Lout, Tag); }

lout_inline int LoutEmptyAttr(lout *Lout, char *TagName, char *Attributes)
{ lout_tag Tag = { TagName, Attributes, 1}; return LoutPushTag(Lout, Tag); }

lout_inline int LoutEmpty(lout *Lout, char *TagName)
{ lout_tag Tag = { TagName, 0, 1 }; return LoutPushTag(Lout, Tag); }

int LoutPop(lout *Lout)
{
	int Result = 0;
	int Depth = Lout->Stack.Depth;
	if(Depth > 0)
	{
		LoutIndent(Lout, Depth - 1);
		lout_tag Tag = Lout->Stack.Tags[Depth];
		Result = fprintf(Lout->File, "</%s>\n", Tag.Name);

		if(Result)
		{ Result = --Lout->Stack.Depth; }
		else
		{ Result = -1; }
	}
	return Result;
}

int LoutPopTo(lout *Lout, int Depth)
{
	int Result = 0;
	for(; (int)Lout->Stack.Depth > Depth ; ++Result)
	{
		int CharsPrinted = LoutPop(Lout);
		if(CharsPrinted > 0)
		{ Result += CharsPrinted; } 
		else
		{ Result = -1; break; }
	}

	return Result;
}

lout_inline int LoutFinish(lout *Lout)
{ return LoutPopTo(Lout, 0); }

int LoutPrint(lout *Lout, char *String)
{
	int Result = 0;
	LoutIndent(Lout, Lout->Stack.Depth);
	Result = fprintf(Lout->File, "%s\n", String);
	return Result;
}

int LoutPushAttrText(lout *Lout, char *TagName, char *Attributes, char *Text)
{
	lout_tag Tag = { TagName, Attributes, 0 };
	int Result = LoutPushTag(Lout, Tag);
	if(Result)
	{
		Result &= LoutPrint(Lout, Text);
		Result &= LoutPop(Lout);
	}

	return Result;
}

int LoutPushText(lout *Lout, char *TagName, char * Text)
{ return LoutPushAttrText(Lout, TagName, 0, Text); }

int LoutComment(lout *Lout, char *String)
{
	int Result = 0;
	LoutIndent(Lout, Lout->Stack.Depth);
	Result = fprintf(Lout->File, "<!-- %s -->\n", String);
	return Result;
}

#define LOUT_H
#endif
