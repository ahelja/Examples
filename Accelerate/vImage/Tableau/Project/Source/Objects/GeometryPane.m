#import "GeometryPane.h"

#include "Filters.h"
#include "GeometryFilters.h"

@implementation GeometryPane

#include <objc/objc-runtime.h>

void Multiply3x3( float result[9], const float A[9], const float B[9] )
{
    float r0, r1, r2;
    float a;
    
    a = A[0];		r0  = a * B[0];		r1  = a * B[1];		r2  = a * B[2];
    a = A[1];		r0 += a * B[3];		r1 += a * B[4];		r2 += a * B[5];
    a = A[2];		r0 += a * B[6];		r1 += a * B[7];		r2 += a * B[8];
    result[0] = r0;	result[1] = r1;		result[2] = r2;

    a = A[3];		r0  = a * B[0];		r1  = a * B[1];		r2  = a * B[2];
    a = A[4];		r0 += a * B[3];		r1 += a * B[4];		r2 += a * B[5];
    a = A[5];		r0 += a * B[6];		r1 += a * B[7];		r2 += a * B[8];
    result[3] = r0;	result[4] = r1;		result[5] = r2;

    a = A[6];		r0  = a * B[0];		r1  = a * B[1];		r2  = a * B[2];
    a = A[7];		r0 += a * B[3];		r1 += a * B[4];		r2 += a * B[5];
    a = A[8];		r0 += a * B[6];		r1 += a * B[7];		r2 += a * B[8];
    result[6] = r0;	result[7] = r1;		result[8] = r2;
}

- (IBAction)doColor:(id)sender
{
    NSColorPanel *panel = [ NSColorPanel sharedColorPanel ];
    
    [ panel setContinuous: YES ];
    [ panel setTarget: self ];
    [ panel setAction: @selector( changeColor: ) ];
    [ panel makeKeyAndOrderFront: nil ];

}

- (IBAction)go:(id)sender
{
    quitting = YES;
    [ self flushImage ];
    [ self orderOut: nil ];
}


-(void)initWithTarget:(id)the_target action:(SEL)the_action function:(uint32_t)func height:(int)h width:(int)w
{
    uint32_t set = func >> 16;
    uint32_t item = func & 0xFFFFUL;
    uint32_t flags = filterLists[ set ].list[item].flags;

    target = the_target;
    action = the_action;
    height = h;
    width = w;
    r = g = b = a = 1.0f;
    quitting = NO;
    
    //Init the transform matrix display
    [ transformMatrix setAutoresizesAllColumnsToFit: YES ];
    
    //Enable / Disable various controls
    [ shear_X setEnabled: ( flags & kShearX ) != 0 ];
    [ shear_Y setEnabled: ( flags & kShearY ) != 0 ];
    [ scale_X setEnabled: ( flags & kScaleX ) != 0 ];
    [ scale_Y setEnabled: ( flags & kScaleY ) != 0 ];
    [ rotate  setEnabled: ( flags & kRotate ) != 0 ];
    [ translate_X setEnabled: ( flags & kTranslateX ) != 0 ];
    [ translate_Y setEnabled: ( flags & kTranslateY ) != 0 ];
    
    [ self updateTransformMatrix: nil ];
    
    //Make sure we don't go away when closed
    [ self setReleasedWhenClosed: NO ];
    
    //Link into the matrix display
    {
        NSRect matrixBounds = [ transformMatrix bounds ];
        NSArray *columns = [ transformMatrix tableColumns ];
        int i;
        
        [ transformMatrix setDataSource: self ];
        
        //Resize the darn thing because the standard sizing widget looks awful
        [ transformMatrix setRowHeight: matrixBounds.size.height / 3.0 - 2 ];
        for( i = 0; i < 3; i++ )
        {
            NSTableColumn *column = [ columns objectAtIndex: i ];
            [ column setWidth: matrixBounds.size.width / 3.0 - 2 ];
            [ column setIdentifier: [ NSNumber numberWithInt: i ] ];
        }
        
    //    [ transformMatrix deselectAll: nil ];
        [ transformMatrix setDrawsGrid: YES ];
        [ transformMatrix setGridColor: [ NSColor grayColor ] ];
    }

    
    [ self flushImage ];
    [ self makeKeyAndOrderFront: nil ];
}

