#import "KernelPane.h"
#include "Filters.h"
#include "Kernel.h"

@implementation KernelPane

- (IBAction)applyChanges:(id)sender
{
    Kernel *currentKernel = [ kernelData dataSource ];

    //Load the new values from the height and width fields
    int new_height = [[ kernelSize cellAtIndex: 0 ] intValue ];
    int new_width = [[ kernelSize cellAtIndex: 1 ] intValue ];
    
    if( new_height != [ currentKernel height ]  && new_width != [ currentKernel width ] )
    {
        [ self setSize: nil ];
    }
    else
    {
        Kernel *newKernel = [ currentKernel copyWithZone: (NSZone*) nil ];
        int filter = [ currentKernel filter];
        int set = filter >> 16;
        int item = filter & 0x0000FFFF;
        NSArray *the_kernelList = kernelList;
        NSMutableArray *row;
        
        if([ self isShowingFP ]  )
            the_kernelList = kernelListFP;
    
        row = [ the_kernelList objectAtIndex: set ];
            
        [ row 	replaceObjectAtIndex: item 
                    withObject: newKernel	];

        [ newKernel setDataModifiedFromOriginal: NO ];
        [ newKernel setFuncModifiedFromOriginal: NO ];
        [ currentKernel setDataModifiedFromOriginal: NO ];
        [ currentKernel setFuncModifiedFromOriginal: NO ];
        [ kernelData reloadData ];
        [ self setupPrefabMenu ];
    }
}

- (IBAction)cancelChanges:(id)sender
{
    Kernel *currentKernel = [ kernelData dataSource ];
    Kernel *oldKernel, *newKernel;
    int filter = [ currentKernel filter];
    int set = filter >> 16;
    int item = filter & 0x0000FFFF;
    NSArray *the_kernelList = kernelList;
    NSMutableArray *row;
    
    if( [ self isShowingFP ] )
        the_kernelList = kernelListFP;

    row = [ the_kernelList objectAtIndex: set ];
        
    oldKernel = [ row objectAtIndex: item ];
    newKernel = [ oldKernel copyWithZone: (NSZone*) nil ];

    [ kernelData setDataSource: newKernel ];
    [ newKernel setDataModifiedFromOriginal: NO ];
    [ newKernel setFuncModifiedFromOriginal: NO ];
    [ kernelData reloadData ];
    [ self close ];
}

- (void)initObject
{
    int set, item;

    //Make sure the radio buttons dont do crazy things like having both on
    [ intFloatSelector setAllowsEmptySelection: NO ];

    //Create a dictionary to hold the kernels
    kernelList = [[ NSMutableArray alloc ] init ];
    kernelListFP = [[ NSMutableArray alloc ] init ];
    
    //Create entries for filters
    for( set = 0; set < kListCount; set++ )
    {
        NSMutableArray *subArray = [ [ NSMutableArray alloc ] initWithCapacity:filterLists[set].count  ];
        NSMutableArray *subArrayFP = [ [ NSMutableArray alloc ] initWithCapacity:filterLists[set].count ];
                
        for( item = 0; item < filterLists[set].count; item++ )
        {
            KernelInitFunctionList *currentFilter =  filterLists[set].list[item].kernelInitList;
            Kernel	*newKernel = [ [ Kernel alloc ] initWithFunctionInt: currentFilter allocFromZone:(NSZone*)nil];
            Kernel	*newKernelFP = [ [ Kernel alloc ] initWithFunctionFP: currentFilter  allocFromZone:(NSZone*)nil];
            
            [ newKernel setFilter: (set << 16) | item ];
            [ newKernelFP setFilter: (set << 16) | item ];
            
            [ newKernel applyInitFunction ];
            [ newKernelFP applyInitFunction ];
            
            [ newKernel setDataModifiedFromOriginal: NO ];
            [ newKernelFP setDataModifiedFromOriginal: NO ];
            
            [ newKernel setFuncModifiedFromOriginal: NO ];
            [ newKernelFP setFuncModifiedFromOriginal: NO ];
            
            [ subArray addObject: newKernel ];
            [ subArrayFP addObject: newKernelFP ];    

            [ newKernel release ];
            [ newKernelFP release ];
            
        }

        [ kernelList addObject: subArray ];
        [ kernelListFP addObject: subArrayFP ];

        [ subArray release ];
        [ subArrayFP release ];    
    }

    //Init the current filter to no filter
    [ self setFilter: kNoFilter ];
    [ self setSize: nil ];
    
    //Configure the view a little bit more
    [ kernelData setDrawsGrid: YES ];
    
    //Make sure we don't go away when closed
    [ self setReleasedWhenClosed: NO ];
}

- (BOOL)isShowingFP
{
    int tag = [ intFloatSelector selectedTag ];
    
    if( 1 == tag )
        return YES;
    
    return NO;
}


