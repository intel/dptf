/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

// friend classes
#define _ESIFDATA_CLASS
#define _ESIFDATALIST_CLASS
#define _ISTRINGLIST_CLASS
#define _EQLPARSER_CLASS
#define _EQLCMD_CLASS
#define _EQLPROVIDER_CLASS

#include "esif_lib_eqlparser.h"
#include "esif_lib_eqlprovider.h"
#include "esif_lib_istring.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define PARSER_EOF  ((char)(-1))

//////////////////////////////////////////////////////////////////////////////
// EqlToken Enumerated Type

// EqlParserToken enumerated type
#define ENUM_EQLTOKEN(ENUM)         \
	ENUM(EqlTokenNone)          \
                                    \
	/* Multi-Char Tokens */     \
	ENUM(EqlTokenIdent)         \
	ENUM(EqlTokenNumber)        \
	ENUM(EqlTokenHex)           \
	ENUM(EqlTokenString)        \
                                    \
	/* Reserved Words */        \
	ENUM(EqlTokenAs)            \
	ENUM(EqlTokenDebug)	/*temp*/ \
	ENUM(EqlTokenWith)          \
                                    \
	/* 1-Character Tokens*/     \
	ENUM(EqlTokenSemicolon)     \
	ENUM(EqlTokenDollar)        \
	ENUM(EqlTokenEq)            \
	ENUM(EqlTokenDot)           \
	ENUM(EqlTokenComma)         \
	ENUM(EqlTokenColon)         \
	ENUM(EqlTokenLeftParen)     \
	ENUM(EqlTokenRightParen)    \
	ENUM(EqlTokenLeftBrace)     \
	ENUM(EqlTokenRightBrace)    \
	ENUM(EqlTokenPercent)       \
	ENUM(EqlTokenAsterisk)      \

// Define Enumerated Data Type and an Array of Enum/String Pairs for Conversion
typedef enum EqlParserToken_e {
	ENUM_EQLTOKEN(ENUMDECL)
} EqlParserToken;
const struct EqlParserTokenList_s {
	EqlParserToken  id;
	const char      *str;
}

EqlParserTokenList[] = {
	ENUM_EQLTOKEN(ENUMLIST)
	{
		(EqlParserToken)0, 0
	}
};

// Reserved Word List. Each XXXX word is converted to EqlTokenXXXX
// This list must be sorted alphabetically for performing Binary Searches [Call Parser_Init()]
static char *EqlReservedWords[] = {
	"AS", "DEBUG", "WITH", 0
};

//////////////////////////////////////////////////////////////////////////////
// EqlContext Enumerated Type

#define ENUM_EQLCONTEXT(ENUM)       \
	ENUM(EqlContextNone)        \
                                    \
	/* EQL Commands */          \
	ENUM(EqlContextAction)      \
	ENUM(EqlContextAdapter)     \
	ENUM(EqlContextSubtype)     \
	ENUM(EqlContextParameter)   \
	ENUM(EqlContextValue)       \
	ENUM(EqlContextBinValue)    \
	ENUM(EqlContextDataType)    \
	ENUM(EqlContextOption)      \

// Define Enumerated Data Type and an Array of Enum/String Pairs for Conversion
typedef enum EqlParserContext_e {
	ENUM_EQLCONTEXT(ENUMDECL)
} EqlParserContext;

////////////////////////////////////////////////////////
// EqlParser Class
struct EqlParser_s {
	StringPtr  buffer;
	int bufidx;
	char       peek;

	EqlParserToken  token;
	StringPtr       value;
	EqlParserToken  lasttoken;
	StringPtr       lastvalue;
	int  error;
	int  debug;

	EqlCmdPtr  eqlcmd;
};

////////////////////////////////////////////////////////
// Misc Functions

#ifdef ESIF_ATTR_OS_WINDOWS
# define ESIF_CDECL __cdecl
#else
# define ESIF_CDECL
#endif