-(void)flushImage
{
    TransformInfo *info = malloc( sizeof( TransformInfo ) );
    
    memset( info, 0, sizeof( info ) ) ;
    
    //Copy the relevant info into the TransformInfo struct
    info->xScale = transform.a;
    info->xShear = transform.c;
    info->yShear = transform.b / transform.a;
    info->yScale = transform.d - info->xShear * info->yShear;
    info->xTranslation = transform.tx;
    info->yTranslation = transform.ty;
    info->rotate = [ rotate floatValue ];
    info->a = a;
    info->r = r;
    info->g = g;
    info->b = b;
    info->transformMatrix = transform;
    info->quitting = quitting;
    
    //Call the appropriate rendering function
    objc_msgSend( target, action, info);
}

-(IBAction)updateTransformMatrix:(id)sender
{
    //Init matrices to the identity matrix
    NSAffineTransform	*affine_transform = [ [ NSAffineTransform alloc ] init ];
    NSAffineTransformStruct 	data = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };	//Identity
    
    //Set to identity matrix
    [ affine_transform setTransformStruct: data ];

    //Translate
    if( (YES == [ translate_X isEnabled ]) || ( YES == [ translate_Y isEnabled ] ) ) 
    {
        float xt = 0.0f;
        float yt = 0.0f;
        
        if( YES == [ translate_X isEnabled ] )
            xt = [translate_X floatValue] * width;
        
        if( YES == [ translate_Y isEnabled ] )
            yt = [translate_Y floatValue] * height;
        
        [ affine_transform translateXBy: xt 	yBy: yt ];
    }
    
    //Scale
    if( (YES == [ scale_X isEnabled ]) || ( YES == [ scale_Y isEnabled ]) )
    { 
        float sx = 4.0f;
        float sy = 4.0f;
        
        if( YES == [ scale_X isEnabled ] )
            sx = [scale_X floatValue];
            
        if( YES == [ scale_Y isEnabled ] )
            sy = [ scale_Y floatValue ];

        if( sx < 0 )
            sx = -pow( 2.0, -sx - 4.0f);
        else
            sx = pow( 2.0, sx - 4.0f );

        if( sy < 0 )
            sy = -pow( 2.0, -sy - 4.0f );
        else
            sy = pow( 2.0, sy - 4.0f );

        [ affine_transform scaleXBy: sx		yBy: sy ];
    }
                
    //Rotate
    if( YES == [ rotate isEnabled ] )
        [ affine_transform rotateByDegrees: [ rotate floatValue ] ];

    //Shear 
    {
        NSAffineTransform	*shear_transform = [ [ NSAffineTransform alloc ] init ];
        
        //X
        if( YES == [ shear_X isEnabled ] )
            data.m21 = tan( [ shear_X floatValue ] * M_PI / 180.0f  );
    
        //Y
        if( YES == [ shear_Y isEnabled ] )
            data.m12 = tan( [ shear_Y floatValue ] * M_PI / 180.0f );
    
        //Move the shear data to a NSAffineTransform
        [ shear_transform setTransformStruct: data ];
        
        //Apply it to our transform
        [ affine_transform appendTransform: shear_transform ];
        
        //Clean up
        [ shear_transform release ];
    }

    //extract the transform matrix
    ((NSAffineTransformStruct*) &transform)[0] = [ affine_transform transformStruct ];
    
    [ transformMatrix reloadData ];
    [ self flushImage ];
}


//functions for supporting the table view holding the matrix
-(int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    return 3;
}

-(id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
    int column = [ [ aTableColumn identifier ] intValue ];
    float	result = 0.0f;
    
    switch( rowIndex )
    {
        case 0:
            switch( column )
            {
                case 0:
                    result = transform.a;
                    break;
                case 1:
                    result = transform.b;
                    break;
            }
            break;
        case 1: 
            switch( column )
            {
                case 0:
                    result = transform.c;
                    break;
                case 1:
                    result = transform.d;
                    break;
            }
            break;
        case 2:
            switch( column )
            {
                case 0:
                    result = transform.tx;
                    break;
                case 1:
                    result = transform.ty;
                    break;
                case 2:
                    result = 1.0f;
                    break;
            }
            break;
    }
    
    return [ NSNumber numberWithFloat: result ];
}

-(void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
    
}

//functions for the color panel
-(void)changeColor:(id)sender
{
    NSColor *color = [sender color ];
    
    [ color getRed:&r green:&g blue:&b alpha:&a ];
    
    [ self flushImage ];
}


@end
