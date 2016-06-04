/**
 *	filename: ExpressionEvaluator.m
 *	created : Wed May  3 15:13:25 2000
 *	LastEditDate Was "Mon Apr  2 17:36:08 2001"
 *
 */

/*
 	Copyright (c) 1997-2001 Apple Computer, Inc.
 	All rights reserved.

        IMPORTANT: This Apple software is supplied to you by Apple Computer,
        Inc. ("Apple") in consideration of your agreement to the following terms,
        and your use, installation, modification or redistribution of this Apple
        software constitutes acceptance of these terms.  If you do not agree with
        these terms, please do not use, install, modify or redistribute this Apple
        software.

        In consideration of your agreement to abide by the following terms, and
        subject to these terms, Apple grants you a personal, non-exclusive
        license, under Apple's copyrights in this original Apple software (the
        "Apple Software"), to use, reproduce, modify and redistribute the Apple
        Software, with or without modifications, in source and/or binary forms;
        provided that if you redistribute the Apple Software in its entirety and
        without modifications, you must retain this notice and the following text
        and disclaimers in all such redistributions of the Apple Software.
        Neither the name, trademarks, service marks or logos of Apple Computer,
        Inc. may be used to endorse or promote products derived from the Apple
        Software without specific prior written permission from Apple. Except as
        expressly stated in this notice, no other rights or licenses, express or
        implied, are granted by Apple herein, including but not limited to any
        patent rights that may be infringed by your derivative works or by other
        works in which the Apple Software may be incorporated.

        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
        NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
        IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
        PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
        ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
        SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
        MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
        WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
        LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
        OF SUCH DAMAGE.
*/

#import "ExpressionEvaluator.h"

static NSCharacterSet *__numbersCharSet    = nil; /* 0 to 9 */
static NSCharacterSet *__mathOperations    = nil; /* + - * / ( ) */
static NSCharacterSet *__whiteSpaceCharSet = nil; /* space tab return */

@interface ExpressionEvaluator(Private)
- (BOOL)_isValidNumber:(NSString *)value;
- (NSString *)_nextToken;
- (void)_tokenizeExpression;
- (BOOL)_nextArgIsEqual:(NSString *)string;
- (BOOL)_noMoreArgs;
- (NSDecimalNumber *)_evaluate;
- (NSDecimalNumber *)_evaluate1;
- (NSDecimalNumber *)_evaluate2;
@end

@implementation ExpressionEvaluator

+ (NSDecimalNumber *)evaluateExpression:(NSString *)expression
{
    ExpressionEvaluator *eval;

    eval = [[[self class] alloc] initWithExpression:expression];
        /* Make sure to call autorelease on the object
         * before calling evaluate, if there is a parse error
         * evaluate raises and exception
         */
    [eval autorelease];
    return [eval evaluate];
}

- initWithExpression:(NSString *)expression
{
    [super init];
    _expression    = [expression copy];
    _tokenPosition = 0;
    _length        = [_expression length];

    if (!__numbersCharSet){
        __numbersCharSet    = [NSCharacterSet characterSetWithCharactersInString:@"01234567890.,"];
        __mathOperations    = [NSCharacterSet characterSetWithCharactersInString:@"+-*/()"];
        __whiteSpaceCharSet = [NSCharacterSet whitespaceCharacterSet];

        [__numbersCharSet retain];
        [__mathOperations retain];
    }

    return self;
}

- (NSString *)expression
{
    return _expression;
}

- (void)dealloc
{
    [_expression release];
    [_tokens release];
    [super dealloc];
}

- (NSDecimalNumber *)evaluate
{
    NSDecimalNumber *value;

    [self _tokenizeExpression];
    if (_tokenPosition < _length){
        [NSException raise:NSInvalidArgumentException format:@"parse error after %@",
                     [_tokens lastObject]];
    }
        /* Okay so far there are no bogus characters in the stream so its okay */
    value = [self _evaluate];
    return value;
}

@end

@implementation ExpressionEvaluator(Private)

- (BOOL)_isValidNumber:(NSString *)value
{
    unsigned int length;
    unsigned int i;

    length = [value length];
    for(i=0;i<length;i++){
        if (![__numbersCharSet characterIsMember:[value characterAtIndex:i]]){
            return NO;
        }
    }
    return YES;
}