// qsort() callback to do case-insenstive sort for an array of null-terminated strings
static int ESIF_CDECL StringPtrArray_Qsort (
	const void *arg1,
	const void *arg2
	)
{
	return esif_ccb_stricmp(*(char**)arg1, *(char**)arg2);
}


////////////////////////////////////////////////////////
// EqlParserToken Class

// Convert EqlParserToken to String
static const char*EqlParserToken_ToString (EqlParserToken t)
{
	switch (t) {
		ENUM_EQLTOKEN(ENUMSWITCH)
	default:
		break;
	}
	return NULL;
}


// Convert String to EqlParserToken
static EqlParserToken EqlParserToken_FromString (const char *name)
{
	// TODO: Convert this to do a Binary Search for speed
	int i;
	for (i = 0; EqlParserTokenList[i].str; i++)
		if (esif_ccb_stricmp(name, EqlParserTokenList[i].str) == 0) {
			return EqlParserTokenList[i].id;
		}
	return EqlTokenNone;
}


////////////////////////////////////////////////////////
// EqlParserContext Class

// Convert EqlParserContext to String
static const char*EqlParserContext_ToString (EqlParserContext c)
{
	switch (c) {
		ENUM_EQLCONTEXT(ENUMSWITCH)
	default:
		break;
	}
	return NULL;
}


// EqlParserContext_FromString() not implemented

//////////////////////////////////////////////////////////////////////////////
// EqlParser Class

// constructor
static ESIF_INLINE void EqlParser_ctor (EqlParserPtr self)
{
	if (self) {
		WIPEPTR(self);
		self->token  = EqlTokenNone;
		self->eqlcmd = EqlCmd_Create();
	}
}


// destructor
static ESIF_INLINE void EqlParser_dtor (EqlParserPtr self)
{
	if (self) {
		String_Destroy(self->value);
		String_Destroy(self->lastvalue);
		EqlCmd_Destroy(self->eqlcmd);
		WIPEPTR(self);
	}
}


// new operator
EqlParserPtr EqlParser_Create ()
{
	EqlParserPtr self = (EqlParserPtr)esif_ccb_malloc(sizeof(*self));
	EqlParser_ctor(self);
	return self;
}


// delete operator
void EqlParser_Destroy (EqlParserPtr self)
{
	EqlParser_dtor(self);
	esif_ccb_free(self);
}


// Initialize Parser Class. Call once before creating any Parser Objects
void EqlParser_Init ()
{
	static int initialized = 0;
	if (!initialized) {
		// Sort Reserved Words to fascilate Binary Search
		qsort((void*)EqlReservedWords, (size_t)(sizeof(EqlReservedWords) / sizeof(char*) - 1), sizeof(char*), StringPtrArray_Qsort);
		initialized = 1;
	}
}


// Cleanup Parser Class. Call Once after all Parser Objects destroyed
void EqlParser_Cleanup ()
{
}


//////////////////////////////////////////////////////////////////////////////
// EqlParser Static Members

// Reset Parser for a new command
static void EqlParser_Reset (EqlParserPtr self)
{
	EqlParser_dtor(self);
	EqlParser_ctor(self);
}


// Do a Case-Insenstive Binary Search on EqlReservedWords[] to see if ident is a Reserved Word
// NOTE: EqlParser_Init() must be called once before using this function.
static int EqlParser_IsReservedWord (
	EqlParserPtr self,
	const char *ident
	)
{
	static int items = sizeof(EqlReservedWords) / sizeof(*EqlReservedWords) - 1;
	int start = 0, end = items - 1, node = items / 2;
	self = self;
	while (start <= end) {
		int comp = esif_ccb_stricmp(ident, EqlReservedWords[node]);
		if (comp == 0) {
			return 1;
		} else if (comp > 0) {
			start = node + 1;
		} else {
			end = node - 1;
		}
		node = (end - start) / 2 + start;
	}
	return 0;
}


