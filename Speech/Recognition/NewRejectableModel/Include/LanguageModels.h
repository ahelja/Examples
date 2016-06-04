/*
 *  LanguageModels.h
 *  NewRejectableModel
 *
 *  Created by tbonura on Tue Aug 28 2001.
 *  Copyright (c) 2001 Apple. All rights reserved.
 *
 */
//  languageModels.h  - language models for the kSRRejectable demo
//		<top>{kSRRefCon = 1000} = meet with <person> on <day>;
// 		<person>{kSRRejectable = true}{kSRRefCon = 100} = matthias | jerome | devang | kim | kevin ;
// 		<day>{kSRRejectable = true}{kSRRefCon = 200} = monday | tuesday | wednesday;

const short 	kStartingResID			=	128;
const long 		kNamesRefCon			=	100;
const long 		kDatesRefCon			=	200;
const long		kDefaultTopLMRefcon		= 	1000;
//  NOTE:  The following have been implemented using plists to be more stylish with Cocoa!
// const char *	kDates[]				= 	{"monday" , "tuesday" , "wednesday" , NULL};
// const char *	kNames[]				= 	{"matthias", "jerome" , "devang" , "kim" , "kevin" , NULL};