- (NSString *)_nextToken;
{
    NSString       *token;
    unichar         character;
    unichar        *buffer;
    int             i;

    if (_tokenPosition >= _length){
        return nil;
    }

    character  = [_expression characterAtIndex:_tokenPosition];

    while(_tokenPosition < _length){
        if ([__whiteSpaceCharSet characterIsMember:character]){
            if (_tokenPosition < _length){
                _tokenPosition++; /* Skip white space */
                character = [_expression characterAtIndex:_tokenPosition];
            }
        }else{
                /* If it's not a space, break out of here */
            break;
        }
    }

        /* If it's an operation character then return it */
    if ([__mathOperations characterIsMember:character]){
        _tokenPosition++;
        return [NSString stringWithCharacters:&character length:1];
    }

    buffer = (unichar *)NSZoneMalloc([self zone],sizeof(unichar)*_length+1);
    i = 0;

    while([__numbersCharSet characterIsMember:character]){
        buffer[i++] = character;
        _tokenPosition++;
        if (_tokenPosition >= _length){
            break;
        }
        character = [_expression characterAtIndex:_tokenPosition];
    }

    if (i){
        token = [NSString stringWithCharacters:buffer length:i];
    }else{
        token = nil;
    }
    NSZoneFree([self zone],buffer);
    return token;
}

- (void)_tokenizeExpression
{
    NSString          *token;
    NSAutoreleasePool *pool;

    if (!_tokens){
        pool    = [[NSAutoreleasePool allocWithZone:[self zone]] init];
        token   = [self _nextToken];
        _tokens = [[NSMutableArray allocWithZone:[self zone]] init];

        while(token){
            [_tokens addObject:token];
            token = [self _nextToken];
        }
        [pool release];
    }
}

- (BOOL)_nextArgIsEqual:(NSString *)string
{
    if (![self _noMoreArgs]){
        if ([[_tokens objectAtIndex:_arg] isEqualToString:string]){
            return YES;
        }
    }
    return NO;
}

- (BOOL)_noMoreArgs
{
        /* If _arg is the same as the number of things in _tokens
         * then we have hit the end of the array
         */
    if (_arg == [_tokens count]){
        return YES;
    }
    return NO;
}

- (NSDecimalNumber *)_evaluate
{
    NSDecimalNumber *lvalue;
    NSDecimalNumber *rvalue;
    char             fxn;

    lvalue = [self _evaluate1];

    while(1){
        if ([self _nextArgIsEqual:@"+"]){
            fxn = '+';
        } else {
            if ([self _nextArgIsEqual:@"-"]){
                fxn = '-';
            } else {
                return lvalue;
            }
        }
        _arg++; /* Move to the next argument */
        rvalue = [self _evaluate1];

        if (fxn == '+'){
            lvalue = [lvalue decimalNumberByAdding:rvalue];
            fxn = '\000';
        }
        if (fxn == '-'){
            lvalue = [lvalue decimalNumberBySubtracting:rvalue];
            fxn = '\000';
        }
    }
}

/* Handle *, / */
- (NSDecimalNumber *)_evaluate1
{
    NSDecimalNumber *lvalue;
    NSDecimalNumber *rvalue;
    char             fxn;

    lvalue = [self _evaluate2];

    while(1){
        if ([self _nextArgIsEqual:@"*"]){
            fxn = '*';
        }else{
            if ([self _nextArgIsEqual:@"/"]){
                fxn = '/';
            }else{
                return lvalue;
            }
        }
        _arg++; /* Move to the next argument */
        rvalue = [self _evaluate2];

        if (fxn == '*'){
            lvalue = [lvalue decimalNumberByMultiplyingBy:rvalue];
            fxn = '\000';
        }
        if (fxn == '/'){
            lvalue = [lvalue decimalNumberByDividingBy:rvalue];
            fxn = '\000';
        }
    }
}

- (NSDecimalNumber *)_evaluate2
{
    NSDecimalNumber *value;
    BOOL             negate;

    negate = NO;
    if ([self _noMoreArgs]){
        [NSException raise:NSInvalidArgumentException
                    format:@"parse error at '%@' missing arguments",[_tokens objectAtIndex:_arg-1]];
    }

    if ([self _nextArgIsEqual:@"("]){
        _arg++; /* Move to the next argument */
        value = [self _evaluate];
        if (![self _nextArgIsEqual:@")"]){
            [NSException raise:NSInvalidArgumentException
                        format:@"parse error missing ) after '%@'",[_tokens objectAtIndex:_arg-1]];
        }
        _arg++;
        return value;
    }

        /* Handle signed numbers here
         */
    if ([self _nextArgIsEqual:@"+"]){
        _arg++;
        negate = NO;
    }
    if ([self _nextArgIsEqual:@"-"]){
        _arg++;
        negate = YES;
    }

    if (![self _isValidNumber:[_tokens objectAtIndex:_arg]]){
        [NSException raise:NSInvalidArgumentException
                    format:@"parse error at '%@' is not a valid number",
                     [_tokens objectAtIndex:_arg]];
    }
        /* Check if value is infact a number */
    value = [NSDecimalNumber decimalNumberWithString:[_tokens objectAtIndex:_arg]];
    if (negate){
        value = [value decimalNumberByMultiplyingBy:[NSDecimalNumber decimalNumberWithString:@"-1"]];
    }
    _arg++;
    return value;
}

@end