// Generate an Error Message
static void EqlParser_Error (
	EqlParserPtr self,
	char *msg,
	...
	)
{
	IStringPtr errmsg = IString_Create();
	va_list args;
	va_start(args, msg);
	IString_VSprintfTo(errmsg, 0, msg, args);
	va_end(args);
	IString_Strcat(errmsg, "\n");
	CMD_DEBUG("%s", IString_GetString(errmsg));
	IString_Destroy(errmsg);
	self->error = 1;
}


// Get Next Character from Input Buffer
static void EqlParser_GetChar (EqlParserPtr self)
{
	if (!self->buffer || !self->buffer[self->bufidx]) {
		self->peek   = PARSER_EOF;
		self->bufidx = 0;
		self->buffer = 0;
	} else {
		self->peek = self->buffer[self->bufidx++];
	}
}


//////////////////////////////////////////////////////////////////////////////
// EqlParser Implmementation
//
// This containts the actual implementation of the Lexical Analyzer and
// Recursive Descent Parser used for validating an EQL string and placing it
// into an EqlCmd object. Production Rules are implememented as separate
// functions and the Lexical Analyzer as a single function.
//

// The Lexical Analyzer that converts characters from an input stream into an EqlToken
static void EqlParser_GetToken (EqlParserPtr self)
{
	char idx = 0;
	if (!self->peek) {
		EqlParser_GetChar(self);
	}

	while (self->peek != PARSER_EOF) {
		self->token = EqlTokenNone;

		// Skip Whitespace
		if (isspace(self->peek)) {
			EqlParser_GetChar(self);
			continue;
		} else if (isdigit(self->peek)) {
			self->value[idx++] = self->peek;
			EqlParser_GetChar(self);

			// Hex String: Allow 0x, 0x01, 0xFEEDFACE, 0xdeadf00d
			if (self->peek == 'x') {
				self->token = EqlTokenHex;
				self->value[idx++] = self->peek;
				EqlParser_GetChar(self);
				while (isxdigit(self->peek)) {
					self->value[idx++] = (char)toupper(self->peek);
					EqlParser_GetChar(self);
				}
				break;
			}
			// Number: Allow 0, 123, NOT 12.3, 12.
			else {
				// int dots=0;
				self->token = EqlTokenNumber;
				while (isdigit(self->peek) /*|| (self->peek=='.' && (dots++)== 0)*/) {
					self->value[idx++] = self->peek;
					EqlParser_GetChar(self);
				}
				break;
			}
		}
		// Ident or Reserved Word
		else if (isalpha(self->peek) || self->peek == '_') {
			self->value[idx++] = self->peek;
			EqlParser_GetChar(self);
			while (isalnum(self->peek) || self->peek == '_') {
				self->value[idx++] = self->peek;
				EqlParser_GetChar(self);
			}
			self->value[idx] = 0;
			self->token = EqlTokenIdent;

			// Convert Reserved Words to Tokens: XXXX => EqlTokenXXXX
			if (EqlParser_IsReservedWord(self, self->value)) {
				char TokenName[128];
				esif_ccb_strcpy(TokenName, "EqlToken", sizeof(TokenName));
				esif_ccb_strcat(TokenName, self->value, sizeof(TokenName));
				self->token = EqlParserToken_FromString(TokenName);

				if (self->token == EqlTokenDebug) {
					self->debug    = 1;
					idx = 0;
					self->value[0] = 0;
					continue;
				}
			}
			break;
		}
		// String: Allow "string", 'string', "string's", 'string"s', 'string''s', "string""s", 'string\'s', "string\"s"
		// Also Allow [string], [string[s\]]
		else if (self->peek == '\"' || self->peek == '\'' || self->peek == '[') {
			char startquote = self->peek;
			char quote = self->peek;
			char escape     = '\\';
			if (quote == '[') {
				quote = ']';
			}
			EqlParser_GetChar(self);
			while (self->peek != PARSER_EOF) {
				if (self->peek == quote) {
					EqlParser_GetChar(self);
					if (quote == startquote && self->peek == quote) {
						EqlParser_GetChar(self);
						self->value[idx++] = quote;
						continue;
					}
					self->token = EqlTokenString;
					break;
				} else if (self->peek == escape) {
					EqlParser_GetChar(self);
					if (self->peek == quote) {
						self->value[idx++] = quote;
					} else {
						self->value[idx++] = escape;
						self->value[idx++] = self->peek;
					}
					EqlParser_GetChar(self);
				} else {
					self->value[idx++] = self->peek;
					EqlParser_GetChar(self);
				}
			}
			self->value[idx] = 0;
			break;
		}
		// One-Character Tokens
		else {
			switch (self->peek) {
			case ';':
				self->token = EqlTokenSemicolon;
				self->peek  = PARSER_EOF;
				break;

			case '$':
				self->token = EqlTokenDollar;
				break;

			case '=':
				self->token = EqlTokenEq;
				break;

			case '.':
				self->token = EqlTokenDot;
				break;

			case ':':
				self->token = EqlTokenColon;
				break;

			case ',':
				self->token = EqlTokenComma;
				break;

			case '(':
				self->token = EqlTokenLeftParen;
				break;

			case ')':
				self->token = EqlTokenRightParen;
				break;

			case '{':
				self->token = EqlTokenLeftBrace;
				break;

			case '}':
				self->token = EqlTokenRightBrace;
				break;

			case '%':
				self->token = EqlTokenPercent;
				break;

			case '*':
				self->token = EqlTokenAsterisk;
				break;

			case 4:	// CTRL-D
			case 26:// CTRL-Z
				self->peek = PARSER_EOF;
				break;

			default:
				EqlParser_GetChar(self);
				EqlParser_Error(self, "Syntax Error: Illegal Character: '%c' (%d)", self->peek, self->peek);
				self->error = 1;
				break;
			}

			if (self->token != EqlTokenNone && self->peek != PARSER_EOF) {
				self->value[idx++] = self->peek;
				EqlParser_GetChar(self);
				break;
			}
		}
	}
	self->value[idx] = 0;

	if (self->debug) {
		CMD_DEBUG("  GetToken: %s (%s)\n", EqlParserToken_ToString(self->token), self->value);
	}
}