- (IBAction)setSize:(id)sender
{
    Kernel *currentKernel = [ kernelData dataSource ];
    int new_height = [[ kernelSize cellAtIndex: 0 ] intValue ];
    int new_width = [[ kernelSize cellAtIndex: 1 ] intValue ];
    int new_type = [ currentKernel dataType ];  
    int oldColumns, i;
    NSTableColumn *column;
    
    [ currentKernel resizeToHeight:new_height 
                    width: new_width 
                    type: new_type 	];
    
    oldColumns = [ kernelData numberOfColumns ];
    
    for( i = oldColumns; i < new_width; i++ )
    {
        NSNumber *identifier = [ NSNumber numberWithInt: i ];
        column = [[ NSTableColumn alloc ] initWithIdentifier: identifier ];
        [ kernelData addTableColumn: column ];
        [ column release ];
    }

    for( i = oldColumns; i > new_width; i-- )
    {
        NSArray *columnList = [ kernelData tableColumns ];
        column = [ columnList objectAtIndex: i-1 ];
        [ kernelData removeTableColumn: column ];
    }
    
    [ kernelData reloadData ];
    [ self setupPrefabMenu ];
}

- (IBAction)setPrefabType:(id)sender
{
    Kernel *currentKernel = [ kernelData dataSource ];
    
    if( nil != currentKernel )
    {
        int function = [ sender indexOfSelectedItem ];
        
        [ kernelData deselectAll: nil ];
        [ currentKernel setInitFunction: function];
        [ currentKernel applyInitFunction ];
        [ self setupPrefabMenu ];

        [ kernelData reloadData ];
    }
}

- (IBAction)setIntOrFloat:(id)sender
{
    Kernel *currentKernel = [ kernelData dataSource ];
    int filter = kNoFilter; 
    
    if( nil != currentKernel )
        filter = [ currentKernel filter ];
    
    [ self setFilter: filter ];
    [ self setupPrefabMenu ];
}

- (void)setFilter:(int)filter
{
    NSFormCell *heightCell = [ kernelSize cellAtIndex: 0 ];
    NSFormCell *widthCell = [ kernelSize cellAtIndex: 1 ];
    int	set = filter >> 16;
    int item = filter & 0xFFFF;
    NSArray *kernelSet, *row;
    Kernel *currentKernel;
    Kernel *source = nil;
        
    //Nuke all the tabView Items
    if( kNoFilter == filter )
    {
        [ kernelData setDataSource: nil ];
        
    	[ heightCell setIntValue:0  ];
    	[ widthCell setIntValue:0  ];
    
        [ applyButton setEnabled: NO ];
        
        [ kernelData setDataSource:nil ];
        [ kernelData reloadData ];
        [ self setTitle: @"(No Filter)" ];
    
        return;
    }

    kernelSet = kernelList;
    
    if( [ self isShowingFP ]  )
        kernelSet = kernelListFP;
        
    row = [ kernelSet objectAtIndex: set ];
    currentKernel = [ row objectAtIndex: item ];
    if( nil != currentKernel )
        source = [ currentKernel copyWithZone: nil ];

    [ kernelData setDataSource: source];
    [ kernelData reloadData ];
    
    [ heightCell setIntValue: [source height]  ];
    [ widthCell setIntValue: [source width]  ];
    [[ widthCell controlView ] setNeedsDisplay: YES];

    [ self setSize: nil ];
    [ self setTitle: filterLists[set].list[item].name ];
    [ self setupPrefabMenu ];
}

- (void)setupPrefabMenu
{
    Kernel *currentKernel = [ kernelData dataSource ];
    int filter = [ currentKernel filter ];
    int set = filter >> 16;
    int item = filter & 0xFFFF;
    KernelInitFunctionList *list = filterLists[ set ].list[item].kernelInitList;
    
    //Remove all the entries in the popup
    [ kernelPrefabType removeAllItems ];


    if( NULL == list || list->count <= 0 )
    {
        [ kernelPrefabType addItemWithTitle: @"none" ];
        [ kernelPrefabType setEnabled: NO ];
        [ applyButton setEnabled: [ currentKernel wasDataModifiedFromOriginal ] ];
    }
    else
    {
        int i;
        
        for( i = 0; i < list->count; i++ )
        {
            KernelInitFunction *info = list->list + i;
        
            [ kernelPrefabType addItemWithTitle: info->name ];
        }

        [ kernelPrefabType addItemWithTitle: @"Custom" ];
        
        [ applyButton setEnabled: [ currentKernel wasDataModifiedFromOriginal ] ];
        if( YES == [ currentKernel wasDataModifiedFromLastPrefab ] )
        {
            [ currentKernel setInitFunction: [ kernelPrefabType indexOfItem: [ kernelPrefabType lastItem]  ]];
        }
        
        
            

        [ kernelPrefabType selectItemAtIndex: [ currentKernel initFunction] ];
        [ kernelPrefabType setEnabled: YES ];
    }

}


- (Kernel*)kernelForFilter:(int)filter  isFP:(BOOL)isFP
{
    int set = filter >> 16;
    int item = filter & 0x0000FFFF;
    NSArray *the_kernelList = kernelList;
    NSArray *row;

    if( YES == isFP )
        the_kernelList = kernelListFP;

    row = [ the_kernelList objectAtIndex: set ];

    return [ row objectAtIndex: item ];
}


@end