// Accept the given Token and Context and get the next Token ONLY IF it matches the current token
static int EqlParser_Accept (
	EqlParserPtr self,
	EqlParserToken s,
	EqlParserContext c
	)
{
	if (self->error) {
		return 0;
	}

	if (self->token == s) {
		self->lasttoken = s;
		esif_ccb_strcpy(self->lastvalue, self->value, esif_ccb_strlen(self->value, MAXAUTOLEN));
		if (c != EqlContextNone) {
			EqlCmdPtr eqlcmd = self->eqlcmd;
			switch (c) {
			case EqlContextAdapter:
				String_Set(eqlcmd->adapter, self->value);
				break;

			case EqlContextSubtype:
				String_Set(eqlcmd->subtype, self->value);
				break;

			case EqlContextAction:
				if (s == EqlTokenDollar) {
					String_Set(eqlcmd->action, "GET");	// EXEC
				} else {
					String_Set(eqlcmd->action, self->value);
				}
				break;

			case EqlContextParameter:
				StringList_Add(eqlcmd->parameters, self->value);
				break;

			case EqlContextDataType:
				StringList_Add(eqlcmd->datatypes, self->value);
				break;

			case EqlContextValue:
				StringList_Add(eqlcmd->values, self->value);
				if (eqlcmd->datatypes->items < eqlcmd->values->items) {
					StringPtr datatype = "STRING";
					switch (self->token) {
					case EqlTokenNumber:
						datatype = "UINT32";
						break;

					case EqlTokenHex:
						if (esif_ccb_strlen(self->value, MAXAUTOLEN) - 2 <= 8) {
							datatype = "UINT32";
						} else {
							datatype = "BINARY";
						}
						break;

					default:
						break;
					}
					StringList_Add(eqlcmd->datatypes, datatype);
				}
				break;

			case EqlContextBinValue:
				StringList_Add(eqlcmd->values, self->value);
				StringList_Add(eqlcmd->datatypes, "%");	// Indicates a Binary Parameter value to be passed by caller
				break;

			case EqlContextOption:
				StringList_Add(eqlcmd->options, self->value);
				break;

			default:
				break;
			}
		}
		EqlParser_GetToken(self);
		return 1;
	}
	return 0;
}


// Expect the given Token and Context and generate an error if it does not match the next Token
static int EqlParser_Expect (
	EqlParserPtr self,
	EqlParserToken s,
	EqlParserContext c
	)
{
	if (self->error) {
		return 0;
	}

	if (EqlParser_Accept(self, s, c)) {
		return 1;
	}
	EqlParser_Error(self, "Error: Expected %s", EqlParserToken_ToString(s));
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
// EqlParser Grammar Productions
//////////////////////////////////////////////////////////////////////////////

// Literal ::= String| Number | Hex
static void EqlParser_Literal (
	EqlParserPtr self,
	EqlParserContext c
	)
{
	if (EqlParser_Accept(self, EqlTokenString, c)) {
		return;
	}
	if (EqlParser_Accept(self, EqlTokenNumber, c)) {
		return;
	}
	if (EqlParser_Accept(self, EqlTokenHex, c)) {
		return;
	}

	EqlParser_Error(self, "Syntax Error: Expected Literal");
}


// Parameter ::= Ident | Literal
static void EqlParser_Parameter (
	EqlParserPtr self,
	EqlParserContext c
	)
{
	if (!EqlParser_Accept(self, EqlTokenIdent, c)) {
		EqlParser_Literal(self, c);
	}
}


// Value ::= "{" Ident ":" Literal "}" | "%" Number | Literal
static void EqlParser_Value (
	EqlParserPtr self,
	EqlParserContext c
	)
{
	if (EqlParser_Accept(self, EqlTokenLeftBrace, EqlContextNone)) {
		EqlParser_Expect(self, EqlTokenIdent, EqlContextDataType);
		EqlParser_Expect(self, EqlTokenColon, EqlContextNone);
		EqlParser_Literal(self, c);
		EqlParser_Expect(self, EqlTokenRightBrace, EqlContextNone);
	} else if (EqlParser_Accept(self, EqlTokenPercent, EqlContextNone)) {
		EqlParser_Expect(self, EqlTokenNumber, EqlContextBinValue);
	} else {
		EqlParser_Literal(self, c);
	}
}


// OptionList ::= Ident ["," Ident ...]
static void EqlParser_OptionList (EqlParserPtr self)
{
	do {
		EqlParser_Expect(self, EqlTokenIdent, EqlContextOption);
	} while (EqlParser_Accept(self, EqlTokenComma, EqlContextNone));

	if (self->error) {
		EqlParser_Error(self, "Syntax Error: Expected OptionList");
	}
}


// ParameterList ::= Parameter [ "," ... ]
static void EqlParser_ParameterList (EqlParserPtr self)
{
	if (!EqlParser_Accept(self, EqlTokenAsterisk, EqlContextParameter)) {
		do {
			EqlParser_Parameter(self, EqlContextParameter);
		} while (EqlParser_Accept(self, EqlTokenComma, EqlContextNone));
	}
	if (self->error) {
		EqlParser_Error(self, "Syntax Error: Expected ParameterList");
	}
}


// Stmt ::= Action Adapter [ ":" Subtype ] [ "(" ParameterList ")" ] [ "=" Value | "AS" DataType ] [ [ "WITH" ] OptionList ]
static void EqlParser_Stmt (EqlParserPtr self)
{
	// Action Adapter [ ":" Subtype ] [ "(" ParameterList ")" ] [ "=" Value | "AS" DataType ] [ [ "WITH" ] OptionList ]
	if (EqlParser_Accept(self, EqlTokenDollar, EqlContextAction) || EqlParser_Accept(self, EqlTokenIdent, EqlContextAction)) {
		EqlParser_Expect(self, EqlTokenIdent, EqlContextAdapter);

		if (EqlParser_Accept(self, EqlTokenColon, EqlContextNone)) {
			// EqlParser_Expect(self, EqlTokenIdent, EqlContextSubtype);
			if (!EqlParser_Accept(self, EqlTokenIdent, EqlContextSubtype)) {
				EqlParser_Expect(self, EqlTokenString, EqlContextSubtype);
			}
		}

		if (EqlParser_Accept(self, EqlTokenLeftParen, EqlContextNone)) {
			EqlParser_ParameterList(self);
			EqlParser_Expect(self, EqlTokenRightParen, EqlContextNone);
		}

		if (EqlParser_Accept(self, EqlTokenEq, EqlContextNone)) {
			EqlParser_Accept(self, EqlTokenLeftParen, EqlContextNone);
			//// Uncomment this to support multiple vaules such as "set (a,b,c)=(x,y,z)"
			// do {
			EqlParser_Value(self, EqlContextValue);
			// } while (EqlParser_Accept(self, EqlTokenComma, EqlContextNone));
			// if (self->error)
			// EqlParser_Error(self, "Syntax Error: Expected ValueList");

			EqlParser_Accept(self, EqlTokenRightParen, EqlContextNone);
		} else if (EqlParser_Accept(self, EqlTokenAs, EqlContextNone)) {
			EqlParser_Expect(self, EqlTokenIdent, EqlContextDataType);
		}

		if (EqlParser_Accept(self, EqlTokenWith, EqlContextNone) || self->token == EqlTokenIdent) {
			EqlParser_OptionList(self);
		}
	} else {
		EqlParser_Error(self, "Syntax Error: Expected Statement");
	}
}


// High-Level Interface to Parser: Accept a new Command and Parse it into an EqlCmd object
EqlCmdPtr EqlParser_Parse (
	EqlParserPtr self,
	const StringPtr command
	)
{
	// Reset the Parser and allocate workspace
	EqlParser_Reset(self);
	self->buffer    = command;
	self->value     = String_CreateAs(esif_ccb_strlen(command, MAXAUTOLEN) + 1);// formerly min(strlen(command)+1, EQLPARSER_MAXVALUE)
	self->lastvalue = String_CreateAs(esif_ccb_strlen(command, MAXAUTOLEN) + 1);// formerly min(strlen(command)+1, EQLPARSER_MAXVALUE)

	// Get first Token and Call the Top-Level Production (Stmt)
	EqlParser_GetToken(self);
	EqlParser_Stmt(self);
	EqlParser_Accept(self, EqlTokenSemicolon, EqlContextNone);

	if (self->debug) {
		CMD_DEBUG("\n\n%s\n", (self->error ? "ERROR" : "SUCCESS!"));
		if (!self->error) {
			EqlCmd_DebugDump(self->eqlcmd);
		}
	}
	return self->error || self->debug ? NULL : self->eqlcmd;
}


// High-Level Interface to Parser: Parse a Command and Execute it using optional arguments
eEsifError EqlParser_ExecuteEql (
	esif_string eql,
	EsifDataPtr *arguments
	)
{
	eEsifError rc       = ESIF_E_UNSPECIFIED;
	EqlParserPtr parser = 0;
	EqlCmdPtr eqlcmd    = 0;

	parser = EqlParser_Create();
	if (parser) {
		eqlcmd = EqlParser_Parse(parser, eql);
	}
	if (eqlcmd) {
		rc = EqlCmd_Dispatch(eqlcmd, arguments);

		if (rc == ESIF_OK && eqlcmd->results->size > 0) {
			EqlCmd_DisplayResults(eqlcmd);
		} else {
			CMD_OUT("%s\n", esif_rc_str(rc));
		}
	}
	EqlParser_Destroy(parser);
	return rc;
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

